cmake_minimum_required(VERSION 3.20)

find_package(Git QUIET)
if(GIT_FOUND)
	execute_process(
		COMMAND ${GIT_EXECUTABLE} describe --tags --always --dirty
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		OUTPUT_VARIABLE DAMC_GIT_VERSION
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)
	if(DAMC_GIT_VERSION_TRIMMED MATCHES "^v?([0-9]+\\.[0-9]+(\\.[0-9]+)?).*\$")
		string(REGEX REPLACE "^v?([0-9]+\\.[0-9]+(\\.[0-9]+)?).*\$" "\\1" DAMC_GIT_VERSION_TRIMMED ${DAMC_GIT_VERSION})
	else()
		set(DAMC_GIT_VERSION_TRIMMED "0.0.0")
		set(DAMC_GIT_VERSION "unknown-rev")
	endif()
else()
	set(DAMC_GIT_VERSION_TRIMMED "0.0.0")
	set(DAMC_GIT_VERSION "unknown-rev")
endif()

project(damc_stm32 VERSION "${DAMC_GIT_VERSION_TRIMMED}" LANGUAGES C CXX ASM)

message(STATUS "Building for ${TOOLCHAIN_TARGET_NAME}")
set(TARGET_NAME damc_${TOOLCHAIN_TARGET_NAME})

add_subdirectory(src)

# Packaging
set(CPACK_GENERATOR ZIP)
set(CPACK_STRIP_FILES ON)
set(CPACK_PACKAGE_VERSION ${DAMC_GIT_VERSION})
set(CPACK_ARCHIVE_COMPONENT_INSTALL ON)

# Remove system name from filename (it is "Generic" and this is not useful to have it)
set(CPACK_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}-${TOOLCHAIN_TARGET_NAME}-${CPACK_PACKAGE_VERSION}")

include(CPack)
