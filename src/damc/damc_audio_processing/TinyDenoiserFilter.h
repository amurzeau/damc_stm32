#pragma once

#include "uv.h"
#include <BiquadFilter.h>
#include <Osc/OscContainer.h>
#include <Osc/OscVariable.h>
#include <array>
#include <stddef.h>

class TinyDenoiserFilter : public OscContainer {
public:
	TinyDenoiserFilter(OscContainer* parent);
	void processSamples(float** samples, size_t count);

protected:
	static void modelRunStatic(uv_async_t* handle);
	void modelRun();

private:
	static constexpr size_t WINDOW_SIZE = 512;
	static constexpr size_t WINDOW_MARGIN = 16;   // 1ms margin as we need to have a window every 6.25ms
	static constexpr size_t WINDOW_PERIOD = 100;  // 6.25ms
	std::array<float, WINDOW_SIZE + WINDOW_PERIOD + WINDOW_MARGIN> sampleWindowInput;
	std::array<float, WINDOW_SIZE + WINDOW_PERIOD + WINDOW_MARGIN> sampleWindowOutput;
	size_t sampleAddedToWindowSinceLastModelRun;

	std::array<float, WINDOW_SIZE> modelWindowInput;
	std::array<float, WINDOW_SIZE> modelWindowOutput;

	BiquadFilter resamplingFilterInput;
	BiquadFilter resamplingFilterOutput;

	OscVariable<bool> enable;

	bool firstRun;

	uv_async_t asyncRunModel;
};
