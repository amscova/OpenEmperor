# Testing OpenEmperor

Use the current source and your own original game files. Record the source
revision, dirty-worktree status, map, profile, rule version and save schema with
each result. A display version alone does not identify the behavior tested.
OpenEmperor saves are JSON documents; original Emperor savegames are unsupported.

## Build and automated checks

From the repository root, with the dependencies described in the
[build guide](../README.md):

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DOPENEMPEROR_USE_SYSTEM_SDL3=ON
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
```

For the complete macOS arm64 validation pipeline:

```sh
python3 tools/alpha_check.py --system-sdl --jobs 2
# Include the optional read-only original-data check and package validation:
python3 tools/alpha_check.py --system-sdl --jobs 2 --package --data /path/to/emperor
```

The default build directories are `build/`, `build-alpha-release/` and
`build-sanitize/`. Build, report and package destinations must be new paths;
existing outputs are preserved. Select isolated paths with `--build-dir`,
`--release-build-dir`, `--sanitize-build-dir`, `--report`,
`--package-build-dir` and `--package-dist-dir`. See the
[packaging guide](macos-packaging.md#alpha-acceptance) for a complete command.
The default report is `.local/reports/alpha-check.json`. The runner records the
actual registered-test inventory and JUnit results. It runs the registered tests,
100,000-tick deterministic endurance with real JSON roundtrips, render stress
and repeated menu/session lifecycles. The optional data check verifies the
original files it actually uses remain unchanged. On macOS LeakSanitizer is
disabled; project-owned texture counters provide a separate lifetime check.

Feature-specific original-data checks are additional evidence. Use the runner
documented with the affected feature, for example
[Ramps](city-v16-ramps.md), [Bridges](city-v16-bridges.md),
[Service coverage](city-v16-service-coverage.md) or
[joined ramp rendering](rendering/multi-ramp-integration.md). A finite scripted
scenario does not prove every layout, long-play outcome or original-game parity.

For a bounded original-map start and paid transport fixtures:

```sh
sh tools/test_map_playability.sh /path/to/emperor "Chang-an Zhou"
sh tools/test_map_playability.sh /path/to/emperor Handan
sh tools/test_map_playability.sh /path/to/emperor Badaling
```

Run one at a time. The launcher builds the current Release app and creates a
fresh ignored test root. New Sandbox uses City-v16 rule 4/policy 2 and Empty
City: scenery with zero Sandbox buildings, residents or walkers is expected.
Its optional A–E technical saves retain historical rule 3/policy 1. Load
`B-paid-courier` and press Space for a real Clay trip, or `C-paid-supply` for
the paid compact starter. An unavailable exact map or legal fixture reports
its preparation error rather than substituting a fabricated map. These finite
checks establish technical behavior, not full map or human acceptance; use
[road diagnostics](road-connectivity-diagnostics.md) to inspect a concrete route.

## Inspector layout regression

The registered `inspector-layout` test checks the shared layout at 1280×800,
1600×900 and 1920×1080 with UI scales 1 and 2, both document orders, large
lists, long wrapped hints and small resize bounds. `sandbox-software-view`
exercises actual production row selection, clipping, scroll limits and press
cancellation with a 34-building World. Run these alongside the existing
`sandbox-input-reliability`, `original-house-inspector-view`, operation/demolition,
modal and save-view checks when changing inspector geometry.

For the optional private OE41 reproduction, build
`openemperor-system-playtest-acceptance` and run its `--inspector-layout` mode
with DATA, RESOURCES, a fresh app root, a fresh capture directory and the explicit
paid checkpoint. Both output paths must be below `.local/oe42/`. Accepted inputs
are the existing `.local/oe41/run-01/journey-create-02/journey.save.json`
(Rule14/Xia/tick901, Market7) and the documented
`.local/original-economy/oe40/run-01/gate-c-release-lean-med-02/first-success.save.json`
(tick7333, 65 paid buildings including FireWatch). The diagnostic loads a working
copy through MenuSession; original input bytes remain protected. For example:

```sh
cmake --build build --target openemperor-system-playtest-acceptance
./build/openemperor-system-playtest-acceptance --inspector-layout \
  /path/to/emperor "$PWD/resources" \
  "$PWD/.local/oe42/check-app" "$PWD/.local/oe42/check-captures" \
  "$PWD/.local/oe41/run-01/journey-create-02/journey.save.json"
```

The matrix uses actual SDL window resizes, camera zooms 1/2/4, ordinary list/body
selection and wheel events in production SandboxView. It checks complete row
reachability and World equality, compares permitted operation clicks with the
same commands on an independent reference World, then restores the full saved
checkpoint. F5/Step/F9/F5 must reproduce canonical document bytes. Open the
captured top/end views and review text, fixed controls and hints before recording
visual acceptance. Scripted SDL/software evidence does not complete the separate
[human input checklist](testing-input-acceptance.md).

## Manual play and persistence

Use a separate test root so the exercise leaves personal settings and saves alone:

```sh
./build/openemperor --data /path/to/emperor --app-root "$PWD/.local/manual-test"
```

1. Create a supported city through the normal menu. Record the chosen profile
   and rules. A prepared starter must spend its normal budget without free goods,
   extra funds or unexplained simulation progress.
2. Observe actual deliveries, Service arrival and fully supplied House demand.
   Check treasury changes against construction, tax and maintenance rather than
   treating a moving walker or current stock count as proof of complete supply.
3. Expand using earned funds. Pause/resume a producer and change a permitted
   workforce priority; inspect workers, stocks, progress and active trips.
4. Save a test copy, remove a future road segment, observe safe waiting or
   rerouting, and repair it. Cargo and reservations must remain coherent.
5. F5 save, quit the whole application, restart the same build and load the save.
   It must open paused with the saved tick, funds, buildings, goods, population,
   operations, identities and active transport intact, then continue normally.

For safe demolition, use a disposable City-v11 rule-4 save. An older rule-3 save
requires **Load Sandbox → Enable demolition in a copy**; ordinary loading must
retain the old rule and leave its source bytes unchanged. Cancel Demolish once,
then remove an empty building and rebuild with a new ID. House removal must
remove its residents/workers. A stocked Warehouse must remain blocked: stop
upstream production, drain it through ordinary delivery/demand and wait for
trips to return before removal. Repeat with a drained Market, then save, fully
restart and verify deleted IDs stay absent. Demolition gives no refund.

The detailed [human input checklist](testing-input-acceptance.md) covers camera,
zoom, picking, drag cancellation, focus/layout/modal boundaries and Service
overlays. Keep its human results separate from queued SDL events and software
pixel checks. Report **NOT RUN**, **PASS** or **FAIL** for the observed scope.

## Packages and failure reports

`tools/package_macos.sh` builds and checks the local development app; see
[macOS packaging](macos-packaging.md) for dependency, relocation, signature and
resource rules. A local package check is not a published release, notarization,
Finder/Gatekeeper acceptance or an independent clean-Mac test.

Report exact steps, expected/actual behavior, the recorded version domains and
terminal errors. An affected OpenEmperor JSON save can make a failure reproducible.
Never attach original maps, SG3/.555 files, installers or decoded game images.
Review [known issues](../KNOWN_ISSUES.md) before interpreting a new result.
