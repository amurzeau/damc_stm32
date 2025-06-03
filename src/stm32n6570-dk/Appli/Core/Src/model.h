#pragma once

#if defined(STM32N657xx)

#include <stdint.h>

#ifndef __x86_64
#include <arm_mve.h>
#endif

typedef _Float16 model_data_type_t;

#ifdef __cplusplus
extern "C" {
#endif

void arm_nn_sigmoid_f16(const float16_t *pSrc, float16_t *pDst, uint32_t blockSize);
void arm_nn_tanh_f16(const float16_t *pSrc, float16_t *pDst, uint32_t blockSize);
void arm_mat_vec_mult_add_f16(uint32_t numRows, uint32_t numCols, const model_data_type_t *mat, const model_data_type_t *vec, model_data_type_t *pDst);
void arm_elementwise_max_f16(const float16_t *pSrc, float16_t *pDst, float16_t low, uint32_t numSamples);
void arm_mult_accumulate_f16(const float16_t *pSrcA, float16_t pSrcB, float16_t *pDst, uint32_t blockSize);

__attribute__((noinline)) void deepfilternet_run_enc(const model_data_type_t tensor_feat_erb[1][1][1][32],
                                                     const model_data_type_t tensor_feat_spec[1][2][1][96],
                                                     model_data_type_t tensor_e0[1][64][1][32],
                                                     model_data_type_t tensor_e1[1][64][1][16],
                                                     model_data_type_t tensor_e2[1][64][1][8],
                                                     model_data_type_t tensor_e3[1][64][1][8],
                                                     model_data_type_t tensor_emb[1][1][512],
                                                     model_data_type_t tensor_c0[1][64][1][96],
                                                     model_data_type_t tensor_lsnr[1][1][1]);

__attribute__((noinline)) void deepfilternet_run_erb_dec(const model_data_type_t tensor_emb[1][1][512],
                                                         const model_data_type_t tensor_e3[1][64][1][8],
                                                         const model_data_type_t tensor_e2[1][64][1][8],
                                                         const model_data_type_t tensor_e1[1][64][1][16],
                                                         const model_data_type_t tensor_e0[1][64][1][32],
                                                         model_data_type_t tensor_m[1][1][1][32]);

__attribute__((noinline)) void deepfilternet_run_df_dec(const model_data_type_t tensor_emb[1][1][512],
                                                        const model_data_type_t tensor_c0[1][64][1][96],
                                                        model_data_type_t tensor_coefs[1][1][96][10],
                                                        model_data_type_t tensor_235[1][1][1]);

void save_counters(const char *name);

#ifdef __cplusplus
}
#endif

#endif