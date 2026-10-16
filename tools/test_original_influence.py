#!/usr/bin/env python3
"""Run the paid Rule8 fixture with byte-identical original model data, privately."""
import argparse
import hashlib
import json
from pathlib import Path
import shutil
import subprocess
import tempfile

MODEL_SHA = "7e51df8cd49ab39fea7397b7c1a9e7edefdc98a8f09b5e2139987346b4b972e6"
PRIVATE_OUTPUT = Path(".local/original-economy/oe12/influence-fixtures")


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
    binary = build / "openemperor-original-influence-fixture"
    if not binary.is_file():
        raise RuntimeError(f"Build target openemperor-original-influence-fixture first: {build}")
    # Reject invalid output before inventory/preparation and before any mkdir/write.
    preflight_output(repo, args.output, original, build)
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
    restarts = []
    for checkpoint in fixture["checkpoints"]:
        folder = output / ("resume-" + checkpoint["tag"])
        source = Path(checkpoint["save"])
        source_sha = digest(source)
        run(binary, ["--resume", stage, folder, source], output / (checkpoint["tag"] + "-resume.log"))
        expected = Path(checkpoint["expected_next"])
        actual = folder / "result.json"
        assert json.loads(actual.read_text()) == json.loads(expected.read_text()), "Full SaveDocument process replay differs"
        assert actual.read_bytes() == expected.read_bytes(), "Canonical process save bytes differ"
        assert json.loads((folder / "result-report.json").read_text()) == json.loads(
            Path(checkpoint["expected_report"]).read_text()), "Process provenance/House values differ"
        saved = json.loads(actual.read_text())
        assert saved["schema_version"] == 24 and saved["rules"]["version"] == 8
        assert digest(source) == source_sha, "Process modified its input save"
        restarts.append({"checkpoint": checkpoint["tag"], "separate_os_process": True,
                         "ticks_advanced": 20000, "full_savedocument_equal": True,
                         "canonical_bytes_equal": True, "provenance_equal": True,
                         "source_save_unchanged": True})
    after = inventory(original)
    assert before == after, "Original installation changed"
    assert digest(stage / "Model/EmperorBuildingModels.txt") == MODEL_SHA
    report = {"status": "PASS", "evidence": "Scripted paid production commands; original Model; authored geography",
              "original_data": str(original), "original_inventory_before": before,
              "original_inventory_after": after, "original_unchanged": True,
              "original_model_sha256": MODEL_SHA, "model_copy_byte_identical": True,
              "fixture_binary_sha256": digest(binary), "snapshot_injection": False,
              "city13_comparison": "Unchanged pure algorithm; authored Well72-to-Well mapping; no City13 World claim",
              "fixture": fixture, "restarts": restarts,
              "limits": ["Not original-map geometry or original game execution",
                         "Not SDL/native/human acceptance or a complete Gate-B suite"]}
    (output / "report.json").write_text(json.dumps(report, indent=2) + "\n")
    print(json.dumps({"status": "PASS", "report": str(output / "report.json"),
                      "paid_houses": 20, "separate_process_replays": len(restarts),
                      "original_unchanged": True}))


if __name__ == "__main__":
    main()
