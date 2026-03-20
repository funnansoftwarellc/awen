find_program(CLANG_FORMAT_EXECUTABLE clang-format QUIET)

if(CLANG_FORMAT_EXECUTABLE)
    file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS
        ${CMAKE_SOURCE_DIR}/app/*
        ${CMAKE_SOURCE_DIR}/src/*
    )

    list(FILTER SOURCE_FILES INCLUDE REGEX ".*\\.(cpp|h|ixx)$")

    add_custom_target(clang-format-check
        COMMAND ${CLANG_FORMAT_EXECUTABLE} --dry-run -Werror ${SOURCE_FILES}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Checking format of source files..."
        VERBATIM
    )

    add_custom_target(clang-format
        COMMAND ${CLANG_FORMAT_EXECUTABLE} -i ${SOURCE_FILES}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Formatting source files..."
        VERBATIM
    )
else()
    message(WARNING "Clang-format executable not found - clang-format target will be skipped.")
endif()