#!/bin/sh
set -eu
if [ "$#" -lt 1 ] || [ "$#" -gt 2 ]; then
    echo 'Usage: sh tools/test_city_v16_bridges.sh DATA [--audit|--gameplay|--ui|--ui-build|--all]' >&2
    exit 2
fi
mode=${2---all}
case "$mode" in --audit|--gameplay|--ui|--ui-build|--all) ;; *) echo 'Unknown bridge test mode' >&2; exit 2 ;; esac
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
if [ "$mode" = --ui ] || [ "$mode" = --ui-build ]; then
    player_save=02-outbound.json
    if [ "$mode" = --ui-build ]; then
        player_save=01-build-bridge.json
        echo 'Build your own bridge: rule 5 / policy 3 / schema 21, paused before construction.'
        echo 'Press B or click B Bridge. Drag from an existing road on one bank to the road across the small stream.'
        echo 'Cyan marks a starting road; green preview is affordable, amber needs funds, red explains a blocker.'
        echo 'Release to pay $60 for this one-cell span. Click Continue to start the Clay delivery.'
    else
        echo 'Opening the paid Bridge V1 save directly: rule 5 / policy 3 / schema 21, paused at tick 56.'
        echo 'The built bridge is centered; a loaded Clay courier is on its deck. Click Continue to watch delivery and return.'
    fi
    echo 'F5 saves, F9 loads paused. L opens the save list only in the main menu.'
    echo '1..8 water cells, straight X/Y, equal-height banks. Whole-span removal is unavailable in V1.'
    echo "Only the test save is used: $run_dir/player-app/saves/$player_save"
    echo "To build the bridge yourself, load: $run_dir/player-app/saves/01-build-bridge.json"
    exec env -u SDL_VIDEO_DRIVER -u SDL_VIDEODRIVER -u SDL_RENDER_DRIVER \
        "$build_dir/openemperor" --data "$original_data" \
        --load-sandbox "$run_dir/player-app/saves/$player_save"
fi
