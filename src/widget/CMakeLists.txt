project(awen-widget)


add_library(${PROJECT_NAME})

target_sources(${PROJECT_NAME} PUBLIC
    FILE_SET cxx_modules TYPE CXX_MODULES FILES
        Node.ixx
        Widget.ixx
        Window.ixx
)

target_link_libraries(${PROJECT_NAME} PUBLIC
    awen-scene
    awen-graphics
    awen-core
)

# if(NOT ANDROID)
#     add_subdirectory(test)
# endif()