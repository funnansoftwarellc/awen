project(awen-graphics)

add_library(${PROJECT_NAME})

target_sources(${PROJECT_NAME} PUBLIC
    FILE_SET cxx_modules TYPE CXX_MODULES FILES
        Event.ixx
        Color.ixx
        DrawList.ixx
        Window.ixx
        Renderer.ixx
        Graphics.ixx
)

find_package(Threads REQUIRED)
find_package(magic_enum CONFIG REQUIRED)
find_package(raylib CONFIG REQUIRED)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        awen-core
        magic_enum::magic_enum
        Threads::Threads
        raylib
)

if(NOT ANDROID)
    # add_subdirectory(test)  # TraversalPass test removed; traversal is now tested via awen-scene-test
endif()

if(EMSCRIPTEN)
    target_link_options(${PROJECT_NAME}
        PUBLIC
            -sUSE_GLFW=3
            -sASYNCIFY
    )
elseif(ANDROID)
    find_library(log-lib log)
    find_library(android-lib android)
    find_library(GLESv2-lib GLESv2)
    find_library(EGL-lib EGL)

    target_link_libraries(${PROJECT_NAME}
        PUBLIC
            ${log-lib}
            ${android-lib}
            ${GLESv2-lib}
            ${EGL-lib}
    )
else()
    find_package(glfw3 CONFIG REQUIRED)

    target_link_libraries(${PROJECT_NAME}
        PUBLIC
            glfw
    )

    if(MSVC)
        target_link_options(${PROJECT_NAME}
            PUBLIC
                $<$<CONFIG:Debug>:/NODEFAULTLIB:MSVCRT>
        )

        target_link_libraries(${PROJECT_NAME}
            PUBLIC
                winmm
        )
    endif()
endif()


