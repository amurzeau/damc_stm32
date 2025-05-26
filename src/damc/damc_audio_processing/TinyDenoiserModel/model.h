#pragma once

#include <arm_mve.h>

typedef float16_t model_data_type_t;

#ifdef __cplusplus
extern "C" {
#endif

void tinydenoiser_model_init();
void tinydenoiser_model_reset();
void tinydenoiser_run(const float tensor_input[512], float tensor_output[512]);

#ifdef __cplusplus
}
#endif
