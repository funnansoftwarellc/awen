set(VCPKG_TARGET_ARCHITECTURE arm64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)
set(VCPKG_CMAKE_SYSTEM_NAME Android)
set(VCPKG_CMAKE_SYSTEM_VERSION 28)
set(VCPKG_MAKE_BUILD_TRIPLET "--host=aarch64-linux-android")
set(VCPKG_CMAKE_CONFIGURE_OPTIONS -DANDROID_ABI=arm64-v8a)
set(VCPKG_BUILD_TYPE release)

# Pass through env vars that affect NDK discovery so vcpkg cache keys are correct.
set(VCPKG_ENV_PASSTHROUGH_UNTRACKED ANDROID_NDK_HOME ANDROID_HOME LOCALAPPDATA)

# Resolve NDK — same priority order as cmake/android/toolchain.cmake.
set(_NDK_VERSION "29.0.14206865")

if(DEFINED ENV{ANDROID_NDK_HOME} AND EXISTS "$ENV{ANDROID_NDK_HOME}")
    set(_NDK_HOME "$ENV{ANDROID_NDK_HOME}")
elseif(DEFINED ENV{ANDROID_HOME} AND EXISTS "$ENV{ANDROID_HOME}/ndk/${_NDK_VERSION}")
    set(_NDK_HOME "$ENV{ANDROID_HOME}/ndk/${_NDK_VERSION}")
elseif(DEFINED ENV{LOCALAPPDATA} AND EXISTS "$ENV{LOCALAPPDATA}/Android/Sdk/ndk/${_NDK_VERSION}")
    set(_NDK_HOME "$ENV{LOCALAPPDATA}/Android/Sdk/ndk/${_NDK_VERSION}")
elseif(EXISTS "/opt/android/sdk/ndk/${_NDK_VERSION}")
    set(_NDK_HOME "/opt/android/sdk/ndk/${_NDK_VERSION}")
endif()

if(NOT _NDK_HOME)
    message(FATAL_ERROR
        "Android NDK ${_NDK_VERSION} not found.\n"
        "Install it via Android Studio SDK Manager, or set ANDROID_NDK_HOME / ANDROID_HOME.")
endif()

set(VCPKG_CHAINLOAD_TOOLCHAIN_FILE "${_NDK_HOME}/build/cmake/android.toolchain.cmake")
