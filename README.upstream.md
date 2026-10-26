# OpenEmperor

A clean-room reimplementation of _Emperor: Rise of the Middle Kingdom_ for
native macOS on Apple Silicon. **0.1.0-alpha.2** includes a playable city
sandbox and an optional original-economy experiment. Gameplay and visual
parity remain incomplete; original campaigns and savegames are unsupported.

You need your own installed or extracted Emperor game data. Original assets
are read directly and are never bundled.

## Build and run

Install Xcode Command Line Tools, CMake and OpenSSL 3. CMake fetches pinned
SDL3 and JSON dependencies when needed and uses system zlib.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES=arm64
cmake --build build --parallel
./build/openemperor
```

On first launch, select the Emperor data directory, then **New Sandbox**,
a supported map and a rules profile. Existing saves retain their rules.

For the optional original economy, choose **O Original economy**,
**Cities/Xia.map**, **Normal** and **Paid four-house starter**. Start the
sandbox and press **Space** to run it. The [player guide](docs/original-economy/player-handoff.md)
covers production, house development, saving and the remaining limitations.

## Controls

- **Space** pauses or resumes; **.** advances one paused tick.
- **Arrow keys** pan; the mouse wheel zooms; **Z** cycles 1×/2×/4×.
- **F5 / F9** save and load; **Escape** returns to the menu.
- **Help** lists the current profile's tools and shortcuts.

Saves and settings normally live in
`~/Library/Application Support/OpenEmperor/OpenEmperor/` on macOS.

See [known issues](KNOWN_ISSUES.md) for current limitations and
[documentation](docs/README.md) for gameplay, development, testing and packaging.
