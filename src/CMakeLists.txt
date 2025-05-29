cmake_minimum_required(VERSION 3.20)

include(cmake/st-project.cmake)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(${TOOLCHAIN_TARGET_NAME})


add_library(common_flags INTERFACE)
target_compile_options(common_flags INTERFACE
	-ggdb3
	-gdwarf-4
)

if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	target_compile_options(common_flags INTERFACE
		-ggdb3
		-gdwarf-4
		"$<$<CONFIG:Release>:-Ofast>"
		"$<$<CONFIG:Release>:-flto=auto>"
	)
else()
	target_compile_options(common_flags INTERFACE
		-ggdb3
		-gdwarf-4
		"$<$<CONFIG:Release>:-O3>"
		"$<$<CONFIG:Release>:-ffast-math>"
		"$<$<CONFIG:Release>:-flto=auto>"
	)
endif()

set(DISABLEFLOAT16 OFF)
add_subdirectory(CMSIS-DSP)
target_link_libraries(CMSISDSP PUBLIC stm32cube_headers common_flags)


file(GLOB_RECURSE SOURCE_LIST
	USBClass/*.c USBClass/*.h USBClass/*.cpp
	damc/*.c damc/*.h damc/*.cpp
	memtester/*.c memtester/*.h memtester/*.cpp
    Fonts/*.c Fonts/*.h Fonts/*.cpp
)

add_executable(${TARGET_NAME} ${SOURCE_LIST})
add_st_target_properties(${TARGET_NAME})
target_link_libraries(${TARGET_NAME} PUBLIC CMSISDSP)
target_link_libraries(${TARGET_NAME} PUBLIC stm32cube)
target_link_libraries(${TARGET_NAME} PUBLIC common_flags)

target_include_directories(
    ${TARGET_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/USBClass/AUDIO/Inc
    ${CMAKE_CURRENT_LIST_DIR}/USBClass/COMPOSITE/Inc
    ${CMAKE_CURRENT_LIST_DIR}/USBClass/CDC/Inc
    ${CMAKE_CURRENT_LIST_DIR}/Fonts
    ${CMAKE_CURRENT_LIST_DIR}/damc/deps
    ${CMAKE_CURRENT_LIST_DIR}/damc/deps/libuv_microcontroller
    ${CMAKE_CURRENT_LIST_DIR}/damc/damc_audio_processing
    ${CMAKE_CURRENT_LIST_DIR}/damc/damc_common
    ${CMAKE_CURRENT_LIST_DIR}/damc/damc_simple_lib
    ${CMAKE_CURRENT_LIST_DIR}/memtester
)

target_compile_options(
    ${TARGET_NAME} PRIVATE
	-Wall
	-Wdouble-promotion
	-Wno-psabi
)
if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	target_compile_options(
		${TARGET_NAME} PRIVATE
		# memcpy is slower than 32 bits word copies
		-fno-builtin-memcpy
		-fno-tree-loop-distribute-patterns
	)
endif()

if(TARGET ${TARGET_NAME}_bootloader AND STM32_SIGNINGTOOL_CLI)
	# Generate concatenated image with
	# FSBL at first address
	# Appli at address 0x10000 (64KB)
	add_custom_command(
		OUTPUT ${TARGET_NAME}-nor.bin
		DEPENDS
			$<TARGET_FILE_DIR:${TARGET_NAME}>/${TARGET_NAME}_bootloader-trusted.bin
			${TARGET_NAME}
		COMMAND
			   dd if=$<TARGET_FILE_DIR:${TARGET_NAME}>/${TARGET_NAME}_bootloader-trusted.bin of=${TARGET_NAME}-nor.bin status=none
			&& dd if=${TARGET_NAME}.bin of=${TARGET_NAME}-nor.bin bs=1024 seek=64 conv=notrunc status=none
	)
	add_custom_target(${TARGET_NAME}-nor
		ALL
		DEPENDS ${TARGET_NAME}-nor.bin
	)
endif()