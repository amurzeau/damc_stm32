#pragma once

#include "model_npu_weights.h"
#include <iterator>
#include <stddef.h>
#include <stdint.h>
#include <type_traits>

extern "C" {
#include "ll_aton.h"
#include "ll_aton_NN_interface.h"
#include "ll_aton_dbgtrc.h"
#include "npu_cache.h"
#include "stm32n657xx.h"
#include <ll_aton_runtime.h>
}

#define ATONN_SRCPORT2(S, J, U, I, P) ATONN_SRCPORT(S, J, U, I, P)
#define ATONN_DSTPORT2(S, J, U, I, P) ATONN_DSTPORT(S, J, U, I, P)

template<size_t X, size_t Y>
uint32_t matmul_npu_impl(const int16_t* input, const model_weights_16bits<X, Y>& weights, uint8_t* output) {
	/* Unit= 13 [CONV_ACC_V2 0] */
	/* kind=Conv node=gemm_lsb */
	static LL_Convacc_InitTypeDef gemm_lsb = {
	    .rounding_f = 0,
	    .saturation_f = 0,
	    .round_mode_f = 0,
	    .inbytes_f = 2,
	    .rounding_o = 1,
	    .saturation_o = 1,
	    .round_mode_o = 1,
	    .relu_mode_o = 0,
	    .outbytes_o = 3,
	    .simd = 0,
	    .accumulate = 1,
	    .accumulate_first = 1,
	    .accumulate_gen_first = 1,
	    .raw_o = 0,
	    .deepmode = 0,
	    .dss2mode = 0,
	    .f_unsigned = 0,
	    .k_unsigned = 0,
	    .kseten = 0,
	    .shift_f = 0,
	    .shift_a = 0,
	    .shift_o = 0,
	    .fWidth = 1,
	    .fHeight = 1,
	    .kernelWidth = 1,
	    .kernelHeight = 1,
	    .nKernels = NPU_NKERNELS,
	    .batchDepth = NPU_BATCHSIZE,
	    .hstride = 1,
	    .vstride = 1,
	    .left_padding = 0,
	    .right_padding = 0,
	    .top_padding = 0,
	    .bot_padding = 0,
	    .left_crop = 0,
	    .right_crop = 0,
	    .top_crop = 0,
	    .bot_crop = 0,
	    .afilt_mode = AFILT_MODE_FRAMEZERO,
	    .afilt_tot = 8,
	    .afilt_first = 1,
	    .afilt_last = 7,
	    .fsub = 0,
	    .zfbias = 0,
	};

	static LL_Convacc_InitTypeDef gemm_msb = gemm_lsb;

	static LL_Convacc_InitTypeDef gemm2_lsb = gemm_lsb;
	static LL_Convacc_InitTypeDef gemm2_msb = gemm_msb;

	static LL_Streng_TensorInitTypeDef dma_in = {
	    .dir = 0,
	    .raw = 0,
	    .noblk = 0,
	    .align_right = 0,
	    .nbits_unsigned = 0,
	    .addr_base = {NULL},
	    .offset_start = 0,
	    .offset_limit = 0,
	    .frame_count = 0,
	    .fwidth = 1,
	    .fheight = 1,
	    .batch_depth = NPU_BATCHSIZE,
	    .batch_offset = 0,
	    .frame_offset = NPU_BATCHSIZE * 2u,
	    .line_offset = 0,
	    .loop_offset = 0,
	    .frame_loop_cnt = 0,
	    .frame_tot_cnt = 0,
	    .nbits_in = 16,
	    .nbits_out = 16,
	};

	static LL_Streng_TensorInitTypeDef dma_weight_lsb = {
	    .dir = 0,
	    .raw = 1,
	    .continuous = 1,
	    .noblk = 0,
	    .align_right = 0,
	    .nbits_unsigned = 0,
	    .addr_base = {NULL},
	    .offset_start = 0,
	    .offset_end = 0,
	    .offset_limit = 0,
	    .frame_count = 0,
	    .fwidth = 0,
	    .fheight = 0,
	    .batch_depth = 0,
	    .batch_offset = 0,
	    .frame_offset = 0,
	    .line_offset = 0,
	    .loop_offset = 0,
	    .frame_loop_cnt = 0,
	    .frame_tot_cnt = 1,
	    .nbits_in = 16,
	    .nbits_out = 16,
	};
	static LL_Streng_TensorInitTypeDef dma_weight_msb = dma_weight_lsb;
	static LL_Streng_TensorInitTypeDef dma_weight2_lsb = dma_weight_lsb;
	static LL_Streng_TensorInitTypeDef dma_weight2_msb = dma_weight_msb;

	static LL_Streng_TensorInitTypeDef dma_out_lsb = {
	    .dir = 1,
	    .raw = 1,
	    .noblk = 0,
	    .align_right = 0,
	    .nbits_unsigned = 0,
	    .addr_base = {NULL},
	    .offset_start = 0,
	    .offset_end = 0,
	    .offset_limit = 0,
	    .frame_count = 0,
	    .fwidth = 0,
	    .fheight = 0,
	    .batch_depth = 0,
	    .batch_offset = 0,
	    .frame_offset = 0,
	    .line_offset = 0,
	    .loop_offset = 0,
	    .frame_loop_cnt = 0,
	    .frame_tot_cnt = 1,
	    .nbits_in = 24,
	    .nbits_out = 24,
	};

	static LL_Streng_TensorInitTypeDef dma_out_msb = dma_out_lsb;
	static LL_Streng_TensorInitTypeDef dma_out2_lsb = dma_out_lsb;
	static LL_Streng_TensorInitTypeDef dma_out2_msb = dma_out2_lsb;

	size_t input_size_aligned = weights.input_size_aligned;
	size_t output_size_aligned = weights.output_size_aligned;

	gemm_lsb.afilt_tot = input_size_aligned / gemm_lsb.batchDepth;
	gemm_lsb.afilt_first = 1;
	gemm_lsb.afilt_last = gemm_lsb.afilt_tot - 1;
	gemm_lsb.shift_a = weights.convacc_shift_o;
	gemm_lsb.shift_o = weights.convacc_shift_o;

	gemm_msb.afilt_tot = gemm_lsb.afilt_tot;
	gemm_msb.afilt_first = gemm_lsb.afilt_first;
	gemm_msb.afilt_last = gemm_lsb.afilt_last;
	gemm_msb.shift_a = gemm_lsb.shift_a;
	gemm_msb.shift_o = gemm_lsb.shift_o;

	gemm2_lsb.afilt_tot = gemm_lsb.afilt_tot;
	gemm2_lsb.afilt_first = gemm_lsb.afilt_first;
	gemm2_lsb.afilt_last = gemm_lsb.afilt_last;
	gemm2_lsb.shift_a = gemm_lsb.shift_a;
	gemm2_lsb.shift_o = gemm_lsb.shift_o;

	gemm2_msb.afilt_tot = gemm_lsb.afilt_tot;
	gemm2_msb.afilt_first = gemm_lsb.afilt_first;
	gemm2_msb.afilt_last = gemm_lsb.afilt_last;
	gemm2_msb.shift_a = gemm_lsb.shift_a;
	gemm2_msb.shift_o = gemm_lsb.shift_o;

	dma_in.addr_base.p = (uint8_t*) input;
	dma_in.offset_limit = input_size_aligned * 2;
	dma_in.batch_offset = dma_in.offset_limit;
	dma_in.frame_loop_cnt = input_size_aligned / gemm_lsb.batchDepth;
	dma_in.frame_tot_cnt = output_size_aligned / gemm_lsb.nKernels * gemm_lsb.afilt_tot;

	dma_weight_lsb.addr_base.p = (uint8_t*) weights.data[0].data();
	dma_weight_lsb.offset_end = output_size_aligned * input_size_aligned * 2;
	dma_weight_lsb.offset_limit = dma_weight_lsb.offset_end;

	dma_weight_msb.addr_base.p = (uint8_t*) weights.data[1].data();
	dma_weight_msb.offset_end = dma_weight_lsb.offset_end;
	dma_weight_msb.offset_limit = dma_weight_lsb.offset_end;

	dma_weight2_lsb.addr_base.p = (uint8_t*) weights.data[2].data();
	dma_weight2_lsb.offset_end = dma_weight_lsb.offset_end;
	dma_weight2_lsb.offset_limit = dma_weight_lsb.offset_end;

	dma_weight2_msb.addr_base.p = (uint8_t*) weights.data[3].data();
	dma_weight2_msb.offset_end = dma_weight_lsb.offset_end;
	dma_weight2_msb.offset_limit = dma_weight_lsb.offset_end;

	dma_out_lsb.addr_base.p = (uint8_t*) output;
	dma_out_lsb.offset_end = output_size_aligned * 3;
	dma_out_lsb.offset_limit = dma_out_lsb.offset_end;
	dma_out_lsb.frame_offset = dma_out_lsb.offset_end;

	dma_out_msb.addr_base.p = (uint8_t*) output + output_size_aligned * 3;
	dma_out_msb.offset_end = output_size_aligned * 3;
	dma_out_msb.offset_limit = dma_out_lsb.offset_end;
	dma_out_msb.frame_offset = dma_out_lsb.offset_end;

	dma_out2_lsb.addr_base.p = (uint8_t*) output + output_size_aligned * 3 * 2;
	dma_out2_lsb.offset_end = output_size_aligned * 3;
	dma_out2_lsb.offset_limit = dma_out_lsb.offset_end;
	dma_out2_lsb.frame_offset = dma_out_lsb.offset_end;

	dma_out2_msb.addr_base.p = (uint8_t*) output + output_size_aligned * 3 * 3;
	dma_out2_msb.offset_end = output_size_aligned * 3;
	dma_out2_msb.offset_limit = dma_out_lsb.offset_end;
	dma_out2_msb.frame_offset = dma_out_lsb.offset_end;

#define STRENG_INDEX_DMA_IN 0
#define STRENG_INDEX_DMA_WEIGHT_LSB 1
#define STRENG_INDEX_DMA_WEIGHT_MSB 2
#define STRENG_INDEX_DMA_WEIGHT2_LSB 5
#define STRENG_INDEX_DMA_WEIGHT2_MSB 6
#define STRENG_INDEX_DMA_OUT_LSB 7
#define STRENG_INDEX_DMA_OUT_MSB 8
#define STRENG_INDEX_DMA_OUT2_LSB 3
#define STRENG_INDEX_DMA_OUT2_MSB 4

	static const LL_Switch_InitTypeDef switch_init[] = {
	    // CONVACC 0: gemm_lsb
	    {
	        LL_Switch_Init_Source(0) = ATONN_SRCPORT2(STRSWITCH, 0, STRENG, STRENG_INDEX_DMA_IN, 0),
	        LL_Switch_Init_Dest() = ATONN_DSTPORT2(STRSWITCH, 0, CONVACC, 0, 0),
	        LL_Switch_Init_Frames(0) = 0,
	        LL_Switch_Init_Context(0) = 1,
	    },
	    {
	        LL_Switch_Init_Source(0) = ATONN_SRCPORT2(STRSWITCH, 0, STRENG, STRENG_INDEX_DMA_WEIGHT_LSB, 0),
	        LL_Switch_Init_Dest() = ATONN_DSTPORT2(STRSWITCH, 0, CONVACC, 0, 1),
	        LL_Switch_Init_Frames(0) = 0,
	        LL_Switch_Init_Context(0) = 1,
	    },
	    {
	        LL_Switch_Init_Source(0) = ATONN_SRCPORT2(STRSWITCH, 0, CONVACC, 0, 0),
	        LL_Switch_Init_Dest() = ATONN_DSTPORT2(STRSWITCH, 0, CONVACC, 0, 2),
	        LL_Switch_Init_Frames(0) = 0,
	        LL_Switch_Init_Context(0) = 1,
	    },
	    // CONVACC 1: gemm_msb
	    {
	        LL_Switch_Init_Source(0) = ATONN_SRCPORT2(STRSWITCH, 0, STRENG, STRENG_INDEX_DMA_IN, 0),
	        LL_Switch_Init_Dest() = ATONN_DSTPORT2(STRSWITCH, 0, CONVACC, 1, 0),
	        LL_Switch_Init_Frames(0) = 0,
	        LL_Switch_Init_Context(0) = 1,
	    },
	    {
	        LL_Switch_Init_Source(0) = ATONN_SRCPORT2(STRSWITCH, 0, STRENG, STRENG_INDEX_DMA_WEIGHT_MSB, 0),
	        LL_Switch_Init_Dest() = ATONN_DSTPORT2(STRSWITCH, 0, CONVACC, 1, 1),
	        LL_Switch_Init_Frames(0) = 0,
	        LL_Switch_Init_Context(0) = 1,
	    },
	    {
	        LL_Switch_Init_Source(0) = ATONN_SRCPORT2(STRSWITCH, 0, CONVACC, 1, 0),
	        LL_Switch_Init_Dest() = ATONN_DSTPORT2(STRSWITCH, 0, CONVACC, 1, 2),
	        LL_Switch_Init_Frames(0) = 0,
	        LL_Switch_Init_Context(0) = 1,
	    },
	    // CONVACC 2: gemm2_lsb
	    {
	        LL_Switch_Init_Source(0) = ATONN_SRCPORT2(STRSWITCH, 0, STRENG, STRENG_INDEX_DMA_IN, 0),
	        LL_Switch_Init_Dest() = ATONN_DSTPORT2(STRSWITCH, 0, CONVACC, 2, 0),
	        LL_Switch_Init_Frames(0) = 0,
	        LL_Switch_Init_Context(0) = 1,
	    },
	    {
	        LL_Switch_Init_Source(0) = ATONN_SRCPORT2(STRSWITCH, 0, STRENG, STRENG_INDEX_DMA_WEIGHT2_LSB, 0),
	        LL_Switch_Init_Dest() = ATONN_DSTPORT2(STRSWITCH, 0, CONVACC, 2, 1),
	        LL_Switch_Init_Frames(0) = 0,
	        LL_Switch_Init_Context(0) = 1,
	    },
	    {
	        LL_Switch_Init_Source(0) = ATONN_SRCPORT2(STRSWITCH, 0, CONVACC, 2, 0),
	        LL_Switch_Init_Dest() = ATONN_DSTPORT2(STRSWITCH, 0, CONVACC, 2, 2),
	        LL_Switch_Init_Frames(0) = 0,
	        LL_Switch_Init_Context(0) = 1,
	    },
	    // CONVACC 3: gemm2_msb
	    {
	        LL_Switch_Init_Source(0) = ATONN_SRCPORT2(STRSWITCH, 0, STRENG, STRENG_INDEX_DMA_IN, 0),
	        LL_Switch_Init_Dest() = ATONN_DSTPORT2(STRSWITCH, 0, CONVACC, 3, 0),
	        LL_Switch_Init_Frames(0) = 0,
	        LL_Switch_Init_Context(0) = 1,
	    },
	    {
	        LL_Switch_Init_Source(0) = ATONN_SRCPORT2(STRSWITCH, 0, STRENG, STRENG_INDEX_DMA_WEIGHT2_MSB, 0),
	        LL_Switch_Init_Dest() = ATONN_DSTPORT2(STRSWITCH, 0, CONVACC, 3, 1),
	        LL_Switch_Init_Frames(0) = 0,
	        LL_Switch_Init_Context(0) = 1,
	    },
	    {
	        LL_Switch_Init_Source(0) = ATONN_SRCPORT2(STRSWITCH, 0, CONVACC, 3, 0),
	        LL_Switch_Init_Dest() = ATONN_DSTPORT2(STRSWITCH, 0, CONVACC, 3, 2),
	        LL_Switch_Init_Frames(0) = 0,
	        LL_Switch_Init_Context(0) = 1,
	    },
	    // Output
	    {
	        LL_Switch_Init_Source(0) = ATONN_SRCPORT2(STRSWITCH, 0, CONVACC, 0, 0),
	        LL_Switch_Init_Source(1) = ATONN_SRCPORT2(STRSWITCH, 0, CONVACC, 0, 0),
	        LL_Switch_Init_Dest() = ATONN_DSTPORT2(STRSWITCH, 0, STRENG, STRENG_INDEX_DMA_OUT_LSB, 0),
	        LL_Switch_Init_Frames(0) = (uint8_t) (gemm_lsb.afilt_tot - 1),
	        LL_Switch_Init_Frames(1) = 1,
	        LL_Switch_Init_Context(0) = 0,
	        LL_Switch_Init_Context(1) = 1,
	    },
	    {
	        LL_Switch_Init_Source(0) = ATONN_SRCPORT2(STRSWITCH, 0, CONVACC, 1, 0),
	        LL_Switch_Init_Source(1) = ATONN_SRCPORT2(STRSWITCH, 0, CONVACC, 1, 0),
	        LL_Switch_Init_Dest() = ATONN_DSTPORT2(STRSWITCH, 0, STRENG, STRENG_INDEX_DMA_OUT_MSB, 0),
	        LL_Switch_Init_Frames(0) = (uint8_t) (gemm_msb.afilt_tot - 1),
	        LL_Switch_Init_Frames(1) = 1,
	        LL_Switch_Init_Context(0) = 0,
	        LL_Switch_Init_Context(1) = 1,
	    },
	    {
	        LL_Switch_Init_Source(0) = ATONN_SRCPORT2(STRSWITCH, 0, CONVACC, 2, 0),
	        LL_Switch_Init_Source(1) = ATONN_SRCPORT2(STRSWITCH, 0, CONVACC, 2, 0),
	        LL_Switch_Init_Dest() = ATONN_DSTPORT2(STRSWITCH, 0, STRENG, STRENG_INDEX_DMA_OUT2_LSB, 0),
	        LL_Switch_Init_Frames(0) = (uint8_t) (gemm2_lsb.afilt_tot - 1),
	        LL_Switch_Init_Frames(1) = 1,
	        LL_Switch_Init_Context(0) = 0,
	        LL_Switch_Init_Context(1) = 1,
	    },
	    {
	        LL_Switch_Init_Source(0) = ATONN_SRCPORT2(STRSWITCH, 0, CONVACC, 3, 0),
	        LL_Switch_Init_Source(1) = ATONN_SRCPORT2(STRSWITCH, 0, CONVACC, 3, 0),
	        LL_Switch_Init_Dest() = ATONN_DSTPORT2(STRSWITCH, 0, STRENG, STRENG_INDEX_DMA_OUT2_MSB, 0),
	        LL_Switch_Init_Frames(0) = (uint8_t) (gemm2_msb.afilt_tot - 1),
	        LL_Switch_Init_Frames(1) = 1,
	        LL_Switch_Init_Context(0) = 0,
	        LL_Switch_Init_Context(1) = 1,
	    },
	};
	static const LL_ATON_EnableUnits_InitTypeDef dma_units[] = {
	    {{STRENG, STRENG_INDEX_DMA_OUT2_MSB}},
	    {{STRENG, STRENG_INDEX_DMA_OUT2_LSB}},
	    {{STRENG, STRENG_INDEX_DMA_OUT_MSB}},
	    {{STRENG, STRENG_INDEX_DMA_OUT_LSB}},
	    {{CONVACC, 0}},
	    {{CONVACC, 1}},
	    {{CONVACC, 2}},
	    {{CONVACC, 3}},
	    {{STRENG, STRENG_INDEX_DMA_WEIGHT2_MSB}},
	    {{STRENG, STRENG_INDEX_DMA_WEIGHT2_LSB}},
	    {{STRENG, STRENG_INDEX_DMA_WEIGHT_MSB}},
	    {{STRENG, STRENG_INDEX_DMA_WEIGHT_LSB}},
	    {{STRENG, STRENG_INDEX_DMA_IN}},
	};

#define streng_out_mask \
	((1 << STRENG_INDEX_DMA_OUT2_MSB) | (1 << STRENG_INDEX_DMA_OUT2_LSB) | (1 << STRENG_INDEX_DMA_OUT_MSB) | \
	 (1 << STRENG_INDEX_DMA_OUT_LSB))

#define streng_in_mask \
	((1 << STRENG_INDEX_DMA_IN) | (1 << STRENG_INDEX_DMA_WEIGHT_LSB) | (1 << STRENG_INDEX_DMA_WEIGHT_MSB) | \
	 (1 << STRENG_INDEX_DMA_WEIGHT2_LSB) | (1 << STRENG_INDEX_DMA_WEIGHT2_MSB))

	SCB_CleanDCache_by_Addr((volatile void*) input, weights.input_size * 2);

	LL_Streng_TensorInit(STRENG_INDEX_DMA_IN, &dma_in, 1);
	LL_Streng_TensorInit(STRENG_INDEX_DMA_WEIGHT_LSB, &dma_weight_lsb, 1);
	LL_Streng_TensorInit(STRENG_INDEX_DMA_WEIGHT_MSB, &dma_weight_msb, 1);
	LL_Streng_TensorInit(STRENG_INDEX_DMA_WEIGHT2_LSB, &dma_weight2_lsb, 1);
	LL_Streng_TensorInit(STRENG_INDEX_DMA_WEIGHT2_MSB, &dma_weight2_msb, 1);
	LL_Convacc_Init(0, &gemm_lsb);
	LL_Convacc_Init(1, &gemm_msb);
	LL_Convacc_Init(2, &gemm2_lsb);
	LL_Convacc_Init(3, &gemm2_msb);
	LL_Streng_TensorInit(STRENG_INDEX_DMA_OUT_LSB, &dma_out_lsb, 1);
	LL_Streng_TensorInit(STRENG_INDEX_DMA_OUT_MSB, &dma_out_msb, 1);
	LL_Streng_TensorInit(STRENG_INDEX_DMA_OUT2_LSB, &dma_out2_lsb, 1);
	LL_Streng_TensorInit(STRENG_INDEX_DMA_OUT2_MSB, &dma_out2_msb, 1);

	// See https://stedgeai-dc.st.com/assets/embedded-docs/stneuralart_profiler.html
	// LL_Dbgtrc_EnableClock();
	// LL_Dbgtrc_Init(0);

	// LL_Dbgtrc_Count_StrengActive_Config(streng_in_mask, streng_out_mask, 0);
	// LL_Dbgtrc_Count_StrengActive_Start(streng_in_mask, streng_out_mask, 0);

	LL_Switch_Init(switch_init, sizeof(switch_init) / sizeof(switch_init[0]));

	LL_ATON_EnableUnits_Init(dma_units, sizeof(dma_units) / sizeof(dma_units[0]));
	LL_Streng_Wait(streng_out_mask);
	LL_ATON_DisableUnits_Init(dma_units, sizeof(dma_units) / sizeof(dma_units[0]));

	LL_Switch_Deinit(switch_init, sizeof(switch_init) / sizeof(switch_init[0]));

	// LL_Dbgtrc_Count_StrengActive_Stop(streng_in_mask, streng_out_mask, 0);

	SCB_InvalidateDCache_by_Addr(output, weights.output_size * 3);

	return 0;
}

static int16_t npu_input[512] __attribute__((aligned(64))) __attribute__((section(".dtcm")));
static uint8_t npu_output[64 + 1024 * 3] __attribute__((aligned(64))) __attribute__((section(".dtcm")));

// This implementation requires the byteArray to be valid at offset -1
static int32_t interpret24bitAsInt32(uint8_t* byteArray) {
	// return ((byteArray[0] << 8) | (byteArray[1] << 16) | (byteArray[2] << 24));
	return (*(int32_t*) (byteArray - 1));
}

template<class T> constexpr auto get_array_data(T data) {
	return data;
}

template<class T, size_t N> constexpr auto get_array_data(const std::array<T, N>& data) {
	return data.data();
}

template<size_t X, size_t Y, typename InputT, typename BiasT, typename OutputT>
void matmul_npu(const model_weights_16bits<X, Y>& weights, InputT& vec, BiasT& bias, OutputT& pDst) {
	// ARM_PMU_CYCCNT_Reset();

	static_assert(Y <= std::size(npu_input), "npu_input array is not large enough");
	static_assert(X * 3 + 64 <= std::size(npu_output), "npu_output array is not large enough");
	static_assert(sizeof(std::remove_pointer_t<std::remove_reference_t<decltype(vec)>>) == Y * sizeof(_Float16),
	              "wrong input size");
	static_assert(sizeof(std::remove_pointer_t<std::remove_reference_t<decltype(bias)>>) == X * sizeof(_Float16),
	              "wrong bias size");
	static_assert(sizeof(std::remove_pointer_t<std::remove_reference_t<decltype(pDst)>>) == X * sizeof(_Float16),
	              "wrong output size");

	const _Float16* input_vec = (const _Float16*) get_array_data(vec);
	const _Float16* input_bias = (const _Float16*) get_array_data(bias);
	_Float16* output_vec = (_Float16*) get_array_data(pDst);

	float max = fabsf((float) input_vec[0]);
	for(size_t i = 0; i < Y; i++) {
		max = fmaxf(max, fabsf((float) input_vec[i]));
	}

	// Using float16 here cause too much accuracy loss when max is < 1.0 and will cause int16 value overflows.
	float input_scale = 32767.0f / max;

	for(size_t i = 0; i < Y; i++) {
		int32_t value = roundf((float) input_vec[i] * input_scale);
		npu_input[i] = value;
	}

	// uint32_t npu_start_time = ARM_PMU_Get_CCNTR();
	matmul_npu_impl(npu_input, weights, npu_output + 64);
	// uint32_t npu_time = ARM_PMU_Get_CCNTR() - npu_start_time;
	/*
	uint32_t npu_remove_time = 0;
	uint32_t npu_time = 0;
	*/

	// Compensate quantization in activations, weights and shift in CONVACC
	float output_scale = (1u << weights.convacc_shift_o) / (input_scale * weights.scale);

	// printf("Output:\n");
	for(size_t i = 0; i < X; i++) {
		int32_t value = interpret24bitAsInt32(&npu_output[64 + i * 3]) >> 8;
		output_vec[i] = (_Float16) ((float) input_bias[i] + value * output_scale);
	}

	// uint32_t time = ARM_PMU_Get_CCNTR();
	// LL_Dbgtrc_Count_StrengActive_Print(streng_in_mask, streng_out_mask, 0);
}
