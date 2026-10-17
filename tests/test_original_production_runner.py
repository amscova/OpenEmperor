#!/usr/bin/env python3
"""Production-runner private-output and real-origin guards without licensed data."""
import copy
import importlib.util
import json
from pathlib import Path
import tempfile
import unittest

SPEC = importlib.util.spec_from_file_location(
    "original_production_runner", Path(__file__).resolve().parents[1] / "tools/test_original_production.py")
runner = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(runner)


class ProductionRunnerTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(prefix="production-output-guards-")
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name).resolve(strict=True)
        self.repo, self.original, self.build, self.outside = (self.root / name for name in (
            "repo", "original", "build", "outside"))
        for path in (self.repo, self.original, self.build, self.outside):
            path.mkdir()
        self.private = self.repo / runner.PRIVATE_OUTPUT

    def entries(self):
        return sorted(str(path.relative_to(self.root)) for path in self.root.rglob("*"))

    def create(self, output):
        return runner.create_output(self.repo, output, self.original, self.build)

    def test_valid_preflight_does_not_write(self):
        before = self.entries()
        output = self.private / "new" / "run"
        self.assertEqual(runner.preflight_output(self.repo, output, self.original, self.build), (self.private, output))
        self.assertEqual(runner.preflight_output(self.repo, None, self.original, self.build), (self.private, None))
        self.assertEqual(self.entries(), before)

    def test_outside_and_older_generation_roots_are_rejected_before_write(self):
        before = self.entries()
        for output in (self.outside / "run", self.repo / "report", self.original / "run", self.build / "run",
                       self.private, self.private / ".." / "escape",
                       self.repo / ".local/original-economy/oe14/production-fixtures/run",
                       self.repo / ".local/original-economy/oe15/production-fixtures/run",
                       self.repo / ".local/original-economy/oe16/production-fixtures/run",
                       self.repo / ".local/original-economy/oe13/goods-fixtures/run"):
            with self.subTest(output=output), self.assertRaises(RuntimeError):
                self.create(output)
            self.assertEqual(self.entries(), before)

    def test_existing_output_and_marker_are_preserved(self):
        output = self.private / "existing"
        output.mkdir(parents=True)
        marker = output / "report.json"
        marker.write_bytes(b"retain earlier proof\n")
        before = self.entries()
        for path in (output, marker):
            with self.subTest(path=path), self.assertRaises(RuntimeError):
                self.create(path)
        self.assertEqual(marker.read_bytes(), b"retain earlier proof\n")
        self.assertEqual(self.entries(), before)

    def test_symlinked_private_ancestor_is_rejected_for_default_and_explicit(self):
        (self.repo / ".local").symlink_to(self.outside, target_is_directory=True)
        before = self.entries()
        for output in (None, self.private / "run"):
            with self.subTest(output=output), self.assertRaises(RuntimeError):
                self.create(output)
        self.assertEqual(self.entries(), before)
        self.assertEqual(list(self.outside.iterdir()), [])

    def test_external_internal_and_dangling_symlinks_are_rejected(self):
        self.private.mkdir(parents=True)
        inside = self.private / "real"
        inside.mkdir()
        for index, target in enumerate((self.outside, inside, self.outside / "missing")):
            link = self.private / f"link-{index}"
            link.symlink_to(target, target_is_directory=True)
            before = self.entries()
            for output in (link, link / "run"):
                with self.subTest(output=output), self.assertRaises(RuntimeError):
                    self.create(output)
            self.assertEqual(self.entries(), before)
        self.assertEqual(list(inside.iterdir()), [])
        self.assertEqual(list(self.outside.iterdir()), [])

    def test_original_and_build_cannot_overlap_output_in_either_direction(self):
        nested = self.private / "source"
        nested.mkdir(parents=True)
        for original, build in ((nested, self.build), (self.original, nested),
                                (self.repo, self.build), (self.original, self.repo)):
            before = self.entries()
            with self.subTest(original=original, build=build), self.assertRaises(RuntimeError):
                runner.preflight_output(self.repo, self.private / "run", original, build)
            self.assertEqual(self.entries(), before)

    def test_non_directory_parent_is_preserved(self):
        self.private.mkdir(parents=True)
        marker = self.private / "file"
        marker.write_bytes(b"keep\n")
        before = self.entries()
        with self.assertRaises(RuntimeError):
            self.create(marker / "run")
        self.assertEqual(self.entries(), before)
        self.assertEqual(marker.read_bytes(), b"keep\n")

    def test_new_default_and_explicit_output_are_private_and_empty(self):
        default = self.create(None)
        explicit = self.create(self.private / "new" / "explicit")
        self.assertTrue(default.name.startswith("run-"))
        for output in (default, explicit):
            self.assertTrue(output.resolve().is_relative_to(self.private))
            self.assertEqual(list(output.iterdir()), [])
        self.assertEqual(list(self.original.iterdir()), [])
        self.assertEqual(list(self.build.iterdir()), [])

    def test_record_paths_cannot_escape_or_follow_links(self):
        output = self.create(None)
        good = output / "save.json"
        good.write_text("{}")
        self.assertEqual(runner.checked_path(self.repo, output, good), good)
        link = output / "alias.json"
        link.symlink_to(good)
        for path in (Path("save.json"), self.outside / "save.json", output / ".." / "escape", link):
            with self.subTest(path=path), self.assertRaises(RuntimeError):
                runner.checked_path(self.repo, output, path)

    @staticmethod
    def scene():
        return {"balances": [{"good": index, "supported": index < 2, "conserved": True,
                    "produced": 0, "imported": 0, "consumed": 0, "demolished": 0,
                    "stock": 0, "cargo": 0, "source_reserved": 0, "incoming_reserved": 0}
                for index in range(6)], "pottery_handed_off": 0, "pottery_completed": 0,
                "hemp_harvested": 0, "hemp_farms": [],
                "residents_admitted": 0, "population": 0, "workforce_supply": 0,
                "houses": [], "stocks": [], "couriers": []}

    def test_balance_guards_distinguish_supported_origin_and_actual_population(self):
        scene = self.scene()
        runner.check_balances(scene)
        with self.assertRaises(AssertionError):
            runner.check_balances(scene, positive=True)
        scene["balances"][1]["produced"] = scene["balances"][1]["stock"] = 2
        scene["balances"][0]["produced"] = scene["balances"][0]["stock"] = 2
        scene.update(pottery_handed_off=2, pottery_completed=2, hemp_harvested=2, hemp_farms=[{"harvested": 2}],
                     residents_admitted=42, population=42, workforce_supply=42,
                     houses=[{"occupants": 7, "hemp": 2 if index == 0 else 0,
                              "ceramics": 2 if index == 0 else 0, "consumed": [0] * 6} for index in range(6)])
        runner.check_balances(scene, positive=True)
        for field in ("imported", "consumed", "demolished", "cargo"):
            bad = copy.deepcopy(scene)
            bad["balances"][1][field] = 1
            with self.subTest(field=field), self.assertRaises(AssertionError):
                runner.check_balances(bad)
        carrying = copy.deepcopy(scene)
        carrying["balances"][1].update(stock=0, cargo=2, source_reserved=0, incoming_reserved=2)
        carrying["houses"][0]["ceramics"] = 0
        carrying["couriers"] = [{"role": 8, "cargo": 0, "reserved": 0, "typed_cargo": 2, "typed_cargo_good": 1}]
        runner.check_balances(carrying, positive=False)
        mirrored = copy.deepcopy(carrying)
        mirrored["couriers"][0]["cargo"] = 2
        with self.assertRaises(AssertionError):
            runner.check_balances(mirrored)
        unsupported_stock = copy.deepcopy(scene)
        unsupported_stock["stocks"] = [{"kind": 3, "amounts": [0, 0, 1, 0, 0, 0]}]
        with self.assertRaises(AssertionError):
            runner.check_balances(unsupported_stock)
        for key in ("pottery_completed", "pottery_handed_off", "hemp_harvested", "residents_admitted", "population"):
            bad = copy.deepcopy(scene)
            bad[key] = 0
            with self.subTest(key=key), self.assertRaises(AssertionError):
                runner.check_balances(bad)

    def test_balanced_unsupported_forgery_wrong_identity_flags_and_domains_fail(self):
        for good in (2, 3, 4, 5):
            bad = self.scene()
            bad["balances"][good]["produced"] = bad["balances"][good]["stock"] = 1
            with self.subTest(good=good), self.assertRaises(AssertionError):
                runner.check_balances(bad)
        for field, value in (("good", 0), ("supported", False), ("conserved", False),
                             ("stock", -1), ("produced", True), ("cargo", 0.0)):
            bad = self.scene()
            bad["balances"][1][field] = value
            with self.subTest(field=field), self.assertRaises(AssertionError):
                runner.check_balances(bad)

    def test_inventory_and_save_identity_detect_bytes_and_metadata_changes(self):
        source = self.original / "file"
        source.write_bytes(b"original\n")
        before = runner.inventory(self.original)
        identity = runner.file_identity(source)
        mode = source.stat().st_mode
        self.assertEqual(runner.inventory(self.original), before)
        self.assertEqual(runner.file_identity(source), identity)
        source.chmod(mode ^ 0o200)
        self.assertNotEqual(runner.inventory(self.original), before)
        self.assertNotEqual(runner.file_identity(source), identity)
        source.chmod(mode)
        source.write_bytes(b"changed\n")
        self.assertNotEqual(runner.file_identity(source)["sha256"], identity["sha256"])

    def test_consumption_requires_exact_house_sinks_and_conservation(self):
        scene = self.scene()
        scene.update(pottery_handed_off=3, pottery_completed=3, hemp_harvested=2, hemp_farms=[{"harvested": 2}],
                     residents_admitted=7, population=7, workforce_supply=7,
                     houses=[{"occupants": 7, "hemp": 1, "ceramics": 1, "consumed": [1, 2, 0, 0, 0, 0]}])
        scene["balances"][0].update(produced=2, stock=1, consumed=1)
        scene["balances"][1].update(produced=3, stock=1, consumed=2)
        runner.check_balances(scene)
        for field, value in (("consumed", [2, 2, 0, 0, 0, 0]), ("consumed", [1, 2, 0, 0, 0, 1]),
                             ("consumed", [True, 2, 0, 0, 0, 0]), ("consumed", [1, 2]), ("occupants", 0)):
            bad = copy.deepcopy(scene)
            bad["houses"][0][field] = value
            with self.subTest(field=field, value=value), self.assertRaises(AssertionError):
                runner.check_balances(bad)

    def test_authored_model_copy_changes_only_two_explicit_need_bytes(self):
        fields = [b"0"] * 24
        fields[17] = b"7"
        source = b"other original rows\r\n1: Shelter,{," + b",".join(fields) + b",,,,,\r\nunchanged footer\r\n"
        expected = fields.copy()
        expected[9] = expected[10] = b"1"
        changed = runner.authored_consumption_model(source)
        self.assertEqual(changed, b"other original rows\r\n1: Shelter,{," + b",".join(expected) + b",,,,,\r\nunchanged footer\r\n")
        for bad in (source + source, source.replace(b"1: Shelter", b"1: Unknown"), changed,
                    source.replace(b"7", b"8"), b"1: Shelter,{,0"):
            with self.subTest(source=bad), self.assertRaises(RuntimeError):
                runner.authored_consumption_model(bad)

    def test_boundary_guard_checks_scheduler_delta_and_no_second_booking(self):
        before = self.scene()
        before.update(tick=855, shelter_needs=[1, 1], pottery_handed_off=12, pottery_completed=12,
                      hemp_harvested=6, hemp_farms=[{"harvested": 6}], residents_admitted=42,
                      population=42, workforce_supply=42,
                      houses=[{"id": i + 1, "occupants": 7, "hemp": 1, "ceramics": 2,
                               "consumed": [0] * 6} for i in range(6)])
        before["balances"][0].update(produced=6, stock=6)
        before["balances"][1].update(produced=12, stock=12)
        at = copy.deepcopy(before)
        at["tick"] = 856
        at["balances"][0].update(stock=0, consumed=6)
        at["balances"][1].update(stock=6, consumed=6)
        for house in at["houses"]:
            house.update(hemp=0, ceramics=1, consumed=[1, 1, 0, 0, 0, 0])
        after = copy.deepcopy(at)
        after["tick"] = 857
        boundary = {"step": 0, "house_arrivals_at_boundary": False, "before": before, "at": at, "after": after,
                    "expected_house_deltas": [{"building_id": i + 1, "consumed": [1, 1, 0, 0, 0, 0]} for i in range(6)]}
        runner.check_consumption_boundary(boundary)
        for field, value in (("step", 7), ("house_arrivals_at_boundary", True)):
            bad = copy.deepcopy(boundary)
            bad[field] = value
            with self.subTest(field=field), self.assertRaises(AssertionError):
                runner.check_consumption_boundary(bad)
        bad = copy.deepcopy(boundary)
        bad["after"]["houses"][0]["consumed"][0] += 1
        with self.assertRaises(AssertionError):
            runner.check_consumption_boundary(bad)

    def test_frozen_rule9_requires_pinned_completed_review(self):
        root = self.repo / runner.RULE9_REFERENCE
        root.mkdir(parents=True)
        (root / "report.json").write_text(json.dumps({"status": "PASS"}))
        (root / "review-validation.json").write_text(json.dumps({"status": "PASS", "all_files": []}))
        before = self.entries()
        with self.assertRaises(RuntimeError):
            runner.frozen_rule9(self.repo)
        self.assertEqual(self.entries(), before)


if __name__ == "__main__":
    unittest.main()
