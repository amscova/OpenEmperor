#!/bin/sh
set -eu

repo_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd -P)
if [ "$#" -ne 2 ]; then
    echo "Usage: sh tools/test_ordinary_towers.sh <original-data-directory> <exact-map-name>" >&2
    exit 2
fi
original_data=$(CDPATH= cd -- "$1" && pwd -P)
map_file=$(python3 - "$original_data" "$2" <<'PYMAP'
import pathlib, sys
root = pathlib.Path(sys.argv[1])
name = sys.argv[2]
if not name or name in (".", "..") or any(c in name for c in ("/", "\\", "\n", "\r")):
    raise SystemExit("Choose an exact original Cities map name, without a directory.")
filename = name if name.endswith(".map") else name + ".map"
if filename not in ("Kaifeng.map", "Zhengzhou.map"):
    raise SystemExit("This bounded Tower starter supports exact Kaifeng or Zhengzhou.")
directory = root / "Cities"
try:
    directory.resolve().relative_to(root)
except ValueError:
    raise SystemExit("The original Cities directory must remain inside the data root.")
if not directory.is_dir():
    raise SystemExit("The original Cities directory is missing.")
matches = [p for p in directory.iterdir() if p.name == filename and p.is_file()]
if len(matches) != 1:
    raise SystemExit("No unique exact Cities filename: " + filename)
path = matches[0].resolve()
try:
    path.relative_to(directory.resolve())
except ValueError:
    raise SystemExit("The selected map must remain inside the original Cities directory.")
print(matches[0].name)
PYMAP
)
private_dir=$(python3 - "$repo_dir" "$original_data" <<'PYROOT'
import pathlib, sys
path = (pathlib.Path(sys.argv[1]) / ".local/ordinary-towers").resolve()
try:
    path.relative_to(pathlib.Path(sys.argv[2]))
except ValueError:
    print(path)
else:
    raise SystemExit("The test workspace must be outside the original-data directory.")
PYROOT
)
mkdir -p "$private_dir"
build_dir="$private_dir/player-build"
cmake -S "$repo_dir" -B "$build_dir" -DCMAKE_BUILD_TYPE=Release \
    -DOPENEMPEROR_USE_SYSTEM_SDL3=ON
cmake --build "$build_dir" --parallel 4 --target openemperor
app_data_dir=$(mktemp -d "$private_dir/player-${map_file%.map}-XXXXXX")
python3 - "$app_data_dir/settings.json" "$original_data" "$map_file" <<'PYSETTINGS'
import hashlib, json, pathlib, sys
pathlib.Path(sys.argv[1]).write_text(json.dumps({
    "version": 1, "data_root": sys.argv[2],
    "last_map": "Cities/" + sys.argv[3], "last_save": "",
    "prepared_starter": False, "autosave_enabled": True,
    "profile": "sandbox-city-v16"
}, indent=2) + "\n")
print("City-v16 rule 4 / MapPermissions policy 2 / Save schema 20:", sys.argv[3])
print("Choose New Sandbox, wait for Map rules checked, then Start Sandbox.")
print("Empty City: the original Towers need no Sandbox building or purchase.")
map_sha = hashlib.sha256((pathlib.Path(sys.argv[2]) / "Cities" / sys.argv[3]).read_bytes()).hexdigest()
print("Selected original-map SHA-256:", map_sha)
checked = {
    "Kaifeng.map": ("c25855e4db083128d9344b9177dbbc12c2f0fb3f28c25b63d01c93349912e7b3",
                    "Checked original cTower ID 3: storage (86,120), serialized local (57,91)."),
    "Zhengzhou.map": ("d7a212431f25134f7ffc22c888ebbba4a7d82053263ce8338a3990c891427c4c",
                      "Checked original cTower ID 4: storage (93,82), serialized local (35,24).")
}
expected_sha, location = checked[sys.argv[3]]
if map_sha == expected_sha:
    print(location)
else:
    print("This map differs from the checked data revision; F1 reports the loaded original objects.")
print("F1 inspects Tower provenance; F8 compares layers and Historical Snapshot.")
print("Zoom and arrows inspect the full body and its adjoining original walls.")
PYSETTINGS
echo "Private settings, saves and recovery: $app_data_dir"
exec "$build_dir/openemperor" --data "$original_data" --app-root "$app_data_dir" \
    --great-wall-presentation auto
