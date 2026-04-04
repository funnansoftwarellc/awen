project(awen-core)


add_library(${PROJECT_NAME})

target_sources(${PROJECT_NAME} PUBLIC
    FILE_SET cxx_modules TYPE CXX_MODULES FILES
        Awen.ixx
        Engine.ixx
        Module.ixx
        Object.ixx
        ObjectPtr.ixx
        Overloaded.ixx
        Signal.ixx
)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

find_package(Threads REQUIRED)
find_package(flecs CONFIG REQUIRED)

target_link_libraries(${PROJECT_NAME} PUBLIC
    Threads::Threads
    $<IF:$<TARGET_EXISTS:flecs::flecs>,flecs::flecs,flecs::flecs_static>
)

# MSVC's /external mechanism cannot suppress warnings that originate from template
# instantiations: even with /external:anglebrackets and /external:W0, the warning is
# attributed to the instantiation site in user code (e.g. Scene.ixx) rather than to
# the template definition in the external header (e.g. component.hpp).
# The only reliable fix is per-warning /wd suppressions:
#   /wd4702 — unreachable code        (flecs component.hpp uses after-return statements)
#   /wd4127 — conditional expression is constant  (flecs uses non-constexpr if guards)
target_compile_options(${PROJECT_NAME} PUBLIC
    $<$<CXX_COMPILER_ID:MSVC>:/external:anglebrackets /external:W0 /wd4702 /wd4127>
)

if(NOT ANDROID)
    add_subdirectory(test)
endif()