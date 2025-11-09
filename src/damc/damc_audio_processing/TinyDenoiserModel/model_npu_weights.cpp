
#if defined(STM32N657xx)

#include "model_npu_weights.h"
#include <cmath>
#include <math.h>

#include "model_weights.h"

template<size_t X, size_t Y, size_t Z> struct matrix_index_access {};

template<size_t X, size_t Y> struct matrix_index_access<1, X, Y> {
	static constexpr std::array<size_t, 2> dim = {X, Y};

	template<typename FloatT> static constexpr auto index(const FloatT (&weights)[1][X][Y], size_t x, size_t y) {
		return weights[0][x][y];
	}
};

template<size_t X, size_t Y> struct matrix_index_access<X, Y, 1> {
	static constexpr std::array<size_t, 2> dim = {X, Y};

	template<typename FloatT> static constexpr auto index(const FloatT (&weights)[X][Y][1], size_t x, size_t y) {
		return weights[x][y][0];
	}
};

template<size_t X, size_t Y, size_t Z, typename FloatT>
consteval auto make_weights_int16(const FloatT (&weights)[X][Y][Z]) {
	constexpr std::array<size_t, 2> dimensions = matrix_index_access<X, Y, Z>::dim;
	constexpr size_t convacc_split = model_weights_16bits<dimensions[0], dimensions[1]>::convacc_split;
	constexpr size_t input_size_aligned = model_weights_16bits<dimensions[0], dimensions[1]>::input_size_aligned;
	constexpr size_t output_size_aligned = model_weights_16bits<dimensions[0], dimensions[1]>::output_size_aligned;

	model_weights_16bits<dimensions[0], dimensions[1]> quantized_weights = {};

	float max = std::abs((float) matrix_index_access<X, Y, Z>::index(weights, 0, 0));
	float max_acc = 0;
	for(size_t i = 0; i < dimensions[0]; i++) {
		float acc = 0;
		for(size_t j = 0; j < dimensions[1]; j++) {
			float a = std::abs((float) matrix_index_access<X, Y, Z>::index(weights, i, j));
			acc += a;
			if(max < a)
				max = a;
		}
		if(acc > max_acc)
			max_acc = acc;
	}

	quantized_weights.scale = 32767.0f / max;

	uint32_t max_bit_in_weights = 0;  // Note: sign bit is not taken into account here
	uint32_t max_weights_sum_int = std::ceil(max_acc * quantized_weights.scale);
	while(max_weights_sum_int >>= 1) {
		max_bit_in_weights++;
	}
	// Output size is 24 bits, 16x16 output takes 32 bits
	// We need to shift right so the max output value of feature_max * weights_sum_max can be represented in 24 bits
	// output.
	// The output bit requirements without any shift is 1 for the sign + 15 (input) + max_bit_in_weights
	// To not overflow 24 bits, we need to shift:
	quantized_weights.convacc_shift_o = (1 + 15 + max_bit_in_weights) - 24;

	for(size_t m = 0; m < convacc_split; m++) {
		size_t o = 0;
		for(size_t i = 0; i < output_size_aligned / NPU_NKERNELS; i++) {
			for(size_t j = 0; j < input_size_aligned / NPU_BATCHSIZE; j++) {
				for(size_t k = 0; k < NPU_NKERNELS; k++) {
					for(size_t l = 0; l < NPU_BATCHSIZE; l++) {
						int32_t value = 0;
						size_t index1 = m * output_size_aligned + i * NPU_NKERNELS + k;
						size_t index2 = j * NPU_BATCHSIZE + l;

						if(index1 < dimensions[0] && index2 < dimensions[1]) {
							value = std::round((float) matrix_index_access<X, Y, Z>::index(weights, index1, index2) *
							                   quantized_weights.scale);
						}
						quantized_weights.data[m][o] = value;
						o++;
					}
				}
			}
		}
	}

	return quantized_weights;
}

template<size_t X> consteval auto make_weights_float16(const float (&weights)[X]) {
	std::array<_Float16, X> data = {};

	for(size_t i = 0; i < X; i++) {
		data[i] = (_Float16) weights[i];
	}

	return data;
}

template<size_t X, size_t Y> consteval auto make_weights_float16(const float (&weights)[X][Y]) {
	std::array<std::array<_Float16, Y>, X> data = {};

	for(size_t i = 0; i < X; i++) {
		for(size_t j = 0; j < Y; j++) {
			data[i][j] = (_Float16) weights[i][j];
		}
	}

	return data;
}

template<size_t X, size_t Y, size_t Z> consteval auto make_weights_float16(const float (&weights)[X][Y][Z]) {
	std::array<std::array<std::array<_Float16, Z>, Y>, X> data = {};

	for(size_t i = 0; i < X; i++) {
		for(size_t j = 0; j < Y; j++) {
			for(size_t k = 0; k < Z; k++) {
				data[i][j][k] = (_Float16) weights[i][j][k];
			}
		}
	}

	return data;
}

const model_weights_16bits<257, 257> compressed_tensor_ConvBnFusion_W_fc0_weight __attribute__((section(".npu_ram"))) =
    make_weights_int16<257, 257>(tensor_ConvBnFusion_W_fc0_weight);
const model_weights_16bits<1024, 257> compressed_tensor_97 __attribute__((section(".npu_ram"))) =
    make_weights_int16<1, 1024, 257>(tensor_97);
const model_weights_16bits<1024, 256> compressed_tensor_98 __attribute__((section(".npu_ram"))) =
    make_weights_int16<1, 1024, 256>(tensor_98);
const model_weights_16bits<1024, 256> compressed_tensor_165 __attribute__((section(".npu_ram"))) =
    make_weights_int16<1, 1024, 256>(tensor_165);
const model_weights_16bits<1024, 256> compressed_tensor_166 __attribute__((section(".npu_ram"))) =
    make_weights_int16<1, 1024, 256>(tensor_166);
const model_weights_16bits<257, 256> compressed_tensor_ConvBnFusion_W_fc1_weight __attribute__((section(".npu_ram"))) =
    make_weights_int16<257, 256>(tensor_ConvBnFusion_W_fc1_weight);
const model_weights_16bits<257, 257> compressed_tensor_fc2_weight __attribute__((section(".npu_ram"))) =
    make_weights_int16<257, 257>(tensor_fc2_weight);

const std::array<_Float16, 257> compressed_tensor_ConvBnFusion_BN_B_norm0_bias =
    make_weights_float16(tensor_ConvBnFusion_BN_B_norm0_bias);
const std::array<std::array<_Float16, 2048>, 1> compressed_tensor_99 = make_weights_float16(tensor_99);
const std::array<std::array<std::array<_Float16, 256>, 1>, 1> compressed_tensor_175 = make_weights_float16(tensor_175);
const std::array<std::array<_Float16, 2048>, 1> compressed_tensor_167 = make_weights_float16(tensor_167);
const std::array<_Float16, 257> compressed_tensor_ConvBnFusion_BN_B_norm1_bias =
    make_weights_float16(tensor_ConvBnFusion_BN_B_norm1_bias);
const std::array<_Float16, 257> compressed_tensor_fc2_bias = make_weights_float16(tensor_fc2_bias);

#endif
