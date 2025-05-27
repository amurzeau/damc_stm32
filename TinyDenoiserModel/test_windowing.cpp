#include <algorithm>
#include <array>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

static constexpr size_t WINDOW_SIZE = 512;
static constexpr size_t WINDOW_MARGIN = 16;   // 1ms margin as we need to have a window every 6.25ms
static constexpr size_t WINDOW_PERIOD = 100;  // 6.25ms
std::array<uint32_t, WINDOW_SIZE + WINDOW_MARGIN> sampleWindowInput;
std::array<uint32_t, WINDOW_SIZE + WINDOW_MARGIN> sampleWindowOutput;
size_t sampleAddedToWindowSinceLastModelRun;

std::array<uint32_t, WINDOW_SIZE> modelWindowInput;
std::array<uint32_t, WINDOW_SIZE> modelWindowOutput;

void processSamples(uint32_t** samples, size_t count) {
	size_t resampledCount16khz = count / 3;

	// Downsample signal and put new sample to the window end
	for(size_t i = 0; i < resampledCount16khz; i++) {
		sampleWindowInput[sampleWindowInput.size() - WINDOW_MARGIN - WINDOW_PERIOD +
		                  sampleAddedToWindowSinceLastModelRun + i] = samples[0][i * 3];
	}

	// Put denoised data to output
	// Upsample back to 48Khz
	for(size_t i = 0; i < resampledCount16khz; i++) {
		uint32_t value = sampleWindowOutput[sampleAddedToWindowSinceLastModelRun + i];
		samples[0][i * 3] = value;
		samples[0][i * 3 + 1] = value;
		samples[0][i * 3 + 2] = value;
	}
	// Filter upsampled signal

	sampleAddedToWindowSinceLastModelRun += resampledCount16khz;
	if(sampleAddedToWindowSinceLastModelRun >= WINDOW_PERIOD) {
		// Merge output data from previous processing
		// Sum of hann windows separated by 6.25ms gives maximum of ~2.55
		// https://www.wolframalpha.com/input?i=max%28hann%28x%29+%2B+hann%28x+%2B+0.19%29+%2B+hann%28x+%2B+0.39%29+%2B+hann%28x+%2B+0.5859%29+%2B+hann%28x+%2B+0.78125%29+%2B+hann%28x+%2B+0.9765%29%29
		static constexpr uint32_t OVERLAPPING_MAX = 1.0f;
		static constexpr uint32_t modelWindowOffset = sampleWindowOutput.size() - modelWindowOutput.size();
		size_t i;
		for(i = 0; i < modelWindowOffset; i++) {
			sampleWindowOutput[i] = sampleWindowOutput[i + WINDOW_PERIOD];
		}
		for(; i < modelWindowOffset + WINDOW_PERIOD; i++) {
			uint32_t value =
			    sampleWindowOutput[i + WINDOW_PERIOD] + modelWindowOutput[i - modelWindowOffset] / OVERLAPPING_MAX;
			sampleWindowOutput[i] = value;
		}
		for(; i < sampleWindowOutput.size() - WINDOW_PERIOD; i++) {
			sampleWindowOutput[i] =
			    sampleWindowOutput[i + WINDOW_PERIOD] + modelWindowOutput[i - modelWindowOffset] / OVERLAPPING_MAX;
		}
		for(; i < sampleWindowOutput.size(); i++) {
			sampleWindowOutput[i] = modelWindowOutput[i - modelWindowOffset] / OVERLAPPING_MAX;
		}

		// Copy data for next processing
		std::copy_n(sampleWindowInput.begin(), modelWindowInput.size(), modelWindowInput.begin());
		std::copy_n(modelWindowInput.begin(), modelWindowInput.size(), modelWindowOutput.begin());
		// Make room for new samples to add to window
		std::copy_n(&sampleWindowInput[WINDOW_PERIOD], sampleWindowInput.size() - WINDOW_PERIOD, &sampleWindowInput[0]);

		sampleAddedToWindowSinceLastModelRun -= WINDOW_PERIOD;

		// printf("modelWindowInput: {");
		// uint32_t previousValue = 0;
		// for(auto value : modelWindowInput) {
		//	printf("%u, ", value);
		//	previousValue = value;
		// }
		// printf(" }\n");
	}
}

int main() {
	uint32_t samples[48];
	uint32_t* samplesArray = samples;

	size_t counter = 0;
	uint32_t previousValue = 0;

	for(size_t j = 0; j < (WINDOW_PERIOD * 5) / 3; j++) {
		for(size_t i = 0; i < 48 / 3; i++) {
			uint32_t value = (counter / 100) * 100 + 1;
			samples[i * 3] = value;
			samples[i * 3 + 1] = value;
			samples[i * 3 + 2] = value;

			counter++;
		}
		processSamples(&samplesArray, 48);

		printf("samples: {");
		for(size_t i = 0; i < 48 / 3; i++) {
			auto value = samples[i * 3];
			printf("%u, ", value);
			previousValue = value;
		}
		printf(" }\n");
	}
}