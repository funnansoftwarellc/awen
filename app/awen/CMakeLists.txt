project(awen)

qt_add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME}
    PRIVATE
        main.cpp
)

qt_add_qml_module(${PROJECT_NAME}
    URI AwenApp
    VERSION 1.0
    QML_FILES
        Main.qml
    IMPORTS
        awen.entity
)

target_compile_definitions(${PROJECT_NAME}
    PRIVATE
        $<$<CONFIG:Debug>:QT_QML_DEBUG>
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        awen-entity
        Qt6::Quick
)

install(TARGETS ${PROJECT_NAME}
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

qt_generate_deploy_qml_app_script(
    TARGET ${PROJECT_NAME}
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
    NO_TRANSLATIONS
)

install(SCRIPT ${deploy_script})

