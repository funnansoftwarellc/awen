plugins {
    id("com.android.application")
}

android {
    namespace  = "com.funnansoftware.awen"
    compileSdk = 36

    defaultConfig {
        applicationId = "com.funnansoftware.awen"
        minSdk        = 26
        targetSdk     = 36
        versionCode   = 1
        versionName   = "1.0"

        ndk {
            // Only package the arm64-v8a ABI; add "x86_64" for emulator support.
            // The pre-built .so is staged into jniLibs/${ANDROID_ABI}/ by the
            // CMake apk target before Gradle runs.
            abiFilters += "arm64-v8a"
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
