"""Owned paid fixtures, frozen start saves, test-only schedulers, complete restarts.

Reuse the long-play executable and source inventory; never search personal saves.
"""
from pathlib import Path
import hashlib
import json
import os
import shutil
import subprocess
import sys
import time

from run_city_v16_longplay import inventory, write


def main():
    repo, data, build, root = [Path(x).resolve(strict=True) for x in sys.argv[1:5]]
    mode = sys.argv[5]
    if mode not in ("diagnostics", "--benchmark", "--ui", "--all"):
        raise RuntimeError("unsupported mode")
    if not root.is_relative_to(repo / ".local/city-v16-service-coverage") or any(root.iterdir()):
        raise RuntimeError("fresh isolated root required")
    if root.is_relative_to(data) or data.is_relative_to(root):
        raise RuntimeError("output must be outside original data")
    original = inventory(data)
    pinned = {"files": 1464, "bytes": 819711091,
              "manifest_sha256": "dde2382fe1c348da765c8776149a73210b81f6cba7f93a0c6159d79ce3154feb"}
    if original != pinned:
        raise RuntimeError("original revision differs; stopped before simulation")
    head = subprocess.check_output(["git", "rev-parse", "HEAD"], cwd=repo, text=True).strip()
    status = subprocess.check_output(["git", "status", "--short"], cwd=repo, text=True)
    env = {k: v for k, v in os.environ.items() if not k.startswith(("SDL_", "DYLD_", "OPENEMPEROR_"))}
    executable = build / "openemperor-city-v16-longplay-fixture"
    report = {"production_commit": head, "workspace_status": status, "mode": mode,
              "original_revision": original, "rules": "City-v16 rule4 / policy2 / schema20",
              "production_scheduler_changed": False, "human_acceptance": "OPEN", "runs": {}}
    digest = lambda p: hashlib.sha256(p.read_bytes()).hexdigest()

    def run(tag, args, program=executable):
        destination = root / tag
        destination.mkdir()
        started = time.monotonic()
        with (root / (tag + ".log")).open("w") as log:
            subprocess.run([str(program), str(data), str(destination), *map(str, args)],
                           cwd=repo, env=env, stdout=log, stderr=subprocess.STDOUT, check=True)
        row = {"directory": str(destination), "elapsed_seconds": time.monotonic() - started}
        report["runs"][tag] = row
        write(root / "summary.json", report)
        print(f"{tag}: technical PASS ({row['elapsed_seconds']:.2f}s)", flush=True)
        return destination

    xia = run("fresh-Xia", ["--fresh", "Cities/Xia.map"])
    reconstructed = run("reconstructed18", ["--reconstructed", xia / "first-goal.json"])
    recovery = run("recovery-controls", ["--service-prepare", reconstructed / "under-supplied-baseline.json"])
    source = recovery / "service-start.json"
    source_bytes = source.read_bytes()
    report["baseline_input_sha256"] = digest(source)
    # A supplied development freeze is compared only at the same full input;
    # no historical miss count is forced for a different state.
    freeze = repo / ".local/city-v16-service-coverage/baseline-before/recovery-end.json"
    source_freeze = repo / ".local/city-v16-service-coverage/prepare-development/service-start.json"
    same_input = source_freeze.exists() and source.read_bytes() == source_freeze.read_bytes()

    def provenance(folder, save, strategy):
        doc = json.loads(save.read_text())
        j = json.loads((folder / "summary.json").read_text())
        meta = {"source_save": str(save), "input_sha256": digest(save), "map": doc["map"]["relative_path"],
                "rules": "sandbox-city-v16", "rule": 4, "policy": 2, "schema": 20,
                "start_tick": j["start"]["tick"], "end_tick": j["final"]["tick"],
                "production_commit": head, "dirty_workspace": bool(status), "strategy": strategy,
                "evidence": "CURRENT / OBSERVED / RECONSTRUCTED" if strategy == "current" else "EXPERIMENTAL / RECONSTRUCTED"}
        write(folder / "provenance.json", meta)
        for name in ("baseline.json", "endurance-summary.json"):
            value = json.loads((folder / name).read_text()); value["provenance"] = meta; write(folder / name, value)
        for name in ("service-visits.jsonl", "service-couriers.jsonl", "house-coverage.jsonl",
                     "demand-results.jsonl", "service-failures.jsonl", "finance-400t.jsonl"):
            path = folder / name
            rows = [dict(json.loads(line), provenance=meta) for line in path.read_text().splitlines()]
            path.write_text("".join(json.dumps(r, separators=(",", ":")) + "\n" for r in rows))
        j["provenance"] = meta
        return j

    comparison, restarts = [], []

    def audit(tag, save, strategy):
        folder = run(tag, ["--service-audit", save, strategy])
        j = provenance(folder, save, strategy)
        comparison.append({"scenario": tag, "input_sha256": digest(save), "metrics": j,
                           "reports": str(folder / "provenance.json")})
        checkpoints = json.loads((folder / "save-restart-comparison.json").read_text())
        for number, checkpoint in enumerate(checkpoints["same_process_checkpoints"]):
            child = run(f"restart-{tag}-{number}", ["--resume", checkpoint["save"], 100,
                        checkpoint["expected_plus100"], strategy])
            equal = json.loads((child / "result.json").read_text()) == json.loads(Path(checkpoint["expected_plus100"]).read_text())
            if not equal:
                raise RuntimeError("complete SaveDocument restart differs")
            restarts.append({"scenario": tag, "strategy": strategy, "checkpoint": checkpoint["tag"],
                             "input_sha256": digest(Path(checkpoint["save"])), "production_commit": head,
                             "provenance": j["provenance"],
                             "complete_savedocument_equal": equal, "separate_os_process": True})
        return folder

    current = audit("baseline-current", source, "current")
    # The old recovery report includes the separate 4000t controls phase.
    # Compare the identical (31200,41200] observation window, retaining every
    # byte of each original event and leaving the freeze untouched.
    start_tick = comparison[0]["metrics"]["start"]["tick"]
    frozen_window = b"".join(line for line in (freeze.parent / "demand-events.jsonl").read_bytes().splitlines(keepends=True)
                            if json.loads(line)["tick"] > start_tick) if freeze.exists() else None
    report["frozen_before_comparison"] = {"same_complete_input": same_input,
        "same_complete_end_save": same_input and freeze.exists() and (current / "service-end.json").read_bytes() == freeze.read_bytes(),
        "same_demand_jsonl_in_endurance_window": same_input and freeze.exists() and (current / "demand-events.jsonl").read_bytes() == frozen_window}
    if same_input and freeze.exists() and not all(report["frozen_before_comparison"].values()):
        raise RuntimeError("frozen current baseline differs")
    for strategy in ("expiry", "risk", "travel"):
        audit("baseline-" + strategy, source, strategy)
    challenges = run("transport-challenges", ["--service-challenges", "authored"])
    report["transport_challenges"] = str(challenges / "service-challenges.json")
    ui = run("scripted-ui18", ["--service-ui", source, repo / "resources"])
    ui_report = json.loads((ui / "ui-overlay-acceptance.json").read_text())
    ui_report["provenance"] = dict(comparison[0]["metrics"]["provenance"], end_tick=start_tick)
    write(ui / "ui-overlay-acceptance.json", ui_report)
    report["scripted_ui"] = str(ui / "ui-overlay-acceptance.json")

    if mode != "--ui":
        handan = run("fresh-Handan", ["--fresh", "Cities/Handan.map"])
        for map_name, fresh in (("Xia", xia), ("Handan", handan)):
            capacities = run("capacity-" + map_name, ["--service-capacity", fresh / "first-goal.json"])
            for count in (10, 14, 18, 20):
                save = capacities / f"houses-{count}/capacity-start.json"
                for strategy in ("current", "expiry", "risk", "travel"):
                    audit(f"capacity-{map_name}-{count}-{strategy}", save, strategy)
                if map_name == "Xia" and count == 20:
                    ui20 = run("scripted-ui20", ["--service-ui", save, repo / "resources"])
                    value = json.loads((ui20 / "ui-overlay-acceptance.json").read_text())
                    value["provenance"] = dict(comparison[-4]["metrics"]["provenance"], end_tick=value["tick"])
                    write(ui20 / "ui-overlay-acceptance.json", value)
        legacy = run("historical-rule3", [repo / "resources"], build / "openemperor-city-v16-playthrough-fixture")
        facts = json.loads((legacy / "fixture-report.json").read_text())
        if facts["first_goal"]["tick"] != 6800 or facts["final"]["tick"] != 9200:
            raise RuntimeError("historical Xia result changed")
        before = repo / ".local/city-v16-longplay/historical-before"
        old_saves = sorted(before.rglob("*.json")) if before.exists() else []
        comparisons = []
        for path in old_saves:
            value = json.loads(path.read_text())
            if value.get("format") != "openemperor-sandbox-save":
                continue
            fresh = legacy / path.relative_to(before)
            equal = fresh.exists() and fresh.read_bytes() == path.read_bytes()
            comparisons.append({"save": str(path.relative_to(before)), "sha256": digest(path), "byte_equal": equal})
        if comparisons and not all(r["byte_equal"] for r in comparisons):
            raise RuntimeError("historical full SaveDocument differs")
        report["historical_save_comparison"] = comparisons
        with (root / "corpus.log").open("w") as log:
            subprocess.run([str(build / "openemperor-audit-original-occupancy"), str(data), str(root / "corpus"), "--summary-only"],
                           cwd=repo, env=env, stdout=log, stderr=subprocess.STDOUT, check=True)
        census = json.loads((root / "corpus/corpus.json").read_text())
        if census["first_failure_distribution"] != {"prepared": 165, "known_conflict": 2}:
            raise RuntimeError("167-map census changed")
        frozen_census = repo / ".local/city-v16-longplay/baseline-corpus/corpus.json"
        report["corpus"] = {"distribution": census["first_failure_distribution"],
            "complete_frozen_census_equal": frozen_census.exists() and json.loads(frozen_census.read_text()) == census}
        if frozen_census.exists() and not report["corpus"]["complete_frozen_census_equal"]:
            raise RuntimeError("complete policy/legacy census changed")
    if mode in ("--benchmark", "--all"):
        benchmark = run("plain-world-benchmark", ["--resume", source, 10000, current / "service-end.json"])
        report["benchmark"] = {"uninstrumented": json.loads((benchmark / "performance.json").read_text()),
            "observed_tick_seconds": comparison[0]["metrics"]["world_tick_seconds_excluding_reports"],
            "render_measured_separately": str(ui / "ui-overlay-acceptance.json")}
    write(root / "scheduler-comparison.json", {"production_commit": head, "scenarios": comparison,
          "fairness_claim": "No never-visited House in these finite windows; not a universal starvation proof",
          "recommendation": "KEEP CURRENT rule4; LAYOUT/CAPACITY LIMITATION; consider a separately versioned targeting upgrade"})
    write(root / "save-restart-comparison.json", {"production_commit": head, "restarts": restarts})
    for name in ("baseline.json", "service-visits.jsonl", "service-couriers.jsonl", "house-coverage.jsonl",
                 "demand-results.jsonl", "service-failures.jsonl", "finance-400t.jsonl", "endurance-summary.json"):
        shutil.copyfile(current / name, root / name)
    shutil.copyfile(ui / "ui-overlay-acceptance.json", root / "ui-overlay-acceptance.json")
    player = root / "player-app"; (player / "saves").mkdir(parents=True)
    save = player / "saves/01-service-baseline.json"; save.write_bytes(source_bytes)
    write(player / "settings.json", {"version": 1, "data_root": str(data), "last_map": "Cities/Xia.map",
          "last_save": str(save), "profile": "sandbox-city-v16", "prepared_starter": False, "autosave_enabled": False})
    report["original_revision_unchanged"] = inventory(data) == original
    if source.read_bytes() != source_bytes or not report["original_revision_unchanged"]:
        raise RuntimeError("source preservation failed")
    report["technical_result"] = "PASS"
    write(root / "summary.json", report)
    s = comparison[0]["metrics"]
    print(f"CURRENT: {s['all_demand_successes']}/{s['actual_demand_deadlines']} supplied; "
          f"Service-only misses {s['service_only_misses']}; covered house ticks {s['covered_house_tick_ratio']:.2%}; "
          f"net Funds {s['net_funds']:+d}.")
    print(f"Technical PASS; complete reports: {root}. Human acceptance remains OPEN.")


if __name__ == "__main__":
    try:
        main()
    except Exception as error:
        print(f"Service coverage failed: {error}", file=sys.stderr)
        sys.exit(1)
