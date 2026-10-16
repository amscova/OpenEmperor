#!/usr/bin/env python3
"""Read-only original asset export through the production catalog and decoder.

The numbered contact sheets are review aids, never compatibility resources or
proof of an original selector. ImageMagick only arranges decoded PNG pixels and
adds labels; its 2x copy uses point sampling. Outputs must remain in .local/.
"""

import argparse
import concurrent.futures
import hashlib
import json
from pathlib import Path
import shutil
import subprocess


def within(path, parent):
    try:
        path.relative_to(parent)
        return True
    except ValueError:
        return False


def run(args):
    result = subprocess.run([str(a) for a in args], capture_output=True, text=True)
    if result.returncode:
        raise RuntimeError(f"Command failed: {args}\n{result.stderr}")
    return result.stdout


def write_json(path, value):
    path.write_text(json.dumps(value, indent=2) + "\n")


def fingerprint(paths, root):
    return {
        str(path.relative_to(root)): {
            "bytes": path.stat().st_size,
            "sha256": hashlib.sha256(path.read_bytes()).hexdigest(),
        }
        for path in paths
    }


def main():
    repo = Path(__file__).resolve().parents[1]
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--data", type=Path, required=True)
    parser.add_argument("--build", type=Path, default=repo / ".local/build-ramp-audit")
    parser.add_argument("--output", type=Path, required=True,
                        help="New or empty directory beneath this checkout's .local/")
    parser.add_argument("--archive", action="append", choices=("Elevation", "Terrain"),
                        help="Repeat to select archives; defaults to both")
    parser.add_argument("--font", type=Path,
                        default=Path("/System/Library/Fonts/Helvetica.ttc"))
    args = parser.parse_args()
    root, output = args.data.resolve(strict=True), args.output.resolve()
    if not root.is_dir() or not (root / "DATA").is_dir():
        parser.error("--data must contain DATA/")
    if (not within(output, repo / ".local") or output == repo / ".local"
            or within(output, root) or within(root, output)):
        parser.error("Output must be below .local/, outside and separate from original data")
    if output.exists() and any(output.iterdir()):
        parser.error("Output directory is not empty; choose a new audit directory")
    inspector = args.build.resolve() / "openemperor-inspect"
    catalog = args.build.resolve() / "openemperor-assets"
    if not inspector.is_file() or not catalog.is_file():
        parser.error("Build the openemperor-inspect and openemperor-assets targets first")
    magick = shutil.which("magick")
    if not magick or not args.font.is_file():
        parser.error("ImageMagick and a readable --font are required")
    output.mkdir(parents=True, exist_ok=True)
    names = list(dict.fromkeys(args.archive or ("Elevation", "Terrain")))
    inputs = [root / "DATA" / f"China_{name}.{suffix}"
              for name in names for suffix in ("sg3", "555")]
    before = fingerprint(inputs, root)
    write_json(output / "inputs-before.json", before)
    summary = {"format": "openemperor-original-ramp-asset-audit-v1",
               "catalog_binary": str(catalog), "decoder_binary": str(inspector),
               "visual_review_performed": False,
               "review_requirement": "Open and inspect every numbered sheet separately",
               "scales": [1, 2], "magnification": "integer nearest neighbor",
               "source_unchanged": False, "archives": []}
    for name in names:
        stem, relative = f"China_{name}", f"DATA/China_{name}.sg3"
        target = output / stem
        target.mkdir()
        archive = json.loads(run([inspector, root / relative]))
        write_json(output / f"{stem}-inspect.json", archive)
        full = json.loads(run([catalog, "--data", root, "--archive", stem, "--json"]))
        write_json(output / f"{stem}-catalog.json", full)
        first, last = 201, archive["header"]["reported_images_in_use"]
        rows = [r for r in full["records"] if r["id"]["archive"] == relative
                and first <= r["id"]["image_index"] <= last]
        if len(rows) != last - first + 1:
            raise RuntimeError("Catalog does not cover the complete regular physical range")
        write_json(output / f"{stem}-active-catalog.json", rows)

        def export(row):
            index = row["id"]["image_index"]
            command = [str(inspector), str(root / relative), "--image", str(index),
                       "--png", str(target / f"{index:04d}.png")]
            result = subprocess.run(command, capture_output=True, text=True)
            (target / f"{index:04d}-decode.txt").write_text(result.stdout + result.stderr)
            return {"id": index, "exit_code": result.returncode, "command": command}

        with concurrent.futures.ThreadPoolExecutor(max_workers=4) as pool:
            decodes = list(pool.map(export, rows))
        write_json(target / "decodes.json", decodes)
        good = [row for row, decode in zip(rows, decodes) if decode["exit_code"] == 0]
        for row in good:
            index = row["id"]["image_index"]
            source = target / f"{index:04d}.png"
            label = (f"{index} g{row['group_id']} {row['width']}x{row['height']} "
                     f"t{row['image_type']} a{row['alpha_length']}")
            run([magick, "-background", "#53575c", "(", source, "-alpha", "remove",
                 "-alpha", "off", "+repage", ")", "(", source, "-filter", "point",
                 "-resize", "200%", "-alpha", "remove", "-alpha", "off", "+repage",
                 ")", "+append", "+repage", "-gravity", "north", "-splice", "0x26",
                 "-font", args.font, "-pointsize", "13", "-fill", "white", "-annotate",
                 "+0+4", label, target / f"{index:04d}-cell.png"])
        sheets = []
        for group in sorted({row["group_id"] for row in good}):
            for size, large in (("small", False), ("large", True)):
                items = [row for row in good if row["group_id"] == group
                         and (row["width"] > 158) == large]
                if not items:
                    continue
                per, columns = (24, 4) if not large else (
                    (6, 2) if max(row["width"] for row in items) <= 238 else (2, 1))
                for page, start in enumerate(range(0, len(items), per), 1):
                    batch = items[start:start + per]
                    destination = target / f"sheet-g{group}-{size}-{page:02d}.png"
                    run([magick, "montage", "-font", args.font,
                         *[target / f"{row['id']['image_index']:04d}-cell.png" for row in batch],
                         "-background", "#34383d", "-gravity", "northwest", "-geometry",
                         "+8+8", "-tile", f"{columns}x", destination])
                    sheets.append({"path": str(destination.relative_to(output)),
                                   "records": [row["id"]["image_index"] for row in batch],
                                   "scales": [1, 2]})
        write_json(target / "sheets.json", sheets)
        summary["archives"].append({"archive": relative, "physical_range": [first, last],
                                    "records": len(rows), "decoded": len(good),
                                    "failed": len(rows) - len(good), "sheets": len(sheets)})
        print(f"{relative}: decoded {len(good)}/{len(rows)}, {len(sheets)} numbered sheets",
              flush=True)
    after = fingerprint(inputs, root)
    write_json(output / "inputs-after.json", after)
    summary["source_unchanged"] = before == after
    write_json(output / "summary.json", summary)
    if before != after:
        raise RuntimeError("Original input fingerprints changed during audit")
    if any(archive["failed"] for archive in summary["archives"]):
        raise RuntimeError("Some records failed to decode; inspect named decode logs")


if __name__ == "__main__":
    main()
