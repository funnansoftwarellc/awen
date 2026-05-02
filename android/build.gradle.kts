plugins {
    id("com.android.application") version "9.0.0" apply false
}

// When invoked via the CMake `apk` target, BUILD_DIR is passed as a Gradle
// project property (-PBUILD_DIR=<cmake_binary_dir>) so that Gradle output lands
// inside the active CMake preset build directory instead of android/app/build.
if (project.hasProperty("BUILD_DIR")) {
    val buildDir = file(project.property("BUILD_DIR").toString())
    rootProject.layout.buildDirectory.set(buildDir)
    subprojects {
        project.layout.buildDirectory.set(buildDir.resolve(project.name))
    }
}
