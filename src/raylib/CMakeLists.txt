project(awen-raylib)

add_library(${PROJECT_NAME})

target_sources(${PROJECT_NAME} PUBLIC
    FILE_SET cxx_module TYPE CXX_MODULES
        FILES
            Color.ixx
            Module.ixx
            Text.ixx
            Window.ixx
)

if(EMSCRIPTEN)
    set(raylib_USE_STATIC_LIBS ON)
    target_link_options(${PROJECT_NAME}
        PUBLIC
            -sUSE_GLFW=3
            -sASYNCIFY
    )
endif()

find_package(raylib CONFIG REQUIRED)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        raylib
        awen-core
)

if(NOT EMSCRIPTEN)
    find_package(glfw3 CONFIG REQUIRED)

    target_link_libraries(${PROJECT_NAME}
        PUBLIC
            glfw
    
    )
endif()

if(WIN32)
    target_link_libraries(${PROJECT_NAME}
        PUBLIC
            winmm
    )
endif()

install(TARGETS ${PROJECT_NAME}
    EXPORT awen-targets
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    FILE_SET cxx_module DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

if(BUILD_TESTING)
    add_subdirectory(test)
endif()