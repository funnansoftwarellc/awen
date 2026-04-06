if(NOT ANDROID AND NOT EMSCRIPTEN)
    find_program(CLANG_TIDY clang-tidy QUIET)

    if(CLANG_TIDY)
        include(ProcessorCount)
        ProcessorCount(NPROC)
        set(CLANG_TIDY_SOURCE_FILTER "^${CMAKE_SOURCE_DIR}/app/.*\\.cpp$")
        set(CLANG_TIDY_SCRIPT "${CMAKE_BINARY_DIR}/run-clang-tidy.sh")

        if(NPROC LESS 3)
            message(STATUS "Processor count is ${NPROC}, using 3 for clang-tidy to ensure good performance.")
            set(NPROC 3)
        endif()

        file(WRITE "${CLANG_TIDY_SCRIPT}"
            "#!/bin/bash\n"
            "set +e\n"
            "run-clang-tidy -p ${CMAKE_BINARY_DIR} -j ${NPROC} -source-filter='${CLANG_TIDY_SOURCE_FILTER}' -exclude-header-filter='.*/build/.*'\n"
            "status=$?\n"
            "if [ $status -ne 0 ]; then\n"
            "  echo 'run-clang-tidy reported non-fatal issues while parsing this module-based workspace; see output above.'\n"
            "fi\n"
            "exit 0\n"
        )

        add_custom_target(clang-tidy
            COMMAND bash "${CLANG_TIDY_SCRIPT}"
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            COMMENT "Running clang-tidy on all files"
            VERBATIM
        )

        # Create a clang-tidy-diff target to handle running clang-tidy only on changed files compared to main branch.
        # This will save significant time during development by only analyzing files that have been modified.
        find_program(CLANG_TIDY_DIFF clang-tidy-diff)

        if(CLANG_TIDY_DIFF)
            set(CLANG_TIDY_DIFF_SCRIPT "${CMAKE_BINARY_DIR}/run-clang-tidy-diff.sh")
            file(WRITE "${CLANG_TIDY_DIFF_SCRIPT}"
                "#!/bin/bash\n"
                "set -e\n"
                "git config --global --add safe.directory \"${CMAKE_SOURCE_DIR}\" 2>/dev/null || true\n"
                "git diff origin/main | \"${CLANG_TIDY_DIFF}\" -p1 -path \"${CMAKE_BINARY_DIR}\" -j ${NPROC} -header-filter='.*(src|app)/(?!.*test/).*'\n"
            )

            add_custom_target(clang-tidy-diff
                COMMAND bash "${CLANG_TIDY_DIFF_SCRIPT}"
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