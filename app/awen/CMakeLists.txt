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

# Platform-specific GPU surface backend
if(APPLE)
    target_sources(${PROJECT_NAME} PRIVATE gpu_surface_metal.mm)
    set_source_files_properties(gpu_surface_metal.mm PROPERTIES LANGUAGE OBJCXX)
elseif(EMSCRIPTEN)
    target_sources(${PROJECT_NAME} PRIVATE gpu_surface_dawn.cpp)
else()
    # Windows, Linux, Android → Vulkan
    target_sources(${PROJECT_NAME} PRIVATE gpu_surface_vulkan.cpp)
endif()

find_package(SDL3 CONFIG REQUIRED)
find_package(unofficial-skia CONFIG REQUIRED)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        SDL3::SDL3
        unofficial::skia::skia
)

if(APPLE)
    target_link_libraries(${PROJECT_NAME}
        PRIVATE
            "-framework Metal"
            "-framework QuartzCore"
    )
endif()

# SDL3 on Android still needs the core Android and logging system libraries.
# OpenGL ES and EGL are managed internally by SDL3 and do not need to be
# linked explicitly here.
if(ANDROID)
    find_library(log-lib log)
    find_library(android-lib android)

    target_link_libraries(${PROJECT_NAME}
        PRIVATE
            ${log-lib}
            ${android-lib}
    )
endif()

if(EMSCRIPTEN)
    # -sASYNCIFY lets the blocking while loop in main() suspend and yield back
    # to the browser event loop between frames without any Emscripten-specific
    # code in the application source.
    target_link_options(${PROJECT_NAME} PRIVATE -sASYNCIFY)

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
endif()

if(MSVC)
    target_link_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:/NODEFAULTLIB:MSVCRT>
    )
endif()

if(NOT EMSCRIPTEN)
    install(TARGETS ${PROJECT_NAME}
        BUNDLE DESTINATION .
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )
endif()