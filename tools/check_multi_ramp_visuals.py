"""Strict composed-scene witnesses, independent of renderer mesh internals.

The closed scene and built scene must share map, roads, camera and renderer.
At native projected internal boundaries the old implementation leaves literal
closed-cliff pixels visible for the whole run. No palette threshold admits it.
"""
import json
from pathlib import Path
import sys


def check(root):
    report = json.loads((root / "summary.json").read_text())
    states = {s["name"]: s for s in report["states"]}
    if not {"zero", "isolated", "group", "four"} <= states.keys():
        raise RuntimeError("complete zero/isolated/group/four composed matrix required")
    for state in states.values():
        captures = state["captures"]
        zooms = [c["camera"]["zoom"] for c in captures]
        if len(captures) != 4 or zooms[:3] != [1., 2., 4.] or not 1. < zooms[3] < 2.:
            raise RuntimeError("complete 1x/2x/4x/fractional camera matrix required")
    results, flanks = [], []
    for name in ("group", "four"):
        for closed, actual in zip(states["zero"]["captures"], states[name]["captures"]):
            if closed["camera"] != actual["camera"]:
                raise RuntimeError("composed camera mismatch")
            reference = {(l["endpoint_y"], l["lateral_offset"]): l for l in closed["projected_lines"]}
            for line in actual["projected_lines"]:
                if line["background_samples"]:
                    raise RuntimeError("transparent/background witness in actual composed slope")
                if (line["endpoint_y"], line["lateral_offset"]) in ((117, -.85), (119, .85)):
                    closed_flank = reference[(line["endpoint_y"], line["lateral_offset"])]
                    changed = [i for i in range(2, 22) if line["rgb"][i] != closed_flank["rgb"][i]]
                    flanks.append({"state": name, "zoom": actual["camera"]["zoom"],
                                   "endpoint_y": line["endpoint_y"], "side": line["lateral_offset"],
                                   "changed_closed_flank_samples": changed})
                    if changed:
                        raise RuntimeError("ramp group modified unrelated exterior cliff witnesses")
                if line["endpoint_y"] not in (117, 118) or line["lateral_offset"] != .5:
                    continue
                rock = reference[(line["endpoint_y"], .5)]
                # The central 20 witnesses stay away from both endpoint roads.
                equal = [i for i in range(2, 22) if line["rgb"][i] == rock["rgb"][i]]
                results.append({"state": name, "zoom": actual["camera"]["zoom"],
                                "boundary_after_y": line["endpoint_y"],
                                "unchanged_closed_cliff_samples": equal})
                if equal:
                    raise RuntimeError(f"retained closed cliff at actual internal ramp boundary: {name}, "
                                       f"zoom {actual['camera']['zoom']}, y={line['endpoint_y']}+.5, "
                                       f"samples {equal}")
    (root / "composed-regression.json").write_text(json.dumps({
        "result": "PASS", "evidence": "actual production composite; zero-ramp closed-rock witnesses",
        "boundaries": results, "exterior_flanks": flanks,
        "limits": "Hidden mesh absence is checked by renderer topology tests; material quality requires image review",
    }, indent=2) + "\n")
    print("Composed internal-boundary rock and background witnesses: PASS")


if __name__ == "__main__":
    if len(sys.argv) != 2:
        raise SystemExit("Usage: python3 tools/check_multi_ramp_visuals.py CAPTURE_ROOT")
    check(Path(sys.argv[1]).resolve(strict=True))
