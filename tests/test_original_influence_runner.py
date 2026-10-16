#!/usr/bin/env python3
"""Output preflight tests; no original model or compiled fixture is needed."""
import importlib.util
from pathlib import Path
import tempfile
import unittest

SPEC = importlib.util.spec_from_file_location(
    "original_influence_runner", Path(__file__).resolve().parents[1] / "tools/test_original_influence.py")
runner = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(runner)


class OutputGuardTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(prefix="influence-output-guards-")
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


if __name__ == "__main__":
    unittest.main()
