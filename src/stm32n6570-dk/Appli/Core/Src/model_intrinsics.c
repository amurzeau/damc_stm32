#include "dsp/statistics_functions_f16.h"
#include "model.h"
#include <arm_math_types.h>
#include <math.h>
#include <arm_mve.h>
#include <arm_vec_math_f16.h>

/**
  @brief         Floating-point vector of exp values.
  @param[in]     pSrc       points to the input vector
  @param[out]    pDst       points to the output vector
  @param[in]     blockSize  number of samples in each vector
 */
void arm_nn_sigmoid_f16(const float16_t *pSrc, float16_t *pDst, uint32_t blockSize)
{
  uint32_t blkCnt;

#if defined(ARM_MATH_MVE_FLOAT16) && !defined(ARM_MATH_AUTOVECTORIZE)

  f16x8_t src;
  f16x8_t data;

  blkCnt = blockSize >> 3;

  while (blkCnt > 0U)
  {
    src = vld1q(pSrc);

    data = vexpq_f16(src);
    f16x8_t den = vaddq_n_f16(data, 1.0f);
    f16x8_t result_for_negative_input = vdiv_f16(data, den);

    // Above 9, the above computation will produce bad NaN values
    data = vdupq_m_n_f16(result_for_negative_input, 1.0f16, vcmpgeq_n_f16(src, 9.0f16));

    vst1q(pDst, data);

    pSrc += 8;
    pDst += 8;
    /* Decrement loop counter */
    blkCnt--;
  }

  blkCnt = blockSize & 7;
#else
  blkCnt = blockSize;
#endif

  while (blkCnt > 0U)
  {
    /* C = log(A) */

    /* Calculate log and store result in destination buffer. */
    *pDst++ = 1.0f / (1.0f + expf(-(*pSrc++)));

    // float32_t e = expf((*pSrc++));
    //*pDst++ = e / (1.0f16 + e);

    /* Decrement loop counter */
    blkCnt--;
  }
}

#if defined(ARM_MATH_MVE_FLOAT16) && !defined(ARM_MATH_AUTOVECTORIZE)
__STATIC_INLINE f16x8_t vtanhq_hiprec_f16(f16x8_t val)
{
  f16x8_t x = vminnmq_f16(vmaxnmq_f16(val, vdupq_n_f16(-5.f16)), vdupq_n_f16(5.0f16));
  f16x8_t exp2x = vexpq_f16(vmulq_n_f16(x, 2.f16));
  f16x8_t num = vsubq_n_f16(exp2x, 1.f16);
  f16x8_t den = vaddq_n_f16(exp2x, 1.f16);
  f16x8_t tanh = vmulq_f16(num, vrecip_hiprec_f16(den));
  return tanh;
}
#endif

/**
  @brief         Floating-point vector of exp values.
  @param[in]     pSrc       points to the input vector
  @param[out]    pDst       points to the output vector
  @param[in]     blockSize  number of samples in each vector
 */
void arm_nn_tanh_f16(const float16_t *pSrc, float16_t *pDst, uint32_t blockSize)
{
  uint32_t blkCnt;

#if defined(ARM_MATH_MVE_FLOAT16) && !defined(ARM_MATH_AUTOVECTORIZE)

  f16x8_t data;

  blkCnt = blockSize >> 3;

  while (blkCnt > 0U)
  {
    data = vld1q(pSrc);

    data = vtanhq_hiprec_f16(data);

    vst1q(pDst, data);

    pSrc += 8;
    pDst += 8;
    /* Decrement loop counter */
    blkCnt--;
  }

  blkCnt = blockSize & 7;
#else
  blkCnt = blockSize;
#endif

  while (blkCnt > 0U)
  {
    /* C = log(A) */

    /* Calculate log and store result in destination buffer. */
    *pDst++ = tanhf(*pSrc++);

    /* Decrement loop counter */
    blkCnt--;
  }
}


void arm_mat_vec_mult_add_f16(uint32_t numRows, uint32_t numCols, const float16_t *mat, const float16_t *vec, float16_t *pDst)
{
  for (int h = 0; h < numRows; h++)
  {
    for (int i = 0; i < numCols; i++)
    {
      pDst[h] += (float16_t)(vec[i] * mat[h * numCols + i]);
    }
  }
}


void arm_elementwise_max_f16(const float16_t *pSrc, float16_t *pDst, float16_t low, uint32_t numSamples)
{
  uint32_t blkCnt;
  f16x8_t curVec0, curVec1;
  f16x8_t vecLow;

  vecLow = vdupq_n_f16(low);

  curVec0 = vld1q(pSrc);
  pSrc += 8;
  /*
     * unrolled x 2 to allow
     * vldr/vstr/vmin/vmax
     * stall free interleaving
     */
  blkCnt = numSamples >> 4;
  while (blkCnt--)
  {
    curVec0 = vmaxnmq(curVec0, vecLow);
    curVec1 = vld1q(pSrc);
    pSrc += 8;
    vst1q(pDst, curVec0);
    pDst += 8;
    curVec1 = vmaxnmq(curVec1, vecLow);
    curVec0 = vld1q(pSrc);
    pSrc += 8;
    vst1q(pDst, curVec1);
    pDst += 8;
  }
  /*
     * Tail handling
     */
  blkCnt = numSamples - ((numSamples >> 4) << 4);
  if (blkCnt >= 8)
  {
    curVec0 = vmaxnmq(curVec0, vecLow);
    vst1q(pDst, curVec0);
    pDst += 8;
    curVec0 = vld1q(pSrc);
    pSrc += 8;
  }

  if (blkCnt > 0)
  {
    mve_pred16_t p0 = vctp16q(blkCnt & 7);
    curVec0 = vmaxnmq(curVec0, vecLow);
    vstrhq_p(pDst, curVec0, p0);
  }
}
