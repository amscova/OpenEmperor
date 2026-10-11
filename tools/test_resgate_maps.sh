#!/bin/sh
set -eu

repo_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
if [ "$#" -lt 1 ] || [ "$#" -gt 2 ]; then
    echo 'Usage: sh tools/test_resgate_maps.sh <original-data-directory> [exact city name]' >&2
    exit 2
fi
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

app_data_dir=$(mktemp -d "$repo_dir/.local/resgate/player-XXXXXX")
echo "Checking $map_relative with current City-v16 rule 4 / policy 2."
# Reuse the production CLI: complete manager/permissions, assets and the
# ordinary paid starter are prepared and checked headlessly before any menu.
# A failed check never launches another map or falls back to legacy rules.
if "$build_dir/openemperor" --data "$original_data" --sandbox "$map_relative" \
    --sandbox-rules sandbox-city-v16 --sandbox-check --report-json \
    > "$app_data_dir/check.json" 2> "$app_data_dir/check.stderr"; then
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
    --great-wall-presentation auto
