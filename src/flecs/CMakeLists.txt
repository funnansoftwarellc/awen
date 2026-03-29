project(awen-flecs)

find_package(flecs CONFIG REQUIRED)

add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(${PROJECT_NAME} INTERFACE
    include
)

# /external:anglebrackets — treat all <...> angle-bracket includes as external headers.
# /external:W0          — suppress all warnings from external headers.
# /external:templates   — suppress warnings from templates *defined* in external headers
#                         even when instantiated from user code (e.g. Scene.ixx
#                         instantiating flecs::_::type_impl<TextNode> defined in
#                         component.hpp). Without this flag (or with /external:templates-
#                         which disables the feature), MSVC attributes the warning to
#                         the instantiation site in user code and /external:W0 has no
#                         effect.
target_compile_options(${PROJECT_NAME} INTERFACE
    $<$<CXX_COMPILER_ID:MSVC>:/external:anglebrackets /external:W0 /external:templates>
)

target_link_libraries(${PROJECT_NAME} INTERFACE
    $<IF:$<TARGET_EXISTS:flecs::flecs>,flecs::flecs,flecs::flecs_static>
)
