#!/bin/sh
set -eu
if [ "$#" -ne 1 ]; then
    echo 'Usage: sh tools/test_mp48_occupancy.sh <original-data-directory>' >&2
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
    ('Cities/MP48.map', '5e4a78c880fbf2cdff9286ce45c3effd4532c1986cb41c84715e3c981b8d510f'),
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
print('MP48: City-v16 rule 4 / policy 2 / schema 20; normal menu, Empty City.')
print('Verified Type186 storage origins: (104,80), (102,79), (105,80).')
print('These saved source points remain protected; no original copper/resource/editor operation is imported.')
PY
# Share the existing Release build, exact-name containment, complete preflight
# and fresh private settings/save/recovery root. Failed preflight launches no app.
exec sh "$repo_dir/tools/test_resgate_maps.sh" "$1" MP48
