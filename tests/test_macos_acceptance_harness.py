"""Acceptance-tool boundaries; no compiler, original game or desktop is executed."""
import argparse
import copy
import json
from pathlib import Path
import sys
import tempfile
import unittest
from unittest import mock

REPO = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(REPO / "tools"))
import alpha_check as alpha
import package_macos as package
import verify_macos_bundle as bundle

PACK = REPO / "resources/compatibility" / bundle.COMPATIBILITY_ID


class OutputIsolation(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(prefix="oe-macos-harness-")
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name).resolve()

    def test_fresh_paths_do_not_create_or_replace_anything(self):
        paths = {"build": self.root / "build", "report": self.root / "report.json"}
        self.assertEqual(package.fresh_output_paths(paths), paths)
        self.assertEqual(list(self.root.iterdir()), [])

    def test_prior_evidence_is_preserved(self):
        prior = self.root / "report.json"
        prior.write_bytes(b"retained evidence")
        with self.assertRaisesRegex(ValueError, "existing output is preserved"):
            package.fresh_output_paths({"report": prior})
        self.assertEqual(prior.read_bytes(), b"retained evidence")

    def test_nested_outputs_and_original_overlap_refused(self):
        for paths, data in [({"build": self.root / "new", "report": self.root / "new/report.json"}, None),
                            ({"dist": self.root / "data/new"}, self.root / "data"),
                            ({"dist": self.root / "new"}, self.root / "new/data")]:
            with self.subTest(paths=paths), self.assertRaises(ValueError):
                package.fresh_output_paths(paths, data)

    def test_linked_and_non_directory_components_refused(self):
        (self.root / "link").symlink_to(self.root / "missing")
        (self.root / "file").write_text("input")
        for path in [self.root / "link/out", self.root / "file/out"]:
            with self.subTest(path=path), self.assertRaises(ValueError):
                package.fresh_output_paths({"build": path})

    def test_repository_and_maintained_source_outputs_refused(self):
        for path in [REPO, REPO.parent, REPO / "src/new-build", REPO / ".git/new-report"]:
            with self.subTest(path=path), self.assertRaises(ValueError):
                package.fresh_output_paths({"build": path})

    def test_package_arguments_are_effective_and_fresh(self):
        data = self.root / "original"
        data.mkdir()
        args = package.parse_args(["--build-dir", str(self.root / "build"), "--dist-dir",
            str(self.root / "dist"), "--data", str(data), "--jobs", "2"])
        self.assertEqual((args.build_dir, args.dist_dir, args.data, args.jobs),
                         (self.root / "build", self.root / "dist", data, 2))
        with self.assertRaises(ValueError):
            package.parse_args(["--jobs", "0"])

    def test_alpha_forwards_original_model_root_to_every_configuration(self):
        for kind, sanitized in [("Debug", False), ("Release", False), ("RelWithDebInfo", True)]:
            with self.subTest(kind=kind), mock.patch.object(alpha, "run", return_value=("", 0)) as call:
                alpha.configure(self.root / kind, kind, 2, True, sanitizers=sanitized,
                                data=self.root / "original")
                cmake = call.call_args_list[0].args[0]
                self.assertIn("-DOPENEMPEROR_ORIGINAL_MODEL_TEST_ROOT=" + str(self.root / "original"), cmake)
                self.assertIn("-DOPENEMPEROR_ENABLE_SANITIZERS=" + ("ON" if sanitized else "OFF"), cmake)
                self.assertEqual(call.call_args_list[1].args[0][-2:], ["--parallel", "2"])

    def test_notice_is_owned_by_actual_linked_version(self):
        actual = self.root / "Cellar/openssl@4/4.0.3"
        (actual / "lib").mkdir(parents=True)
        library = actual / "lib/libcrypto.4.dylib"
        library.write_bytes(b"independently authored dependency identity")
        (actual / "LICENSE.txt").write_text("actual notice")
        alias = self.root / "libcrypto.dylib"
        alias.symlink_to(library)
        result = package.dependency_license(alias, "LICENSE.txt")
        self.assertEqual(result["notice"], actual / "LICENSE.txt")
        self.assertEqual(result["version"], "4.0.3")
        (actual / "LICENSE.txt").unlink()
        with self.assertRaises(RuntimeError):
            package.dependency_license(alias, "LICENSE.txt")

    def test_header_license_follows_configured_cmake_package(self):
        prefix = self.root / "json/3.12.0"
        config = prefix / "share/cmake/nlohmann_json"
        config.mkdir(parents=True)
        header = prefix / "include/nlohmann/detail/abi_macros.hpp"
        header.parent.mkdir(parents=True)
        header.write_text("#define NLOHMANN_JSON_VERSION_MAJOR 3\n#define NLOHMANN_JSON_VERSION_MINOR 12\n#define NLOHMANN_JSON_VERSION_PATCH 0\n")
        (prefix / "LICENSE.MIT").write_text("configured notice")
        build = self.root / "build"
        build.mkdir()
        (build / "CMakeCache.txt").write_text("nlohmann_json_DIR:PATH=" + str(config) + "\n")
        self.assertEqual(package.json_license(build)["prefix"], prefix)
        header.write_text(header.read_text().replace("MINOR 12", "MINOR 13"))
        with self.assertRaises(RuntimeError):
            package.json_license(build)

    def test_alpha_package_forwards_private_paths_without_falling_back(self):
        args = argparse.Namespace(package_build_dir=self.root / "package-build",
            package_dist_dir=self.root / "dist", data=self.root / "original", jobs=2)
        with mock.patch.object(alpha, "run", return_value=("", 0)) as call:
            with self.assertRaisesRegex(alpha.CheckFailure, "did not produce"):
                alpha.package_check(args)
        command = call.call_args.args[0]
        for flag, expected in [("--build-dir", args.package_build_dir),
                               ("--dist-dir", args.package_dist_dir), ("--data", args.data)]:
            self.assertEqual(command[command.index(flag) + 1], str(expected))
        self.assertEqual(command[command.index("--jobs") + 1], "2")

    def test_ctest_skip_cannot_count_as_acceptance(self):
        build = self.root / "skip-build"
        build.mkdir()
        outputs = [(json.dumps({"tests": [{"name": "original-real"}]}), 0),
                   ("1/1 Test #1: original-real ...***Skipped\n100% tests passed, 0 tests failed out of 1", 1)]
        with mock.patch.object(alpha, "run", side_effect=outputs):
            with self.assertRaisesRegex(alpha.CheckFailure, "skipped or omitted"):
                alpha.test_build(build, 2)


class VisualMetadata(unittest.TestCase):
    def setUp(self):
        self.manifest = json.loads((PACK / "manifest.json").read_text())
        self.documents = {name: bundle.validate_compatibility_json(PACK / name)
                          for name in bundle.COMPATIBILITY_FILES}

    def test_current_maintained_house_contract_is_complete(self):
        bundle.validate_original_house_metadata(self.manifest, self.documents)

    def test_house_semantic_and_geometry_counterexamples_refused(self):
        mutations = [
            lambda d: d["stages"].pop(),
            lambda d: d["stages"][1].update(house_type=2),
            lambda d: d["stages"][-1].update(original_side=2),
            lambda d: d["stages"][-1].update(ground_anchor=[0, 0]),
            lambda d: d["stages"][-1].update(image_index=0),
            lambda d: d["stages"][-1].update(group_key=1),
            lambda d: d["stages"][-1].update(variant=True),
            lambda d: d["stages"][-1].update(base_bytes=1),
            lambda d: d["stages"][0].update(archive="DATA/China_General.sg3"),
            lambda d: d["files"][0].update(sha256="0" * 64),
            lambda d: d.update(selection="invented"),
        ]
        for index, mutate in enumerate(mutations):
            with self.subTest(index=index):
                documents = copy.deepcopy(self.documents)
                mutate(documents["original-houses.json"])
                with self.assertRaises(ValueError):
                    bundle.validate_original_house_metadata(self.manifest, documents)

    def test_house_supplement_cannot_substitute_source_authority(self):
        changed = copy.deepcopy(self.manifest)
        changed["optional_original_houses"]["core_files"] = ["DATA/SprMain.sg3"]
        with self.assertRaises(ValueError):
            bundle.validate_original_house_metadata(changed, self.documents)

    def test_duplicate_metadata_key_refused(self):
        with tempfile.TemporaryDirectory() as root:
            path = Path(root) / "metadata.json"
            path.write_text('{"schema_version":1,"schema_version":2}')
            with self.assertRaises(ValueError):
                bundle.validate_compatibility_json(path)

    def test_visual_count_is_derived_from_declared_physical_identities(self):
        args = argparse.Namespace(walker_visuals=None, building_visuals=None, road_visuals=None)
        result = {"walker_visuals": {"unique_assets": len(alpha.profile_assets(PACK / "walkers.json"))},
                  "building_visuals": {"decoded_unique_assets": len(alpha.profile_assets(PACK / "buildings.json")),
                      "draws_by_role": {r: 1 for r in ("clay_source", "pottery", "warehouse", "household")},
                      "placeholder_fallbacks_by_role": {r: 0 for r in ("clay_source", "pottery", "warehouse", "household")}},
                  "road_visuals": {"unique_assets": len(alpha.profile_assets(PACK / "roads.json")),
                                   "draws": 1, "configured_masks": list(range(16))}}
        alpha.validate_smoke_visuals(result, args, PACK)
        for mutate in [lambda d: d["building_visuals"].update(decoded_unique_assets=0),
                       lambda d: d["road_visuals"].update(unique_assets=0),
                       lambda d: d["building_visuals"]["placeholder_fallbacks_by_role"].update(pottery=1),
                       lambda d: d["building_visuals"]["draws_by_role"].update(pottery=0)]:
            candidate = copy.deepcopy(result)
            mutate(candidate)
            with self.assertRaises(alpha.CheckFailure):
                alpha.validate_smoke_visuals(candidate, args, PACK)


if __name__ == "__main__":
    unittest.main()
