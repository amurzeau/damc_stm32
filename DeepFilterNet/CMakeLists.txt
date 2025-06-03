cmake_minimum_required(VERSION 3.13)
project(test_windowing)

add_executable(test_windowing test_windowing.cpp)


set(HOST ON)
set(DISABLEFLOAT16 OFF)

add_subdirectory("${CMAKE_SOURCE_DIR}/../src/CMSIS-DSP" cmsis_dsp)
target_compile_definitions(CMSISDSP PUBLIC __GNUC_PYTHON__ ARM_FLOAT16_SUPPORTED float16_t=_Float16)

add_executable(test_model
	test_model.cpp
	${CMAKE_SOURCE_DIR}/../src/stm32n6570-dk/Appli/Core/Src/model.h
	${CMAKE_SOURCE_DIR}/../src/stm32n6570-dk/Appli/Core/Src/model_intrinsics.c
	${CMAKE_SOURCE_DIR}/../src/stm32n6570-dk/Appli/Core/Src/df_dec.c
	${CMAKE_SOURCE_DIR}/../src/stm32n6570-dk/Appli/Core/Src/df_dec_weights.h
)
target_compile_definitions(test_model PRIVATE STM32N657xx)
target_include_directories(test_model PRIVATE ${CMAKE_SOURCE_DIR}/../src/stm32n6570-dk/Appli/Core/Src)
target_link_libraries(test_model PRIVATE CMSISDSP)
