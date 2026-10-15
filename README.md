# OpenEmperor for Windows

This is a **Windows x64 adaptation of [OpenEmperor](https://github.com/oliverjessner/OpenEmperor)** by Oliver Jessner, a clean-room, open-source reimplementation of *Emperor: Rise of the Middle Kingdom* written in C++20 with SDL3.

Upstream targets native macOS on Apple Silicon first. This fork makes the same project build and run natively on Windows 10/11, without Visual Studio, and tracks upstream so the game logic stays OpenEmperor's.

> **Not affiliated with** Oliver Jessner, Impressions Games, Sierra, Activision or GOG. This is a community fork.

## What this fork is

- **OpenEmperor's interpretation of the game.** The simulation, rules, rendering and file readers come from upstream. Where this fork and upstream differ in game behaviour, upstream is the reference.
- **A Windows port.** The changes here are for building, linking and running on Windows. They are listed below.
- **A rebase-friendly fork.** The `windows-port` branch is kept on top of upstream `main`. Upstream's own README is preserved unchanged as [README.upstream.md](README.upstream.md); it describes the alpha status, City rule sets, saves and the map browser in full, and still applies.

## What is different on Windows

| Area | Change |
|---|---|
| Toolchain | Builds with Clang (llvm-mingw, UCRT, x64) and Ninja. No Visual Studio needed. |
| Linking | Optional fully static link, so `openemperor.exe` needs only Windows system DLLs. |
| Stack size | Windows reserves 1 MiB of stack per thread against 8 MiB on macOS/Linux. CMake raises it to 8 MiB; without that, loading a map crashes with a stack overflow (`0xC00000FD`). |
| Tests | Tests that relaunch their own executable use `tests/ProcessCommand.h` to quote arguments correctly for `cmd.exe`. |
| Docs | [docs/windows-build.md](docs/windows-build.md) has exact build steps. |

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

Windows test results (Clang 22.1.8, SDL 3.4.10): **151 of 154** CTest cases pass.

- `texture-compatibility-production-pixels` and `texture-compatibility-sdl-pixels` fail under the software renderer used by tests. The interactive renderer is not affected.
- `map-corpus-runner` fails because its Python runner assumes POSIX pipes and shell stand-ins.

These are tracked in [docs/windows-build.md](docs/windows-build.md). Interactive play on Windows has had less testing than the automated suite, so expect rough edges and please report them.

## Contributing and upstream

- Windows-specific fixes belong here. Game-logic changes are better proposed upstream, so this fork doesn't diverge.
- The project is clean-room. Do not contribute decompiled code, proprietary source or original assets. See [AGENTS.md](AGENTS.md) for the rules.

## License

GPL-3.0, the same as upstream. See [LICENSE](LICENSE).
