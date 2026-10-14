#!/bin/sh
set -eu
if [ "$#" -lt 1 ] || [ "$#" -gt 2 ]; then
    echo 'Usage: sh tools/test_city_v16_bridges.sh DATA [--audit|--gameplay|--ui|--all]' >&2
    exit 2
fi
mode=${2---all}
case "$mode" in --audit|--gameplay|--ui|--all) ;; *) echo 'Unknown bridge test mode' >&2; exit 2 ;; esac
repo_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd -P)
original_data=$(CDPATH= cd -- "$1" && pwd -P)
private_dir="$repo_dir/.local/city-v16-bridges"
mkdir -p "$private_dir"
run_dir=$(mktemp -d "$private_dir/run-XXXXXX")
build_dir="$repo_dir/.local/reswall/release-build"
cmake -S "$repo_dir" -B "$build_dir" -DCMAKE_BUILD_TYPE=Release -DOPENEMPEROR_USE_SYSTEM_SDL3=ON
cmake --build "$build_dir" --parallel 2 --target openemperor openemperor-original-bridge-audit \
    openemperor-audit-original-occupancy openemperor-city-v16-bridges-fixture \
    openemperor-city-v16-playthrough-fixture openemperor-city-v16-longplay-fixture \
    openemperor-bridge-transport-tests openemperor-bridge-rendering-tests openemperor-sandbox-view-tests
python3 "$repo_dir/tools/run_city_v16_bridges.py" "$repo_dir" "$original_data" "$build_dir" "$run_dir" "$mode"
if [ "$mode" = --ui ]; then
    echo 'Isolated original-map Bridge V1 test. L -> 01-build-bridge -> Enter; the save starts paused.'
    echo 'B Bridge: drag from the first existing bank road to the opposite existing bank road.'
    echo 'Green = legal/affordable; amber = not enough funds; red = named blocker. F5 saves, F9 loads paused.'
    echo '1..8 water cells, straight X/Y, equal-height banks. Whole-span removal is unavailable in V1.'
    echo "Only test settings and saves are used: $run_dir/player-app"
    exec env -u SDL_VIDEO_DRIVER -u SDL_VIDEODRIVER -u SDL_RENDER_DRIVER \
        "$build_dir/openemperor" --data "$original_data" --app-root "$run_dir/player-app"
fi
