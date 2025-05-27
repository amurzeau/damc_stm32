#pragma once

#ifndef __x86_64
#include <arm_mve.h>
#endif

typedef float16_t model_data_type_t;

#ifdef __cplusplus
extern "C" {
#endif

void tinydenoiser_model_init();
void tinydenoiser_model_reset();
void tinydenoiser_inference(const model_data_type_t tensor_input[1][257][1],
                            model_data_type_t tensor_output[1][257][1]);
void tinydenoiser_run(const float tensor_input[512], float tensor_output[512]);

#ifdef __cplusplus
}
#endif
