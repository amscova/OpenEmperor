#!/usr/bin/env python3
"""Goods-runner output and unsupported balance guards without licensed inputs."""
import importlib.util
import hashlib
import json
from pathlib import Path
import tempfile
import unittest

SPEC = importlib.util.spec_from_file_location(
    "original_goods_runner", Path(__file__).resolve().parents[1] / "tools/test_original_goods.py")
runner = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(runner)


class OutputGuardTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(prefix="goods-output-guards-")
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name).resolve(strict=True)
        self.repo = self.root / "repo"
        self.original = self.root / "original"
        self.build = self.root / "build"
        self.outside = self.root / "outside"
        for path in (self.repo, self.original, self.build, self.outside):
            path.mkdir()
        self.private = self.repo / runner.PRIVATE_OUTPUT

    def preflight(self, output):
        return runner.preflight_output(self.repo, output, self.original, self.build)

    def entries(self):
        return sorted(str(path.relative_to(self.root)) for path in self.root.rglob("*"))

    def test_valid_preflight_does_not_create_any_output(self):
        before = self.entries()
        requested = self.private / "new-parent" / "run"
        self.assertEqual(self.preflight(requested), (self.private, requested))
        self.assertEqual(self.preflight(None), (self.private, None))
        self.assertEqual(self.entries(), before)

    def test_outside_request_is_rejected_before_any_directory_or_file_write(self):
        before = self.entries()
        for requested in (self.outside / "new-output", self.repo / "reports" / "new-output",
                          self.original / "new-output", self.build / "new-output",
                          self.private, self.private / ".." / "escape"):
            with self.subTest(requested=requested), self.assertRaises(RuntimeError):
                runner.create_output(self.repo, requested, self.original, self.build)
            self.assertEqual(self.entries(), before)

    def test_existing_output_is_rejected_and_preserved(self):
        existing = self.private / "already-used"
        existing.mkdir(parents=True)
        marker = existing / "previous-report.json"
        marker.write_bytes(b"previous result must survive\n")
        before = self.entries()
        for requested in (existing, marker):
            with self.subTest(requested=requested), self.assertRaises(RuntimeError):
                runner.create_output(self.repo, requested, self.original, self.build)
        self.assertEqual(marker.read_bytes(), b"previous result must survive\n")
        self.assertEqual(self.entries(), before)

    def test_symlinked_private_ancestor_is_rejected_even_for_default(self):
        (self.repo / ".local").symlink_to(self.outside, target_is_directory=True)
        before = self.entries()
        for requested in (None, self.private / "run"):
            with self.subTest(requested=requested), self.assertRaises(RuntimeError):
                runner.create_output(self.repo, requested, self.original, self.build)
        self.assertEqual(self.entries(), before)
        self.assertEqual(list(self.outside.iterdir()), [])

    def test_symlink_escape_internal_alias_and_dangling_link_are_rejected(self):
        self.private.mkdir(parents=True)
        real = self.private / "real-parent"
        real.mkdir()
        targets = (self.outside, real, self.outside / "absent")
        for index, target in enumerate(targets):
            link = self.private / f"link-{index}"
            link.symlink_to(target, target_is_directory=True)
            before = self.entries()
            for requested in (link, link / "new-output"):
                with self.subTest(requested=requested), self.assertRaises(RuntimeError):
                    runner.create_output(self.repo, requested, self.original, self.build)
            self.assertEqual(self.entries(), before)
        self.assertEqual(list(self.outside.iterdir()), [])
        self.assertEqual(list(real.iterdir()), [])

    def test_private_root_cannot_overlap_original_or_build_in_either_direction(self):
        for source in (self.original, self.build):
            nested = self.private / source.name
            nested.mkdir(parents=True)
            args = (nested, self.build) if source == self.original else (self.original, nested)
            before = self.entries()
            with self.subTest(source=source), self.assertRaises(RuntimeError):
                runner.preflight_output(self.repo, None, *args)
            self.assertEqual(self.entries(), before)
        for original, build in ((self.repo, self.build), (self.original, self.repo)):
            with self.subTest(original=original, build=build), self.assertRaises(RuntimeError):
                runner.preflight_output(self.repo, self.private / "run", original, build)

    def test_non_directory_parent_is_rejected_without_writing(self):
        self.private.mkdir(parents=True)
        marker = self.private / "file-parent"
        marker.write_bytes(b"keep\n")
        before = self.entries()
        with self.assertRaises(RuntimeError):
            runner.create_output(self.repo, marker / "new-output", self.original, self.build)
        self.assertEqual(marker.read_bytes(), b"keep\n")
        self.assertEqual(self.entries(), before)

    def test_default_and_explicit_creation_stay_in_private_root(self):
        default = runner.create_output(self.repo, None, self.original, self.build)
        explicit = runner.create_output(self.repo, self.private / "new-parent" / "explicit",
                                        self.original, self.build)
        self.assertTrue(default.name.startswith("run-"))
        for output in (default, explicit):
            self.assertTrue(output.is_dir())
            self.assertTrue(output.resolve().is_relative_to(self.private))
            self.assertEqual(list(output.iterdir()), [])
        self.assertEqual(list(self.original.iterdir()), [])
        self.assertEqual(list(self.build.iterdir()), [])
        self.assertEqual(list(self.outside.iterdir()), [])

    def test_rule8_output_tree_is_not_a_rule9_output_target(self):
        old_output = self.repo / ".local/original-economy/oe12/influence-fixtures/new-run"
        before = self.entries()
        with self.assertRaises(RuntimeError):
            runner.create_output(self.repo, old_output, self.original, self.build)
        self.assertEqual(self.entries(), before)

    def test_unsupported_balance_rejects_balanced_positive_forgery_and_flags(self):
        zero = {"balances": [{"good": index, "supported": False, "conserved": True,
                "produced": 0, "imported": 0, "consumed": 0, "demolished": 0,
                "stock": 0, "cargo": 0, "source_reserved": 0, "incoming_reserved": 0}
                for index in range(6)]}
        runner.check_balances(zero)
        import copy
        for field in ("produced", "imported", "consumed", "demolished", "stock", "cargo",
                      "source_reserved", "incoming_reserved", "supported"):
            bad = copy.deepcopy(zero)
            bad["balances"][0][field] = True if field == "supported" else 1
            with self.subTest(field=field), self.assertRaises(AssertionError):
                runner.check_balances(bad)
        bad = copy.deepcopy(zero)
        bad["balances"][1]["produced"] = bad["balances"][1]["stock"] = 1
        with self.assertRaises(AssertionError):
            runner.check_balances(bad)
        bad = copy.deepcopy(zero)
        bad["balances"][1]["good"] = 0
        with self.assertRaises(AssertionError):
            runner.check_balances(bad)

    def baseline(self):
        root = self.repo / runner.BASELINE
        root.mkdir(parents=True)
        payload = root / "authored-golden.json"
        payload.write_bytes(b'{"authored":true}\n')
        capture = {"status": "PASS", "rule7_oe11_golden_replayed_byte_exact": True,
                   "files": [{"path": payload.name, "sha256": hashlib.sha256(payload.read_bytes()).hexdigest()}]}
        (root / "capture.json").write_text(json.dumps(capture))
        return root, payload, capture

    def test_frozen_baseline_is_verified_without_rewriting_it(self):
        root, payload, capture = self.baseline()
        before = self.entries()
        self.assertEqual(runner.frozen_baseline(self.repo), (root, capture))
        payload.write_bytes(b'{"authored":"changed"}\n')
        with self.assertRaises(RuntimeError):
            runner.frozen_baseline(self.repo)
        self.assertEqual(self.entries(), before)
        self.assertEqual(payload.read_bytes(), b'{"authored":"changed"}\n')

    def test_frozen_manifest_cannot_escape_or_follow_a_symlink(self):
        root, payload, capture = self.baseline()
        for path in ("../outside", str(payload.resolve())):
            capture["files"][0]["path"] = path
            (root / "capture.json").write_text(json.dumps(capture))
            with self.subTest(path=path), self.assertRaises(RuntimeError):
                runner.frozen_baseline(self.repo)
        link = root / "linked.json"
        link.symlink_to(payload)
        capture["files"][0]["path"] = link.name
        (root / "capture.json").write_text(json.dumps(capture))
        with self.assertRaises(RuntimeError):
            runner.frozen_baseline(self.repo)


if __name__ == "__main__":
    unittest.main()
