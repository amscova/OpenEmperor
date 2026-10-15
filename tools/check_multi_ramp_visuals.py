"""Strict composed-scene witnesses, independent of renderer mesh internals.

The closed scene and built scene must share map, roads, camera and renderer.
At native projected internal boundaries the old implementation leaves literal
closed-cliff pixels visible for the whole run. No palette threshold admits it.
"""
import json
from pathlib import Path
import struct
import sys
import zlib


def fixture_png(path):
    """Read the engine's own bounded RGBA/filter0 acceptance PNGs."""
    data = path.read_bytes()
    if data[:8] != b"\x89PNG\r\n\x1a\n":
        raise RuntimeError("invalid fixture PNG")
    position, packed = 8, bytearray()
    width = height = 0
    while position < len(data):
        length = struct.unpack_from(">I", data, position)[0]
        kind, body = data[position + 4:position + 8], data[position + 8:position + 8 + length]
        if kind == b"IHDR":
            width, height, depth, color, compression, filtering, interlace = struct.unpack(">IIBBBBB", body)
            if (depth, color, compression, filtering, interlace) != (8, 6, 0, 0, 0) or width > 4096 or height > 4096:
                raise RuntimeError("unsupported fixture PNG format/bounds")
        elif kind == b"IDAT":
            packed.extend(body)
        position += length + 12
    raw = zlib.decompress(packed)
    stride = width * 4 + 1
    if len(raw) != height * stride or any(raw[y * stride] != 0 for y in range(height)):
        raise RuntimeError("engine filter0 fixture PNG required")
    return width, height, b"".join(raw[y * stride + 1:(y + 1) * stride] for y in range(height))


def crop_pixels(image, rect):
    width, height, rgba = image
    x, y, w, h = rect
    if min(x, y, w, h) < 0 or x + w > width or y + h > height:
        raise RuntimeError("comparison crop outside fixture PNG")
    return b"".join(rgba[((y + row) * width + x) * 4:((y + row) * width + x + w) * 4] for row in range(h))


def compare(before, after):
    old = json.loads((before / "summary.json").read_text())
    new = json.loads((after / "summary.json").read_text())
    states = ("zero", "isolated", "group", "four", "walker")
    previous, current = [{s["name"]: s for s in r["states"]} for r in (old, new)]
    checks = []
    for name in states:
        if (before / (name + ".json")).read_bytes() != (after / (name + ".json")).read_bytes():
            raise RuntimeError("before/after complete SaveDocument differs: " + name)
        checks.append(name + ": complete SaveDocument exact")
        for left, right in zip(previous[name]["captures"], current[name]["captures"]):
            if left["camera"] != right["camera"] or left["joins"] != right["joins"]:
                raise RuntimeError("before/after camera or projected signed-height join differs")
            prefix = right["file"].removesuffix("-full.png")
            files = [prefix + "-f6-off.png"]
            if name == "zero":
                files.append(right["file"])
            if prefix.endswith("-z0"):
                files.append(prefix + "-f8-0.png")
            for file in files:
                a, b = fixture_png(before / file), fixture_png(after / file)
                rect = right["camera"]["map_rect"]
                if crop_pixels(a, rect) != crop_pixels(b, rect):
                    raise RuntimeError("unchanged F6/flat-Snapshot/no-Ramp control changed: " + file)
                checks.append(file + ": map pixels exact")
            # The existing unchanged Bridge is visible at1x/2x. Bound its deck,
            # rails and banks relative to the exact lower road join projection.
            if right["camera"]["zoom"] in (1., 2.):
                lower = right["joins"][1]
                cell_x, cell_y = lower["cell"]
                scale = right["camera"]["zoom"]
                sx = lower["screen"][0] + ((119 - cell_x) - (114 - cell_y)) * 40 * scale
                sy = lower["screen"][1] + ((119 - cell_x) + (114 - cell_y)) * 20 * scale
                rect = [int(sx - 72 * scale), int(sy - 36 * scale), int(144 * scale), int(72 * scale)]
                map_x, map_y, map_w, map_h = right["camera"]["map_rect"]
                if rect[0] >= map_x and rect[1] >= map_y and rect[0] + rect[2] <= map_x + map_w and rect[1] + rect[3] <= map_y + map_h:
                    if crop_pixels(fixture_png(before / right["file"]), rect) != crop_pixels(fixture_png(after / right["file"]), rect):
                        raise RuntimeError("unchanged Bridge/banks pixel crop differs: " + right["file"])
                    checks.append(right["file"] + ": Bridge/banks pixels exact")
    (after / "before-after-regression.json").write_text(json.dumps({
        "result": "PASS", "before": str(before), "checks": checks,
        "evidence": "identical paid saved states, cameras, signed join positions and unchanged pixel controls",
    }, indent=2) + "\n")
    print(f"Before/after complete saves and unchanged scene controls: {len(checks)} exact comparisons PASS")


def check_atlas(root, atlas):
    cases = atlas["cases"]
    expected = {(axis, delta, base, count, zoom)
                for axis in ("X", "Y") for delta in (-1, 1) for base in (-2, 0, 2)
                for count in (1, 2, 3, 4) for zoom in (1., 2., 4.)}
    indexed = {(c["axis"], c["delta"], c["base_height"], c["ramps"], c["zoom"]): c for c in cases}
    if indexed.keys() != expected or len(cases) != len(expected):
        raise RuntimeError("complete 144-case axis/gradient/signed-height/group/zoom atlas required")
    repetition = []
    witnesses = 0
    for key, case in indexed.items():
        if not (root / case["file"]).is_file():
            raise RuntimeError("missing material atlas PNG: " + case["file"])
        for sample in case["two_background_witnesses"]:
            if sample["opacity_required"]:
                witnesses += 1
                if not sample["opaque"]:
                    raise RuntimeError("material atlas has a background-dependent tread/half seam")
        if case["delta"] < 0 and case["ramps"] == 4:
            # Both middle ramps have the same mask, material and geometry.
            # The entire bounded interior sequence must not repeat unchanged
            # after a one-cell lateral translation. Finite texel reuse remains
            # valid; exact repetition of the whole visible strip is rejected.
            sequences = [[s["rgb"] for s in case["two_background_witnesses"]
                          if s["ramp"] == i and .2 < s["t"] < .8] for i in (1, 2)]
            repetition.append({"axis": key[0], "base_height": key[2], "zoom": key[4],
                               "samples_per_strip": len(sequences[0]),
                               "identical_middle_strips": sequences[0] == sequences[1]})
    result = {"result": "FAIL" if any(r["identical_middle_strips"] for r in repetition) else "PASS",
              "evidence": atlas["evidence"], "cases": len(cases), "opaque_required_witnesses": witnesses,
              "repeated_middle_strips": repetition,
              "limits": "Ascending slopes are edge-on; opacity checks use their authoritative center footline"}
    (root / "material-atlas-regression.json").write_text(json.dumps(result, indent=2) + "\n")
    if result["result"] != "PASS":
        raise RuntimeError("whole interior material strips repeat on adjacent ramps; see material-atlas-regression.json")
    print("144-case production material atlas, two-background seams and whole-strip repetition: PASS")


def check(root):
    report = json.loads((root / "summary.json").read_text())
    states = {s["name"]: s for s in report["states"]}
    if not {"zero", "isolated", "group", "four", "walker"} <= states.keys():
        raise RuntimeError("complete zero/isolated/group/four/actual-walker composed matrix required")
    walker = states["walker"]
    if walker["tick"] <= 0 or not walker["walker"] or walker["walker"]["edge_progress"] != 2:
        raise RuntimeError("actual production courier mid-edge witness required")
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
    if "material_atlas" in report:
        check_atlas(root, report["material_atlas"])


if __name__ == "__main__":
    if len(sys.argv) not in (2, 3):
        raise SystemExit("Usage: python3 tools/check_multi_ramp_visuals.py CAPTURE_ROOT [BEFORE_ROOT]")
    target = Path(sys.argv[1]).resolve(strict=True)
    check(target)
    if len(sys.argv) == 3:
        compare(Path(sys.argv[2]).resolve(strict=True), target)
