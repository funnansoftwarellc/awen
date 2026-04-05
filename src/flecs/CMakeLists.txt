project(awen-flecs)

add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(${PROJECT_NAME} INTERFACE
    ${CMAKE_SOURCE_DIR}/include
)

find_package(flecs CONFIG REQUIRED)

target_link_libraries(${PROJECT_NAME} INTERFACE
    $<IF:$<TARGET_EXISTS:flecs::flecs>,flecs::flecs,flecs::flecs_static>
)

# MSVC's /external mechanism cannot suppress warnings that originate from template
# instantiations: even with /external:anglebrackets and /external:W0, the warning is
# attributed to the instantiation site in user code (e.g. Scene.ixx) rather than to
# the template definition in the external header (e.g. component.hpp).
# The only reliable fix is per-warning /wd suppressions:
#   /wd4702 - unreachable code        (flecs component.hpp uses after-return statements)
#   /wd4127 - conditional expression is constant  (flecs uses non-constexpr if guards)
target_compile_options(${PROJECT_NAME} INTERFACE
    $<$<CXX_COMPILER_ID:MSVC>:/external:anglebrackets /external:W0 /wd4702 /wd4127>
)
