cmake_minimum_required(VERSION 3.13)
project(test_windowing)

add_executable(test_windowing test_windowing.cpp)


set(HOST ON)
set(DISABLEFLOAT16 OFF)

add_subdirectory("${CMAKE_SOURCE_DIR}/../src/CMSIS-DSP" cmsis_dsp)
target_compile_definitions(CMSISDSP PUBLIC __GNUC_PYTHON__ ARM_FLOAT16_SUPPORTED float16_t=_Float16)

add_executable(test_model
	test_model.cpp
	${CMAKE_SOURCE_DIR}/../src/damc/damc_audio_processing/TinyDenoiserModel/model.h
	${CMAKE_SOURCE_DIR}/../src/damc/damc_audio_processing/TinyDenoiserModel/model.c
	${CMAKE_SOURCE_DIR}/../src/damc/damc_audio_processing/TinyDenoiserModel/model_weights.h
)
target_link_libraries(test_model PRIVATE CMSISDSP)
