#!/bin/sh
set -eu
if [ "$#" -lt 1 ] || [ "$#" -gt 2 ]; then
    echo 'Usage: sh tools/test_mpwall1_occupancy.sh <original-data-directory> [great-wall-presentation]' >&2
    exit 2
fi
great_wall_mode=${2:-auto}
case "$great_wall_mode" in
    auto|historical|preview-ruined|preview-earthen|preview-stone) ;;
    *) echo 'Great Wall presentation must be auto, historical, preview-ruined, preview-earthen or preview-stone.' >&2; exit 2 ;;
esac
repo_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
python3 - "$1" <<'PY'
import hashlib, pathlib, sys
try:
    root = pathlib.Path(sys.argv[1]).resolve(strict=True)
except OSError:
    raise SystemExit('Original-data directory is missing or unreadable; no app launched.')
for relative, expected in (
    ('Cities/MPWall1.map', '8d9932ab051b80b140b533da641d9448cb8fc42b2359211933265940f81a0cf9'),
    ('Emperor.exe', '6373328bfc5c4886d9abc9544eb706e89e7d465b18176ea8205fe27aaee53c0e'),
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
print('MPWall1: City-v16 rule 4 / policy 2 / schema 20; normal menu, Empty City.')
print('Verified Type151 storage origins: (168,131).')
print('These saved source points remain protected; no original terrain-flag/editor operation is imported.')
PY
# Share the existing Release build, exact-name containment, complete preflight
# and fresh private settings/save/recovery root. Failed preflight launches no app.
exec sh "$repo_dir/tools/test_resgate_maps.sh" "$1" MPWall1 "$great_wall_mode"
