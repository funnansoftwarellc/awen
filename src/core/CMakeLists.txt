project(awen-core)

find_package(Threads REQUIRED)

add_library(${PROJECT_NAME})

target_sources(${PROJECT_NAME} PUBLIC
    FILE_SET cxx_modules TYPE CXX_MODULES FILES
        Awen.ixx
)

target_link_libraries(${PROJECT_NAME} PUBLIC Threads::Threads)

add_subdirectory(test)