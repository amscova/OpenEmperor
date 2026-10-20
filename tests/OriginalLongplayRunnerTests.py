"""CLI guard/process-propagation tests using explicitly fake child programs.

These tests prove runner failure handling, never original gameplay acceptance.
The actual C++ original-data/100k workflow is executed separately.
"""
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

REPO = Path(__file__).resolve().parents[1]
RUNNER = REPO / "tools/test_original_longplay.py"
PRIVATE = REPO / ".local/original-economy/oe32"


class LongplayRunnerContract(unittest.TestCase):
    def setUp(self):
        PRIVATE.mkdir(parents=True, exist_ok=True)
        self.temporary = tempfile.TemporaryDirectory(prefix="runner-contract-", dir=PRIVATE)
        self.root = Path(self.temporary.name)
        self.data = self.root / "original"
        (self.data / "Model").mkdir(parents=True)
        (self.data / "Cities").mkdir()
        (self.data / "Model/EmperorBuildingModels.txt").write_text("Explicit fake original input for guard tests only.\n")
        (self.data / "Cities/Xia.map").write_bytes(b"Fake map, never accepted as original gameplay evidence.\n")
        self.build = self.root / "build"
        self.build.mkdir()
        self.binary = self.build / "explicit-fake-child"
        self.output = self.root / "output"
        self.fake("fail")

    def tearDown(self):
        self.temporary.cleanup()

    def fake(self, behavior):
        self.binary.write_text("#!/usr/bin/env python3\n" + "BEHAVIOR = " + repr(behavior) + "\n" + '''
import json
from pathlib import Path
import sys
a = dict(zip(sys.argv[1::2], sys.argv[2::2]))
mode = a["--mode"]
if BEHAVIOR == "fail":
    print("Explicit fake child failure; no gameplay executed")
    raise SystemExit(17)
out = Path(a["--output"]); out.mkdir()
ticks = 2000 if mode == "seed" else int(a["--ticks"])
manifest = {"status":"PASS", "scenario":a["--scenario"], "ticks":ticks,
            "begin_tick":0 if mode == "seed" else 2000,
            "end_tick":2000 if mode == "seed" else 2000+ticks,
            "creator_mode":mode != "resume", "restored_live_world":mode == "resume",
            "warmup_ticks":0 if mode == "resume" else 2000,
            "protected_reference_compared":mode == "run"}
if BEHAVIOR == "restore-expected" and mode == "run":
    manifest["creator_mode"] = False
    manifest["restored_live_world"] = True
(out/"manifest.json").write_text(json.dumps(manifest))
if mode == "seed":
    (out/"seed.save.json").write_text('{"explicitly_fake_paid_input":true}')
else:
    if mode == "run":
        (out/"creator-reference.save.json").write_bytes(Path(a["--input"]).read_bytes())
        if BEHAVIOR == "bad-creator-reference":
            (out/"creator-reference.save.json").write_text('{"different_unrestored_creator":true}')
    for point in ("midpoint", "result"):
        for kind in ("save", "snapshot", "report"):
            value = {"explicit_fake":True, "point":point, "kind":kind}
            if BEHAVIOR == "mismatch" and mode == "resume" and point == "result" and kind == "report":
                value["intentional_mismatch"] = True
            (out/(point+"."+kind+".json")).write_text(json.dumps(value))
    if BEHAVIOR == "mutate-input" and mode == "resume":
        Path(a["--input"]).write_text('{"mutated_by_explicit_fake_child":true}')
''')
        self.binary.chmod(0o700)

    def run_cli(self, *extra, data=None, output=None):
        argv = [sys.executable, str(RUNNER), "--binary", str(self.binary), "--data", str(data or self.data),
                "--output", str(output or self.output), "--scenario", "districts", "--ticks", "10", *extra]
        return subprocess.run(argv, capture_output=True, text=True, timeout=20, check=False)

    def test_help_does_not_need_inputs_or_execute_child(self):
        result = subprocess.run([sys.executable, str(RUNNER), "--help"], capture_output=True, text=True, check=False)
        self.assertEqual(result.returncode, 0)
        self.assertIn("--ticks", result.stdout)
        self.assertFalse(self.output.exists())

    def test_invalid_ticks_refused_before_output_or_child(self):
        for value in ("0", "-1", "100001", "not-an-integer"):
            with self.subTest(value=value):
                result = self.run_cli("--ticks", value)
                self.assertNotEqual(result.returncode, 0)
                self.assertFalse(self.output.exists())

    def test_missing_original_is_blocked_without_skip(self):
        result = self.run_cli(data=self.root / "absent-installation")
        self.assertEqual(result.returncode, 2)
        self.assertIn("BLOCKED", result.stderr)
        self.assertFalse(self.output.exists())

    def test_existing_output_preserved(self):
        self.output.mkdir(); protected = self.output / "prior-proof.json"
        protected.write_text("Preserve exact previous bytes\n")
        before = protected.read_bytes()
        result = self.run_cli()
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("output already exists", result.stderr)
        self.assertEqual(protected.read_bytes(), before)
        self.assertEqual(list(self.output.iterdir()), [protected])

    def test_outputs_disjoint_from_original_and_build(self):
        for output in (self.data / "output", self.build / "output"):
            with self.subTest(output=output):
                result = self.run_cli(output=output)
                self.assertNotEqual(result.returncode, 0)
                self.assertIn("overlaps", result.stderr)
                self.assertFalse(output.exists())

    def test_output_cannot_escape_private_root(self):
        result = self.run_cli(output=REPO / "unowned-longplay-output")
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("private OE32", result.stderr)
        self.assertFalse((REPO / "unowned-longplay-output").exists())

    def test_symlink_parent_refused(self):
        target = self.root / "unowned"; target.mkdir()
        linked = self.root / "linked"; linked.symlink_to(target, target_is_directory=True)
        result = self.run_cli(output=linked / "output")
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("symlink", result.stderr)
        self.assertEqual(list(target.iterdir()), [])

    def test_original_symlink_refused(self):
        linked = self.root / "linked-original"; linked.symlink_to(self.data, target_is_directory=True)
        result = self.run_cli(data=linked)
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("symlink", result.stderr)
        self.assertFalse(self.output.exists())

    def test_actual_child_failure_propagates_and_is_recorded(self):
        original = (self.data / "Cities/Xia.map").read_bytes()
        result = self.run_cli()
        self.assertNotEqual(result.returncode, 0)
        report = json.loads((self.output / "report.json").read_text())
        self.assertEqual(report["status"], "FAIL")
        self.assertEqual(len(report["processes"]), 1)
        self.assertEqual(report["processes"][0]["exit_code"], 17)
        self.assertGreater(report["processes"][0]["actual_pid"], 0)
        self.assertTrue(report["original_bytes_modes_mtimes_unchanged"])
        self.assertTrue(report["binary_unchanged"])
        self.assertEqual((self.data / "Cities/Xia.map").read_bytes(), original)
        self.assertIn("Explicit fake child failure", (self.output / "seed.log").read_text())

    def test_independent_child_mismatch_cannot_be_pass(self):
        self.fake("mismatch")
        result = self.run_cli()
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("Separate-process bytes differ: result.report.json", result.stderr)
        report = json.loads((self.output / "report.json").read_text())
        self.assertEqual(report["status"], "FAIL")
        self.assertEqual(len({p["actual_pid"] for p in report["processes"]}), 3)
        self.assertTrue(all(p["exit_code"] == 0 for p in report["processes"]))
        self.assertEqual((self.output / "seed/seed.save.json").read_text(), '{"explicitly_fake_paid_input":true}')

    def test_supplied_input_mutation_propagates(self):
        self.fake("mutate-input")
        result = self.run_cli()
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("modified their supplied paid input", result.stderr)
        report = json.loads((self.output / "report.json").read_text())
        self.assertEqual(report["status"], "FAIL")
        self.assertTrue(report["original_bytes_modes_mtimes_unchanged"])

    def test_expected_run_cannot_be_another_restore(self):
        self.fake("restore-expected")
        result = self.run_cli()
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("creator and actual restored continuation", result.stderr)
        report = json.loads((self.output / "report.json").read_text())
        self.assertEqual(report["status"], "FAIL")
        self.assertEqual(len(report["processes"]), 2)

    def test_fresh_creator_must_equal_the_whole_seed(self):
        self.fake("bad-creator-reference")
        result = self.run_cli()
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("Fresh unrestored creator differs", result.stderr)
        report = json.loads((self.output / "report.json").read_text())
        self.assertEqual(report["status"], "FAIL")
        self.assertEqual(len(report["processes"]), 2)
        self.assertEqual((self.output / "seed/seed.save.json").read_text(), '{"explicitly_fake_paid_input":true}')

    def test_equal_fake_files_are_not_gameplay_acceptance(self):
        self.fake("equal-invalid")
        result = self.run_cli()
        self.assertNotEqual(result.returncode, 0)
        report = json.loads((self.output / "report.json").read_text())
        self.assertEqual(report["status"], "FAIL")
        self.assertEqual(len(report["processes"]), 3)


if __name__ == "__main__":
    unittest.main()
