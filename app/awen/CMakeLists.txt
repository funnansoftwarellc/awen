qt_add_executable(awen
    main.cpp
)

qt_add_qml_module(awen
    URI AweApp
    VERSION 1.0
    QML_FILES
        Main.qml
)

target_link_libraries(awen
    PRIVATE Qt6::Quick
)

install(TARGETS awen
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

qt_generate_deploy_qml_app_script(
    TARGET awen
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)

install(SCRIPT ${deploy_script})

