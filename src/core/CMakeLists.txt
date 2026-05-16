project(awen-core)

add_library(${PROJECT_NAME})

target_sources(${PROJECT_NAME} PUBLIC
    FILE_SET cxx_module TYPE CXX_MODULES
        FILES
            Object.ixx
)

find_package(PalSigslot CONFIG REQUIRED)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        Pal::Sigslot
)

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