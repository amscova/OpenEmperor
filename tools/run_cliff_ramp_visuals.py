"""Same authored source, paid roads/two ramps, isolated normal View captures."""
from pathlib import Path
import json
import os
import subprocess
import sys
import tempfile
from author_ramp_terrain import author
from run_city_v16_longplay import inventory
from run_city_v16_bridges import source_digest


def main():
    repo, data, build = [Path(p).resolve(strict=True) for p in sys.argv[1:4]]
    mode = sys.argv[4]
    if mode not in ("--ui", "--capture", "--regression"):
        raise RuntimeError("unknown visual mode")
    pinned = {"files": 1464, "bytes": 819711091,
              "manifest_sha256": "dde2382fe1c348da765c8776149a73210b81f6cba7f93a0c6159d79ce3154feb"}
    if inventory(data) != pinned:
        raise RuntimeError("original data revision differs")
    source = source_digest(repo)
    private = repo / ".local/city-v16-ramps"
    private.mkdir(parents=True, exist_ok=True)
    root = Path(tempfile.mkdtemp(prefix="cliff-", dir=private))
    env = {k: v for k, v in os.environ.items() if not k.startswith(("SDL_", "DYLD_", "OPENEMPEROR_"))}

    def run(tag, binary, arguments):
        with (root / (tag + ".log")).open("w") as log:
            subprocess.run([str(build / binary), *map(str, arguments)], cwd=repo,
                           env=env, stdout=log, stderr=subprocess.STDOUT, check=True)
        print(tag + ": PASS", flush=True)

    for binary in ("openemperor-authored-terrain-side-tests", "openemperor-ramp-rendering-tests"):
        run(binary, binary, [])
    author(data, root / "authored-data")
    authored = root / "authored-data"
    run("paid-gameplay", "openemperor-city-v16-ramps-fixture", [authored, root / "gameplay"])
    arguments = [authored, root / "gameplay/before-ramp.json", root / "captures"]
    if mode == "--ui":
        arguments.append("--quick")
    run("same-scene", "openemperor-cliff-ramp-visual-fixture", arguments)
    if mode != "--ui":
        run("original-controls", "openemperor-cliff-ramp-visual-fixture", [data, root / "originals", "--originals"])
        compared = 0
        for state in range(4):
            run("new-process-" + str(state), "openemperor-cliff-ramp-visual-fixture",
                [authored, root / f"captures/state-{state}.json", root / f"restart-{state}", "--resume"])
            for suffix in (".json", "-stable.png"):
                name = f"state-{state}" + suffix
                if (root / f"restart-{state}" / name).read_bytes() != (root / "captures" / name).read_bytes():
                    raise RuntimeError("new-process visual/SaveDocument mismatch: " + name)
                compared += 1
        if compared != 8:
            raise RuntimeError("incomplete new-process comparison")
    if inventory(data) != pinned or source_digest(repo) != source:
        raise RuntimeError("original inputs or implementation changed during acceptance")
    (root / "acceptance.json").write_text(json.dumps({
        "result": "PASS", "source_sha256": source, "original_revision": pinned,
        "mode": mode, "rule": 6, "policy": 3, "schema": 22,
        "ramp_edges": [[[115, 114], [116, 114]], [[115, 117], [116, 117]]],
        "evidence": "scripted paid commands/software rendering; human acceptance remains separate",
        "new_process_exact_comparisons": 0 if mode == "--ui" else 8,
    }, indent=2) + "\n")
    print("Visual evidence and paid two-ramp save:", root, flush=True)
    if mode == "--regression":
        subprocess.run(["sh", "tools/test_city_v16_ramps.sh", str(data), "--all"], cwd=repo, check=True)
    if mode == "--ui":
        print("Opening two paid ramps and the existing bridge. Continue starts actual Clay transport; F5/F9 saves/loads.", flush=True)
        os.execv(str(build / "openemperor"), [str(build / "openemperor"), "--data", str(authored),
                 "--load-sandbox", str(root / "captures/state-2.json"), "--authored-terrain-sides"])


if __name__ == "__main__":
    main()
