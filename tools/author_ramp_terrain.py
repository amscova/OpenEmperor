"""Independent authored map, never a repaired original map or original-ramp claim.

Private local assets are read-only hard links to the user's data. Only the newly
authored map is written; no linked original file is ever opened for writing.
"""
from pathlib import Path
import json
import os
import struct
import sys
import zlib


def author(data, target):
    data, target = Path(data).resolve(strict=True), Path(target).resolve()
    allowed_roots = (Path.cwd().resolve() / ".local/city-v16-ramps", Path.cwd().resolve() / ".local/oe43")
    if target.exists() or not any(target != base and target.is_relative_to(base) for base in allowed_roots):
        raise RuntimeError("fresh private authored terrain root required")
    if target.is_relative_to(data) or data.is_relative_to(target):
        raise RuntimeError("authored output must be disjoint from original inputs")
    target.mkdir(parents=True)
    for folder in ("DATA", "Model", "Res"):
        for source in sorted((data / folder).rglob("*")):
            if source.is_file() and not source.is_symlink():
                output = target / source.relative_to(data)
                output.parent.mkdir(parents=True, exist_ok=True)
                os.link(source, output)
    os.link(data / "Emperor.exe", target / "Emperor.exe")
    # Read only: choose a real supported singleton ground/water image reference
    # from the pinned installation. Image bytes remain outside the repository.
    source = (data / "Cities/Xia.map").read_bytes()
    position, decoded = 4, bytearray()
    while position < len(source):
        marker, packed, length = struct.unpack_from("<III", source, position)
        if packed == 0 or packed > 65536 or length == 0 or length > 32768 or position + 12 + packed > len(source):
            raise RuntimeError("invalid bounded original block")
        block = zlib.decompress(source[position + 12:position + 12 + packed])
        if len(block) != length:
            raise RuntimeError("unexpected original block length")
        decoded.extend(block)
        position += 12 + packed

    def original_reference(terrain):
        for index in range(228 * 228):
            t = struct.unpack_from("<I", decoded, 261455 + 4 * index)[0]
            obj = struct.unpack_from("<I", decoded, 469391 + 4 * index)[0]
            graphic = struct.unpack_from("<I", decoded, 1535 + 4 * index)[0]
            if t == terrain and obj == 0 and graphic // 16384 == 3:
                return graphic
        raise RuntimeError("pinned singleton image reference unavailable")

    ground, water = original_reference(0x80), original_reference(4)
    raw = bytearray(1093613)
    raw[:8] = bytes([5, 0, 0xFE, 0xCA, 0, 0, 2, 0])
    struct.pack_into("<I", raw, 84, 84)
    struct.pack_into("<HI", raw, 1093607, 1, 0)  # Complete empty authored manager.
    for y in range(228):
        for x in range(228):
            index = y * 228 + x
            start = 72 + 114 - y - 1 if y < 114 else 72 + y - 114
            stop = 114 + y + 1 - 72 if y < 114 else 342 - y - 72
            inside = 72 <= y < 156 and start <= x < stop
            river = inside and x == 119
            terrain = (4 if river else 0x80) if inside else 0x80000
            struct.pack_into("<I", raw, 261455 + 4 * index, terrain)
            struct.pack_into("<I", raw, 1535 + 4 * index, water if river else ground)
            raw[209471 + index] = 64
            raw[989275 + index] = 1 if x < 116 else 0
    output = bytearray([0xAA, 0xBA, 0xDC, 0xFE])
    for position in range(0, len(raw), 32768):
        block = raw[position:position + 32768]
        zipped = zlib.compress(block, 6)
        output.extend(struct.pack("<III", 0x12345678, len(zipped), len(block)))
        output.extend(zipped)
    (target / "Cities").mkdir()
    (target / "Cities/Authored Ramps.map").write_bytes(output)
    (target.parent / "authored-terrain.json").write_text(json.dumps({
        "evidence": "OPENEMPEROR AUTHORED RAMP; independently authored grids/manager, not an original-map PASS",
        "map": "Cities/Authored Ramps.map", "height_boundary_x": 116,
        "height_left": 1, "height_right": 0, "water_x": 119,
        "original_pixels_checked_in": False, "original_inputs_written": False,
        "ground_saved_reference": ground, "water_saved_reference": water,
    }, indent=2) + "\n")


if __name__ == "__main__":
    author(*sys.argv[1:])
