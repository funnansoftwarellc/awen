if(ANDROID)
    if(CMAKE_HOST_WIN32)
        set(_GRADLEW "${CMAKE_SOURCE_DIR}/android/gradlew.bat")
    else()
        set(_GRADLEW "${CMAKE_SOURCE_DIR}/android/gradlew")
    endif()

    if(EXISTS "${_GRADLEW}")
        if(CMAKE_BUILD_TYPE STREQUAL "Release")
            set(_APK_GRADLE_TASK "assembleRelease")
            set(_APK_VARIANT "release")
            set(_APK_NAME "app-release.apk")
        else()
            set(_APK_GRADLE_TASK "assembleDebug")
            set(_APK_VARIANT "debug")
            set(_APK_NAME "app-debug.apk")
        endif()

        set(_APK_OUT "${CMAKE_BINARY_DIR}/app/outputs/apk/${_APK_VARIANT}/${_APK_NAME}")

        add_custom_target(apk
            COMMAND "${_GRADLEW}" ${_APK_GRADLE_TASK}
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/android"
            COMMENT "Building Android APK (${_APK_GRADLE_TASK}) → ${_APK_OUT}"
            VERBATIM
        )

        # Probe the Android SDK platform-tools dir from the same sources as
        # cmake/android/toolchain.cmake, so adb is found even when it isn't on PATH.
        find_program(_ADB adb HINTS
            "$ENV{ANDROID_HOME}/platform-tools"
            "$ENV{LOCALAPPDATA}/Android/Sdk/platform-tools"
            "$ENV{HOME}/Library/Android/sdk/platform-tools"
            "$ENV{HOME}/Android/Sdk/platform-tools"
            "/opt/android/sdk/platform-tools"
        )
        if(_ADB)
            # Use a cmake -P script so we can uninstall first without failing
            # when the package isn't present, then install fresh.  This handles
            # signature-mismatch errors (debug ↔ release key) that `adb install -r`
            # cannot recover from.
            set(_ADB_INSTALL_SCRIPT "${CMAKE_CURRENT_LIST_DIR}/adb-install.cmake")
            add_custom_target(adb-install
                COMMAND "${CMAKE_COMMAND}"
                    "-DADB=${_ADB}"
                    "-DAPK=${_APK_OUT}"
                    "-DPACKAGE=com.funnansoftware.awen"
                    -P "${_ADB_INSTALL_SCRIPT}"
                DEPENDS apk
                COMMENT "Installing ${_APK_NAME} via adb"
                VERBATIM
            )
        else()
            message(STATUS "adb not found — adb-install target will be skipped.")
        endif()
    else()
        message(WARNING "gradlew not found at android/gradlew — apk target will be skipped.")
    endif()
endif()
