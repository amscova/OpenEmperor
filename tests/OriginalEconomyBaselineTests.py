#!/usr/bin/env python3
"""Synthetic negative cases for the read-only OE-00 baseline harness."""
import copy
import json
from pathlib import Path
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
import original_economy_baseline as baseline


def document():
    return {"format": "openemperor-sandbox-save", "schema_version": 22,
            "rules": {"id": "sandbox-city-v16", "version": 6},
            "map": {"sha256": "a" * 64}, "profiles": {"synthetic": True},
            "world": {"ticks": 7, "couriers": [{"cargo": 1}], "ramps": []}}


def corpus():
    item = {"policy_fingerprint": "a" * 64, "canonical_sha256": "b" * 64,
            "legacy_mask_sha256": "c" * 64}
    row = {"relative_path": "Cities/Synthetic.map", "input_sha256": "d" * 64,
           "manager_status": "complete", "first_category": "prepared"}
    for name, version in (("policies", 1), ("policy2", 2), ("policy3", 3)):
        row[name] = {mask: dict(item, policy_version=version)
                     for mask in ("zero_legacy", "maximal_legacy", "production_legacy")}
    return {"maps": [row], "original_input_inventory_unchanged": True,
            "first_failure_distribution": {"prepared": 1, "known_conflict": 0}}


class BaselineTests(unittest.TestCase):
    def test_raw_bytes_and_actual_snapshot_are_separate_contracts(self):
        value = document()
        first = baseline.savedoc_contract(json.dumps(value).encode())
        reformatted = baseline.savedoc_contract(json.dumps(value, indent=2).encode())
        self.assertEqual(first["snapshot_sha256"], reformatted["snapshot_sha256"])
        self.assertNotEqual(first["save_sha256"], reformatted["save_sha256"])
        value["world"]["couriers"][0]["cargo"] += 1
        self.assertNotEqual(first["snapshot_sha256"], baseline.savedoc_contract(json.dumps(value).encode())["snapshot_sha256"])

    def test_future_schema_cannot_replace_frozen_legacy(self):
        value = document()
        value["schema_version"] = 23
        with self.assertRaises(RuntimeError):
            baseline.savedoc_contract(json.dumps(value).encode())

    def test_truncated_save_is_rejected(self):
        with self.assertRaises(ValueError):
            baseline.savedoc_contract(b'{"format":"openemperor-sandbox-save"')

    def test_complete_permission_grid_and_missing_cases(self):
        self.assertEqual(len(baseline.permission_contract(corpus(), expected_maps=1, expected_prepared=1)), 1)
        value = corpus()
        del value["maps"][0]["policy3"]["zero_legacy"]
        with self.assertRaises(RuntimeError):
            baseline.permission_contract(value, expected_maps=1, expected_prepared=1)

    def test_duplicate_map_is_rejected(self):
        value = corpus()
        value["maps"].append(copy.deepcopy(value["maps"][0]))
        with self.assertRaises(RuntimeError):
            baseline.permission_contract(value, expected_maps=2, expected_prepared=2)

    def test_malformed_digest_and_partial_manager_are_rejected(self):
        for field, replacement in (("manager_status", "partial"), ("input", "bad-digest")):
            value = corpus()
            if field == "input":
                value["maps"][0]["policy2"]["production_legacy"]["canonical_sha256"] = replacement
            else:
                value["maps"][0][field] = replacement
            with self.assertRaises(RuntimeError):
                baseline.permission_contract(value, expected_maps=1, expected_prepared=1)

    def test_refusal_rows_and_map_input_digest_are_checked(self):
        value = corpus()
        value["maps"][0]["input_sha256"] = "not-a-map-hash"
        with self.assertRaises(RuntimeError):
            baseline.permission_contract(value, expected_maps=1, expected_prepared=1)
        value = corpus()
        value["first_failure_distribution"] = {"prepared": 0, "known_conflict": 1}
        row = value["maps"][0]
        for field in ("policies", "policy2", "policy3"):
            del row[field]
        row["first_category"] = "parser_failure"
        with self.assertRaises(RuntimeError):
            baseline.permission_contract(value, expected_maps=1, expected_prepared=0)

    def test_any_changed_frozen_contract_is_rejected(self):
        reference = {"format": baseline.FORMAT, "original_inputs": {},
                     "saves": {"synthetic": {"snapshot_sha256": "a"}}, "permissions": []}
        baseline.compare(reference, copy.deepcopy(reference))
        for field, replacement in (("original_inputs", {"changed": True}), ("saves", {}), ("permissions", ["changed"])):
            current = copy.deepcopy(reference)
            current[field] = replacement
            with self.assertRaises(RuntimeError):
                baseline.compare(reference, current)

    def test_output_is_fresh_private_and_disjoint_from_originals(self):
        with tempfile.TemporaryDirectory() as directory:
            repo = Path(directory).resolve()
            data = repo / "original-inputs"
            data.mkdir()
            output = repo / ".local/original-economy/oe00/capture"
            self.assertEqual(baseline.fresh_output(repo, data, output), output)
            for refused in (output, repo / "outside", data / "forbidden"):
                with self.assertRaises(RuntimeError):
                    baseline.fresh_output(repo, data, refused)

    def test_symlink_output_escape_is_rejected(self):
        with tempfile.TemporaryDirectory() as directory:
            repo = Path(directory).resolve()
            data = repo / "original-inputs"
            data.mkdir()
            (repo / ".local").symlink_to(data, target_is_directory=True)
            with self.assertRaises(RuntimeError):
                baseline.fresh_output(repo, data, repo / ".local/original-economy/oe00/escape")

    def test_build_must_match_source_and_not_overlap_originals(self):
        with tempfile.TemporaryDirectory() as directory:
            repo = Path(directory).resolve()
            data = repo / "originals"
            data.mkdir()
            build = repo / "build"
            build.mkdir()
            cache = build / "CMakeCache.txt"
            cache.write_text("CMAKE_HOME_DIRECTORY:INTERNAL=" + str(repo) + "\n")
            baseline.validate_build(repo, data, build)
            cache.write_text("CMAKE_HOME_DIRECTORY:INTERNAL=" + str(repo / "other") + "\n")
            with self.assertRaises(RuntimeError):
                baseline.validate_build(repo, data, build)
            with self.assertRaises(RuntimeError):
                baseline.validate_build(repo, data, data / "build")

    def test_both_fixture_parents_checked_before_creating_directories(self):
        for name in ("city-v16-ramps", "city-v16-bridges"):
            with self.subTest(parent=name), tempfile.TemporaryDirectory() as directory:
                repo = Path(directory).resolve()
                data = repo / "originals"
                data.mkdir()
                (repo / ".local").mkdir()
                (repo / ".local" / name).symlink_to(data, target_is_directory=True)
                before = sorted(data.iterdir())
                with self.assertRaises(RuntimeError):
                    baseline.fixture_parents(repo, data)
                self.assertEqual(sorted(data.iterdir()), before)
                other = "city-v16-bridges" if name == "city-v16-ramps" else "city-v16-ramps"
                self.assertFalse((repo / ".local" / other).exists())

    def test_frozen_artifact_corruption_is_rejected(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory).resolve()
            for folder in ("saves", "snapshots"):
                (root / folder).mkdir()
            raw = json.dumps(document()).encode()
            entry = baseline.savedoc_contract(raw)
            snapshot = baseline.canonical({"rules": document()["rules"], "world": document()["world"]})
            (root / "saves/synthetic.json").write_bytes(raw)
            (root / "snapshots/synthetic.json").write_bytes(snapshot)
            reference = {"format": baseline.FORMAT, "saves": {"synthetic.json": entry}}
            (root / "contract.json").write_text(json.dumps(reference))
            baseline.load_reference(root / "contract.json")
            for folder in ("saves", "snapshots"):
                artifact = root / folder / "synthetic.json"
                original = artifact.read_bytes()
                artifact.write_bytes(original + b" ")
                with self.assertRaises(RuntimeError):
                    baseline.load_reference(root / "contract.json")
                artifact.write_bytes(original)


if __name__ == "__main__":
    unittest.main()
