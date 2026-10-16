#!/usr/bin/env python3
"""Read-only source checks and independent pixels for the isolated prototype.

The machine report deliberately does not grant visual or original-game parity.
Screenshots must be opened separately, including native-size scenes.
"""
from __future__ import annotations

import argparse
from functools import lru_cache
import hashlib
import json
import math
import os
from pathlib import Path
import struct
import subprocess
import tempfile
import zlib


PINNED_DATA = {
    "files": 1464,
    "bytes": 819711091,
    "manifest_sha256": "dde2382fe1c348da765c8776149a73210b81f6cba7f93a0c6159d79ce3154feb",
}


def write_json(path: Path, value: object) -> None:
    path.write_text(json.dumps(value, indent=2) + "\n")


def inventory(data: Path) -> dict:
    """Fingerprint every original file; the supplied tree is never modified."""
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


def png(path: Path) -> tuple[int, int, bytes]:
    """Decode bounded noninterlaced RGB(A) PNG without a third-party package."""
    data = path.read_bytes()
    if data[:8] != b"\x89PNG\r\n\x1a\n":
        raise RuntimeError(f"invalid PNG: {path}")
    position, packed = 8, bytearray()
    width = height = channels = 0
    while position < len(data):
        if position + 12 > len(data):
            raise RuntimeError(f"truncated PNG: {path}")
        length = struct.unpack_from(">I", data, position)[0]
        if length > len(data) - position - 12:
            raise RuntimeError(f"truncated PNG chunk: {path}")
        kind, body = data[position + 4:position + 8], data[position + 8:position + 8 + length]
        crc = struct.unpack_from(">I", data, position + 8 + length)[0]
        if zlib.crc32(kind + body) & 0xffffffff != crc:
            raise RuntimeError(f"PNG checksum mismatch: {path}")
        if kind == b"IHDR":
            width, height, depth, color, compression, filtering, interlace = struct.unpack(">IIBBBBB", body)
            if (depth, color, compression, filtering, interlace) not in (
                    (8, 6, 0, 0, 0), (8, 2, 0, 0, 0)) or not 0 < width <= 8192 or not 0 < height <= 8192:
                raise RuntimeError(f"unsupported PNG format or dimensions: {path}")
            channels = 4 if color == 6 else 3
        elif kind == b"IDAT":
            packed.extend(body)
        elif kind == b"IEND":
            break
        position += length + 12
    if not width or not height or not channels:
        raise RuntimeError(f"PNG header missing: {path}")
    stride = width * channels
    raw = zlib.decompress(packed)
    if len(raw) != height * (stride + 1):
        raise RuntimeError(f"invalid PNG decompressed size: {path}")
    output, previous = bytearray(), bytearray(stride)
    for y in range(height):
        offset = y * (stride + 1)
        filter_type, row = raw[offset], bytearray(raw[offset + 1:offset + stride + 1])
        if filter_type > 4:
            raise RuntimeError(f"invalid PNG filter: {path}")
        if filter_type:
            for x in range(stride):
                left = row[x - channels] if x >= channels else 0
                above = previous[x]
                upper_left = previous[x - channels] if x >= channels else 0
                prediction = 0
                if filter_type == 1:
                    prediction = left
                elif filter_type == 2:
                    prediction = above
                elif filter_type == 3:
                    prediction = (left + above) // 2
                elif filter_type == 4:
                    p = left + above - upper_left
                    a, b, c = abs(p - left), abs(p - above), abs(p - upper_left)
                    prediction = left if a <= b and a <= c else above if b <= c else upper_left
                row[x] = (row[x] + prediction) & 255
        if channels == 4:
            output.extend(row)
        else:
            for x in range(0, stride, 3):
                output.extend(row[x:x + 3])
                output.append(255)
        previous = row
    return width, height, bytes(output)


def pixel(image: tuple[int, int, bytes], x: int, y: int) -> tuple[int, ...]:
    width, height, rgba = image
    if not 0 <= x < width or not 0 <= y < height:
        raise RuntimeError(f"pixel witness outside image: ({x}, {y}) vs {width}x{height}")
    return tuple(rgba[(y * width + x) * 4:(y * width + x + 1) * 4])


def capture_file(root: Path, value: str) -> Path:
    candidate = (root / value).resolve(strict=True)
    if not candidate.is_relative_to(root):
        raise RuntimeError("capture report references a path outside its own directory")
    return candidate


def finite_vector(value: list[float], size: int, label: str) -> None:
    if len(value) != size or not all(isinstance(n, (int, float)) and math.isfinite(n) for n in value):
        raise RuntimeError(f"invalid {label}")


def check(root: Path) -> dict:
    """Validate actual pixels, rather than trusting engine-reported booleans."""
    report = json.loads((root / "summary.json").read_text())
    write_json(root / "pixel-validation.json", {"result": "INCOMPLETE", "visual_review_performed": False})
    if report.get("format") != "openemperor-original-ramp-prototype-v1":
        raise RuntimeError("unknown original-ramp prototype report format")
    expected = {(axis, delta, base, count, style, zoom)
                for axis in ("X", "Y") for delta in (-1, 1) for base in (-2, 0, 2)
                for count in range(1, 5) for style in ("Earth", "Stairs", "Cobbled")
                for zoom in (1, 2, 4)}
    cases = report["cases"]
    indexed = {(case["axis"], case["delta"], case["base_height"], case["count"],
                case["style"], case["zoom"]): case for case in cases}
    if len(cases) != len(expected) or indexed.keys() != expected:
        raise RuntimeError("complete 432-case source-cell axis/sign/height/group/style/zoom matrix required")
    @lru_cache(maxsize=6)
    def image(name: str) -> tuple[int, int, bytes]:
        return png(capture_file(root, name))

    case_files = set()
    case_by_file = {}
    for case in cases:
        if case.get("height_step", 40) != 40:
            raise RuntimeError("the complete matrix must use the evidenced original 40-pixel height step")
        image(case["file"])
        if case["file"] in case_files:
            raise RuntimeError("each matrix case must have its own actual scene capture")
        case_files.add(case["file"])
        case_by_file[case["file"]] = case

    # These PNGs are exported independently by openemperor-inspect, directly
    # from the pinned source archive. No prototype-exported reference is used.
    native = {i: png(root / f"reference-native/{i}.png") for i in range(233, 245)}
    known_heights = (66, 85, 84, 68, 71, 88, 90, 71, 68, 89, 89, 69)
    for physical_id, source in native.items():
        if source[:2] != (78, known_heights[physical_id - 233]):
            raise RuntimeError(f"unexpected native sprite dimensions: {physical_id}")
        if not set(source[2][3::4]) <= {0, 255}:
            raise RuntimeError(f"unexpected nonbinary native alpha: {physical_id}")
        # Independently check the recovered full-source-cell center footline.
        # Exact low boundaries of 234/235 and their road variants intentionally
        # require the neighboring ground, since the art is 78px in an 80px grid.
        vx, vy = ((-40, 20), (-40, -20), (40, -20), (40, 20))[(physical_id - 233) % 4]
        for point in range(1, 100):
            progress = -.5 + point / 100
            x = round(39 + progress * vx)
            y = round(source[1] - 20 + progress * vy - 40 * (progress + .5))
            if pixel(source, x, y)[3] != 255:
                raise RuntimeError(f"native source-cell center footline leaves original sprite {physical_id}")
    stamps = report["sprite_stamps"]
    wanted_stamps = {(i, z) for i in range(233, 245) for z in (1, 2, 4)}
    actual_stamps = {(stamp["physical_id"], stamp["zoom"]) for stamp in stamps}
    if len(stamps) != len(wanted_stamps) or actual_stamps != wanted_stamps:
        raise RuntimeError("all 12 native sprites at 1x/2x/4x require actual rendered reference stamps")
    exact_pixels = transparent_pixels = 0
    for stamp in stamps:
        source = native[stamp["physical_id"]]
        actual = image(stamp["file"])
        origin, scale, background = stamp["origin"], stamp["zoom"], tuple(stamp["background"])
        finite_vector(origin, 2, "native stamp origin")
        if not all(int(n) == n for n in origin) or len(background) != 4 or background[3] != 255:
            raise RuntimeError("native stamps require integer pixel origins and an opaque background")
        for y in range(source[1] * scale):
            for x in range(source[0] * scale):
                reference = pixel(source, x // scale, y // scale)
                wanted = reference if reference[3] else background
                found = pixel(actual, int(origin[0]) + x, int(origin[1]) + y)
                if wanted != found:
                    raise RuntimeError(f"native RGBA/nearest-neighbor mismatch: sprite {stamp['physical_id']}, "
                                       f"zoom {scale}, local pixel ({x},{y}), expected {wanted}, got {found}")
                exact_pixels += 1
                transparent_pixels += not reference[3]
    write_json(root / "native-stamp-validation.json", {
        "result": "PASS", "stamps": len(stamps), "exact_rgba_rendered_pixels": exact_pixels,
        "transparent_exterior_pixels": transparent_pixels,
        "reference": "independent openemperor-inspect export from pinned original SG3",
    })

    coverage = report["coverage"]
    required_coverage = 0
    covered_cases = set()
    crossing_progress = {}
    for witness in coverage:
        location = witness["xy"]
        finite_vector(location, 2, "coverage pixel")
        if not all(int(n) == n for n in location):
            raise RuntimeError("coverage witnesses must name exact rendered pixels")
        if witness["file"] not in case_by_file:
            raise RuntimeError("coverage witness must belong to a declared composed matrix scene")
        case = case_by_file[witness["file"]]
        center, origin = witness["source_cell"], case["camera"]["origin"]
        finite_vector(center, 2, "coverage source cell")
        finite_vector(origin, 2, "camera origin")
        progress, lateral = witness["local_progress"], witness["lateral_offset"]
        if not all(isinstance(n, (int, float)) and math.isfinite(n) for n in (progress, lateral)):
            raise RuntimeError("coverage source-cell coordinates must be finite")
        if not -.5 <= progress <= .5 or not -.5 <= lateral <= .5:
            raise RuntimeError("coverage witness lies outside its source-cell surface")
        x = center[0] + (case["delta"] * progress if case["axis"] == "X" else lateral)
        y = center[1] + (lateral if case["axis"] == "X" else case["delta"] * progress)
        height = case["base_height"] + progress + .5
        expected_xy = [origin[0] + (x - y) * 40 * case["zoom"],
                       origin[1] + ((x + y) * 20 - height * 40) * case["zoom"]]
        if any(abs(a - b) > .500001 for a, b in zip(location, expected_xy)):
            raise RuntimeError(f"coverage pixel does not project its signed source-cell surface: {witness['file']}")
        primary = pixel(image(witness["file"]), int(location[0]), int(location[1]))
        alternate = pixel(image(witness["second_file"]), int(location[0]), int(location[1]))
        if witness["required_opaque"]:
            required_coverage += 1
            covered_cases.add(witness["file"])
            crossing_progress.setdefault(witness["file"], set()).add(progress)
            if primary != alternate or primary[3] != 255:
                raise RuntimeError(f"background-dependent source-cell foot/join at {witness['file']} {location}")
    if not required_coverage or not case_files <= covered_cases:
        raise RuntimeError("every matrix scene needs actual two-background opaque-foot/join witnesses")
    if any(not {-.5, .5} <= crossing_progress[name] or
           not any(-.5 < value < .5 for value in crossing_progress[name]) for name in case_files):
        raise RuntimeError("every scene requires both exact source-cell boundary joins and an interior foot witness")
    write_json(root / "coverage-validation.json", {
        "result": "PASS", "cases": len(cases), "required_background_independent_pixels": required_coverage,
        "projection": "independent 40/20/40 signed source-cell surface projection",
        "limits": "finite center foot and endpoint join witnesses; image review remains separate",
    })

    feet = report["feet"]
    foot_cases = set()
    visible_foot_pixels = 0
    foot_diagnostics = []
    unmarked_actor_pixels = 0
    for foot in feet:
        case_name = foot["file"].removesuffix("-walker.png") + ".png"
        if case_name not in case_by_file:
            raise RuntimeError("walker witness does not belong to a declared scene")
        case = case_by_file[case_name]
        key = (case["axis"], case["delta"], case["style"], case["zoom"])
        if key in foot_cases or case["base_height"] != 0 or case["count"] != 3:
            raise RuntimeError("walker matrix must contain each orientation/style/zoom exactly once")
        foot_cases.add(key)
        position, center, higher = foot["position"], foot["source_cell"], foot["higher"]
        finite_vector(position, 3, "walker position")
        finite_vector(center, 2, "walker source cell")
        expected_higher = [case["delta"], 0] if case["axis"] == "X" else [0, case["delta"]]
        if higher != expected_higher or foot["base_height"] != case["base_height"]:
            raise RuntimeError("walker source-cell height/direction differs from its rendered scene")
        along = (position[0] - center[0]) * higher[0] + (position[1] - center[1]) * higher[1]
        if not -.5 <= along <= .5 or abs(position[2] - (foot["base_height"] + along + .5)) > 1e-9:
            raise RuntimeError("actual moving walker does not use the complete source-cell height surface")
        camera = foot["camera"]
        if camera["origin"] != case["camera"]["origin"] or camera["zoom"] != case["zoom"]:
            raise RuntimeError("walker and composed scene camera differ")
        sx = camera["origin"][0] + (position[0] - position[1]) * 40 * camera["zoom"]
        sy = camera["origin"][1] + ((position[0] + position[1]) * 20 - position[2] * 40) * camera["zoom"]
        actual = image(foot["file"])
        unmarked_file = foot["file"].removesuffix("-walker.png") + "-walker-unmarked.png"
        unmarked, baseline = image(unmarked_file), image(case_name)
        changed_actor_pixels = 0
        zoom = camera["zoom"]
        # Compare actual unmarked actor pixels with the same prepared scene.
        # The region excludes the HUD; its position is derived independently.
        for y in range(max(108, math.floor(sy - 48 * zoom)), min(unmarked[1] - 48, math.ceil(sy + 8 * zoom))):
            for x in range(max(0, math.floor(sx - 24 * zoom)), min(unmarked[0], math.ceil(sx + 24 * zoom))):
                changed_actor_pixels += pixel(unmarked, x, y) != pixel(baseline, x, y)
        if not changed_actor_pixels:
            raise RuntimeError(f"no actual unmarked actor pixels near the projected foot: {unmarked_file}")
        unmarked_actor_pixels += changed_actor_pixels
        # Read the actual renderer's foot cross. A correct number in JSON alone
        # would not show that the composed actor used the same signed height.
        witnesses = []
        for dx, dy in ((0, 0), (-3, 0), (3, 0), (0, -3), (0, 3)):
            found = pixel(actual, math.floor(sx) + dx, math.floor(sy) + dy)
            visible = found == (255, 64, 80, 255)
            witnesses.append({"offset": [dx, dy], "rgba": found, "diagnostic_visible": visible})
            visible_foot_pixels += visible
        foot_diagnostics.append({"file": foot["file"], "unmarked_file": unmarked_file,
                                 "projected_foot": [sx, sy], "unmarked_actor_pixels": changed_actor_pixels,
                                 "witnesses": witnesses})
    expected_foot_cases = {(axis, delta, style, zoom) for axis in ("X", "Y") for delta in (-1, 1)
                          for style in ("Earth", "Stairs", "Cobbled") for zoom in (1, 2, 4)}
    if foot_cases != expected_foot_cases:
        raise RuntimeError("all 36 actual moving-walker orientation/style/zoom captures are required")
    write_json(root / "feet-validation.json", {
        "result": "PASS" if visible_foot_pixels == len(feet) * 5 else "FAIL",
        "scope": "signed surface position, actual actor presence and post-composition diagnostic overlay",
        "rendered_marker_pixels": visible_foot_pixels,
        "expected_marker_pixels": len(feet) * 5,
        "unmarked_actor_pixels": unmarked_actor_pixels,
        "actual_actor_visibility": "requires separate unmarked scene review; an overlay marker cannot establish it",
        "captures": foot_diagnostics,
    })
    if visible_foot_pixels != len(feet) * 5:
        raise RuntimeError("rendered foot diagnostic is occluded or differs from independent signed projection; see feet-validation.json")
    counters = report["counters"]
    expected_counters = {"asset_decodes", "texture_uploads", "file_reads", "file_writes", "world_copies",
                         "world_executes", "simulation_ticks", "bfs_calls", "route_refreshes"}
    if counters.keys() != expected_counters or any(value != 0 for value in counters.values()):
        raise RuntimeError("pure frame pass performed a prohibited preparation/World/navigation operation")
    if report["pure_frames"] < 100:
        raise RuntimeError("at least 100 actual prepared render frames are required")
    interactions = report["interaction_checks"]
    expected_interactions = {"three native styles", "four directions", "three zooms", "native alpha picking",
                             "source-cell erase/build", "road placement", "walker placement", "right-click route",
                             "actual half-height passage", "upper-road arrival"}
    if interactions.get("result") != "PASS" or set(interactions["checks"]) != expected_interactions:
        raise RuntimeError("complete interactive SDL-event edit/pick/movement smoke test is required")
    comparisons = report["composition_comparisons"]
    if len(comparisons) != 12:
        raise RuntimeError("all 12 orientation/style native-versus-Base/Overlay comparisons are required")
    for comparison in comparisons:
        image(comparison["native_per_cell"])
        image(comparison["base_overlay"])

    result = {
        "result": "PASS",
        "format": report["format"],
        "cases": len(cases),
        "native_sprite_stamps": len(stamps),
        "native_exact_rendered_pixels": exact_pixels,
        "native_transparent_exterior_pixels": transparent_pixels,
        "native_source_cell_interior_footline_alpha_checkpoints": 12 * 99,
        "required_two_background_coverage_pixels": required_coverage,
        "moving_walker_captures": len(feet),
        "independently_projected_visible_foot_pixels": visible_foot_pixels,
        "actual_unmarked_actor_changed_pixels": unmarked_actor_pixels,
        "actor_visibility_limits": "finite changed pixels prove presence; diagnostic overlay does not establish visible boots or original painter parity",
        "scripted_SDL_event_checks": interactions,
        "native_vs_base_overlay_comparisons": len(comparisons),
        "pure_render_frames": report["pure_frames"],
        "prohibited_frame_counters": counters,
        "visual_review_performed": False,
        "original_game_parity": "not established by this machine report",
    }
    write_json(root / "pixel-validation.json", result)
    print(f"Native RGBA/nearest-neighbor: {len(stamps)} stamps, {exact_pixels} exact pixels PASS", flush=True)
    print(f"432-case composed matrix: {required_coverage} two-background required coverage pixels PASS", flush=True)
    print(f"Walker projection diagnostics: {len(feet)} actual actor captures, {visible_foot_pixels} overlay marker pixels PASS", flush=True)
    print(f"Prepared render frames: {report['pure_frames']}, all prohibited counters zero PASS", flush=True)
    return result


def export_native(data: Path, build: Path, root: Path) -> None:
    """Use the existing inspector as an independent source-pixel reference."""
    inspector = build / "openemperor-inspect"
    if not inspector.is_file():
        raise RuntimeError("build openemperor-inspect for independent native-pixel comparison")
    destination = root / "reference-native"
    destination.mkdir()
    with (root / "native-export.log").open("w") as log:
        for physical_id in range(233, 245):
            subprocess.run([str(inspector), str(data / "DATA/China_Elevation.sg3"),
                            "--image", str(physical_id), "--png", str(destination / f"{physical_id}.png")],
                           check=True, stdout=log, stderr=subprocess.STDOUT)


def run(args: argparse.Namespace) -> None:
    repo = Path(__file__).resolve().parents[1]
    data, build = args.data.resolve(strict=True), args.build.resolve(strict=True)
    if not data.is_dir() or not (data / "DATA").is_dir():
        raise RuntimeError("--data must contain DATA/")
    binary = build / "openemperor-original-ramp-prototype"
    if not binary.is_file():
        raise RuntimeError("build the isolated openemperor-original-ramp-prototype target first")
    initial = inventory(data)
    if initial != PINNED_DATA:
        raise RuntimeError("original data revision differs from the investigated assets")
    private = repo / ".local/original-ramp-prototype"
    private.mkdir(parents=True, exist_ok=True)
    root = Path(tempfile.mkdtemp(prefix="capture-" if args.run == "capture" else "session-", dir=private))
    write_json(root / "inputs-before.json", initial)
    print("Prototype evidence:", root, flush=True)
    env = {key: value for key, value in os.environ.items()
           if not key.startswith(("SDL_", "DYLD_", "OPENEMPEROR_"))}
    command = [str(binary), str(data)]
    if args.run == "capture":
        command += ["--capture", str(root)]
        env["SDL_VIDEODRIVER"] = "dummy"
    result = None
    try:
        if args.run == "capture":
            export_native(data, build, root)
            with (root / "prototype.log").open("w") as log:
                subprocess.run(command, cwd=repo, env=env, check=True, stdout=log, stderr=subprocess.STDOUT)
            result = check(root)
        else:
            print("Opening isolated original-sprite prototype.", flush=True)
            subprocess.run(command, cwd=repo, env=env, check=True)
    finally:
        final = inventory(data)
        write_json(root / "inputs-after.json", final)
        if final != initial:
            raise RuntimeError("original input files changed during the prototype session")
    write_json(root / "acceptance.json", {
        "result": "PASS" if args.run == "capture" else "interactive session exited successfully",
        "source_inventory_unchanged": True,
        "original_revision": initial,
        "pixel_validation": result,
        "native_human_acceptance": "not inferred from automated checks or window delivery",
        "production_renderer_modified_by_runner": False,
    })
    print("Original inputs unchanged; evidence:", root, flush=True)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("capture_root", nargs="?", type=Path)
    parser.add_argument("--run", choices=("ui", "capture"))
    parser.add_argument("--data", type=Path)
    parser.add_argument("--build", type=Path)
    args = parser.parse_args()
    if args.run:
        if args.capture_root or not args.data or not args.build:
            parser.error("--run requires --data and --build, without CAPTURE_ROOT")
        run(args)
    elif args.capture_root and not args.data and not args.build:
        check(args.capture_root.resolve(strict=True))
    else:
        parser.error("provide CAPTURE_ROOT or --run {ui,capture} --data DATA --build BUILD")


if __name__ == "__main__":
    main()
