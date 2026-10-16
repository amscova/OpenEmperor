#!/bin/sh
# Explicit developer launcher; never changes the normal application's defaults.
set -eu
repo=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd -P)
cd "$repo"
if [ "$#" -gt 4 ]; then
    echo 'Usage: sh tools/play_original_goods.sh [DATA [MAP [SAVE [--load]]]]' >&2
    exit 2
fi
data=${1:-.local/gog-extracted/app}
map=${2:-Cities/Xia.map}
save=${3:-.local/original-economy/player/rule9.json}
build=.local/original-economy/oe13/play-build
cmake -S . -B "$build" -DCMAKE_BUILD_TYPE=Release -DOPENEMPEROR_USE_SYSTEM_SDL3=ON
cmake --build "$build" --parallel 2 --target openemperor-original-goods-play
if [ "$#" -eq 4 ]; then
    if [ "$4" != '--load' ]; then echo 'The fourth argument must be --load' >&2; exit 2; fi
    exec "$build/openemperor-original-goods-play" "$data" "$map" "$save" --load
fi
exec "$build/openemperor-original-goods-play" "$data" "$map" "$save"
