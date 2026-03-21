cmake_minimum_required(VERSION 3.31.6)

project(awen)

include(GNUInstallDirs)
include(cmake/target/clang-format.cmake)
include(cmake/target/clang-tidy.cmake)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(CTest)
include(GoogleTest)

add_subdirectory(app)
add_subdirectory(src)
