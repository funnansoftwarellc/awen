cmake_minimum_required(VERSION 3.28.3)

file(READ "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg.json" _vcpkgJson)
string(JSON _awenVersion GET "${_vcpkgJson}" "version-string")

project(awen VERSION "${_awenVersion}" LANGUAGES CXX)

include(GNUInstallDirs)
include(GenerateExportHeader)
include(CMakePackageConfigHelpers)
include(cmake/target/clang-format.cmake)
include(cmake/target/clang-tidy.cmake)

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

set(AWEN_DATADIR "${CMAKE_INSTALL_DATADIR}/awen")

configure_package_config_file(
	cmake/awen-config.cmake.in
	"${CMAKE_CURRENT_BINARY_DIR}/awen-config.cmake"
	INSTALL_DESTINATION "${AWEN_DATADIR}"
)

write_basic_package_version_file(
	"${CMAKE_CURRENT_BINARY_DIR}/awen-config-version.cmake"
	VERSION "${PROJECT_VERSION}"
	COMPATIBILITY SameMajorVersion
)

add_subdirectory(app)
add_subdirectory(src)

install(EXPORT awen-targets
	FILE awen-targets.cmake
	NAMESPACE awen::
	DESTINATION "${AWEN_DATADIR}"
)

install(FILES
	"${CMAKE_CURRENT_BINARY_DIR}/awen-config.cmake"
	"${CMAKE_CURRENT_BINARY_DIR}/awen-config-version.cmake"
	DESTINATION "${AWEN_DATADIR}"
)
