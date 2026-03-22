project(awen)

# Android NativeActivity requires a shared library (.so), not an executable
if(ANDROID)
    add_library(${PROJECT_NAME} SHARED)
else()
    add_executable(${PROJECT_NAME})
endif()

target_sources(${PROJECT_NAME}
    PRIVATE
        main.cpp
)

find_package(raylib CONFIG REQUIRED)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        raylib
)

# Link OpenGL ES and EGL when building for Android
if(ANDROID)
    find_library(log-lib log)
    find_library(android-lib android)
    find_library(GLESv2-lib GLESv2)
    find_library(EGL-lib EGL)

    target_link_libraries(${PROJECT_NAME}
        PRIVATE
            ${log-lib}
            ${android-lib}
            ${GLESv2-lib}
            ${EGL-lib}
    )

    # ANativeActivity_onCreate is in raylib's static archive but called by the
    # Android runtime at load time (not via a link reference), so the linker
    # would otherwise strip it as unreferenced. Force it to be kept.
    target_link_options(${PROJECT_NAME} PRIVATE
        "-Wl,-u,ANativeActivity_onCreate"
    )
endif()

if(NOT ANDROID AND NOT EMSCRIPTEN)
    find_package(glfw3 CONFIG REQUIRED)

    target_link_libraries(${PROJECT_NAME}
        PRIVATE
            glfw
    )
endif()

if(MSVC)
    target_link_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:/NODEFAULTLIB:MSVCRT>
    )

    target_link_libraries(${PROJECT_NAME}
        PRIVATE
            winmm
    )
endif()

install(TARGETS ${PROJECT_NAME}
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)