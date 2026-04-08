cmake_minimum_required(VERSION 3.28.3)

project(awen)

include(GNUInstallDirs)
include(cmake/target/apk.cmake)
include(cmake/target/clang-format.cmake)
include(cmake/target/clang-tidy.cmake)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
	find_program(nodeExecutable node REQUIRED)

	set(CMAKE_CROSSCOMPILING_EMULATOR
		"${nodeExecutable}"
		CACHE FILEPATH "Path to the emulator for the target system."
		FORCE
	)
endif()

include(CTest)
include(GoogleTest)

include_directories(${CMAKE_SOURCE_DIR}/include)

add_subdirectory(app)
add_subdirectory(src)
