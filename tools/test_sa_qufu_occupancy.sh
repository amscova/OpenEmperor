#!/bin/sh
set -eu
if [ "$#" -ne 1 ]; then
    echo 'Usage: sh tools/test_sa_qufu_occupancy.sh <original-data-directory>' >&2
    exit 2
fi
repo_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
python3 - "$1" <<'PY'
import hashlib, pathlib, sys
try:
    root = pathlib.Path(sys.argv[1]).resolve(strict=True)
except OSError:
    raise SystemExit('Original-data directory is missing or unreadable; no app launched.')
for relative, expected in (
    ('Cities/SA-Qufu.map', '4fc8275f5c639b34da43435dee901b0539a0743af941109b7be3f510d11cef9e'),
    ('Emperor.exe', '6373328bfc5c4886d9abc9544eb706e89e7d465b18176ea8205fe27aaee53c0e'),
    ('Model/EmperorBuildingModels.txt', '7e51df8cd49ab39fea7397b7c1a9e7edefdc98a8f09b5e2139987346b4b972e6'),
):
    path = root / relative
    try:
        path.resolve(strict=True).relative_to(root)
    except (ValueError, OSError):
        raise SystemExit(f'{relative} is missing or escapes the original-data directory; no app launched.')
    if not path.is_file() or path.is_symlink() or path.parent.is_symlink():
        raise SystemExit(f'{relative} must be a regular contained original file; no app launched.')
    if hashlib.sha256(path.read_bytes()).hexdigest() != expected:
        raise SystemExit(f'{relative} differs from the verified revision; no app launched.')
print('SA-Qufu: City-v16 rule 4 / policy 2 / schema 20; normal menu, Empty City.')
print('Verified Type173 storage origins: (160,91), (144,75), (74,84), (68,136), (88,158), (89,158).')
print('These saved invasion points remain protected; no invasion/NPC operation is imported.')
PY
# Ask the current production MapCatalog for the exact verified file, then
# delegate complete preflight and isolated normal-menu startup to the shared runner.
build_dir="$repo_dir/.local/resgate/test-build"
cmake -S "$repo_dir" -B "$build_dir" -DCMAKE_BUILD_TYPE=Release \
    -DOPENEMPEROR_USE_SYSTEM_SDL3=ON
cmake --build "$build_dir" --parallel 2 --target openemperor
map_name=$("$build_dir/openemperor" --data "$1" --list-maps --report-json | python3 -c '
import json, pathlib, sys
catalog = json.load(sys.stdin)
matches = [entry for entry in catalog["entries"] if entry["relative_path"] == "Cities/SA-Qufu.map"]
if catalog["scan_errors"] or len(matches) != 1 or not matches[0]["container_valid"] or not matches[0]["map_profile"] or matches[0]["declared_map_size"] != 140:
    raise SystemExit("Verified SA-Qufu standalone map is unavailable in the production catalog; no app launched.")
print(pathlib.PurePosixPath(matches[0]["relative_path"]).stem)
')
exec sh "$repo_dir/tools/test_resgate_maps.sh" "$1" "$map_name"
