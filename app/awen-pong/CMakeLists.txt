project(awen-pong)

if(ANDROID)
    add_library(${PROJECT_NAME} SHARED)
else()
    add_executable(${PROJECT_NAME})
endif()

target_sources(${PROJECT_NAME}
    PRIVATE
        main.cpp
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        awen-widget
)

if(EMSCRIPTEN)
    set(awen_shell_file "${CMAKE_CURRENT_SOURCE_DIR}/shell.html")

    target_link_options(${PROJECT_NAME}
        PRIVATE
            -sASYNCIFY
            --shell-file
            "${awen_shell_file}"
    )

    set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".html")

    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.html
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.js
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.wasm
        ${CMAKE_SOURCE_DIR}/banner.png
        DESTINATION web
    )
endif()

if(ANDROID)
    # ANativeActivity_onCreate is in SDL3's static archive but called by the
    # Android runtime at load time (not via a link reference), so the linker
    # would otherwise strip it as unreferenced. Force it to be kept.
    target_link_options(${PROJECT_NAME}
        PRIVATE
            "-Wl,-u,ANativeActivity_onCreate"
    )
endif()

if(NOT EMSCRIPTEN)
    install(TARGETS ${PROJECT_NAME}
        BUNDLE DESTINATION .
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )
endif()