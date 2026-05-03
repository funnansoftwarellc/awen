project(caerwyn)

if(ANDROID)
    add_library(${PROJECT_NAME} SHARED)
else()
    add_executable(${PROJECT_NAME})
endif()

if(ANDROID)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E make_directory
            "${CMAKE_SOURCE_DIR}/android/app/src/main/jniLibs/${ANDROID_ABI}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
            "$<TARGET_FILE:${PROJECT_NAME}>"
            "${CMAKE_SOURCE_DIR}/android/app/src/main/jniLibs/${ANDROID_ABI}/$<TARGET_FILE_NAME:${PROJECT_NAME}>"
        COMMENT "Staging $<TARGET_FILE_NAME:${PROJECT_NAME}> → android/app/src/main/jniLibs/${ANDROID_ABI}/"
        VERBATIM
    )
endif()

target_sources(${PROJECT_NAME}
    PRIVATE
        main.cpp
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        awen-flecs
        awen-sdl-ecs
)

if(EMSCRIPTEN)
    set(awen_shell_file "${CMAKE_CURRENT_SOURCE_DIR}/shell.html")

    target_link_options(${PROJECT_NAME}
        PRIVATE
            -sASYNCIFY
            --shell-file
            "${awen_shell_file}"
            "--preload-file"
            "${CMAKE_SOURCE_DIR}/assets/fonts/DejaVuSans.ttf@/fonts/DejaVuSans.ttf"
    )

    set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".html")

    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.html
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.js
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.wasm
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.data
        ${CMAKE_SOURCE_DIR}/banner.png
        DESTINATION web
    )
endif()

if(NOT EMSCRIPTEN)
    install(TARGETS ${PROJECT_NAME}
        BUNDLE DESTINATION .
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )

    install(FILES
        ${CMAKE_SOURCE_DIR}/assets/fonts/DejaVuSans.ttf
        DESTINATION ${CMAKE_INSTALL_BINDIR}/fonts
    )
endif()