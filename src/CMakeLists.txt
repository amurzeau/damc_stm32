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
	USB/*.c USB/*.h USB/*.cpp
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
    ${CMAKE_CURRENT_LIST_DIR}/USB/USB_DEVICE/App
    ${CMAKE_CURRENT_LIST_DIR}/USB/USB_DEVICE/Target
    ${CMAKE_CURRENT_LIST_DIR}/USB/stm32-mw-usb-device/Core/Inc
    ${CMAKE_CURRENT_LIST_DIR}/USB/stm32-mw-usb-device/Class/AUDIO/Inc
    ${CMAKE_CURRENT_LIST_DIR}/USB/stm32-mw-usb-device/Class/COMPOSITE/Inc
    ${CMAKE_CURRENT_LIST_DIR}/USB/stm32-mw-usb-device/Class/CDC/Inc
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

install(FILES
	$<TARGET_FILE:${TARGET_NAME}>
	$<TARGET_FILE_DIR:${TARGET_NAME}>/${TARGET_NAME}.bin
	$<TARGET_FILE_DIR:${TARGET_NAME}>/${TARGET_NAME}.hex
	$<TARGET_FILE_DIR:${TARGET_NAME}>/${TARGET_NAME}.list
	DESTINATION .
	COMPONENT all
)

install(FILES
	$<TARGET_FILE_DIR:${TARGET_NAME}>/${TARGET_NAME}.bin
	$<TARGET_FILE_DIR:${TARGET_NAME}>/${TARGET_NAME}.hex
	../dist/README.stlink_dragdrop.txt
	DESTINATION .
	COMPONENT stlink_dragdrop
)

install(FILES
	../dist/README.stm32cubeprogrammer.${TOOLCHAIN_TARGET_NAME}.txt
	DESTINATION .
	RENAME README.stm32cubeprogrammer.txt
	COMPONENT stm32cubeprogrammer
)

if(TARGET ${TARGET_NAME}_bootloader AND STM32_SIGNINGTOOL_CLI)
	# Generate concatenated image with
	# FSBL at first address
	# Appli at address 0x10000 (64KB)
	add_custom_command(
		OUTPUT ${TARGET_NAME}-flash.bin
		DEPENDS
			$<TARGET_FILE_DIR:${TARGET_NAME}>/${TARGET_NAME}_bootloader-trusted.bin
			${TARGET_NAME}
		COMMAND
			   dd if=$<TARGET_FILE_DIR:${TARGET_NAME}>/${TARGET_NAME}_bootloader-trusted.bin of=${TARGET_NAME}-flash.bin status=none
			&& dd if=${TARGET_NAME}.bin of=${TARGET_NAME}-flash.bin bs=1024 seek=64 conv=notrunc status=none
	)
	add_custom_target(${TARGET_NAME}-nor
		ALL
		DEPENDS ${TARGET_NAME}-flash.bin
	)

	install(FILES
		$<TARGET_FILE:${TARGET_NAME}_bootloader>
		$<TARGET_FILE_DIR:${TARGET_NAME}>/${TARGET_NAME}_bootloader.bin
		$<TARGET_FILE_DIR:${TARGET_NAME}>/${TARGET_NAME}_bootloader.hex
		$<TARGET_FILE_DIR:${TARGET_NAME}>/${TARGET_NAME}_bootloader.list
		DESTINATION .
		COMPONENT all
	)

	install(FILES
		$<TARGET_FILE_DIR:${TARGET_NAME}>/${TARGET_NAME}-flash.bin
		DESTINATION .
		COMPONENT stm32cubeprogrammer
	)
else()
	install(FILES
		$<TARGET_FILE:${TARGET_NAME}>
		DESTINATION .
		COMPONENT stm32cubeprogrammer
	)
endif()