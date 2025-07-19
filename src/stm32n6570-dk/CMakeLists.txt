cmake_minimum_required(VERSION 3.20)

# For CMSIS-DSP
set(HELIUM ON PARENT_SCOPE)

# Common files

add_library(stm32cube_headers INTERFACE)

target_include_directories(stm32cube_headers INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/Drivers/STM32N6xx_HAL_Driver/Inc
    ${CMAKE_CURRENT_LIST_DIR}/Drivers/STM32N6xx_HAL_Driver/Inc/Legacy
    ${CMAKE_CURRENT_LIST_DIR}/Drivers/CMSIS/Device/ST/STM32N6xx/Include
    ${CMAKE_CURRENT_LIST_DIR}/Drivers/CMSIS/Include
)

target_compile_definitions(stm32cube_headers INTERFACE
    "$<$<CONFIG:Debug>:DEBUG>"
    "$<$<CONFIG:Debug>:USE_FULL_ASSERT>"
    USE_HAL_DRIVER
    STM32N657xx
	PLATFORM_HEADER=\"stm32n657xx.h\"
	PLATFORM_HAL_HEADER=\"stm32n6xx_hal.h\"
)

# FSBL bootloader

file(GLOB_RECURSE FSBL_SOURCE_LIST
	FSBL/*.c
	FSBL/*.h
	FSBL/*.cpp
	FSBL/*.s
	Drivers/STM32N6xx_HAL_Driver/*/stm32n6xx_hal_cortex.*
	Drivers/STM32N6xx_HAL_Driver/*/stm32n6xx_hal_dma_ex.*
	Drivers/STM32N6xx_HAL_Driver/*/stm32n6xx_hal_dma.*
	Drivers/STM32N6xx_HAL_Driver/*/stm32n6xx_hal_exti.*
	Drivers/STM32N6xx_HAL_Driver/*/stm32n6xx_hal_gpio.*
	Drivers/STM32N6xx_HAL_Driver/*/stm32n6xx_hal_pwr_ex.*
	Drivers/STM32N6xx_HAL_Driver/*/stm32n6xx_hal_pwr.*
	Drivers/STM32N6xx_HAL_Driver/*/stm32n6xx_hal_rcc_ex.*
	Drivers/STM32N6xx_HAL_Driver/*/stm32n6xx_hal_rcc.*
	Drivers/STM32N6xx_HAL_Driver/*/stm32n6xx_hal_uart.*
	Drivers/STM32N6xx_HAL_Driver/*/stm32n6xx_hal_uart_ex.*
	Drivers/STM32N6xx_HAL_Driver/*/stm32n6xx_hal_xspi.*
	Drivers/STM32N6xx_HAL_Driver/*/stm32n6xx_hal.*

	../memtester/*.h
	../memtester/*.c

	BSP/stm32n6570_discovery_xspi.c
	BSP/stm32n6570_discovery_xspi.h
	BSP/stm32n6570_discovery_conf.h
	BSP/stm32n6570_discovery_errno.h
	BSP/stm32n6570_discovery.c
	Components/aps256xx/*.c
	Components/aps256xx/*.h
	Components/mx66uw1g45g/*.c
	Components/mx66uw1g45g/*.h

	Middlewares/ST/STM32_ExtMem_Manager/*.c
	Middlewares/ST/STM32_ExtMem_Manager/*.h
	Middlewares/ST/STM32_ExtMem_Manager/*.cpp
)
# Exclude template files
list(FILTER FSBL_SOURCE_LIST EXCLUDE REGEX .*/Source/Templates/.*)

add_executable(${TARGET_NAME}_bootloader ${FSBL_SOURCE_LIST})
set_target_properties(${TARGET_NAME}_bootloader PROPERTIES RUNTIME_OUTPUT_DIRECTORY ..)
add_st_target_properties(${TARGET_NAME}_bootloader)

if(STM32_SIGNINGTOOL_CLI)
	add_custom_command(
		TARGET ${TARGET_NAME}_bootloader POST_BUILD
		BYPRODUCTS ../${TARGET_NAME}_bootloader-trusted.bin
		COMMAND ${STM32_SIGNINGTOOL_CLI}
			--silent
			--binary-image $<TARGET_FILE_DIR:${TARGET_NAME}>/${TARGET_NAME}_bootloader.bin
			--no-keys
			--option-flags 0x80000000
			--type fsbl
			--output $<TARGET_FILE_DIR:${TARGET_NAME}>/${TARGET_NAME}_bootloader-trusted.bin
			--header-version 2.3
			--dump-header $<TARGET_FILE_DIR:${TARGET_NAME}>/${TARGET_NAME}_bootloader-trusted.bin
	)
endif()

target_include_directories(
    ${TARGET_NAME}_bootloader PRIVATE
	${CMAKE_CURRENT_LIST_DIR}/FSBL/Core/Inc
    ${CMAKE_CURRENT_LIST_DIR}/BSP
    ${CMAKE_CURRENT_LIST_DIR}/Components/Common
    ${CMAKE_CURRENT_LIST_DIR}/Middlewares/ST/STM32_ExtMem_Manager
	${CMAKE_CURRENT_LIST_DIR}/Middlewares/ST/STM32_ExtMem_Manager/boot
	${CMAKE_CURRENT_LIST_DIR}/Middlewares/ST/STM32_ExtMem_Manager/sal
	${CMAKE_CURRENT_LIST_DIR}/Middlewares/ST/STM32_ExtMem_Manager/nor_sfdp
	${CMAKE_CURRENT_LIST_DIR}/Middlewares/ST/STM32_ExtMem_Manager/psram
	${CMAKE_CURRENT_LIST_DIR}/Middlewares/ST/STM32_ExtMem_Manager/sdcard
	${CMAKE_CURRENT_LIST_DIR}/Middlewares/ST/STM32_ExtMem_Manager/user
    ${CMAKE_CURRENT_LIST_DIR}/../memtester
)

target_link_libraries(${TARGET_NAME}_bootloader PRIVATE common_flags stm32cube_headers)

target_link_options(${TARGET_NAME}_bootloader PRIVATE
    -T ${CMAKE_CURRENT_LIST_DIR}/FSBL/STM32N657X0HXQ_AXISRAM2_fsbl.ld
)
set_target_properties(${TARGET_NAME}_bootloader PROPERTIES LINK_DEPENDS ${CMAKE_CURRENT_LIST_DIR}/FSBL/STM32N657X0HXQ_AXISRAM2_fsbl.ld)

# Application

file(GLOB_RECURSE SOURCE_LIST
	Appli/*.c
	Appli/*.h
	Appli/*.cpp
	Appli/*.s
	Drivers/*.c
	Drivers/*.h
	Drivers/*.cpp
	BSP/*.c
	BSP/*.h
	BSP/*.cpp
	Components/*.c
	Components/*.h
	Components/*.cpp
)
# Exclude template files
list(FILTER SOURCE_LIST EXCLUDE REGEX .*/Source/Templates/.*)

add_library(stm32cube INTERFACE)
target_sources(stm32cube INTERFACE ${SOURCE_LIST})

target_include_directories(
    stm32cube INTERFACE
	${CMAKE_CURRENT_LIST_DIR}/Appli/Core/Inc
    ${CMAKE_CURRENT_LIST_DIR}/BSP
    ${CMAKE_CURRENT_LIST_DIR}/Components/Common
	${CMAKE_CURRENT_LIST_DIR}/Secure_nsclib
)

target_link_libraries(stm32cube INTERFACE stm32cube_headers)

target_link_options(stm32cube INTERFACE
    -T ${CMAKE_CURRENT_LIST_DIR}/Appli/STM32N657X0HXQ_LRUN.ld
)
set_target_properties(stm32cube PROPERTIES INTERFACE_LINK_DEPENDS ${CMAKE_CURRENT_LIST_DIR}/Appli/STM32N657X0HXQ_LRUN.ld)
