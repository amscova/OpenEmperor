#!/usr/bin/env python3
"""Verify the paid Rule9 unsupported-goods fixture and frozen Rule7/8 bytes privately."""
import argparse
import hashlib
import json
from pathlib import Path
import shutil
import subprocess
import tempfile

MODEL_SHA = "7e51df8cd49ab39fea7397b7c1a9e7edefdc98a8f09b5e2139987346b4b972e6"
PRIVATE_OUTPUT = Path(".local/original-economy/oe13/goods-fixtures")
BASELINE = Path(".local/original-economy/oe13/pre-implementation-baseline")


def overlaps(left, right):
    return left == right or left in right.parents or right in left.parents


def check_output_path(repo, path):
    """Reject linked/non-directory parents before any private output is written."""
    current = repo
    for part in path.relative_to(repo).parts:
        current /= part
        if current.is_symlink():
            raise RuntimeError(f"Private output must not traverse a symlink: {current}")
        if current != path and current.exists() and not current.is_dir():
            raise RuntimeError(f"Private output parent is not a directory: {current}")
    if path.resolve() != path:
        raise RuntimeError("Private output must use its canonical path")


def preflight_output(repo, requested, original, build):
    """Validate without creating directories or copying original model bytes."""
    repo = repo.resolve(strict=True)
    private = repo / PRIVATE_OUTPUT
    check_output_path(repo, private)
    if private.exists() and not private.is_dir():
        raise RuntimeError(f"Private fixture root is not a directory: {private}")
    for source in (original.resolve(strict=True), build.resolve(strict=True)):
        if overlaps(private, source):
            raise RuntimeError("Private fixture root must not overlap original data or build directory")
    if requested is None:
        return private, None
    output = requested.absolute()
    if ".." in output.parts or output == private or not output.is_relative_to(private):
        raise RuntimeError(f"Output must be a new subdirectory of {private}")
    check_output_path(repo, output)
    if output.exists():
        raise RuntimeError(f"Output already exists: {output}")
    return private, output


def create_output(repo, requested, original, build):
    private, output = preflight_output(repo, requested, original, build)
    if output is None:
        private.mkdir(parents=True, exist_ok=True)
        # Recheck after parent creation before making the new run directory.
        preflight_output(repo, None, original, build)
        output = Path(tempfile.mkdtemp(prefix="run-", dir=private))
    else:
        output.mkdir(parents=True, exist_ok=False)
    # The actual directory must remain private before any licensed bytes enter it.
    check_output_path(repo.resolve(strict=True), output)
    return output


def digest(path):
    h = hashlib.sha256()
    with path.open("rb") as source:
        for block in iter(lambda: source.read(1024 * 1024), b""):
            h.update(block)
    return h.hexdigest()


def inventory(root):
    rows, size = [], 0
    for path in sorted(root.rglob("*")):
        if not path.is_file():
            continue
        stat = path.stat()
        rows.append([str(path.relative_to(root)), stat.st_size, stat.st_mode,
                     stat.st_mtime_ns, digest(path)])
        size += stat.st_size
    encoded = json.dumps(rows, separators=(",", ":"), ensure_ascii=True).encode()
    return {"files": len(rows), "bytes": size,
            "manifest_sha256": hashlib.sha256(encoded).hexdigest()}


def run(binary, args, log):
    with log.open("w") as output:
        result = subprocess.run([str(binary), *map(str, args)], stdout=output,
                                stderr=subprocess.STDOUT, check=False)
    if result.returncode:
        raise RuntimeError(f"Fixture exited {result.returncode}: {log}")


def frozen_baseline(repo):
    root = repo / BASELINE
    check_output_path(repo, root)
    capture = json.loads((root / "capture.json").read_text())
    if capture.get("status") != "PASS" or not capture.get("rule7_oe11_golden_replayed_byte_exact"):
        raise RuntimeError("Missing independently captured pre-implementation Rule7/8 baseline")
    for record in capture["files"]:
        relative = Path(record["path"])
        if relative.is_absolute() or ".." in relative.parts:
            raise RuntimeError("Invalid frozen baseline manifest path")
        path = root / relative
        check_output_path(repo, path)
        if not path.is_file() or digest(path) != record["sha256"]:
            raise RuntimeError(f"Frozen pre-implementation file changed: {path}")
    return root, capture


def check_balances(scene):
    balances = scene["balances"]
    assert len(balances) == 6
    for index, balance in enumerate(balances):
        assert balance["good"] == index and balance["supported"] is False and balance["conserved"] is True
        assert all(balance[field] == 0 for field in ("produced", "imported", "consumed", "demolished",
                                                   "stock", "cargo", "source_reserved", "incoming_reserved"))


def compare_legacy(baseline, binary, influence_binary, stage, output):
    """Use current writers, comparing literal bytes to independently frozen old writers."""
    legacy7 = output / "legacy-rule7"
    run(binary, ["--legacy7", legacy7], output / "legacy-rule7.log")
    reference7 = baseline / "rule7-reference.json"
    assert (legacy7 / "rule7.json").read_bytes() == reference7.read_bytes(), "Rule7/Schema23 writer drift"
    legacy8 = output / "legacy-rule8"
    run(influence_binary, ["--create", stage, legacy8], output / "legacy-rule8.log")
    reference8 = baseline / "rule8-final-output/create"
    saves = []
    for name in ("removed.json", "removed-next.json", "rebought.json", "rebought-next.json"):
        assert (legacy8 / name).read_bytes() == (reference8 / name).read_bytes(), f"Rule8/Schema24 writer drift: {name}"
        saves.append(name)
    for name in ("removed-report.json", "removed-next-report.json", "rebought-report.json", "rebought-next-report.json"):
        assert json.loads((legacy8 / name).read_text()) == json.loads((reference8 / name).read_text()), name
    for tag in ("removed", "rebought"):
        resumed = output / ("legacy-rule8-resume-" + tag)
        run(influence_binary, ["--resume", stage, resumed, legacy8 / (tag + ".json")],
            output / ("legacy-rule8-resume-" + tag + ".log"))
        historical = baseline / ("rule8-final-output/resume-" + tag)
        assert (resumed / "result.json").read_bytes() == (historical / "result.json").read_bytes()
        assert json.loads((resumed / "result-report.json").read_text()) == json.loads((historical / "result-report.json").read_text())
        saves.append("resume-" + tag + "/result.json")
    return {"rule7_schema23_byte_equal": True, "rule7_reference_sha256": digest(reference7),
            "rule8_schema24_byte_equal": True, "rule8_save_files": saves,
            "rule8_provenance_equal": True, "current_rule8_binary_sha256": digest(influence_binary)}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("data", type=Path, help="Read-only original installation root")
    parser.add_argument("--build-dir", type=Path)
    parser.add_argument("--output", type=Path,
                        help=f"New subdirectory of repository/{PRIVATE_OUTPUT} only")
    args = parser.parse_args()
    repo = Path(__file__).resolve().parent.parent
    original = args.data.resolve(strict=True)
    source_model = original / "Model/EmperorBuildingModels.txt"
    if digest(source_model) != MODEL_SHA:
        raise RuntimeError("Expected independently pinned original Model SHA; authored models are unit tests only")
    build = (args.build_dir or repo / ".local/reswall/release-build").resolve(strict=True)
    binary = build / "openemperor-original-goods-fixture"
    if not binary.is_file():
        raise RuntimeError(f"Build target openemperor-original-goods-fixture first: {build}")
    influence_binary = build / "openemperor-original-influence-fixture"
    if not influence_binary.is_file():
        raise RuntimeError(f"Build the legacy openemperor-original-influence-fixture target: {build}")
    # Reject invalid output before inventory/preparation and before any mkdir/write.
    preflight_output(repo, args.output, original, build)
    baseline, capture = frozen_baseline(repo)
    before = inventory(original)
    output = create_output(repo, args.output, original, build)
    stage = output / "data"
    (stage / "Model").mkdir(parents=True)
    (stage / "Cities").mkdir()
    # Byte-identical local copy, never a linked writable original or authored model.
    shutil.copyfile(source_model, stage / "Model/EmperorBuildingModels.txt")
    (stage / "Cities/Influence.map").write_text(
        "Authored24x20 policy3 influence acceptance geography; not an original map.\n")
    assert digest(stage / "Model/EmperorBuildingModels.txt") == MODEL_SHA
    run(binary, ["--create", stage, output / "create"], output / "create.log")
    fixture = json.loads((output / "create/fixture.json").read_text())
    assert fixture["status"] == "PASS" and fixture["paid_house_count"] == 20
    assert fixture["paid_road_count"] == 40 and fixture["well_purchases"] == 3
    assert fixture["final"]["funds"] == 140 and fixture["final"]["construction_spent"] == 1860
    assert fixture["final"]["model_sha256"] == MODEL_SHA
    assert fixture["positive_runtime_goods"] is False and fixture["snapshot_injection"] is False
    check_balances(fixture["initial_both"])
    check_balances(fixture["final"])
    restarts = []
    for checkpoint in fixture["checkpoints"]:
        folder = output / ("resume-" + checkpoint["tag"])
        source = Path(checkpoint["save"])
        source_sha = digest(source)
        run(binary, ["--resume", stage, folder, source], output / (checkpoint["tag"] + "-resume.log"))
        expected = Path(checkpoint["expected_result"])
        actual = folder / "result.json"
        assert json.loads(actual.read_text()) == json.loads(expected.read_text()), "Full SaveDocument process replay differs"
        assert actual.read_bytes() == expected.read_bytes(), "Canonical process save bytes differ"
        assert json.loads((folder / "result-report.json").read_text()) == json.loads(
            Path(checkpoint["expected_report"]).read_text()), "Process provenance/House values differ"
        midpoint = folder / "midpoint.json"
        expected_midpoint = Path(checkpoint["expected_midpoint"])
        assert midpoint.read_bytes() == expected_midpoint.read_bytes(), "20k midpoint canonical bytes differ"
        midpoint_report = json.loads((folder / "midpoint-report.json").read_text())
        assert midpoint_report == json.loads(Path(checkpoint["expected_midpoint_report"]).read_text())
        final_report = json.loads((folder / "result-report.json").read_text())
        check_balances(midpoint_report)
        check_balances(final_report)
        saved = json.loads(actual.read_text())
        assert saved["schema_version"] == 25 and saved["rules"]["version"] == 9
        assert final_report["tick"] == midpoint_report["tick"] + 20000 == 40000
        assert final_report["funds"] == midpoint_report["funds"] - 2
        assert final_report["construction_spent"] == midpoint_report["construction_spent"] + 2
        assert digest(source) == source_sha, "Process modified its input save"
        restarts.append({"checkpoint": checkpoint["tag"], "separate_os_process": True,
                         "ticks_advanced": 40000, "clock_periods": [20000, 20000],
                         "actual_paid_road_continuation": True, "full_savedocument_equal": True,
                         "canonical_bytes_equal": True, "provenance_equal": True,
                         "balances_equal": True, "midpoint_equal": True, "source_save_unchanged": True})
    legacy = compare_legacy(baseline, binary, influence_binary, stage, output)
    # Rule9's existing paid scene must differ from Rule8 only in the authorized
    # generation and new zero-goods payload. This is a semantic cross-version
    # comparison; the independent old-writer byte proof is above.
    for tag in ("removed", "rebought"):
        current = json.loads((output / "create" / (tag + ".json")).read_text())
        old = json.loads((baseline / "rule8-final-output/create" / (tag + ".json")).read_text())
        goods = current["world"].pop("original_goods")
        assert goods["stocks"] == goods["cargo"] == goods["reservations"] == []
        assert len(goods["totals"]) == 6
        for index, row in enumerate(goods["totals"]):
            assert row == {"good": index, "produced": 0, "imported": 0, "consumed": 0, "demolished": 0}
        current["schema_version"] = 24
        current["rules"]["version"] = 8
        assert current == old, "Rule9 changed inherited paid-scene authority"
    after = inventory(original)
    assert before == after, "Original installation changed"
    assert digest(stage / "Model/EmperorBuildingModels.txt") == MODEL_SHA
    frozen_baseline(repo)  # The comparison must preserve every frozen byte.
    report = {"status": "PASS", "evidence": "Paid World commands; original Model; authored geography; unsupported goods zero",
              "original_data": str(original), "original_inventory_before": before,
              "original_inventory_after": after, "original_unchanged": True,
              "original_model_sha256": MODEL_SHA, "model_copy_byte_identical": True,
              "fixture_binary_sha256": digest(binary), "snapshot_injection": False,
              "positive_runtime_goods": False, "legacy": legacy,
              "frozen_baseline_capture_sha256": digest(baseline / "capture.json"),
              "frozen_baseline_head": capture["head_before_implementation"],
              "inherited_rule8_scene_equal": True,
              "city13_comparison": "Unchanged pure algorithm; authored Well72-to-Well mapping; no City13 World claim",
              "fixture": fixture, "restarts": restarts,
              "limits": ["No positive runtime goods cycle, producer, transport, worker or consumption",
                         "Not original-map geometry or original game execution",
                         "F5/F9 SDL checks are separate; no native/human or Gate-C acceptance"]}
    (output / "report.json").write_text(json.dumps(report, indent=2) + "\n")
    print(json.dumps({"status": "PASS", "report": str(output / "report.json"),
                      "paid_houses": 20, "separate_process_replays": len(restarts),
                      "original_unchanged": True}))


if __name__ == "__main__":
    main()
