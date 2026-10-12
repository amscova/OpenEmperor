#!/bin/sh
set -eu
if [ "$#" -ne 1 ]; then
    echo 'Usage: sh tools/test_mp34_occupancy.sh <original-data-directory>' >&2
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
    ('Cities/MP34.map', '356285c0d09cd6cac1234f67c7e22dd3062d985dd485c1994f156a2f7aa38a47'),
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
print('MP34: City-v16 rule 4 / policy 2 / schema 20; normal menu, Empty City.')
print('Verified storage origins: type179 (151,118), (133,156), (129,151), (127,142), (163,127).')
print('Type164 (110,50), (113,52), (77,146). These saved points remain protected.')
print('Terrain and neighboring permissions are checked independently; no original fishing/tree function is imported.')
PY
# Share the existing build, contained-name check, full preflight and private root.
exec sh "$repo_dir/tools/test_resgate_maps.sh" "$1" MP34
