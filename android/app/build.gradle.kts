import java.util.Properties

plugins {
    id("com.android.application")
}

// Detect host OS to select the correct vcpkg host triplet.
val vcpkgHostTriplet = if (System.getProperty("os.name").contains("Windows", ignoreCase = true)) {
    "x64-windows-static-md"
} else {
    "x64-linux-release"
}

// Read sdk.dir from local.properties (written by cmake/android/toolchain.cmake with
// forward slashes) so we can pass the NDK path to cmake with forward slashes.
// AGP's NdkHandler uses Java's File.absolutePath which produces backslashes on Windows;
// cmake 4.x strict string parsing then rejects \U, \a, etc. in the generated
// CMakeSystem.cmake.  Passing the same path in forward-slash form last on the command
// line overrides AGP's backslash value (cmake last-write-wins for cache entries).
val ndkVersion = "29.0.14206865"
val ndkDirForCmake: String = run {
    val propsFile = rootDir.resolve("local.properties")
    if (propsFile.exists()) {
        val props = Properties()
        props.load(propsFile.inputStream())
        val sdkDir = props.getProperty("sdk.dir", "")
        if (sdkDir.isNotEmpty()) "$sdkDir/ndk/$ndkVersion" else ""
    } else ""
}

android {
    namespace     = "com.funnansoftware.awen"
    compileSdk    = 36
    // Must match Pkg.Revision in $ANDROID_NDK_HOME/source.properties
    ndkVersion    = "29.0.14206865"

    defaultConfig {
        applicationId = "com.funnansoftware.awen"
        minSdk        = 26
        targetSdk     = 36
        versionCode   = 1
        versionName   = "1.0"

        ndk {
            // Only package the arm64-v8a ABI; add "x86_64" for emulator support
            abiFilters += "arm64-v8a"
        }

        externalNativeBuild {
            cmake {
                // vcpkg is the primary toolchain; it chain-loads the NDK toolchain.
                // AGP still injects ANDROID_ABI / ANDROID_PLATFORM via its own
                // arguments, so those do not need to be repeated here.
                val cmakeArgs = mutableListOf(
                    // Use our wrapper toolchain which discovers the NDK from local.properties
                    // and sets VCPKG_CHAINLOAD_TOOLCHAIN_FILE → NDK android.toolchain.cmake
                    // before including vcpkg.cmake.  This ensures the NDK cross-compiler,
                    // sysroot, and cmake file-API metadata are populated correctly for AGP.
                    "-DCMAKE_TOOLCHAIN_FILE=${rootDir.parentFile.absolutePath.replace('\\', '/')}/cmake/android/toolchain.cmake",
                    "-DVCPKG_TARGET_TRIPLET=arm64-android-release",
                    "-DVCPKG_HOST_TRIPLET=$vcpkgHostTriplet",
                    // Keep all vcpkg dependencies as static libs; only our app
                    // target is explicitly built as SHARED (see app/awen/CMakeLists.txt)
                    "-DBUILD_SHARED_LIBS=OFF"
                )
                // On Windows, AGP's NdkHandler produces backslash paths which cmake 4.x
                // writes verbatim into CMakeSystem.cmake.  cmake then fails to re-read
                // the file because \U, \a, etc. are invalid cmake escape sequences.
                // Override with forward-slash paths from local.properties (last -D wins).
                if (ndkDirForCmake.isNotEmpty()) {
                    cmakeArgs += "-DANDROID_NDK=$ndkDirForCmake"
                    cmakeArgs += "-DCMAKE_ANDROID_NDK=$ndkDirForCmake"
                }
                arguments(*cmakeArgs.toTypedArray())
                // Only build the awen shared library; skip tests and other targets
                targets("awen")
            }
        }
    }

    buildTypes {
        release {
            isMinifyEnabled = false
            // Use the auto-generated debug keystore for now.
            // Replace with a real release keystore before publishing to the Play Store.
            signingConfig = signingConfigs.getByName("debug")
        }
    }

    externalNativeBuild {
        cmake {
            // Path is relative to this build.gradle.kts file (android/app/)
            path    = file("../../CMakeLists.txt")
            version = "3.28.3+"
        }
    }
}

// ── SDL3 Android Java sources ─────────────────────────────────────────────────
//
// SDL3's C library is compiled by vcpkg.  The accompanying Java runtime
// (SDLActivity.java, SDL.java, etc.) must also be present in the Android
// project so that AvenActivity (which extends SDLActivity) can compile.
//
// These files live in SDL3's source tree under:
//   android-project/app/src/main/java/org/libsdl/app/
//
// The task below copies them from vcpkg's buildtrees into
//   src/main/java/org/libsdl/app/
//
// Run it once after vcpkg has downloaded SDL3 (i.e. after a cmake configure
// step with the Android triplet or after `vcpkg install sdl3`):
//
//   ./gradlew syncSdl3AndroidSources
//
val sdl3JavaDest = file("src/main/java/org/libsdl/app")
val vcpkgBuildtreesRoot = rootDir.parentFile.resolve("vcpkg/buildtrees/sdl3/src")

val syncSdl3AndroidSources by tasks.registering {
    description = "Copies SDL3 Android Java sources from vcpkg buildtrees into src/main/java"
    group       = "setup"

    onlyIf { !sdl3JavaDest.resolve("SDLActivity.java").exists() }

    doLast {
        val sdl3SrcDirs = vcpkgBuildtreesRoot.takeIf { it.isDirectory }
            ?.listFiles { f -> f.isDirectory }
            ?: emptyArray()

        val sdl3JavaSrc = sdl3SrcDirs
            .map { it.resolve("android-project/app/src/main/java/org/libsdl/app") }
            .firstOrNull { it.isDirectory }
            ?: error(
                "SDL3 Java sources not found under $vcpkgBuildtreesRoot.\n" +
                "Run a cmake configure step with the Android triplet first so that\n" +
                "vcpkg downloads SDL3, then re-run: ./gradlew syncSdl3AndroidSources"
            )

        copy {
            from(sdl3JavaSrc)
            into(sdl3JavaDest)
        }
        logger.lifecycle("SDL3 Android Java sources copied from $sdl3JavaSrc")
    }
}
