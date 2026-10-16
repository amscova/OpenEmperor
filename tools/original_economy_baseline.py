#!/usr/bin/env python3
"""Freeze or compare legacy production contracts for the original-economy work.

Uses existing paid gameplay, persistence, restart and original-permission fixtures.
This tool does not implement rules, alter original inputs or serialize a World.
All generated saves, snapshots, corpus details and logs remain below .local/.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess
import sys
import tempfile

from run_city_v16_bridges import source_digest
from run_city_v16_longplay import inventory

FORMAT = "openemperor-original-economy-baseline-v1"
PINNED_INPUTS = {
    "files": 1464, "bytes": 819711091,
    "manifest_sha256": "dde2382fe1c348da765c8776149a73210b81f6cba7f93a0c6159d79ce3154feb",
}
RAMP_CHECKPOINTS = (
    "before-ramp", "after-ramp", "before-entry", "mid-ramp", "after-exit",
    "after-delivery", "return-ramp", "ten-thousand", "end", "disconnect-wait", "after-repair",
)
BRIDGE_CHECKPOINTS = (
    "before-bridge", "after-bridge-before-dispatch", "outbound-on-bridge", "return-on-bridge",
    "after-actual-delivery", "ten-thousand", "end", "waiting-bank-disconnected",
    "after-bank-repair", "additional-roles-end",
)
TARGETS = (
    "openemperor-original-height-audit", "openemperor-audit-original-occupancy",
    "openemperor-city-v16-ramps-fixture", "openemperor-authored-terrain-side-tests",
    "openemperor-ramp-transport-tests", "openemperor-ramp-rendering-tests",
    "openemperor-sandbox-view-tests", "openemperor-original-bridge-audit",
    "openemperor-city-v16-bridges-fixture", "openemperor-city-v16-playthrough-fixture",
    "openemperor-city-v16-longplay-fixture", "openemperor-bridge-transport-tests",
    "openemperor-bridge-rendering-tests",
)


def canonical(value: object) -> bytes:
    return json.dumps(value, sort_keys=True, separators=(",", ":"), ensure_ascii=True).encode()


def sha256(value: bytes) -> str:
    return hashlib.sha256(value).hexdigest()


def write_json(path: Path, value: object) -> None:
    path.write_text(json.dumps(value, indent=2, sort_keys=True) + "\n")


def require(condition: bool, reason: str) -> None:
    if not condition:
        raise RuntimeError(reason)


def fresh_output(repo: Path, data: Path, output: Path) -> Path:
    output = output.resolve()
    private = repo.resolve() / ".local/original-economy/oe00"
    require(private.resolve() == private, "private evidence root cannot escape through a symlink")
    require(output != private and output.is_relative_to(private), "output must be below .local/original-economy/oe00/")
    require(not output.is_relative_to(data) and not data.is_relative_to(output), "output overlaps original data")
    require(not output.exists(), "output already exists; frozen baselines are never overwritten")
    output.mkdir(parents=True)
    return output


def validate_build(repo: Path, data: Path, build: Path) -> dict:
    require(not build.is_relative_to(data) and not data.is_relative_to(build), "build overlaps original data")
    cache = build / "CMakeCache.txt"
    require(cache.is_file(), "configured CMake build required")
    values = {}
    for line in cache.read_text().splitlines():
        if line and not line.startswith(("#", "//")) and ":" in line and "=" in line:
            key, value = line.split("=", 1)
            values[key.split(":", 1)[0]] = value
    require(Path(values.get("CMAKE_HOME_DIRECTORY", "")).resolve() == repo.resolve(),
            "build belongs to a different source checkout")
    require(bool(values.get("CMAKE_HOME_DIRECTORY")), "build source identity missing")
    return {key: values.get(key) for key in
            ("CMAKE_HOME_DIRECTORY", "CMAKE_BUILD_TYPE", "OPENEMPEROR_ENABLE_SANITIZERS")}


def fixture_parents(repo: Path, data: Path) -> tuple[Path, Path]:
    parents = tuple(repo.resolve() / (".local/" + name)
                    for name in ("city-v16-ramps", "city-v16-bridges"))
    # Check both before any mkdir/mkdtemp. The existing downstream runner's
    # check happens too late to protect the mkdtemp performed by its caller.
    for parent in parents:
        require(parent.resolve() == parent, "fixture parent escapes through a symlink")
        require(not parent.is_relative_to(data) and not data.is_relative_to(parent),
                "fixture parent overlaps original data")
    for parent in parents:
        parent.mkdir(parents=True, exist_ok=True)
    return parents


def savedoc_contract(raw: bytes) -> dict:
    value = json.loads(raw)
    require(value.get("format") == "openemperor-sandbox-save", "not a complete SaveDocument")
    rule = value["rules"]["version"]
    schema = value["schema_version"]
    require(value["rules"]["id"] == "sandbox-city-v16" and
            (rule, schema) in {(3, 19), (4, 20), (5, 21), (6, 22)}, "unexpected legacy rule/schema")
    require(isinstance(value["world"], dict) and isinstance(value["world"]["ticks"], int), "WorldSnapshot missing")
    # No second C++ serializer: these fields come from the existing fixture's
    # complete SaveDocument.world == actual World::snapshot()/restore check.
    return {
        "save_sha256": sha256(raw),
        "snapshot_sha256": sha256(canonical({"rules": value["rules"], "world": value["world"]})),
        "rule": rule, "schema": schema,
        "map_sha256": value["map"]["sha256"],
        "profiles": value["profiles"],
    }


def permission_contract(corpus: dict, *, expected_maps: int = 167, expected_prepared: int = 165) -> list:
    maps = corpus["maps"]
    require(len(maps) == expected_maps and corpus["original_input_inventory_unchanged"], "incomplete original corpus")
    require(corpus["first_failure_distribution"] ==
            {"prepared": expected_prepared, "known_conflict": expected_maps - expected_prepared}, "corpus admission changed")
    names = [row["relative_path"] for row in maps]
    require(len(set(names)) == len(names), "duplicate map in corpus")
    output = []
    prepared = 0
    for row in sorted(maps, key=lambda row: row["relative_path"]):
        require(row["manager_status"] == "complete", "incomplete original manager")
        require(bool(re.fullmatch("[0-9a-f]{64}", row["input_sha256"])), "invalid map input digest")
        result = {"map": row["relative_path"], "input_sha256": row["input_sha256"], "category": row["first_category"]}
        if row["first_category"] == "prepared":
            prepared += 1
            for name, version in (("policies", 1), ("policy2", 2), ("policy3", 3)):
                values = row[name]
                require(set(values) == {"zero_legacy", "maximal_legacy", "production_legacy"}, "permission mask case missing")
                for item in values.values():
                    require(item["policy_version"] == version, "wrong permission version")
                    for key in ("policy_fingerprint", "canonical_sha256", "legacy_mask_sha256"):
                        require(bool(re.fullmatch("[0-9a-f]{64}", item[key])), "invalid permission digest")
                result[name] = values
        else:
            require(row["first_category"] == "known_conflict", "unexpected refusal category")
            require(not any(name in row for name in ("policies", "policy2", "policy3")), "refused map published permissions")
        output.append(result)
    require(prepared == expected_prepared, "prepared map count differs")
    return output


def compare(reference: dict, current: dict) -> None:
    require(reference["format"] == FORMAT and current["format"] == FORMAT, "unknown baseline format")
    for field in ("original_inputs", "saves", "permissions"):
        require(reference[field] == current[field], "frozen contract changed: " + field)


def load_reference(path: Path) -> dict:
    value = json.loads(path.read_text())
    require(value["format"] == FORMAT, "unknown reference format")
    for name, entry in value["saves"].items():
        file = (path.parent / "saves" / name).resolve(strict=True)
        require(file.is_relative_to(path.parent.resolve()), "frozen save path escapes reference")
        require(savedoc_contract(file.read_bytes()) == entry, "frozen save damaged: " + name)
        snapshot = (path.parent / "snapshots" / name).resolve(strict=True)
        require(snapshot.is_relative_to(path.parent.resolve()), "snapshot path escapes reference")
        require(sha256(snapshot.read_bytes()) == entry["snapshot_sha256"], "frozen snapshot damaged: " + name)
    return value


def freeze_saves(root: Path, ramps: Path, old: Path) -> dict:
    sources = {}
    for label, directory, names in (("rule6", ramps / "gameplay", RAMP_CHECKPOINTS),
                                     ("rule5", old / "gameplay", BRIDGE_CHECKPOINTS)):
        for name in names:
            for suffix in ("", "-plus100-control"):
                filename = name + suffix + ".json"
                sources[label + "/" + filename] = directory / filename
    for folder, expected in (("historical-rule3", 14), ("current-rule4-Xia", 8), ("current-rule4-Handan", 8)):
        found = []
        for file in sorted((old / folder).rglob("*.json")):
            value = json.loads(file.read_text())
            if isinstance(value, dict) and value.get("format") == "openemperor-sandbox-save":
                found.append(file)
        require(len(found) == expected, "legacy saved checkpoint count differs: " + folder)
        for file in found:
            sources[folder + "/" + file.relative_to(old / folder).as_posix()] = file
    require(len(sources) == 72, "complete 72-save contract required")
    result = {}
    for name, file in sorted(sources.items()):
        raw = file.read_bytes()
        result[name] = savedoc_contract(raw)
        target = root / "saves" / name
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_bytes(raw)
        value = json.loads(raw)
        snapshot = root / "snapshots" / name
        snapshot.parent.mkdir(parents=True, exist_ok=True)
        snapshot.write_bytes(canonical({"rules": value["rules"], "world": value["world"]}))
    return result


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("mode", choices=("capture", "verify"))
    parser.add_argument("--data", required=True, type=Path)
    parser.add_argument("--build", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--reference", type=Path)
    args = parser.parse_args()
    require((args.mode == "verify") == (args.reference is not None), "verify requires --reference; capture cannot replace one")
    repo = Path(__file__).resolve().parents[1]
    data, build = args.data.resolve(strict=True), args.build.resolve(strict=True)
    build_identity = validate_build(repo, data, build)
    require(not args.output.resolve().is_relative_to(build) and not build.is_relative_to(args.output.resolve()),
            "evidence output overlaps build")
    reference = load_reference(args.reference.resolve(strict=True)) if args.reference else None
    if args.reference:
        reference_root = args.reference.resolve(strict=True).parent
        output = args.output.resolve()
        require(not output.is_relative_to(reference_root) and not reference_root.is_relative_to(output),
                "new output must be separate from the frozen reference")
    root = fresh_output(repo, data, args.output)
    fixture_parent, _ = fixture_parents(repo, data)
    if reference:
        lock = json.loads((repo / "docs/original-economy/baseline-lock.json").read_text())
        require(sha256(canonical(reference)) == lock["contract_sha256"], "reference is not the committed OE-00 baseline")
    initial = inventory(data)
    require(initial == PINNED_INPUTS, "original data differs from the investigated source")
    source = source_digest(repo)
    report = {"format": FORMAT, "result": "INCOMPLETE", "mode": args.mode,
              "head": subprocess.check_output(["git", "rev-parse", "HEAD"], cwd=repo, text=True).strip(),
              "source_sha256": source, "original_inputs": initial,
              "build_identity": build_identity, "commands": []}
    env = {key: value for key, value in os.environ.items() if not key.startswith(("SDL_", "DYLD_", "OPENEMPEROR_"))}

    def run(tag: str, command: list[str]) -> None:
        with (root / (tag + ".log")).open("w") as log:
            code = subprocess.run(command, cwd=repo, env=env, stdout=log, stderr=subprocess.STDOUT).returncode
        report["commands"].append({"tag": tag, "command": command, "exit": code})
        write_json(root / "run.json", report)
        require(code == 0, "dependent work stopped: " + tag)
        print(tag + ": PASS", flush=True)

    try:
        run("build", ["cmake", "--build", str(build), "--parallel", "2", "--target", *TARGETS])
        ramps = Path(tempfile.mkdtemp(prefix="oe00-", dir=fixture_parent))
        run("legacy-production", [sys.executable, str(repo / "tools/run_city_v16_ramps.py"),
                                  str(repo), str(data), str(build), str(ramps), "--all"])
        summary = json.loads((ramps / "summary.json").read_text())
        require(summary["result"] == "PASS" and summary["new_process_checkpoint_pairs"] == 11, "Ramp production run incomplete")
        old = Path(summary["old_contract_root"])
        require(old.resolve().is_relative_to((repo / ".local/city-v16-bridges").resolve()), "unexpected bridge evidence root")
        bridge_summary = json.loads((old / "summary.json").read_text())
        require(bridge_summary["result"] == "PASS" and len(bridge_summary["restarts"]) == 10, "Bridge production run incomplete")
        corpus = json.loads((ramps / "corpus/corpus.json").read_text())
        contract = {"format": FORMAT, "original_inputs": initial,
                    "saves": freeze_saves(root, ramps, old), "permissions": permission_contract(corpus)}
        if reference:
            compare(reference, contract)
        write_json(root / "contract.json", contract)
        report.update(result="PASS", contract_sha256=sha256(canonical(contract)),
                      saved_documents=len(contract["saves"]), actual_world_snapshots=len(contract["saves"]),
                      original_maps=167, prepared_maps=165, permission_cases=1485,
                      separate_ramp_restart_pairs=11, separate_bridge_restart_pairs=10,
                      fixture_root=str(ramps.relative_to(repo)), bridge_root=str(old.relative_to(repo)),
                      evidence="Existing paid World/Save/Restore fixtures; original-map summaries; no new simulation",
                      original_game_parity=False, native_human_acceptance=False)
    finally:
        report["source_unchanged"] = source_digest(repo) == source
        report["original_inputs_unchanged"] = inventory(data) == initial
        if not report["source_unchanged"] or not report["original_inputs_unchanged"]:
            report["result"] = "FAIL"
        write_json(root / "run.json", report)
    require(report["result"] == "PASS", "baseline run did not complete")
    print("Legacy contracts PASS:", root, flush=True)


if __name__ == "__main__":
    main()
