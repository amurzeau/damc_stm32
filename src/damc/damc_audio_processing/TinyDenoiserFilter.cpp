#include "TinyDenoiserFilter.h"

#include <algorithm>
#include <arm_mve.h>
#include <assert.h>
#include <dsp/support_functions_f16.h>
#include <math.h>
#include <string.h>

#include "BiquadFilter.h"
#include "TinyDenoiserModel/model.h"
#include "uv.h"

// Model from
// https://github.com/GreenWaves-Technologies/tiny_denoiser_v2
// https://arxiv.org/pdf/2210.07692

TinyDenoiserFilter::TinyDenoiserFilter(OscContainer* parent)
    : OscContainer(parent, "tinyDenoiserFilter", 9),
      enable(this, "enable", false),
      ratio(this, "ratio", 1.0f),
      firstRun(true) {
	tinydenoiser_model_init();

	resamplingFilterInput.computeFilter(true, FilterType::LowPass, 8000, 48000, 0, 0.7071);
	resamplingFilterOutput.computeFilter(true, FilterType::LowPass, 8000, 48000, 0, 0.7071);

	uv_async_init(uv_default_loop(), &asyncRunModel, &TinyDenoiserFilter::modelRunStatic);
	asyncRunModel.data = this;

	sampleAddedToWindowSinceLastModelRun = 0;
	sampleWindowInput.fill(0.0f);
	sampleWindowOutput.fill(0.0f);
	modelWindowInput.fill(0.0f);
	modelWindowOutput.fill(0.0f);
}

void TinyDenoiserFilter::processSamples(float** samples, size_t count) {
	if(enable) {
		if(firstRun) {
			tinydenoiser_model_reset();
			firstRun = false;
		}

		size_t resampledCount16khz = count / 3;

		resamplingFilterInput.processFilter(samples[0], count);

		// Downsample signal and put new sample to the window end
		for(size_t i = 0; i < resampledCount16khz; i++) {
			sampleWindowInput[WINDOW_SIZE - WINDOW_PERIOD + sampleAddedToWindowSinceLastModelRun + i] =
			    samples[0][i * 3];
		}

		// Put denoised data to output
		// Upsample back to 48Khz
		for(size_t i = 0; i < resampledCount16khz; i++) {
			float value = sampleWindowOutput[sampleAddedToWindowSinceLastModelRun + i];
			samples[0][i * 3] = value;
			samples[0][i * 3 + 1] = value;
			samples[0][i * 3 + 2] = value;
			samples[1][i * 3] = value;
			samples[1][i * 3 + 1] = value;
			samples[1][i * 3 + 2] = value;
		}
		// Filter upsampled signal
		resamplingFilterOutput.processFilter(samples[0], count);

		sampleAddedToWindowSinceLastModelRun += resampledCount16khz;
		if(sampleAddedToWindowSinceLastModelRun >= WINDOW_PERIOD) {
			// Merge output data from previous processing
			// Sum of hann windows separated by 6.25ms gives maximum of ~2.55
			// https://www.wolframalpha.com/input?i=max%28hann%28x%29+%2B+hann%28x+%2B+0.19%29+%2B+hann%28x+%2B+0.39%29+%2B+hann%28x+%2B+0.5859%29+%2B+hann%28x+%2B+0.78125%29+%2B+hann%28x+%2B+0.9765%29%29
			static constexpr float OVERLAPPING_MAX = 4.0f;
			size_t i;
			for(i = 0; i < WINDOW_MARGIN; i++) {
				sampleWindowOutput[i] = sampleWindowOutput[i + WINDOW_PERIOD];
			}
			for(; i < sampleWindowOutput.size() - WINDOW_PERIOD; i++) {
				sampleWindowOutput[i] =
				    sampleWindowOutput[i + WINDOW_PERIOD] + modelWindowOutput[i - WINDOW_MARGIN] / OVERLAPPING_MAX;
			}
			for(; i < sampleWindowOutput.size(); i++) {
				sampleWindowOutput[i] = modelWindowOutput[i - WINDOW_MARGIN] / OVERLAPPING_MAX;
			}

			// Copy data for next processing
			std::copy_n(sampleWindowInput.begin(), modelWindowInput.size(), modelWindowInput.begin());

			// Make room for new samples to add to window
			std::copy_n(
			    &sampleWindowInput[WINDOW_PERIOD], sampleWindowInput.size() - WINDOW_PERIOD, &sampleWindowInput[0]);

			sampleAddedToWindowSinceLastModelRun -= WINDOW_PERIOD;

			uv_async_send(&asyncRunModel);
		}
	} else {
		firstRun = true;
	}
}

void TinyDenoiserFilter::modelRunStatic(uv_async_t* handle) {
	TinyDenoiserFilter* thisInstance = (TinyDenoiserFilter*) handle->data;
	thisInstance->modelRun();
}

void TinyDenoiserFilter::modelRun() {
	tinydenoiser_run(modelWindowInput.data(), modelWindowOutput.data());
}
