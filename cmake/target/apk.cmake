if(ANDROID)
    set(_GRADLEW "${CMAKE_SOURCE_DIR}/android/gradlew")

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
            COMMAND "${_GRADLEW}" ${_APK_GRADLE_TASK} "-PBUILD_DIR=${CMAKE_BINARY_DIR}"
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/android"
            COMMENT "Building Android APK (${_APK_GRADLE_TASK}) → ${_APK_OUT}"
            VERBATIM
        )
    else()
        message(WARNING "gradlew not found at android/gradlew — apk target will be skipped.")
    endif()
endif()
