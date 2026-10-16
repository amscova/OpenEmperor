# OpenEmperor for Windows

A **Windows x64 build of [OpenEmperor](https://github.com/oliverjessner/OpenEmperor)** by Oliver Jessner, a clean-room, open-source reimplementation of *Emperor: Rise of the Middle Kingdom* written in C++20 with SDL3.

Upstream targets native macOS on Apple Silicon first. This fork makes the same project build, test and run natively on Windows 10/11 without Visual Studio. It is kept level with upstream `main`: **the game itself is upstream's, unchanged.**

> **Not affiliated with** Oliver Jessner, Impressions Games, Sierra, Activision or GOG. This is a community fork.

## How this fork relates to upstream

- **Same game, same rules.** Simulation, rules, rendering, saves, file readers and documentation come from upstream. Where the two ever differ in behaviour, upstream is the reference. For what the project is, its alpha status, the City rule sets, saves and the map browser, read upstream's README, kept unchanged as [README.upstream.md](README.upstream.md).
- **Level with upstream.** `windows-port` is upstream `main` plus the small Windows layer below. Last synced with upstream `41091b6` (2026-10-16, "OE-13 typed goods ledger foundation"). The fork is merged forward with `git merge upstream/main`; upstream's version of any shared file wins.
- **Nothing here is a fork-only feature.** Earlier fork experiments with original-style housing rules were retired when upstream built its own, more thoroughly verified implementation of the same area.

## What is different on Windows

| Area | Change |
|---|---|
| Toolchain | Builds with Clang (llvm-mingw, UCRT, x64) and Ninja. No Visual Studio needed. |
| Linking | Optional fully static link, so `openemperor.exe` needs only Windows system DLLs. |
| Stack size | Windows reserves 1 MiB of stack per thread against 8 MiB on macOS/Linux. CMake raises it to 8 MiB; without that, loading a map crashes with a stack overflow (`0xC00000FD`). |
| Rendering | `renderer/TextureCompatibility.h` also admits SDL 3.4.10 for the software-renderer texture workaround (measured on Windows, see [docs/windows-build.md](docs/windows-build.md)). |
| Tests | Tests that relaunch their own executable quote arguments through `tests/ProcessCommand.h` for `cmd.exe`; Python tests use portable paths and byte-exact fixtures; the map corpus runner works without `select()` on pipes. |
| Line endings | `.gitattributes` keeps the few files whose hashes upstream's evidence tests pin as LF, because Git for Windows' `core.autocrlf` would otherwise change them. |
| Docs | [docs/windows-build.md](docs/windows-build.md) has exact build steps. One extra static-analysis note, [docs/reverse/original-desirability-wall-shielding-addendum.md](docs/reverse/original-desirability-wall-shielding-addendum.md), records observations about original desirability wall shielding. |

Nothing in the game's behaviour has been changed for Windows.

## Requirements

- Windows 10 or 11, x64.
- A legitimate copy of *Emperor: Rise of the Middle Kingdom*. **The game files are not included and never will be.** OpenEmperor reads the installed game and does not redistribute it or decoded copies of it. This fork is developed against the GOG release (2.0.0.2).
- To build: CMake 3.24+, Ninja, llvm-mingw, zlib, SDL3 3.4.10+ and OpenSSL 3.

## Build

See [docs/windows-build.md](docs/windows-build.md) for details. In short, from a shell where `clang`, `cmake` and `ninja` resolve to the toolchain:

```sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ \
  -DZLIB_ROOT=<zlib prefix> \
  -DOPENSSL_ROOT_DIR=<openssl prefix> -DOPENSSL_USE_STATIC_LIBS=ON \
  -DCMAKE_EXE_LINKER_FLAGS=-static
cmake --build build --parallel
ctest --test-dir build -j8
```

## Run

```sh
build\openemperor.exe --data "C:\GOG Games\Emperor - Rise of the Middle Kingdom"
```

Point `--data` at the installed game folder, not the GOG installer. Settings, saves and recovery histories are stored in `%APPDATA%\OpenEmperor\OpenEmperor\`.

## Status

This is an alpha, inheriting upstream's status: it is **not a complete recreation of Emperor**. See [KNOWN_ISSUES.md](KNOWN_ISSUES.md) and [README.upstream.md](README.upstream.md).

Windows test results (Clang 22.1.8, SDL 3.4.10, at upstream `41091b6`): **all 186 CTest cases pass**. Seven of them read the installed game's model file and skip unless configured with `-DOPENEMPEROR_ORIGINAL_MODEL_TEST_ROOT=<game folder>`; with it they pass too. A `--sandbox-check` smoke run on `Cities/Xia.map` succeeds. Interactive play on Windows has had less testing than the automated suite, so expect rough edges and please report them.

## Contributing and upstream

- Windows-specific fixes belong here. Game-logic changes belong upstream, so this fork doesn't diverge.
- The project is clean-room. Do not contribute decompiled code, proprietary source or original assets. See [AGENTS.md](AGENTS.md) for the rules.

## License

GPL-3.0, the same as upstream. See [LICENSE](LICENSE).
