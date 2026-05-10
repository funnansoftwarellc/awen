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

find_package(Qt6 6.10 CONFIG REQUIRED COMPONENTS Quick)

qt_standard_project_setup(REQUIRES 6.10)

# Auto-generate .qmlls.ini next to each QML source directory so the QML
# language server picks up the correct per-build import paths regardless of
# platform or preset. See: https://doc.qt.io/qt-6/qtqml-tooling-qmlls.html
set(QT_QML_GENERATE_QMLLS_INI ON CACHE BOOL "Generate .qmlls.ini for qmlls" FORCE)


add_subdirectory(app)
add_subdirectory(src)
