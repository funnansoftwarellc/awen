project(awen-widget)


add_library(${PROJECT_NAME})

target_sources(${PROJECT_NAME} PUBLIC
    FILE_SET cxx_modules TYPE CXX_MODULES FILES
        Node.ixx
        Node2D.ixx
        Control.ixx
        Window.ixx
)

find_package(glm CONFIG REQUIRED)

target_link_libraries(${PROJECT_NAME} PUBLIC
    awen-scene
    awen-graphics
    awen-core
    glm::glm
)

# if(NOT ANDROID)
#     add_subdirectory(test)
# endif()