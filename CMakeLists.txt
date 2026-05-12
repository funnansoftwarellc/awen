cmake_minimum_required(VERSION 3.28.3)

project(awen)

include(GNUInstallDirs)
include(GenerateExportHeader)
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
include(cmake/qt-vcpkg-fixup.cmake)

find_package(Qt6 6.10 CONFIG REQUIRED COMPONENTS Quick)

qt_standard_project_setup(REQUIRES 6.10)

# Collect every qt_add_qml_module output under a single tree so that all
# in-project QML modules (e.g. awen.entity) are discoverable via a single
# import path. This is what the auto-generated .qmlls.ini files (and any
# downstream tooling, such as qmllint or deploy scripts) will reference.
set(QT_QML_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/qml" CACHE PATH
    "Base output directory for all qt_add_qml_module targets" FORCE)

# Auto-generate .qmlls.ini next to each QML source directory so the QML
# language server picks up the correct per-build import paths regardless of
# platform or preset. See: https://doc.qt.io/qt-6/qtqml-tooling-qmlls.html
set(QT_QML_GENERATE_QMLLS_INI ON CACHE BOOL "Generate .qmlls.ini for qmlls" FORCE)


add_subdirectory(app)
add_subdirectory(src)
