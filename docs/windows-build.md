# Building on Windows (x64)

OpenEmperor builds natively on Windows 10/11 with Clang targeting the
MinGW-w64 UCRT runtime. Visual Studio is not required.

## Requirements

- CMake ≥ 3.24 and Ninja
- [llvm-mingw](https://github.com/mstorsjo/llvm-mingw) (UCRT, x86_64), e.g.
  `llvm-mingw-<date>-ucrt-x86_64`, with its `bin\` on `PATH`
- zlib and SDL3 ≥ 3.4.10 built for the same toolchain (or let CMake fetch
  the pinned SDL3 source by leaving `OPENEMPEROR_USE_SYSTEM_SDL3` off)
- OpenSSL 3 (libcrypto) for MinGW UCRT. The MSYS2 package
  `mingw-w64-ucrt-x86_64-openssl` works; it can be extracted without
  installing MSYS2.

## Configure and build

From a shell where `clang`, `cmake` and `ninja` resolve to the toolchain:

```sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ \
  -DZLIB_ROOT=<zlib prefix> \
  -DOPENSSL_ROOT_DIR=<openssl prefix> -DOPENSSL_USE_STATIC_LIBS=ON \
  -DCMAKE_EXE_LINKER_FLAGS=-static
# optional: -DOPENEMPEROR_USE_SYSTEM_SDL3=ON -DSDL3_DIR=<prefix>/lib/cmake/SDL3
cmake --build build --parallel
ctest --test-dir build -j8
```

`-DCMAKE_EXE_LINKER_FLAGS=-static` links libc++, libunwind, SDL3, zlib and
libcrypto into `openemperor.exe`, so the executable depends only on system
DLLs. Without it, copy `libc++.dll` and `libunwind.dll` from the toolchain's
`bin\` next to the executables.

## Running

```sh
build\openemperor.exe --data "C:\GOG Games\Emperor - Rise of the Middle Kingdom"
```

Point `--data` (or the main-menu data chooser) at the installed game folder,
not at the GOG installer. Settings, saves and recovery histories go to
`%APPDATA%\OpenEmperor\OpenEmperor\` (from `SDL_GetPrefPath`).

## Windows-specific notes

- The default thread stack on Windows is 1 MiB versus 8 MiB on macOS/Linux.
  CMake raises the reserve to 8 MiB on Windows; without it map loading
  crashes with `STATUS_STACK_OVERFLOW` (`0xC00000FD`).
- Tests that relaunch their own executable quote arguments through
  `tests/ProcessCommand.h`, since `std::system` uses `cmd.exe` on Windows.

## Known test gaps on Windows

Tested with llvm-mingw / Clang 22.1.8, SDL 3.4.10: 151 of 154 CTest cases pass.

- `texture-compatibility-production-pixels`, `texture-compatibility-sdl-pixels`:
  pixel samples read the clear colour under the software/dummy renderer.
  Under investigation; the interactive renderer is unaffected.
- `map-corpus-runner`: the Python runner uses `select()` on pipes and POSIX
  shell stand-in executables, neither of which works on Windows.
