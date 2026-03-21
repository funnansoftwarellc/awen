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
- LLVM 21 via Homebrew:
  ```sh
  brew install llvm@21
  ```
  After installing, add LLVM to your `PATH`:
  ```sh
  echo 'export PATH="/opt/homebrew/opt/llvm@21/bin:$PATH"' >> ~/.zprofile
  source ~/.zprofile
  ```

### Linux

- Ubuntu 25.10 (or compatible) — x86-64
- GCC (latest available) **or** Clang (latest via [apt.llvm.org](https://apt.llvm.org/))
- The provided [Dev Container](.devcontainer/Dockerfile) sets up a fully configured environment with both compilers, CMake, and Ninja. Using it is the recommended approach on Linux.

  To build without the Dev Container, install the required tools manually:
  ```sh
  sudo apt-get update
  sudo apt-get install -y cmake ninja-build git pkg-config python3 zip unzip tar
  ```
  Then install GCC or Clang following the instructions in the [Dockerfile](.devcontainer/Dockerfile).

### Windows

- Windows 10/11 — x86-64
- [Visual Studio 2022](https://visualstudio.microsoft.com/) with the **Desktop development with C++** workload (provides MSVC and the Windows SDK)
- CMake and Ninja (included with Visual Studio, or install separately)

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

| Preset | Compiler | Configuration |
|---|---|---|
| `x64-linux-gcc-debug` | GCC | Debug |
| `x64-linux-gcc-release` | GCC | Release |
| `x64-linux-clang-debug` | Clang | Debug |
| `x64-linux-clang-release` | Clang | Release |

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

| Preset | Configuration |
|---|---|
| `x64-windows-msvc-debug` | Debug |
| `x64-windows-msvc-release` | Release |

Run the following from a **Developer Command Prompt for VS 2022** (or after running `vcvarsall.bat amd64`):

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

Android builds target **arm64-v8a** and cross-compile from Linux using the [Android NDK](https://developer.android.com/ndk). The provided [Dev Container](.devcontainer/android/Dockerfile) sets up the required environment (Ubuntu 25.10 with Clang and NDK r29) and is the recommended approach.

Available presets:

| Preset | Configuration |
|---|---|
| `arm64-android-clang-debug` | Debug |
| `arm64-android-clang-release` | Release |

```sh
# Configure
cmake --preset arm64-android-clang-debug

# Build
cmake --build --preset arm64-android-clang-debug

# Install
cmake --build --preset arm64-android-clang-debug --target install
```

### WebAssembly

WebAssembly builds use [Emscripten](https://emscripten.org/) and cross-compile from Linux. The provided [Dev Container](.devcontainer/wasm/Dockerfile) (based on `emscripten/emsdk`) includes all required tooling and is the recommended approach.

Available presets:

| Preset | Configuration |
|---|---|
| `wasm32-emscripten-debug` | Debug |
| `wasm32-emscripten-release` | Release |

```sh
# Configure
cmake --preset wasm32-emscripten-debug

# Build
cmake --build --preset wasm32-emscripten-debug

# Install
cmake --build --preset wasm32-emscripten-debug --target install
```

The installed output is written to `build/<preset>/installed/`.

