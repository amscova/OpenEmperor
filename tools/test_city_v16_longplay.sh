#!/bin/sh
set -eu
if [ "$#" -ne 2 ] && [ "$#" -ne 3 ]; then
    echo 'Usage: sh tools/test_city_v16_longplay.sh DATA --fresh | DATA --save /explicit/save/path' >&2
    exit 2
fi
case "$2:$#" in --fresh:2|--save:3) ;; *) echo 'Use --fresh or --save with one explicitly supplied file.' >&2; exit 2 ;; esac
repo_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd -P)
original_data=$(CDPATH= cd -- "$1" && pwd -P)
private_dir="$repo_dir/.local/city-v16-longplay"
mkdir -p "$private_dir"
run_dir=$(mktemp -d "$private_dir/run-XXXXXX")
build_dir="$repo_dir/.local/reswall/release-build"
cmake -S "$repo_dir" -B "$build_dir" -DCMAKE_BUILD_TYPE=Release -DOPENEMPEROR_USE_SYSTEM_SDL3=ON
cmake --build "$build_dir" --parallel 2 --target openemperor-city-v16-longplay-fixture openemperor-city-v16-playthrough-fixture openemperor-audit-original-occupancy
exec python3 "$repo_dir/tools/run_city_v16_longplay.py" "$repo_dir" "$original_data" "$build_dir" "$run_dir" "$2" "${3-}"
