"""Pinned-data audit, authored paid gameplay, real-process restart and old controls."""
from pathlib import Path
import json
import os
import shutil
import subprocess
import sys
from run_city_v16_longplay import inventory
from run_city_v16_bridges import source_digest
from author_ramp_terrain import author


def main():
    repo, data, build, root = [Path(x).resolve(strict=True) for x in sys.argv[1:5]]
    mode = sys.argv[5]
    if mode not in ("--audit", "--gameplay", "--ui", "--all") or any(root.iterdir()):
        raise RuntimeError("supported mode and fresh isolated root required")
    if not root.is_relative_to(repo / ".local/city-v16-ramps"):
        raise RuntimeError("isolated Ramp output required")
    pinned = {"files": 1464, "bytes": 819711091,
              "manifest_sha256": "dde2382fe1c348da765c8776149a73210b81f6cba7f93a0c6159d79ce3154feb"}
    if inventory(data) != pinned:
        raise RuntimeError("original data revision differs")
    test_env = {k: v for k, v in os.environ.items() if not k.startswith(("SDL_", "DYLD_", "OPENEMPEROR_"))}
    source = source_digest(repo)
    report = {"head": subprocess.check_output(["git", "rev-parse", "HEAD"], text=True).strip(),
              "source_sha256": source, "original_revision": pinned, "rule": 6, "policy": 3, "schema": 22,
              "evidence": "authored terrain and scripted paid gameplay; separate original audit and process restart",
              "original_map_ramp_acceptance": "unavailable: zero eligible nonzero-height pairs",
              "native_human_acceptance": "manual check; not inferred", "runs": {}}

    def write():
        (root / "summary.json").write_text(json.dumps(report, indent=2) + "\n")

    def run(tag, program, args):
        with (root / (tag + ".log")).open("w") as log:
            result = subprocess.run([str(build / program), *map(str, args)], cwd=repo,
                                    stdout=log, stderr=subprocess.STDOUT, env=test_env)
        report["runs"][tag] = {"exit": result.returncode}
        write()
        print(tag + (": PASS" if result.returncode == 0 else ": FAIL"), flush=True)
        if result.returncode:
            raise RuntimeError("dependent work stopped: " + tag)

    run("transport", "openemperor-ramp-transport-tests", [])
    run("pixels", "openemperor-ramp-rendering-tests", [])
    run("terrain-side-pixels", "openemperor-authored-terrain-side-tests", [])
    run("scripted-ui", "openemperor-sandbox-view-tests", ["--ramps-only"])
    run("height-audit", "openemperor-original-height-audit", [data, root / "height-audit"])
    heights = json.loads((root / "height-audit/height-corpus.json").read_text())
    if heights["complete"] != 167 or heights["prepared"] != 165 or heights["totals"].get("legal_land_delta1", 0):
        raise RuntimeError("pinned height audit differs; review candidates before gameplay")
    run("corpus", "openemperor-audit-original-occupancy", [data, root / "corpus", "--summary-only"])
    current = json.loads((root / "corpus/corpus.json").read_text())
    if current["first_failure_distribution"] != {"prepared": 165, "known_conflict": 2}:
        raise RuntimeError("full original preparation changed")
    baseline = repo / ".local/city-v16-bridges/run-7THskW"
    if baseline.exists():
        before = json.loads((baseline / "before-corpus/corpus.json").read_text())
        for a, b in zip(before["maps"], current["maps"], strict=True):
            if a["relative_path"] != b["relative_path"] or any(a.get(p) != b.get(p) for p in ("policies", "policy2", "policy3")):
                raise RuntimeError("frozen original policy bytes changed: " + a["relative_path"])
        report["baseline_permission_comparisons"] = "1485 policy1/2/3 actual/zero/one summaries unchanged"
    if mode != "--audit":
        author(data, root / "authored-data")
        authored = root / "authored-data"
        run("gameplay", "openemperor-city-v16-ramps-fixture", [authored, root / "gameplay"])
        case = root / "gameplay"
        names = ("before-ramp", "after-ramp", "before-entry", "mid-ramp", "after-exit", "after-delivery",
                 "return-ramp", "ten-thousand", "end", "disconnect-wait", "after-repair")
        for name in names:
            target = root / ("restart-" + name)
            run("restart-" + name, "openemperor-city-v16-ramps-fixture", [authored, target, "--resume", case / (name + ".json"), 100])
            if (target / "restarted-immediate.json").read_bytes() != (case / (name + ".json")).read_bytes() or \
               (target / "restarted-end.json").read_bytes() != (case / (name + "-plus100-control.json")).read_bytes():
                raise RuntimeError("full SaveDocument process equality failed: " + name)
        player = root / "player-app/saves"
        player.mkdir(parents=True)
        for name, output in (("before-ramp", "01-build-ramp"), ("mid-ramp", "02-mid-ramp"), ("end", "03-playing-city")):
            shutil.copyfile(case / (name + ".json"), player / (output + ".json"))
        report["gameplay"] = json.loads((case / "summary.json").read_text())
        report["new_process_checkpoint_pairs"] = len(names)
        print("Authored test: (115,114) height1 -> (116,114) height0; bridge (118,114) -> (120,114)", flush=True)
    if mode == "--all":
        control = repo / ".local/city-v16-bridges" / ("ramp-controls-" + root.name)
        control.mkdir()
        report["old_contract_root"] = str(control)
        with (root / "old-contracts.log").open("w") as log:
            subprocess.run([sys.executable, str(repo / "tools/run_city_v16_bridges.py"), str(repo), str(data),
                            str(build), str(control), "--all"], cwd=repo, stdout=log, stderr=subprocess.STDOUT, env=test_env, check=True)
        if baseline.exists():
            comparisons = 0
            for folder in ("historical-rule3", "current-rule4-Xia", "current-rule4-Handan", "gameplay"):
                for file in (baseline / folder).rglob("*.json"):
                    if folder == "gameplay" and ("control" not in file.name and file.stem not in
                            ("before-bridge", "after-bridge-before-dispatch", "outbound-on-bridge", "return-on-bridge", "after-actual-delivery", "ten-thousand", "end", "waiting-bank-disconnected", "after-bank-repair", "additional-roles-end")):
                        continue
                    try:
                        value = json.loads(file.read_text())
                    except (ValueError, OSError):
                        continue
                    if not isinstance(value, dict) or value.get("format") != "openemperor-sandbox-save":
                        continue
                    other = control / folder / file.relative_to(baseline / folder)
                    if file.read_bytes() != other.read_bytes():
                        raise RuntimeError("old complete SaveDocument changed: " + str(file.relative_to(baseline)))
                    comparisons += 1
            report["frozen_old_save_byte_comparisons"] = comparisons
    if inventory(data) != pinned or source_digest(repo) != source:
        raise RuntimeError("original inputs or implementation changed during acceptance")
    report.update(result="PASS", original_inputs_unchanged=True, source_unchanged=True)
    write()
    print("Ramps PASS, reports and private playable saves:", root, flush=True)


if __name__ == "__main__":
    main()
