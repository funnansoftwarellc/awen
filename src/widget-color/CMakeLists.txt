project(awen-widget-color)

add_library(${PROJECT_NAME})

target_sources(${PROJECT_NAME} PUBLIC
    FILE_SET cxx_modules TYPE CXX_MODULES FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/../widget/Color.ixx
)