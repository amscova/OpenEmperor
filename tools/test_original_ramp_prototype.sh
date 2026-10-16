#!/bin/sh
# Isolated executable; never starts or replaces the production renderer.
set -eu
if [ "$#" -lt 1 ] || [ "$#" -gt 2 ]; then
    echo 'Usage: sh tools/test_original_ramp_prototype.sh DATA [--ui|--capture]' >&2
    exit 2
fi
mode=${2---ui}
case "$mode" in
    --ui|--capture) ;;
    *) echo 'Unknown original-ramp prototype mode' >&2; exit 2 ;;
esac
repo_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd -P)
data_dir=$(CDPATH= cd -- "$1" && pwd -P)
build_dir=${OE_ORIGINAL_RAMP_PROTOTYPE_BUILD_DIR-"$repo_dir/.local/build-original-ramp-prototype"}
cmake -S "$repo_dir" -B "$build_dir" -DCMAKE_BUILD_TYPE=Release -DOPENEMPEROR_USE_SYSTEM_SDL3=ON
cmake --build "$build_dir" --parallel 2 --target openemperor-original-ramp-prototype \
    openemperor-original-ramp-cell-tests openemperor-inspect
ctest --test-dir "$build_dir" -R '^original-ramp-cell-terrain$' --no-tests=error --output-on-failure
cd "$repo_dir"
exec python3 tools/check_original_ramp_prototype.py --run "${mode#--}" --data "$data_dir" --build "$build_dir"
