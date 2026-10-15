"""Same authored map, paid four-ramp production captures and separate restarts."""
from pathlib import Path
import json
import os
import subprocess
import sys
import tempfile
from author_ramp_terrain import author
from check_multi_ramp_visuals import check
from run_city_v16_longplay import inventory
from run_city_v16_bridges import source_digest


def main():
    repo, data, build = [Path(p).resolve(strict=True) for p in sys.argv[1:4]]
    mode = sys.argv[4]
    if mode not in ("--ui", "--capture", "--regression"):
        raise RuntimeError("unknown multi-ramp mode")
    pinned = {"files": 1464, "bytes": 819711091,
              "manifest_sha256": "dde2382fe1c348da765c8776149a73210b81f6cba7f93a0c6159d79ce3154feb"}
    if inventory(data) != pinned:
        raise RuntimeError("original data revision differs")
    digest = source_digest(repo)
    private = repo / ".local/city-v16-ramps"
    private.mkdir(parents=True, exist_ok=True)
    root = Path(tempfile.mkdtemp(prefix="multi-ramp-", dir=private))
    env = {k: v for k, v in os.environ.items() if not k.startswith(("SDL_", "DYLD_", "OPENEMPEROR_"))}

    def run(tag, binary, args):
        with (root / f"{tag}.log").open("w") as log:
            subprocess.run([str(build / binary), *map(str, args)], cwd=repo, env=env,
                           stdout=log, stderr=subprocess.STDOUT, check=True)
        print(tag + ": PASS", flush=True)

    for binary in ("openemperor-authored-terrain-side-tests", "openemperor-ramp-rendering-tests",
                   "openemperor-ramp-material-rendering-tests"):
        run(binary, binary, [])
    author(data, root / "authored-data")
    authored = root / "authored-data"
    run("paid-gameplay", "openemperor-city-v16-ramps-fixture", [authored, root / "gameplay"])
    args = [authored, root / "gameplay/before-ramp.json", root / "captures"]
    if mode == "--ui":
        args.append("--quick")
    run("four-ramp-production-scene", "openemperor-multi-ramp-visual-fixture", args)
    check(root / "captures")
    comparisons = 0
    if mode != "--ui":
        for state in ("zero", "isolated", "group", "four"):
            run("new-process-" + state, "openemperor-multi-ramp-visual-fixture",
                [authored, root / f"captures/{state}.json", root / f"restart-{state}", "--resume"])
            for suffix in (".json", "-stable.png"):
                name = state + suffix
                if (root / f"restart-{state}" / name).read_bytes() != (root / "captures" / name).read_bytes():
                    raise RuntimeError("fresh-process SaveDocument/map-pixel mismatch: " + name)
                comparisons += 1
    if inventory(data) != pinned or source_digest(repo) != digest:
        raise RuntimeError("read-only original inputs or implementation changed during acceptance")
    (root / "acceptance.json").write_text(json.dumps({
        "result": "PASS", "source_sha256": digest, "original_revision": pinned,
        "mode": mode, "rule": 6, "policy": 3, "schema": 22,
        "ramp_edges": [[[115, y], [116, y]] for y in (114, 117, 118, 119)],
        "new_process_exact_comparisons": comparisons,
        "evidence": "paid commands; actual SandboxView software composition; human visual acceptance separate",
    }, indent=2) + "\n")
    print("Paid four-ramp save and visual matrix:", root, flush=True)
    if mode == "--regression":
        subprocess.run(["sh", "tools/test_city_v16_ramps.sh", str(data), "--all"], cwd=repo, check=True)
    if mode == "--ui":
        print("Opening isolated and three adjacent paid ramps; Continue starts transport.", flush=True)
        os.execv(str(build / "openemperor"), [str(build / "openemperor"), "--data", str(authored),
                 "--load-sandbox", str(root / "captures/four.json"), "--authored-terrain-sides"])


if __name__ == "__main__":
    main()
