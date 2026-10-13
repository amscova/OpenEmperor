#!/bin/sh
set -eu
if [ "$#" -ne 2 ] && [ "$#" -ne 3 ]; then
    echo 'Usage: sh tools/test_remaining_occupancy.sh DATA --audit | DATA --map "exact-map-stem" | DATA --conflicts "exact-map-stem" | DATA --bounds "exact-map-stem"' >&2
    exit 2
fi
repo_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
case "$2:$#" in --audit:2|--map:3|--conflicts:3|--bounds:3) ;; *) echo 'Use --audit, --map, --conflicts or --bounds with one exact map stem.' >&2; exit 2 ;; esac
original_data=$(CDPATH= cd -- "$1" && pwd -P)
# Pin the complete local original revision, not just one convenient map.
python3 - "$original_data" <<'PY'
import hashlib, pathlib, sys
root = pathlib.Path(sys.argv[1]).resolve(strict=True)
paths = sorted(root.rglob('*'), key=lambda p: p.relative_to(root).as_posix())
manifest = hashlib.sha256()
count = size = 0
for p in paths:
    if p.is_symlink():
        raise SystemExit('Original revision contains a symlink; no app launched.')
    if not p.is_file():
        continue
    digest = hashlib.sha256()
    with p.open('rb') as f:
        for block in iter(lambda: f.read(1024*1024), b''):
            digest.update(block)
    n = p.stat().st_size
    count += 1
    size += n
    manifest.update(f'{p.relative_to(root).as_posix()}\t{n}\t{digest.hexdigest()}\n'.encode())
if (count, size, manifest.hexdigest()) != (1464, 819711091, 'dde2382fe1c348da765c8776149a73210b81f6cba7f93a0c6159d79ce3154feb'):
    raise SystemExit('Original files differ from the verified complete revision; no app launched.')
print('Verified complete original revision: 1,464 files, 819,711,091 bytes.')
PY
build_dir="$repo_dir/.local/reswall/release-build"
cmake -S "$repo_dir" -B "$build_dir" -DCMAKE_BUILD_TYPE=Release -DOPENEMPEROR_USE_SYSTEM_SDL3=ON
cmake --build "$build_dir" --parallel 2 --target openemperor openemperor-audit-original-occupancy
mkdir -p "$repo_dir/.local/original-bounds"
if [ "$2" = --audit ]; then
    run_dir=$(mktemp -d "$repo_dir/.local/original-bounds/wrapper-audit-XXXXXX")
    "$build_dir/openemperor-audit-original-occupancy" "$original_data" "$run_dir/report" --summary-only
    echo "Full corpus report, both policies and actual/zero/one mask summaries: $run_dir/report"
    exit 0
fi
# Production MapCatalog supplies the exact contained standalone map.
run_dir=$(mktemp -d "$repo_dir/.local/original-bounds/wrapper-map-XXXXXX")
"$build_dir/openemperor" --data "$original_data" --list-maps --report-json > "$run_dir/catalog.json"
map_name=$(python3 - "$run_dir/catalog.json" "$3" <<'PY'
import json, pathlib, sys
name = sys.argv[2]
if not name or any(c in name for c in '/\\\r\n') or name in ('.', '..') or name.endswith('.map'):
    raise SystemExit('Use one exact map stem, without directories or .map; no app launched.')
catalog = json.load(open(sys.argv[1]))
entries = [e for e in catalog['entries'] if e['relative_path'] == 'Cities/' + name + '.map']
if catalog['scan_errors'] or len(entries) != 1 or not entries[0]['container_valid'] or not entries[0]['map_profile']:
    raise SystemExit('Exact standalone map unavailable in production MapCatalog; no app launched.')
print(pathlib.PurePosixPath(entries[0]['relative_path']).stem)
PY
)
if [ "$2" = --conflicts ] || [ "$2" = --bounds ]; then
    "$build_dir/openemperor-audit-original-occupancy" "$original_data" "$run_dir/report" "$2" "Cities/$map_name.map"
    echo "Complete original source/overlap diagnosis: $run_dir/report/corpus.json"
    exit 0
fi
# Compile with current CMake audit flags; production classes prepare the exact
# Empty City instead of requiring an unrelated fixed paid-demo arrangement.
python3 "$repo_dir/tools/build_empty_map_preflight.py" "$repo_dir" "$build_dir" "$run_dir"
OPENEMPEROR_TEST_EMPTY_MAP_PREFLIGHT="$run_dir/empty-map-preflight" \
OPENEMPEROR_TEST_APP_ROOT_PARENT="$run_dir" exec sh "$repo_dir/tools/test_resgate_maps.sh" "$original_data" "$map_name"
