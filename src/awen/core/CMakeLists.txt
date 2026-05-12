project(awen-core)

qt_add_qml_module(${PROJECT_NAME}
    URI awen.core
    VERSION 1.0
    SOURCES
        TimerFixed.hpp
        TimerFixed.cpp
)

generate_export_header(${PROJECT_NAME}
    EXPORT_FILE_NAME "${CMAKE_BINARY_DIR}/include/awen/core/export.hxx"
    BASE_NAME AWEN_CORE
)

set_target_properties(${PROJECT_NAME}
    PROPERTIES
        EXPORT_NAME core
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        Qt6::Core
        Qt6::Quick
)

target_sources(${PROJECT_NAME}
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS
            "${CMAKE_SOURCE_DIR}/src"
            "${CMAKE_BINARY_DIR}/include"
        FILES
            TimerFixed.hpp
            "${CMAKE_BINARY_DIR}/include/awen/core/export.hxx"
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>"
        "$<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>"
        "$<INSTALL_INTERFACE:include>"
)

install(TARGETS ${PROJECT_NAME}
    EXPORT awen-targets
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    FILE_SET HEADERS DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

if(BUILD_TESTING)
    add_subdirectory(test)
endif()