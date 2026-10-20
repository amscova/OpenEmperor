# OpenEmperor

OpenEmperor is a clean-room reimplementation of _Emperor: Rise of the Middle Kingdom_, currently targeting native macOS on Apple Silicon. It reads the original game files supplied by the player; original assets and decoded images are never bundled.

The current development version is **0.1.0-alpha.2**. It offers an OpenEmperor-authored city sandbox and an optional model-driven original-economy experiment. Original campaigns and original savegames are unsupported; complete gameplay and visual parity remain **PARTIAL**. See [known issues](KNOWN_ISSUES.md) and [documentation](docs/README.md).

## Build and run

Install Xcode Command Line Tools, CMake and OpenSSL 3. CMake uses system zlib and either an installed compatible nlohmann/json or its pinned dependency. The default SDL3 dependency is fetched at a pinned revision; for an installed SDL3 3.4.10 or newer, add `-DOPENEMPEROR_USE_SYSTEM_SDL3=ON`.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES=arm64
cmake --build build --parallel
./build/openemperor
```

Reuse the same build directory for subsequent builds. On first launch, select an installed or extracted Emperor data directory, rather than the GOG installer file. Choose **New Sandbox**, a supported map and a rules profile. The prepared settlement is built with ordinary paid commands at tick zero.

The general menu default remains City-v11 rule 3. Selecting City-v16 starts rule 4 with original ground roads; Bridges rule 5 and Ramps rule 6 are explicit choices. Loading an existing save preserves its profile and rule version.

**Original economy** (menu shortcut **O**) offers the optional Rule10 experiment.
Choose **Cities/Xia.map**, **Normal** and **Paid four-house starter**, then
**Start sandbox** and **Space**. Production, deliveries and house evolution run
in the real city; manual **F5/F9** and optional recovery are supported.
Verified House-stage sprites coexist with non-house fallbacks and authored
rules. Follow the [short player check](docs/original-economy/player-handoff.md)
or see the [final feature comparison](docs/original-economy/final-comparison.md).

Start City-v16 directly:

```sh
./build/openemperor --data /path/to/emperor \
  --sandbox Cities/Xia.map --sandbox-rules sandbox-city-v16 --sandbox-demo
```

For the playable experiment using original ramp sprites 233–244 and a separate
source-cell height model:

```sh
sh tools/test_original_ramp_prototype.sh .local/gog-extracted/app
```

See the [original ramp cell prototype](docs/rendering/original-ramp-cell-prototype.md)
for controls, measured visuals and the production integration decision.

For the existing production isolated/adjacent ramp graphics test:

```sh
sh tools/test_multi_ramp_visuals.sh .local/gog-extracted/app
```

That starter builds current Release, creates disposable authored terrain and opens a normal paid four-ramp city. See [ramp presentation](docs/rendering/multi-ramp-integration.md) for software captures and regression modes.
The [original ramp asset audit](docs/rendering/original-ramp-asset-audit.md) separates verified original sprite identities from authored-link rendering and records the remaining native-fit limit.

## Playing and saves

The city includes Food and Pottery production, Markets, Service visits, population and workforce controls, fire safety, desirability, water, health, taxes and maintenance. [City systems](docs/city-systems.md) explains their shared rules; [City-v16](docs/city-v16.md) covers upkeep and current rule versions. [The sandbox guide](docs/gameplay-sandbox.md) also records older selectable profiles and save compatibility.

- **Space** pauses or resumes; **.** advances one paused tick.
- **WASD/arrows** pan; the mouse wheel zooms; **Z** cycles 1x/2x/4x; **R** refits.
- **F5/F9** save/load; **Escape/Menu** retains a menu-managed session for **Enter Continue**.
- **Help** lists the current profile's building tools and controls.
- **F1** shows diagnostics; **F6** compares road materials. These controls change presentation only.

Manual saves, settings and bounded recovery history use SDL's per-user preference directory, usually `~/Library/Application Support/OpenEmperor/OpenEmperor/` on macOS. Original game data stays read-only. Recovery loads a separate branch; older profiles and schemas are not silently upgraded. Explicit City-v11 copy upgrades preserve their source saves.

One exactly fingerprinted GOG-derived asset set automatically enables compatible walker, building and road metadata. Other versions retain presentation fallbacks; custom visual JSON profiles remain optional session overrides. Exact recognition is not a claim of original-game visual fidelity.

## Tests and macOS app

```sh
ctest --test-dir build --output-on-failure

# Build and verify a local arm64 app and ZIP.
sh tools/package_macos.sh
open dist/OpenEmperor.app
```

Packaging uses `build-macos-app/` and writes the bundle, ZIP, checksums and report to `dist/`. It requires installed SDL3, OpenSSL 3 and nlohmann/json. The local app is ad-hoc signed, without Developer ID signing or notarization.

[Testing](docs/testing.md) documents Debug/sanitizer, endurance, original-data and manual checks. [macOS packaging](docs/macos-packaging.md) covers bundle verification. Automated checks, native input delivery and human gameplay acceptance are separate results.

## Developer tools and research

```sh
./build/openemperor --data /path/to/emperor --browse-assets
./build/openemperor-inspect /path/to/file.sg3 --summary
./build/openemperor-map-inspect --data /path/to/emperor --list --json
./build/openemperor --data /path/to/emperor --map-debug Cities/Xia.map
```

Use the relevant guide for advanced modes:

- [Architecture](docs/architecture.md) and [scene manifests](docs/scene-format.md).
- [Stored map graphics](docs/stored-graphics-preview.md), [terrain bindings](docs/terrain-bindings.md) and [map playability](docs/map-playability-acceptance.md).
- [Visual profiles](docs/building-visual-profile.md), [road materials](docs/road-visual-profile.md) and [walker profiles](docs/walker-visual-profile.md).
- [Reverse-engineering evidence](docs/reverse/README.md), including [map format](docs/reverse/map-format.md) and [graphics IDs](docs/reverse/graphics-id.md).

Reverse-engineering notes distinguish observed source facts, supported interpretation and authored preview behavior. A successful saved-map render does not prove original first-draw composition or gameplay parity. Historical published release notes remain under [releases](docs/releases/0.1.0-alpha.1.md).
