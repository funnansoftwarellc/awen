plugins {
    id("com.android.application")
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
                arguments(
                    "-DCMAKE_TOOLCHAIN_FILE=${rootDir.parent}/vcpkg/scripts/buildsystems/vcpkg.cmake",
                    "-DVCPKG_CHAINLOAD_TOOLCHAIN_FILE=${System.getenv("ANDROID_NDK_HOME")}/build/cmake/android.toolchain.cmake",
                    "-DVCPKG_TARGET_TRIPLET=arm64-android-release",
                    "-DVCPKG_HOST_TRIPLET=x64-linux-release",
                    // Keep all vcpkg dependencies as static libs; only our app
                    // target is explicitly built as SHARED (see app/awen/CMakeLists.txt)
                    "-DBUILD_SHARED_LIBS=OFF"
                )
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
