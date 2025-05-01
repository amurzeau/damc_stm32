cmake_minimum_required(VERSION 3.20)

include(cmake/st-project.cmake)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(${TOOLCHAIN_TARGET_NAME})


add_library(common_flags INTERFACE)
target_compile_options(common_flags INTERFACE
	-ggdb3
	-gdwarf-4
	"$<$<CONFIG:Release>:-Ofast>"
	"$<$<CONFIG:Release>:-flto=auto>"
)

set(DISABLEFLOAT16 ON)
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
	# memcpy is slower than 32 bits word copies
	-fno-builtin-memcpy
	-fno-tree-loop-distribute-patterns
)
