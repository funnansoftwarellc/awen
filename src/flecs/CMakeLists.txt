project(awen-flecs)

find_package(flecs CONFIG REQUIRED)

add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(${PROJECT_NAME} INTERFACE
    include
)

# /external:anglebrackets — treat all <...> angle-bracket includes as external headers.
# /external:W0          — suppress all warnings from external headers.
# /external:templates-  — use the template *definition* location (external) rather than
#                         the *instantiation* location (user code) when deciding whether
#                         to suppress warnings. Without this, MSVC emits warnings for
#                         templates defined in external headers (e.g. flecs component.hpp)
#                         when they are instantiated from user code (e.g. Scene.ixx).
target_compile_options(${PROJECT_NAME} INTERFACE
    $<$<CXX_COMPILER_ID:MSVC>:/external:anglebrackets /external:W0 /external:templates->
)

target_link_libraries(${PROJECT_NAME} INTERFACE
    $<IF:$<TARGET_EXISTS:flecs::flecs>,flecs::flecs,flecs::flecs_static>
)
