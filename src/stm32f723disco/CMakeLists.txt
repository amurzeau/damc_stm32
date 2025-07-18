cmake_minimum_required(VERSION 3.20)


add_library(stm32cube_headers INTERFACE)

target_include_directories(stm32cube_headers INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/Core/Inc
    ${CMAKE_CURRENT_LIST_DIR}/Drivers/STM32F7xx_HAL_Driver/Inc
    ${CMAKE_CURRENT_LIST_DIR}/Drivers/STM32F7xx_HAL_Driver/Inc/Legacy
    ${CMAKE_CURRENT_LIST_DIR}/Drivers/CMSIS/Device/ST/STM32F7xx/Include
    ${CMAKE_CURRENT_LIST_DIR}/Drivers/CMSIS/Include
)

target_compile_definitions(stm32cube_headers INTERFACE
    "$<$<CONFIG:Debug>:DEBUG>"
    "$<$<CONFIG:Debug>:USE_FULL_ASSERT>"
    USE_STM32F723E_DISCO_REVD
    USE_HAL_DRIVER
    STM32F723xx
	PLATFORM_HEADER=\"stm32f723xx.h\"
	PLATFORM_HAL_HEADER=\"stm32f7xx_hal.h\"
)


file(GLOB_RECURSE SOURCE_LIST
	BSP/*.c BSP/*.h BSP/*.cpp
	Components/*.c Components/*.h Components/*.cpp
	Core/*.c Core/*.h Core/*.cpp Core/*.s
	Drivers/*.c Drivers/*.h Drivers/*.cpp
)

add_library(stm32cube INTERFACE)
target_sources(stm32cube INTERFACE ${SOURCE_LIST})

target_include_directories(
    stm32cube INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/BSP
)

target_link_libraries(stm32cube INTERFACE stm32cube_headers)

target_link_options(stm32cube INTERFACE
    -T ${CMAKE_CURRENT_LIST_DIR}/STM32F723IEKX_FLASH.ld
)
set_target_properties(stm32cube PROPERTIES INTERFACE_LINK_DEPENDS ${CMAKE_CURRENT_LIST_DIR}/STM32F723IEKX_FLASH.ld)
