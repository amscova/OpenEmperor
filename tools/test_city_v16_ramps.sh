#!/bin/sh
set -eu
if [ "$#" -lt 1 ] || [ "$#" -gt 2 ]; then
    echo 'Usage: sh tools/test_city_v16_ramps.sh DATA [--audit|--gameplay|--ui|--all]' >&2
    exit 2
fi
mode=${2---all}
case "$mode" in --audit|--gameplay|--ui|--all) ;; *) echo 'Unknown ramp test mode' >&2; exit 2 ;; esac
repo_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd -P)
original_data=$(CDPATH= cd -- "$1" && pwd -P)
private_dir="$repo_dir/.local/city-v16-ramps"
mkdir -p "$private_dir"
run_dir=$(mktemp -d "$private_dir/run-XXXXXX")
build_dir="$repo_dir/.local/reswall/release-build"
cmake -S "$repo_dir" -B "$build_dir" -DCMAKE_BUILD_TYPE=Release -DOPENEMPEROR_USE_SYSTEM_SDL3=ON
cmake --build "$build_dir" --parallel 2 --target openemperor openemperor-original-height-audit \
    openemperor-audit-original-occupancy openemperor-city-v16-ramps-fixture \
    openemperor-ramp-transport-tests openemperor-ramp-rendering-tests openemperor-sandbox-view-tests \
    openemperor-original-bridge-audit openemperor-city-v16-bridges-fixture \
    openemperor-city-v16-playthrough-fixture openemperor-city-v16-longplay-fixture \
    openemperor-bridge-transport-tests openemperor-bridge-rendering-tests
python3 "$repo_dir/tools/run_city_v16_ramps.py" "$repo_dir" "$original_data" "$build_dir" "$run_dir" "$mode"
if [ "$mode" = --ui ]; then
    echo 'Opening independently authored terrain: rule 6 / policy 3 / schema 22, paused before Ramp construction.'
    echo 'Press P or click P Ramp; drag (115,114), height 1 -> (116,114), height 0. Release the green $40 preview.'
    echo 'Continue starts actual climbing Clay transport and return through the Ramp and existing Bridge. F5 saves; F9 loads paused.'
    echo 'Original-map Ramp acceptance remains unavailable; original heights/permissions are unchanged.'
    exec "$build_dir/openemperor" --data "$run_dir/authored-data" \
        --load-sandbox "$run_dir/player-app/saves/01-build-ramp.json"
fi
