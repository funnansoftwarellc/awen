project(awen-flecs)

find_package(flecs CONFIG REQUIRED)

add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(${PROJECT_NAME} INTERFACE
    include
)

# /external:anglebrackets marks all <...> angle-bracket includes as external headers.
# Combined with /external:W0 (added by CMake for SYSTEM includes), this suppresses
# warnings from third-party headers such as flecs in C++ module global fragments,
# where #pragma warning(push, 0) is not reliably honoured by MSVC.
target_compile_options(${PROJECT_NAME} INTERFACE
    $<$<CXX_COMPILER_ID:MSVC>:/external:anglebrackets /external:W0>
)

target_link_libraries(${PROJECT_NAME} INTERFACE
    $<IF:$<TARGET_EXISTS:flecs::flecs>,flecs::flecs,flecs::flecs_static>
)
