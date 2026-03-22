if(NOT ANDROID AND NOT EMSCRIPTEN)
    find_program(LLVM_PROFDATA llvm-profdata QUIET)
    find_program(LLVM_COV llvm-cov QUIET)

    if(LLVM_PROFDATA AND LLVM_COV)
        get_property(PROJECT_LIB_TARGETS GLOBAL PROPERTY PROJECT_LIB_TARGETS)

        foreach(lib ${PROJECT_LIB_TARGETS})
            list(APPEND LLVM_COV_OBJECTS -object $<TARGET_FILE:${lib}>)
        endforeach()

        message(STATUS "Targets: ${LLVM_COV_OBJECTS}")

        add_custom_target(llvm-coverage
            # Merge raw profiles.
            COMMAND ${LLVM_PROFDATA} merge -sparse
                ${CMAKE_BINARY_DIR}/coverage/*.profraw
                -o ${CMAKE_BINARY_DIR}/coverage/merged.profdata

            COMMAND ${LLVM_COV} report
                -instr-profile=${CMAKE_BINARY_DIR}/coverage/merged.profdata
                ${LLVM_COV_OBJECTS}

            COMMAND ${LLVM_COV} show
                -format=html
                -output-dir=${CMAKE_BINARY_DIR}/coverage/html
                -instr-profile=${CMAKE_BINARY_DIR}/coverage/merged.profdata
                ${LLVM_COV_OBJECTS}

            COMMAND ${LLVM_COV} export
                -format=lcov
                -instr-profile=${CMAKE_BINARY_DIR}/coverage/merged.profdata
                ${LLVM_COV_OBJECTS}
                > ${CMAKE_BINARY_DIR}/coverage/coverage.info
        )
    else()
        message(WARNING "llvm-profdata and llvm-cov executable(s) not found - llvm-coverage target will be skipped.")
    endif()
endif()