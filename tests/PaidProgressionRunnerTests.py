"""OE34 runner guards using explicitly fake children; no gameplay acceptance."""
import importlib.util
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch

REPO = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location("paid_progression_runner", REPO / "tools/test_paid_progression.py")
RUNNER = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(RUNNER)


class PaidProgressionRunnerContract(unittest.TestCase):
    def setUp(self):
        RUNNER.PRIVATE.mkdir(parents=True, exist_ok=True)
        self.temporary = tempfile.TemporaryDirectory(prefix="runner-contract-", dir=RUNNER.PRIVATE)
        self.root = Path(self.temporary.name)
        self.data = self.root / "original"
        (self.data / "Model").mkdir(parents=True)
        (self.data / "Cities").mkdir()
        (self.data / "Model/EmperorBuildingModels.txt").write_text("Explicit fake source for runner guards only.\n")
        (self.data / "Cities/Xia.map").write_bytes(b"Explicit fake map, never gameplay evidence.\n")
        self.resources = self.root / "resources"
        self.resources.mkdir()
        self.build = self.root / "build"
        self.build.mkdir()
        self.binary = self.build / "explicit-fake-child"
        self.output = self.root / "output"
        self.model_patch = patch.object(RUNNER, "MODEL_SHA", RUNNER.digest(self.data / "Model/EmperorBuildingModels.txt"))
        self.model_patch.start()
        # Concurrent source work must not affect tests of fake child behavior.
        self.sources_patch = patch.object(RUNNER, "sources", return_value=[])
        self.sources_patch.start()
        self.fake("fail")

    def tearDown(self):
        self.sources_patch.stop()
        self.model_patch.stop()
        self.temporary.cleanup()

    def fake(self, behavior):
        self.binary.write_text("#!/usr/bin/env python3\nBEHAVIOR = " + repr(behavior) + "\n" + '''
import hashlib
import json
import base64
from pathlib import Path
import sys
a = dict(zip(sys.argv[1::2], sys.argv[2::2]))
mode = a["--mode"]
if BEHAVIOR == "fail":
    print("Explicit fake child error; no World executed")
    raise SystemExit(17)
out = Path(a["--output"]); out.mkdir()
if BEHAVIOR == "partial":
    (out/"report.json").write_text(json.dumps({"status":"PARTIAL", "first_blocker":"explicit fake blocker"}))
    raise SystemExit(1)
data = Path(a["--data"])
def sha(path): return hashlib.sha256(path.read_bytes()).hexdigest()
def doc(tick, homes, occupied=False, maintenance=0, taxes=0):
    spent = 1404 if homes == 4 else 1564
    return {"schema_version":26, "rules":{"version":10}, "profiles":{"map_permissions":{"version":3}},
        "map":{"relative_path":"Cities/Xia.map", "sha256":sha(data/"Cities/Xia.map")},
        "world":{"ticks":tick, "treasury":12000+taxes-spent-maintenance, "construction_spent_total":spent,
            "maintenance_spent_total":maintenance, "taxes_collected_total":taxes,
            "buildings":[{"id":i+1,"fire_until_tick":0} for i in range(homes)],
            "original_production":{"development_revision":27},
            "original_economy":{"model_source":{"difficulty":2,"sha256":sha(data/"Model/EmperorBuildingModels.txt")},
                "houses":[{"building_id":i+1,"house":{"occupants":10 if occupied else 0,"type":5 if occupied else 2}}
                          for i in range(homes)]}}}
if mode == "create":
    for name, value in (("starter",doc(0,4)), ("expanded",doc(0,6)),
                        ("goal",doc(816,6,True,100,600)), ("result",doc(816+int(a["--hold"]),6,True,120,600))):
        if BEHAVIOR == "equal-invalid": value = {"explicitly_fake":True}
        if BEHAVIOR in ("false-goal", "partial-false-goal") and name == "goal": value = doc(816,4,True,100,600)
        if BEHAVIOR == "burning-goal" and name == "goal": value["world"]["buildings"][0]["fire_until_tick"] = 9999
        (out/(name+".save.json")).write_text(json.dumps(value,sort_keys=True))
elif mode == "resume":
    source = Path(a["--input"])
    (out/"loaded.save.json").write_bytes(source.read_bytes())
    (out/"result.save.json").write_bytes((source.parent/"result.save.json").read_bytes())
    if BEHAVIOR in ("mismatch", "partial-mismatch"): (out/"result.save.json").write_text('{"different_complete_state":true}')
    if BEHAVIOR == "mutate-input": source.write_text('{"mutated_input":true}')
elif mode == "render" and BEHAVIOR not in ("render-no-images", "partial-no-images"):
    image = base64.b64decode("iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAQAAAC1HAwCAAAAC0lEQVR42mP8/x8AAwMCAO+a5e8AAAAASUVORK5CYII=")
    if BEHAVIOR == "render-truncated": image = image[:8]
    (out/"explicit-fake-guard-image.png").write_bytes(image)
if BEHAVIOR == "mutate-original": (data/"Cities/Xia.map").write_bytes(b"changed fake source")
(out/"report.json").write_text(json.dumps({"status":"PARTIAL" if BEHAVIOR.startswith("partial-") and mode == "create" else "PASS",
    "input_preserved":True, "scope":"Explicit fake runner guards, never gameplay acceptance"}))
''')
        self.binary.chmod(0o700)

    def run_workflow(self, **overrides):
        arguments = dict(binary=self.binary, data=self.data, output=self.output, resources=self.resources,
                         ticks=1000, hold=20, timeout=10)
        arguments.update(overrides)
        return RUNNER.workflow(**arguments)

    def report(self):
        return json.loads((self.output / "acceptance.json").read_text())

    def test_help_needs_no_inputs(self):
        result = subprocess.run([sys.executable, str(SPEC.origin), "--help"], capture_output=True, text=True, check=False)
        self.assertEqual(result.returncode, 0)
        self.assertIn("--hold", result.stdout)
        self.assertFalse(self.output.exists())

    def test_counts_and_plan_are_validated_before_output(self):
        for arguments in ({"ticks":0}, {"hold":0}, {"ticks":100001}, {"hold":-1}, {"plan":"invented"}):
            with self.subTest(arguments=arguments), self.assertRaises(RuntimeError):
                self.run_workflow(**arguments)
            self.assertFalse(self.output.exists())

    def test_original_model_is_pinned(self):
        with patch.object(RUNNER, "MODEL_SHA", "0"*64), self.assertRaisesRegex(RuntimeError, "pinned unchanged"):
            self.run_workflow()
        self.assertFalse(self.output.exists())

    def test_missing_original_is_blocked_without_skip(self):
        with self.assertRaisesRegex(RuntimeError, "BLOCKED: original installation"):
            self.run_workflow(data=self.root / "absent")
        self.assertFalse(self.output.exists())

    def test_prior_output_is_preserved(self):
        self.output.mkdir()
        protected = self.output / "prior.json"
        protected.write_bytes(b"preserve exact prior proof\n")
        with self.assertRaisesRegex(RuntimeError, "output already exists"):
            self.run_workflow()
        self.assertEqual(protected.read_bytes(), b"preserve exact prior proof\n")
        self.assertEqual(list(self.output.iterdir()), [protected])

    def test_output_cannot_escape_or_overlap_inputs(self):
        for output in (REPO / "unowned-paid-output", self.data / "output", self.build / "output", self.resources / "output"):
            with self.subTest(output=output), self.assertRaisesRegex(RuntimeError, "private OE34|overlaps"):
                self.run_workflow(output=output)
            self.assertFalse(output.exists())

    def test_symlink_parent_refused_before_output(self):
        target = self.root / "target"
        target.mkdir()
        linked = self.root / "linked"
        linked.symlink_to(target, target_is_directory=True)
        with self.assertRaisesRegex(RuntimeError, "symlink"):
            self.run_workflow(output=linked / "output")
        self.assertEqual(list(target.iterdir()), [])

    def test_inventory_refuses_original_symlinks(self):
        (self.data / "linked.map").symlink_to(self.data / "Cities/Xia.map")
        with self.assertRaisesRegex(RuntimeError, "symlink"):
            self.run_workflow()
        self.assertFalse(self.output.exists())

    def test_child_failure_is_recorded_with_actual_status(self):
        with self.assertRaisesRegex(RuntimeError, "child failed.*17"):
            self.run_workflow()
        report = self.report()
        self.assertEqual(report["status"], "FAIL")
        self.assertEqual(report["processes"][0]["exit_code"], 17)
        self.assertGreater(report["processes"][0]["actual_pid"], 0)
        self.assertTrue(report["original_bytes_modes_mtimes_unchanged"])
        self.assertIn("Explicit fake child error", (self.output / "create.log").read_text())

    def test_partial_creator_never_becomes_pass(self):
        self.fake("partial")
        with self.assertRaises(RuntimeError):
            self.run_workflow()
        self.assertEqual(self.report()["status"], "PARTIAL")
        self.assertEqual(self.report()["create"]["first_blocker"], "explicit fake blocker")

    def test_equal_invalid_files_are_not_goal_acceptance(self):
        self.fake("equal-invalid")
        with self.assertRaises((RuntimeError, KeyError)):
            self.run_workflow()
        self.assertEqual(self.report()["status"], "FAIL")
        self.assertEqual(len(self.report()["processes"]), 1)

    def test_claimed_goal_is_checked_independently(self):
        for behavior in ("false-goal", "burning-goal"):
            with self.subTest(behavior=behavior):
                self.fake(behavior)
                self.output = self.root / behavior
                with self.assertRaisesRegex(RuntimeError, "housing goal"):
                    self.run_workflow()
                self.assertEqual(self.report()["status"], "FAIL")

    def test_different_complete_process_save_is_rejected(self):
        self.fake("mismatch")
        with self.assertRaisesRegex(RuntimeError, "complete save bytes differ"):
            self.run_workflow()
        report = self.report()
        self.assertEqual(report["status"], "FAIL")
        self.assertEqual(len(report["processes"]), 2)
        self.assertTrue(all(process["exit_code"] == 0 for process in report["processes"]))

    def test_input_mutation_is_not_ignored(self):
        self.fake("mutate-input")
        with self.assertRaisesRegex(RuntimeError, "complete save bytes differ|modified protected"):
            self.run_workflow()
        self.assertEqual(self.report()["status"], "FAIL")

    def test_original_mutation_is_a_hard_failure(self):
        self.fake("mutate-original")
        with self.assertRaisesRegex(RuntimeError, "Protected original installation"):
            self.run_workflow()
        self.assertEqual(self.report()["status"], "FAIL")
        self.assertFalse(self.report()["original_bytes_modes_mtimes_unchanged"])

    def test_completed_partial_is_preserved_after_all_three_processes(self):
        self.fake("partial-complete")
        report = self.run_workflow()
        self.assertTrue(report["measurement_completed"])
        self.assertEqual(report["status"], "PARTIAL")
        self.assertEqual(report["create"]["status"], "PARTIAL")
        self.assertEqual(len(report["processes"]), 3)
        self.assertEqual(len(report["literal_pairs"]), 2)
        self.assertEqual(len(report["captures"]), 1)

    def test_partial_measurement_never_masks_a_proof_failure(self):
        for behavior, error in (("partial-mismatch", "complete save bytes differ"),
                                ("partial-no-images", "no PNG captures"),
                                ("partial-false-goal", "housing goal")):
            with self.subTest(behavior=behavior):
                self.fake(behavior)
                self.output = self.root / behavior
                with self.assertRaisesRegex(RuntimeError, error):
                    self.run_workflow()
                self.assertEqual(self.report()["status"], "FAIL")
                self.assertEqual(self.report()["create"]["status"], "PARTIAL")

    def test_render_without_actual_png_is_rejected(self):
        self.fake("render-no-images")
        with self.assertRaisesRegex(RuntimeError, "no PNG captures"):
            self.run_workflow()
        self.assertEqual(self.report()["status"], "FAIL")
        self.assertEqual(len(self.report()["processes"]), 3)

    def test_truncated_png_cannot_count_as_capture(self):
        self.fake("render-truncated")
        with self.assertRaisesRegex(RuntimeError, "complete PNG dimension header"):
            self.run_workflow()
        self.assertEqual(self.report()["status"], "FAIL")


if __name__ == "__main__":
    unittest.main()
