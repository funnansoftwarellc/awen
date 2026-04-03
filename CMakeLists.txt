cmake_minimum_required(VERSION 3.28.3)

project(awen)

include(GNUInstallDirs)
include(cmake/target/apk.cmake)
include(cmake/target/clang-format.cmake)
include(cmake/target/clang-tidy.cmake)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(CTest)
include(GoogleTest)

find_package(Qt6 6.10 CONFIG REQUIRED COMPONENTS Quick)

qt_standard_project_setup(REQUIRES 6.10)


add_subdirectory(app)
add_subdirectory(src)
