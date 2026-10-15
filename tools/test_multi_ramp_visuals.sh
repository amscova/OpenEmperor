#!/bin/sh
set -eu
if [ "$#" -lt 1 ] || [ "$#" -gt 2 ]; then
    echo 'Usage: sh tools/test_multi_ramp_visuals.sh DATA [--ui|--capture|--regression]' >&2
    exit 2
fi
mode=${2---ui}
case "$mode" in --ui|--capture|--regression) ;; *) echo 'Unknown multi-ramp mode' >&2; exit 2 ;; esac
repo_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd -P)
data_dir=$(CDPATH= cd -- "$1" && pwd -P)
build_dir="$repo_dir/.local/reswall/release-build"
cmake -S "$repo_dir" -B "$build_dir" -DCMAKE_BUILD_TYPE=Release -DOPENEMPEROR_USE_SYSTEM_SDL3=ON
cmake --build "$build_dir" --parallel 2 --target openemperor openemperor-city-v16-ramps-fixture \
    openemperor-multi-ramp-visual-fixture openemperor-authored-terrain-side-tests openemperor-ramp-rendering-tests
cd "$repo_dir"
exec python3 tools/run_multi_ramp_visuals.py "$repo_dir" "$data_dir" "$build_dir" "$mode"
