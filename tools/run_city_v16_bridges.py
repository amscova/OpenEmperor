"""Explicit read-only original-data audit, paid bridge replay and process restart."""
from pathlib import Path
import hashlib
import json
import os
import shutil
import subprocess
import sys
from run_city_v16_longplay import inventory


def write(path, value):
    path.write_text(json.dumps(value, indent=2) + "\n")


def source_digest(repo):
    digest = hashlib.sha256()
    files = [repo / "CMakeLists.txt"]
    for folder in ("src", "tests", "tools", "resources"):
        files += [p for p in (repo / folder).rglob("*") if p.is_file() and not p.is_symlink() and "__pycache__" not in p.parts]
    for path in sorted(files):
        digest.update(path.relative_to(repo).as_posix().encode() + b"\0")
        digest.update(hashlib.sha256(path.read_bytes()).digest())
    return digest.hexdigest()


def main():
    repo, data, build, root = [Path(x).resolve(strict=True) for x in sys.argv[1:5]]
    mode = sys.argv[5]
    if mode not in ("--audit", "--gameplay", "--ui", "--ui-build", "--all"):
        raise RuntimeError("unknown mode")
    if not root.is_relative_to(repo / ".local/city-v16-bridges") or any(root.iterdir()):
        raise RuntimeError("fresh isolated bridge output root required")
    if root.is_relative_to(data) or data.is_relative_to(root):
        raise RuntimeError("outputs must be outside read-only original inputs")
    initial = inventory(data)
    pinned = {"files": 1464, "bytes": 819711091,
              "manifest_sha256": "dde2382fe1c348da765c8776149a73210b81f6cba7f93a0c6159d79ce3154feb"}
    if initial != pinned:
        raise RuntimeError("original revision differs; no bridge gameplay run")
    source = source_digest(repo)
    env = {k: v for k, v in os.environ.items() if not k.startswith(("SDL_", "DYLD_", "OPENEMPEROR_"))}
    report = {"head": subprocess.check_output(["git", "rev-parse", "HEAD"], cwd=repo, text=True).strip(),
              "source_sha256": source, "original_revision": initial, "runs": {},
              "evidence": "READ-ONLY RAW FACTS / SCRIPTED paid gameplay / separate OS processes",
              "native_human_acceptance": "manual check; not inferred", "versions": {"rule": 5, "policy": 3, "schema": 21}}

    def run(tag, program, arguments):
        with (root / (tag + ".log")).open("w") as log:
            result = subprocess.run([str(build / program), *map(str, arguments)], cwd=repo, env=env,
                                    stdout=log, stderr=subprocess.STDOUT)
        report["runs"][tag] = {"exit": result.returncode, "log": str(root / (tag + ".log"))}
        write(root / "summary.json", report)
        print(f"{tag}: {'PASS' if result.returncode == 0 else 'FAIL'}", flush=True)
        if result.returncode:
            raise RuntimeError(f"{tag} failed; dependent work stopped")

    for binary in ("openemperor-bridge-transport-tests", "openemperor-bridge-rendering-tests"):
        run(binary, binary, [])
    run("scripted-bridge-ui", "openemperor-sandbox-view-tests", ["--bridges-only"])
    run("before-corpus", "openemperor-audit-original-occupancy", [data, root / "before-corpus", "--summary-only"])
    run("water-audit", "openemperor-original-bridge-audit", [data, root / "water-audit"])
    before = json.loads((root / "before-corpus/corpus.json").read_text())
    if before["first_failure_distribution"] != {"known_conflict": 2, "prepared": 165} or len(before["maps"]) != 167:
        raise RuntimeError("complete current corpus differs from 167/165/2 baseline")
    if mode != "--audit":
        run("gameplay", "openemperor-city-v16-bridges-fixture", [data, root / "gameplay", root / "water-audit/water-corpus.json"])
        case = root / "gameplay"
        checkpoints = ("before-bridge", "after-bridge-before-dispatch", "outbound-on-bridge", "return-on-bridge",
                       "after-actual-delivery", "ten-thousand", "end", "waiting-bank-disconnected", "after-bank-repair", "additional-roles-end")
        report["restarts"] = []
        for name in checkpoints:
            target = root / ("restart-" + name)
            run("restart-" + name, "openemperor-city-v16-bridges-fixture", [data, target, "--resume", case / (name + ".json"), 100])
            if (target / "restarted-immediate.json").read_bytes() != (case / (name + ".json")).read_bytes() or \
               (target / "restarted-end.json").read_bytes() != (case / (name + "-plus100-control.json")).read_bytes():
                raise RuntimeError("complete SaveDocument byte equality failed: " + name)
            report["restarts"].append({"checkpoint": name, "new_process": True, "immediate_and100t_equal": True})
        report["gameplay"] = json.loads((case / "summary.json").read_text())
        app = root / "player-app"
        (app / "saves").mkdir(parents=True)
        for name, filename in (("01-build-bridge.json", "before-bridge.json"), ("02-outbound.json", "outbound-on-bridge.json"),
                               ("03-return.json", "return-on-bridge.json"), ("04-playing-city.json", "end.json")):
            shutil.copyfile(case / filename, app / "saves" / name)
        write(app / "settings.json", {"version": 1, "data_root": str(data), "last_map": report["gameplay"]["map"],
                                      "last_save": str(app / "saves/01-build-bridge.json"), "profile": "sandbox-city-v16",
                                      "prepared_starter": False, "autosave_enabled": False})
        print("Actual selected original span:", report["gameplay"]["map"], report["gameplay"]["bank_a"], "->", report["gameplay"]["bank_b"], flush=True)
    if mode == "--all":
        (root / "historical-rule3").mkdir()
        run("historical-rule3", "openemperor-city-v16-playthrough-fixture", [data, root / "historical-rule3"])
        for map_name in ("Xia", "Handan"):
            (root / ("current-rule4-" + map_name)).mkdir()
            run("current-rule4-" + map_name, "openemperor-city-v16-longplay-fixture", [data, root / ("current-rule4-" + map_name), "--fresh", "Cities/" + map_name + ".map"])
    run("after-corpus", "openemperor-audit-original-occupancy", [data, root / "after-corpus", "--summary-only"])
    after = json.loads((root / "after-corpus/corpus.json").read_text())
    if before["first_failure_distribution"] != after["first_failure_distribution"] or len(after["maps"]) != 167:
        raise RuntimeError("after corpus changed its complete preparation decisions")
    for a, b in zip(before["maps"], after["maps"]):
        if a["relative_path"] != b["relative_path"]:
            raise RuntimeError("corpus order changed")
        for policy in ("policies", "policy2", "policy3"):
            if a.get(policy) != b.get(policy):
                raise RuntimeError("full permissions changed during gameplay: " + a["relative_path"])
    if inventory(data) != initial or source_digest(repo) != source:
        raise RuntimeError("original bytes or source changed during acceptance")
    report.update(result="PASS", original_inputs_unchanged=True, source_unchanged=True,
                  complete_corpus="167 complete / 165 prepared / 2 unchanged conflicts",
                  permission_comparisons="policy1/2/3 actual/zero/one, canonical and fingerprints")
    write(root / "summary.json", report)
    print("Bridge V1 PASS; reports and isolated saves:", root, flush=True)


if __name__ == "__main__":
    main()
