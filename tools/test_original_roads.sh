#!/bin/sh
set -eu

repo_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd -P)
if [ "$#" -ne 2 ]; then
    echo "Usage: sh tools/test_original_roads.sh <original-data-directory> <exact-map-name>" >&2
    exit 2
fi
original_data=$(CDPATH= cd -- "$1" && pwd -P)
map_file=$(python3 - "$original_data" "$2" <<'PY'
import pathlib, sys
root = pathlib.Path(sys.argv[1])
name = sys.argv[2]
if not name or name in (".", "..") or any(c in name for c in ("/", "\\", "\n", "\r")):
    raise SystemExit("Choose an exact original Cities map name, without a directory.")
filename = name if name.endswith(".map") else name + ".map"
directory = root / "Cities"
try:
    directory.resolve().relative_to(root)
except ValueError:
    raise SystemExit("The original Cities directory must remain inside the data root.")
matches = [p for p in directory.iterdir() if p.name == filename and p.is_file()]
if len(matches) != 1:
    raise SystemExit("No unique exact Cities filename: " + filename)
path = matches[0].resolve()
try:
    path.relative_to(directory.resolve())
except ValueError:
    raise SystemExit("The selected map must remain inside the original Cities directory.")
print(matches[0].name)
PY
)
private_dir=$(python3 - "$repo_dir" "$original_data" <<'PY'
import pathlib, sys
path = (pathlib.Path(sys.argv[1]) / ".local/original-roads").resolve()
try:
    path.relative_to(pathlib.Path(sys.argv[2]))
except ValueError:
    print(path)
else:
    raise SystemExit("The test workspace must be outside the original-data directory.")
PY
)
mkdir -p "$private_dir"
build_dir="$private_dir/player-build"
cmake -S "$repo_dir" -B "$build_dir" -DCMAKE_BUILD_TYPE=Release \
    -DOPENEMPEROR_USE_SYSTEM_SDL3=ON
cmake --build "$build_dir" --parallel 4 --target openemperor openemperor-original-roads-fixture
app_data_dir=$(mktemp -d "$private_dir/player-${map_file%.map}-XXXXXX")
(cd "$repo_dir" && "$build_dir/openemperor-original-roads-fixture" \
    "$original_data" "Cities/$map_file" "$app_data_dir")
python3 - "$app_data_dir/settings.json" "$original_data" "$map_file" "$app_data_dir/original-roads-report.json" <<'PYSETTINGS'
import json, pathlib, sys
report = json.loads(pathlib.Path(sys.argv[4]).read_text())
if (report["rule"], report["policy"], report["schema"]) != (4, 2, 20):
    raise SystemExit("The prepared scene must explicitly use rule 4 / policy 2 / schema 20.")
pathlib.Path(sys.argv[1]).write_text(json.dumps({
    "version": 1, "data_root": sys.argv[2],
    "last_map": "Cities/" + sys.argv[3], "last_save": "",
    "prepared_starter": False, "autosave_enabled": True,
    "profile": "sandbox-city-v16"
}, indent=2) + "\n")
print("City-v16 rule 4 / Map policy 2 / Save schema 20:", sys.argv[3])
print("Load Sandbox -> A-original-ready, then Space starts the real Clay trip.")
print("Two Houses, Clay Source and Pottery were paid normally; no Road was purchased.")
print("F1, then G on Clay checks the route; F2 compares walker markers.")
print("Old cities retain their saved rules. New Sandbox uses rule 4 on this fresh root.")
PYSETTINGS
echo "Private settings, saves and recovery: $app_data_dir"
exec "$build_dir/openemperor" --data "$original_data" --app-root "$app_data_dir" \
    --great-wall-presentation auto
