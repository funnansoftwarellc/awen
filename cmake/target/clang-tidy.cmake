if(NOT ANDROID AND NOT EMSCRIPTEN)
    find_program(CLANG_TIDY clang-tidy QUIET)

    if(CLANG_TIDY)
        include(ProcessorCount)
        ProcessorCount(NPROC)
        set(CLANG_TIDY_SOURCE_FILTER "^${CMAKE_SOURCE_DIR}/(src|app)/.*\\.(cpp|ixx)$")

        if(NPROC LESS 3)
            message(STATUS "Processor count is ${NPROC}, using 3 for clang-tidy to ensure good performance.")
            set(NPROC 3)
        endif()

        add_custom_target(clang-tidy
            COMMAND run-clang-tidy -p ${CMAKE_BINARY_DIR} -j ${NPROC} -source-filter=${CLANG_TIDY_SOURCE_FILTER} -exclude-header-filter=".*/build/.*"
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            COMMENT "Running clang-tidy on all files"
            VERBATIM
        )

        # Create a clang-tidy-diff target to handle running clang-tidy only on changed files compared to main branch.
        # This will save significant time during development by only analyzing files that have been modified.
        find_program(CLANG_TIDY_DIFF clang-tidy-diff)

        if(CLANG_TIDY_DIFF)
            add_custom_target(clang-tidy-diff
                COMMAND git config --global --add safe.directory "${CMAKE_SOURCE_DIR}" || true
                COMMAND git diff origin/main | clang-tidy-diff -p1 -path ${CMAKE_BINARY_DIR} -j ${NPROC} -header-filter=".*(src|app)/(?!.*test/).*"
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                COMMENT "Running clang-tidy on files changed from main"
                VERBATIM
            )
        else()
            message(WARNING "clang-tidy-diff not found - clang-tidy-diff target will be skipped.")
        endif()
    else()
        message(WARNING "clang-tidy executable not found - clang-tidy target will be skipped.")
    endif()
endif()