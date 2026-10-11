#!/bin/sh
set -eu
if [ "$#" -ne 1 ]; then
    echo 'Usage: sh tools/test_mp56_occupancy.sh <original-data-directory>' >&2
    exit 2
fi
repo_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
python3 - "$1" <<'PY'
import hashlib, pathlib, sys
root = pathlib.Path(sys.argv[1]).resolve(strict=True)
path = root / 'Cities' / 'MP56.map'
path.resolve(strict=True).relative_to(root)
if hashlib.sha256(path.read_bytes()).hexdigest() != '96e3bd832b40ae57a6e7526eb864cfa85a5bacd4dc3cad95aa7e4fff8dc17892':
    raise SystemExit('MP56 differs from the verified input revision; no app launched.')
print('MP56: City-v16 rule 4 / policy 2 / schema 20; normal menu, Empty City.')
print('Verified storage origins: type177 (52,93), (54,97), (51,94), (52,93); type178 (117,195).')
print('These points must remain protected; the complete selected-map check follows.')
print('Visual check: storage row 96, x 110-123; saved masonry and small gate at (111,96), replacing brown fallbacks.')
PY
# Share the existing build, contained-name check, full preflight and private root.
exec sh "$repo_dir/tools/test_resgate_maps.sh" "$1" MP56
