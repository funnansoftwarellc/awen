# awen

![awen banner](banner.png)

2D game engine developed to explore the latest C++ has to offer.

## Table of Contents

- [Prerequisites](#prerequisites)
- [Cloning](#cloning)
- [Building](#building)
  - [macOS](#macos)
  - [Linux](#linux)
  - [Windows](#windows)
  - [Android](#android)
  - [WebAssembly](#webassembly)

---

## Prerequisites

### All Platforms

- [Git](https://git-scm.com/)
- [CMake](https://cmake.org/) 3.25 or later
- [Ninja](https://ninja-build.org/)
- [vcpkg](https://vcpkg.io/) — must be cloned into the `vcpkg/` directory at the project root (see [Cloning](#cloning))

### macOS

- macOS 14 (Sonoma) or later, Apple Silicon (arm64)
- [Homebrew](https://brew.sh/)
- LLVM 22 via Homebrew:
  ```sh
  brew install llvm@22
  ```
  After installing, add LLVM to your `PATH`:
  ```sh
  echo 'export PATH="/opt/homebrew/opt/llvm@22/bin:$PATH"' >> ~/.zprofile
  source ~/.zprofile
  ```

### Linux

- Ubuntu 26.04 (or compatible) — x86-64 or arm64
- GCC 16 **or** Clang 22 (via [apt.llvm.org](https://apt.llvm.org/))
- The provided [Dev Container](.devcontainer/Dockerfile) sets up a fully configured environment with native Linux, Android, and WebAssembly toolchains. Using it is the recommended approach on Linux.

  To build without the Dev Container, install the required tools manually:
  ```sh
  sudo apt-get update
  sudo apt-get install -y cmake ninja-build git pkg-config python3 zip unzip tar
  ```
  Then install GCC 16 or Clang 22 following the instructions in the [Dockerfile](.devcontainer/Dockerfile).

### Windows

- Windows 10/11 — x86-64
- **MSVC / Visual Studio generator:** [Visual Studio 2022](https://visualstudio.microsoft.com/) with the **Desktop development with C++** workload (provides MSVC and the Windows SDK); CMake and Ninja are included
- **MinGW GCC:** [MSYS2](https://www.msys2.org/) with the `mingw-w64-ucrt-x86_64-gcc` and `mingw-w64-ucrt-x86_64-ninja` packages (UCRT64 environment); run builds from the MSYS2 UCRT64 shell

---

## Cloning

Clone the repository and bootstrap vcpkg:

```sh
git clone https://github.com/funnansoftwarellc/awen.git
cd awen
git clone https://github.com/microsoft/vcpkg.git
```

Bootstrap vcpkg for your platform:

**macOS / Linux:**
```sh
./vcpkg/bootstrap-vcpkg.sh
```

**Windows (PowerShell):**
```powershell
.\vcpkg\bootstrap-vcpkg.bat
```

---

## Building

All build steps use CMake presets. Replace `<preset>` with the preset name appropriate for your platform and configuration (see tables below).

### macOS

Available presets:

| Preset | Configuration |
|---|---|
| `arm64-osx-clang-debug` | Debug |
| `arm64-osx-clang-release` | Release |

```sh
# Configure
cmake --preset arm64-osx-clang-debug

# Build
cmake --build --preset arm64-osx-clang-debug

# Test
ctest --preset arm64-osx-clang-debug

# Install
cmake --build --preset arm64-osx-clang-debug --target install
```

### Linux

Available presets:

| Preset | Compiler | Architecture | Configuration |
|---|---|---|---|
| `arm64-linux-gcc-release` | GCC | arm64 | Release |
| `x64-linux-gcc-debug` | GCC | x64 | Debug |
| `x64-linux-gcc-release` | GCC | x64 | Release |
| `x64-linux-clang-debug` | Clang | x64 | Debug |
| `x64-linux-clang-release` | Clang | x64 | Release |

The `arm64-linux-gcc-release` preset is for building natively on an arm64 Linux host. The devcontainer targets x64 presets.

Test presets support filtered runs: append `-unit` to run only unit tests, or `-graphics` for only graphics tests (e.g. `ctest --preset x64-linux-clang-debug-unit`).

```sh
# Configure
cmake --preset x64-linux-clang-debug

# Build
cmake --build --preset x64-linux-clang-debug

# Test
ctest --preset x64-linux-clang-debug

# Install
cmake --build --preset x64-linux-clang-debug --target install
```

### Windows

Available presets:

| Preset | Toolchain | Configuration |
|---|---|---|
| `x64-windows-msvc-debug` | MSVC | Debug |
| `x64-windows-msvc-release` | MSVC | Release |
| `x64-windows-mingw-gcc-debug` | MinGW GCC | Debug |
| `x64-windows-mingw-gcc-release` | MinGW GCC | Release |
| `x64-windows-visualstudio` | MSVC (VS generator) | Debug + Release |

MSVC presets require a **Developer Command Prompt for VS 2022** (or `vcvarsall.bat amd64`). MinGW presets require the MSYS2 UCRT64 shell with GCC and Ninja in `PATH`. The `x64-windows-visualstudio` preset generates a Visual Studio 2022 solution — open `build/x64-windows-visualstudio/` in Visual Studio to build and test.

Test presets for MSVC and MinGW support filtered runs: append `-unit` or `-graphics` (e.g. `ctest --preset x64-windows-msvc-debug-unit`).

Run the following MSVC commands from a **Developer Command Prompt for VS 2022** (or after running `vcvarsall.bat amd64`):

```bat
rem Configure
cmake --preset x64-windows-msvc-release

rem Build
cmake --build --preset x64-windows-msvc-release

rem Test
ctest --preset x64-windows-msvc-release

rem Install
cmake --build --preset x64-windows-msvc-release --target install
```

The installed binary is written to `build/<preset>/installed/bin/`.

### Android

Android builds cross-compile using the [Android NDK](https://developer.android.com/ndk). The provided [Dev Container](.devcontainer/Dockerfile) sets up the required environment (Ubuntu 26.04 with Clang and the Android NDK) and is the recommended approach for Linux hosts.

Available presets:

| Preset | Host | Target ABI | Configuration |
|---|---|---|---|
| `x64-linux-clang-arm64-android-debug` | Linux x64 | arm64-v8a | Debug |
| `x64-linux-clang-arm64-android-release` | Linux x64 | arm64-v8a | Release |
| `x64-windows-clang-arm64-android-debug` | Windows x64 | arm64-v8a | Debug |
| `x64-windows-clang-arm64-android-release` | Windows x64 | arm64-v8a | Release |
| `x64-windows-clang-x64-android-debug` | Windows x64 | x86-64 | Debug (configure only) |
| `x64-windows-clang-x64-android-release` | Windows x64 | x86-64 | Release (configure only) |

```sh
# Configure
cmake --preset x64-linux-clang-arm64-android-debug

# Build
cmake --build --preset x64-linux-clang-arm64-android-debug

# Install
cmake --build --preset x64-linux-clang-arm64-android-debug --target install
```

### WebAssembly

WebAssembly builds use [Emscripten](https://emscripten.org/) and cross-compile from Linux. The provided [Dev Container](.devcontainer/Dockerfile) includes all required tooling and is the recommended approach.

Available presets:

| Preset | Host | Configuration |
|---|---|---|
| `x64-linux-wasm-debug` | Linux x64 | Debug |
| `x64-linux-wasm-release` | Linux x64 | Release |
| `arm64-linux-wasm-debug` | Linux arm64 | Debug |
| `arm64-linux-wasm-release` | Linux arm64 | Release |

```sh
# Configure
cmake --preset x64-linux-wasm-debug

# Build
cmake --build --preset x64-linux-wasm-debug

# Install
cmake --build --preset x64-linux-wasm-debug --target install
```

The installed output is written to `build/<preset>/installed/`.

