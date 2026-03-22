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

if(EMSCRIPTEN)
    # Emscripten provides its own GLFW port via its ports system.
    # -sUSE_GLFW=3 is a linker-only flag that pulls in the JS GLFW implementation.
    # -sASYNCIFY allows blocking C calls (e.g. emscripten_sleep used by SetTargetFPS)
    # to be transformed into async JS continuations at compile time.
    target_link_options(${PROJECT_NAME} PRIVATE -sUSE_GLFW=3 -sASYNCIFY)

    # Emscripten.cmake unconditionally sets CMAKE_EXECUTABLE_SUFFIX to ".js".
    # Override it at the target level so Emscripten emits awen.html alongside
    # awen.js and awen.wasm.
    set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".html")

    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.html
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.js
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.wasm
        DESTINATION ${CMAKE_INSTALL_PREFIX}/web
    )
elseif(NOT ANDROID)
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

if(NOT EMSCRIPTEN)
    install(TARGETS ${PROJECT_NAME}
        BUNDLE DESTINATION .
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )
endif()