#!/bin/sh
set -eu

repo_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
if [ "$#" -lt 1 ] || [ "$#" -gt 3 ]; then
    echo 'Usage: sh tools/test_resgate_maps.sh <original-data-directory> [exact city name] [great-wall-presentation]' >&2
    exit 2
fi
great_wall_mode=${3:-auto}
case "$great_wall_mode" in
    auto|historical|preview-ruined|preview-earthen|preview-stone) ;;
    *) echo 'Great Wall presentation must be auto, historical, preview-ruined, preview-earthen or preview-stone.' >&2; exit 2 ;;
esac
original_data=$(CDPATH= cd -- "$1" && pwd -P)
map_name=${2:-Xiangjun}
map_relative=$(python3 - "$original_data" "$map_name" <<'PY'
import pathlib, sys
root = pathlib.Path(sys.argv[1]).resolve(strict=True)
name = sys.argv[2]
if not name or any(c in name for c in '/\\\r\n') or name in ('.', '..'):
    raise SystemExit('Use one exact city name, without directory components.')
name = name if name.endswith('.map') else name + '.map'
relative = pathlib.Path('Cities') / name
path = root / relative
try:
    path.resolve(strict=True).relative_to(root)
except (ValueError, OSError):
    raise SystemExit('Selected map is missing or escapes the original-data directory.')
if not path.is_file() or path.is_symlink() or path.parent.is_symlink():
    raise SystemExit('Selected map must be a regular catalog file inside Cities.')
print(relative.as_posix())
PY
)

build_dir="$repo_dir/.local/resgate/test-build"
cmake -S "$repo_dir" -B "$build_dir" -DCMAKE_BUILD_TYPE=Release \
    -DOPENEMPEROR_USE_SYSTEM_SDL3=ON
cmake --build "$build_dir" --parallel 2 --target openemperor

app_root_parent=${OPENEMPEROR_TEST_APP_ROOT_PARENT:-"$repo_dir/.local/resgate"}
python3 - "$repo_dir" "$app_root_parent" "$original_data" <<'PYROOT'
import pathlib, sys
repo, parent, data = map(lambda p: pathlib.Path(p).resolve(strict=True), sys.argv[1:])
try:
    parent.relative_to(repo / '.local')
except ValueError:
    raise SystemExit('Test app root must be inside this repository .local.')
if parent == data or data in parent.parents or parent in data.parents:
    raise SystemExit('Test app root must be separate from original data.')
PYROOT
app_data_dir=$(mktemp -d "$app_root_parent/player-XXXXXX")
echo "Checking $map_relative with current City-v16 rule 4 / policy 2."
echo "Great Wall presentation: $great_wall_mode (session only; previews are not verified original restore)."
# Reuse the production CLI: complete manager/permissions, assets and the
# ordinary paid starter are prepared and checked headlessly before any menu.
# A failed check never launches another map or falls back to legacy rules.
preflight_map() {
    if [ -n "${OPENEMPEROR_TEST_EMPTY_MAP_PREFLIGHT:-}" ]; then
        # Explicit remaining-occupancy runner uses the same complete producer
        # with Empty City; the historical paid-demo check stays the default.
        "$OPENEMPEROR_TEST_EMPTY_MAP_PREFLIGHT" "$original_data" "$map_relative"
    else
        "$build_dir/openemperor" --data "$original_data" --sandbox "$map_relative" \
            --sandbox-rules sandbox-city-v16 --sandbox-check --report-json
    fi
}
if preflight_map > "$app_data_dir/check.json" 2> "$app_data_dir/check.stderr"; then
    python3 - "$app_data_dir/check.json" <<'PY'
import json, sys
report = json.load(open(sys.argv[1]))
if report.get('rules') != 'sandbox-city-v16' or report.get('rule_version') != 4:
    raise SystemExit('Check did not use current City-v16 rule 4; no app launched.')
print('Full headless sandbox check passed. Opening the normal menu.')
PY
else
    cat "$app_data_dir/check.stderr" >&2
    echo "No app launched. Exact selected-map check: $app_data_dir" >&2
    exit 1
fi

# The sandbox authority check deliberately has no presentation-mode argument.
# Check an explicit session choice through the existing headless render path.
if [ "$great_wall_mode" != auto ]; then
    if "$build_dir/openemperor" --data "$original_data" --map-debug "$map_relative" \
        --view stored-graphics --graphics-profile exe-6373328b-v213-slot8-runtime-table \
        --multi-tile-preview --footprint-policy edge-byte-4x4 \
        --great-wall-presentation "$great_wall_mode" --render-check --report-json \
        > "$app_data_dir/presentation-check.json" 2> "$app_data_dir/presentation-check.stderr"; then
        echo "Selected presentation load/render check passed."
    else
        cat "$app_data_dir/presentation-check.stderr" >&2
        echo "No app launched. Selected-presentation check: $app_data_dir" >&2
        exit 1
    fi
fi

python3 - "$app_data_dir/settings.json" "$original_data" "$map_relative" <<'PY'
import json, pathlib, sys
pathlib.Path(sys.argv[1]).write_text(json.dumps({
    'version': 1, 'data_root': sys.argv[2], 'last_map': sys.argv[3],
    'last_save': '', 'prepared_starter': False, 'autosave_enabled': True,
    'profile': 'sandbox-city-v16'
}, indent=2) + '\n')
PY
echo "Choose New Sandbox, then Start Sandbox: $map_relative, Empty City, rule 4 / policy 2 / schema 20."
echo "Private settings, saves and recovery: $app_data_dir"
exec "$build_dir/openemperor" --data "$original_data" --app-root "$app_data_dir" \
    --great-wall-presentation "$great_wall_mode"
