project(awen-core)

qt_add_qml_module(${PROJECT_NAME}
    URI awen.core
    VERSION 1.0
    QML_FILES
    SOURCES
        TimerFixed.hpp
        TimerFixed.cpp
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        Qt6::Core
        Qt6::Quick
)

generate_export_header(${PROJECT_NAME}
    EXPORT_FILE_NAME "${CMAKE_BINARY_DIR}/include/awen/core/export.hxx"
    BASE_NAME AWEN_CORE
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>"
        "$<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>"
)

install(TARGETS ${PROJECT_NAME}
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(BUILD_TESTING)
    add_subdirectory(test)
endif()