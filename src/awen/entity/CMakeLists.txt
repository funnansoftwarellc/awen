project(awen-entity)

set(awenEntityQmlOutputTargets)

qt_add_qml_module(${PROJECT_NAME}
    URI awen.entity
    VERSION 1.0
    OUTPUT_TARGETS awenEntityQmlOutputTargets
    SOURCES
        Entity.h
)

generate_export_header(${PROJECT_NAME}
    EXPORT_FILE_NAME "${CMAKE_BINARY_DIR}/include/awen/entity/export.hxx"
    BASE_NAME AWEN_ENTITY
)

set_target_properties(${PROJECT_NAME}
    PROPERTIES
        EXPORT_NAME entity
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
            Entity.h
            "${CMAKE_BINARY_DIR}/include/awen/entity/export.hxx"
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>"
        "$<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>"
        "$<INSTALL_INTERFACE:include>"
)

set(awenEntityQmlResourceTargets)

foreach(awenEntityQmlOutputTarget IN LISTS awenEntityQmlOutputTargets)
    get_target_property(awenEntityQmlResourceName "${awenEntityQmlOutputTarget}" _qt_resource_name)
    if(awenEntityQmlResourceName)
        list(APPEND awenEntityQmlResourceTargets "${awenEntityQmlOutputTarget}")
    endif()
endforeach()

install(TARGETS ${PROJECT_NAME}
    EXPORT awen-targets
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    FILE_SET HEADERS DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

if(awenEntityQmlResourceTargets)
    install(TARGETS ${awenEntityQmlResourceTargets}
        EXPORT awen-targets
        DESTINATION "${CMAKE_INSTALL_DATADIR}/awen"
    )
endif()
