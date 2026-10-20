#!/bin/sh
set -eu
repo=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd -P)
cd "$repo"
if [ "$#" -eq 1 ] && [ "$1" = '--help' ]; then
    echo 'Usage: sh tools/play_original_economy.sh [DATA [MAP [SAVE]]] [--load] [--difficulty very-easy|easy|normal|hard|very-hard]'
    exit 0
fi
data=.local/gog-extracted/app
map=Cities/Xia.map
save=.local/original-economy/player/rule10-revision27.json
position=0
while [ "$#" -gt 0 ]; do
    case "$1" in --*) break ;; esac
    position=$((position + 1))
    case "$position" in
        1) data=$1 ;;
        2) map=$1 ;;
        3) save=$1 ;;
        *) echo 'Expected at most DATA MAP SAVE before options; use --help' >&2; exit 2 ;;
    esac
    shift
done
build=.local/original-economy/completion/play-build
cmake -S . -B "$build" -DCMAKE_BUILD_TYPE=Release -DOPENEMPEROR_USE_SYSTEM_SDL3=ON
cmake --build "$build" --parallel 2 --target openemperor-original-production-play
exec "$build/openemperor-original-production-play" "$data" "$map" "$save" "$@"
