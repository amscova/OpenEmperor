#!/bin/sh
set -eu
if [ "$#" -lt 1 ] || [ "$#" -gt 2 ]; then
    echo 'Usage: sh tools/test_city_v16_service_coverage.sh DATA [--benchmark|--ui|--all]' >&2
    exit 2
fi
mode=${2-diagnostics}
case "$mode" in diagnostics|--benchmark|--ui|--all) ;; *) echo 'Unknown mode' >&2; exit 2 ;; esac
repo_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd -P)
original_data=$(CDPATH= cd -- "$1" && pwd -P)
private_dir="$repo_dir/.local/city-v16-service-coverage"
mkdir -p "$private_dir"
run_dir=$(mktemp -d "$private_dir/run-XXXXXX")
build_dir="$repo_dir/.local/reswall/release-build"
cmake -S "$repo_dir" -B "$build_dir" -DCMAKE_BUILD_TYPE=Release -DOPENEMPEROR_USE_SYSTEM_SDL3=ON
cmake --build "$build_dir" --parallel 2 --target openemperor openemperor-city-v16-longplay-fixture openemperor-city-v16-playthrough-fixture openemperor-audit-original-occupancy openemperor-sandbox-view-tests
python3 "$repo_dir/tools/run_city_v16_service_coverage.py" "$repo_dir" "$original_data" "$build_dir" "$run_dir" "$mode"
case "$mode" in
    --ui|--all)
        echo 'Isolated paid18-House Xia city. L Load Save, select01-service-baseline, Open selected (Enter), then V / V Service.'
        echo 'Green C: current coverage covers next demand. Amber !: expires by then unless revisited. Red X: missing now.'
        echo 'Future visits are unknown. Wheel zoom over the district; arrows pan. F3 + V retains walker role cycling.'
        echo "Only test settings/saves are used: $run_dir/player-app"
        exec "$build_dir/openemperor" --data "$original_data" --app-root "$run_dir/player-app"
        ;;
esac
