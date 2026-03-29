project(awen-core-engine)

add_library(${PROJECT_NAME})

target_sources(${PROJECT_NAME} PUBLIC
    FILE_SET cxx_modules TYPE CXX_MODULES FILES
        Engine.ixx
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        awen-flecs
    awen-core
)
