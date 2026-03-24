project(awen-scene)

add_library(${PROJECT_NAME})

target_sources(${PROJECT_NAME} PUBLIC
    FILE_SET cxx_modules TYPE CXX_MODULES FILES
        NodeId.ixx
        NodePool.ixx
        HierarchyPool.ixx
        Transform.ixx
        Scene.ixx
)

find_package(Threads REQUIRED)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        Threads::Threads
)

if(NOT ANDROID)
    add_subdirectory(test)
endif()
