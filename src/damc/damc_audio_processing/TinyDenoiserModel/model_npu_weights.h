#pragma once

#include <array>
#include <stddef.h>
#include <stdint.h>

#define NPU_NKERNELS 16
#define NPU_BATCHSIZE 16

template<size_t X, size_t Y> struct model_weights_16bits {
	static constexpr size_t input_size = Y;
	static constexpr size_t output_size = X;
	static constexpr size_t convacc_split = 4;

	static constexpr size_t output_size_per_convacc = (output_size + convacc_split - 1) / convacc_split;
	static constexpr size_t input_size_aligned = ((input_size + NPU_BATCHSIZE - 1) / NPU_BATCHSIZE) * NPU_BATCHSIZE;
	static constexpr size_t output_size_aligned =
	    ((output_size_per_convacc + NPU_NKERNELS - 1) / NPU_NKERNELS) * NPU_NKERNELS;
	static constexpr size_t weights_size_aligned = input_size_aligned * output_size_aligned;

	std::array<int16_t, weights_size_aligned> data[convacc_split] __attribute__((aligned(64)));
	float scale;
	uint32_t convacc_shift_o;
};

extern const model_weights_16bits<257, 257> compressed_tensor_ConvBnFusion_W_fc0_weight;
extern const model_weights_16bits<1024, 256> compressed_tensor_165;
extern const model_weights_16bits<1024, 256> compressed_tensor_166;
extern const model_weights_16bits<257, 256> compressed_tensor_ConvBnFusion_W_fc1_weight;
extern const model_weights_16bits<257, 257> compressed_tensor_fc2_weight;

extern const std::array<_Float16, 257> compressed_tensor_ConvBnFusion_BN_B_norm0_bias;
extern const std::array<std::array<std::array<_Float16, 256>, 1>, 1> compressed_tensor_175;
extern const std::array<std::array<_Float16, 2048>, 1> compressed_tensor_167;
extern const std::array<_Float16, 257> compressed_tensor_ConvBnFusion_BN_B_norm1_bias;
extern const std::array<_Float16, 257> compressed_tensor_fc2_bias;

extern const std::array<std::array<_Float16, 2048>, 1> compressed_tensor_99;
extern const model_weights_16bits<1024, 257> compressed_tensor_97;
extern const model_weights_16bits<1024, 256> compressed_tensor_98;
