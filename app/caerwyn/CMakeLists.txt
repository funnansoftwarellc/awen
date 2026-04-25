project(caerwyn)

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

if(NOT EMSCRIPTEN)
    install(TARGETS ${PROJECT_NAME}
        BUNDLE DESTINATION .
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )

    install(FILES
        ${CMAKE_SOURCE_DIR}/assets/fonts/Roboto-Regular.ttf
        ${CMAKE_SOURCE_DIR}/assets/fonts/Roboto-Bold.ttf
        DESTINATION ${CMAKE_INSTALL_BINDIR}/fonts
    )
endif()
