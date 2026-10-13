"""Explicit original-data runner. Save input is copied/read, never searched or changed."""
from pathlib import Path
import hashlib
import json
import os
import subprocess
import sys
import time


def write(path, value):
    path.write_text(json.dumps(value, indent=2) + "\n")


def inventory(data):
    manifest = hashlib.sha256()
    count = size = 0
    for path in sorted(data.rglob("*"), key=lambda p: p.relative_to(data).as_posix()):
        if path.is_symlink():
            raise RuntimeError("original-data symlink rejected")
        if not path.is_file():
            continue
        digest = hashlib.sha256()
        with path.open("rb") as source:
            for block in iter(lambda: source.read(1024 * 1024), b""):
                digest.update(block)
        length = path.stat().st_size
        manifest.update(f"{path.relative_to(data).as_posix()}\t{length}\t{digest.hexdigest()}\n".encode())
        count += 1
        size += length
    return {"files": count, "bytes": size, "manifest_sha256": manifest.hexdigest()}


def main():
    repo, data, build, root = [Path(x).resolve(strict=True) for x in sys.argv[1:5]]
    mode, save_argument = sys.argv[5:]
    if mode not in ("--fresh", "--save") or not root.is_relative_to(repo / ".local/city-v16-longplay"):
        raise RuntimeError("unsupported invocation or output root")
    if data == root or root.is_relative_to(data) or data.is_relative_to(root) or any(root.iterdir()):
        raise RuntimeError("fresh isolated output root required")
    initial = inventory(data)
    pinned = {"files": 1464, "bytes": 819711091,
              "manifest_sha256": "dde2382fe1c348da765c8776149a73210b81f6cba7f93a0c6159d79ce3154feb"}
    if initial != pinned:
        raise RuntimeError("original revision differs; no simulation run")
    env = {k: v for k, v in os.environ.items() if not k.startswith(("SDL_", "DYLD_", "OPENEMPEROR_"))}
    report = {"head": subprocess.check_output(["git", "rev-parse", "HEAD"], cwd=repo, text=True).strip(),
              "original_revision": initial, "evidence": "SCRIPTED VIEW / RECONSTRUCTED SCENARIOS / SEPARATE-PROCESS DIAGNOSTICS",
              "native_human_acceptance": "NOT RUN", "engine_rules_modified": False, "runs": {}}
    write(root / "field-provenance.json", {
        "persisted_state": ["WorldSnapshot", "IDs", "origins", "placement ticks", "stocks",
                            "incoming reservations", "courier cargo/target/phase/path/progress",
                            "population", "demand progress", "coverage/protection deadlines",
                            "operation enabled", "priority", "treasury"],
        "derived_current_state": ["historical/effective levels", "capacity", "water",
                                  "desirability", "current demand readiness", "workers assigned",
                                  "live goal", "road entrances/reachability", "dispatch status", "debt"],
        "cumulative_historical_counters": ["fulfilled/missed demands", "consumed goods", "production",
                                          "actual taxes", "actual maintenance", "construction"],
        "current_rate": ["installed_upkeep_per400 (not historical expenditure)"],
        "interval_delta": ["finance-400t deltas over from_tick..tick", "actual demand/production events"],
        "timing": "Workforce captures actual pre-tick allocation; demand inputs precede same-tick arrivals. "
                  "400t snapshots are after World.tick and before scheduled commands at that tick.",
        "diagnostic_flags": "Current readiness, water/desirability/capacity and NOT_YET_DUE are distinct "
                            "from actual deadline failure reasons. Reason exposures can overlap.",
        "commands": "operation_argument/priority_argument are command parameters, applicable only to "
                    "that command type; current building operation/priority are in workforce/production reports.",
        "routes": "On-demand existing World routing queries in harness only; no runtime instrumentation."
    })
    executable = build / "openemperor-city-v16-longplay-fixture"

    def run(tag, arguments, program=executable):
        destination = root / tag
        destination.mkdir()
        start = time.monotonic()
        with (root / (tag + ".log")).open("w") as log:
            result = subprocess.run([str(program), str(data), str(destination), *arguments],
                                    cwd=repo, env=env, stdout=log, stderr=subprocess.STDOUT)
        row = {"exit": result.returncode, "elapsed_seconds": time.monotonic() - start,
               "directory": str(destination), "log": str(root / (tag + ".log"))}
        if (destination / "summary.json").exists():
            row["summary"] = json.loads((destination / "summary.json").read_text())
        if (destination / "performance.json").exists():
            row["performance"] = json.loads((destination / "performance.json").read_text())
        report["runs"][tag] = row
        if result.returncode:
            report["result"] = "FAIL"
        write(root / "summary.json", report)
        print(f"{tag}: {'PASS' if result.returncode == 0 else 'FAIL'} ({row['elapsed_seconds']:.2f}s)", flush=True)
        if tag.startswith("C-fresh") and "summary" in row:
            s = row["summary"]
            print(f"  Goal tick {s['first_goal_tick']}; {s['stability_ticks']} further ticks; "
                  f"population {s['population_min']}..{s['population_max']}; "
                  f"goal lost {s['goal_lost_ticks']} ticks; net Funds +{s.get('stability_net_funds', 0)}", flush=True)
        if result.returncode:
            raise RuntimeError(f"{tag} failed; dependent runs stopped, see {row['log']}")
        return destination, result.returncode

    copied_save = None
    source_bytes = None
    if mode == "--save":
        source_path = Path(save_argument).resolve(strict=True)
        if not source_path.is_file() or source_path.stat().st_size > 8 * 1024 * 1024:
            raise RuntimeError("explicit input must be one bounded regular SaveDocument file")
        source_bytes = source_path.read_bytes()
        copied_save = root / "player-save-input-copy.json"
        copied_save.write_bytes(source_bytes)
        report["player_save"] = {"evidence": "EXPLICIT SAVE INPUT supplied by --save; human origin is not inferred",
                                 "input_sha256": hashlib.sha256(source_bytes).hexdigest(),
                                 "input_bytes": len(source_bytes), "copy": str(copied_save)}
        report["scenario_A"] = {"status": "RUN", "evidence": "Explicit supplied SaveDocument replay; "
                               "ACTUAL PLAYER SAVE only when caller provides that provenance"}
        run("A-player-no-intervention", ["--analyse", str(copied_save), "14000"])
    else:
        report["player_save"] = {"evidence": "PLAYER OBSERVATION only; no explicitly provided save"}
        report["scenario_A"] = {"status": "NOT RUN", "reason": "No authorized player save; reconstruction is separate"}
    report["player_observation"] = {"verified_save_baseline": False, "reported_tick": 13067,
                                    "reported_population": 60, "reported_houses": 18, "reported_level2": 8,
                                    "reported_funds": 2259, "reported_taxes_total": 6925,
                                    "reported_upkeep_per400": 98, "reported_service_missing": 8,
                                    "reported_pottery_missing": 11,
                                    "conditional_minimum_capacity": 188,
                                    "conditional_maximum_installed_workers": 49}
    xia, _ = run("C-fresh-Xia", ["--fresh", "Cities/Xia.map"])
    run("C-fresh-Handan", ["--fresh", "Cities/Handan.map"])
    if copied_save:
        baseline = copied_save
    else:
        reconstructed, _ = run("B-reconstructed18", ["--reconstructed", str(xia / "first-goal.json")])
        baseline = reconstructed / "under-supplied-baseline.json"
    control, _ = run("B-control-no-intervention", ["--analyse", str(baseline), "14000"])
    priority, _ = run("B-priority-only", ["--priority-only", str(baseline)])
    run("B-minimal-recovery", ["--recovery", str(baseline)])
    control_summary = report["runs"]["B-control-no-intervention"]["summary"]
    priority_summary = report["runs"]["B-priority-only"]["summary"]
    until = priority_summary["final"]["tick"]
    def demands_through(folder):
        return [j for line in (folder / "demand-events.jsonl").read_text().splitlines()
                if (j := json.loads(line))["tick"] <= until]
    report["priority_vs_control"] = {
        "same_actual_demand_events_through10000": demands_through(control) == demands_through(priority),
        "population_control_at10000": control_summary["at10000"]["population"],
        "population_priority_at10000": priority_summary["final"]["population"],
        "funds_control_at10000": control_summary["at10000"]["funds"],
        "funds_priority_at10000": priority_summary["final"]["funds"],
        "interpretation": "Measured comparison, not an assertion that priorities never matter; "
                          "the reconstruction has enough workers and a paused Post."
    }

    # A separate OS process loads each real checkpoint after its writer ended.
    restarts = []
    for tag, row in list(report["runs"].items()):
        path = Path(row["directory"]) / "save-restart-comparison.json"
        if not path.exists():
            continue
        checks = json.loads(path.read_text())
        for number, checkpoint in enumerate(checks["same_process_checkpoints"]):
            child_tag = f"restart-{tag}-{number}"
            child, code = run(child_tag, ["--resume", checkpoint["save"], str(checkpoint["ticks"]),
                                          checkpoint["expected_plus100"]])
            complete_equal = code == 0 and json.loads((child / "result.json").read_text()) == json.loads(Path(checkpoint["expected_plus100"]).read_text())
            restarts.append({"scenario": tag, "checkpoint": checkpoint["tag"], "exit": code,
                             "complete_savedocument_equal": complete_equal, "process_directory": str(child)})
        checks["separate_process"] = [r for r in restarts if r["scenario"] == tag]
        write(path, checks)
    report["separate_process_restarts"] = restarts

    # Preserve the independently frozen historical profile/rule/save result.
    historical, historical_code = run("historical-rule3", [str(repo / "resources")], build / "openemperor-city-v16-playthrough-fixture")
    if historical_code == 0:
        legacy = json.loads((historical / "fixture-report.json").read_text())
        report["historical_rule3"] = {"goal_tick": legacy["first_goal"]["tick"],
                                       "end_tick": legacy["final"]["tick"], "rule": 3, "policy": 1, "schema": 19}
        if report["historical_rule3"] != {"goal_tick": 6800, "end_tick": 9200, "rule": 3, "policy": 1, "schema": 19}:
            raise RuntimeError("frozen historical rule3 result changed")
    with (root / "corpus.log").open("w") as log:
        census = subprocess.run([str(build / "openemperor-audit-original-occupancy"), str(data),
                                 str(root / "corpus"), "--summary-only"], cwd=repo, env=env,
                                stdout=log, stderr=subprocess.STDOUT)
    report["corpus_exit"] = census.returncode
    if census.returncode == 0:
        report["corpus"] = json.loads((root / "corpus/corpus.json").read_text())["first_failure_distribution"]
    report["original_revision_unchanged"] = inventory(data) == initial
    if copied_save:
        report["player_input_unchanged"] = source_path.read_bytes() == source_bytes
    report["result"] = "PASS" if (all(r["exit"] == 0 for r in report["runs"].values())
        and all(r["complete_savedocument_equal"] for r in restarts)
        and report.get("corpus") == {"known_conflict": 2, "prepared": 165}
        and report["original_revision_unchanged"] and report.get("player_input_unchanged", True)) else "FAIL"
    write(root / "summary.json", report)
    print(f"{report['result']}: complete long-play reports: {root}", flush=True)
    print("Player replay:", "explicit supplied save (origin supplied by caller)" if copied_save else "NOT RUN; no authorized player save")
    print("PASS validates scripted diagnostics/invariants; native human acceptance remains NOT RUN.")
    return 0 if report["result"] == "PASS" else 1


if __name__ == "__main__":
    try:
        sys.exit(main())
    except Exception as error:
        print(f"Long-play runner failed: {error}", file=sys.stderr)
        sys.exit(1)
