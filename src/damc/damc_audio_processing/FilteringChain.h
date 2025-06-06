#pragma once

#include "CompressorFilter.h"
#include "DelayFilter.h"
#include "DitheringFilter.h"
#include "EqFilter.h"
#include "ExpanderFilter.h"
#include "PeakMeter.h"
#include "ReverbFilter.h"
#include <Osc/OscArray.h>
#include <Osc/OscContainer.h>
#include <Osc/OscFixedArray.h>
#include <Osc/OscVariable.h>
#include <TinyDenoiserFilter.h>
#include <array>
#include <stddef.h>

class FilterChain : public OscContainer {
public:
	FilterChain(OscContainer* parent,
	            OscReadOnlyVariable<int32_t>* oscNumChannel,
	            OscReadOnlyVariable<int32_t>* oscSampleRate);

	void reset(float fs);
	void processSamples(float** samples, size_t numChannel, size_t count);
	float processSideChannelSample(float input);

	void onFastTimer();

protected:
	void updateNumChannels(size_t numChannel);

private:
	std::array<DelayFilter, 2> delayFilters;
	// OscContainerArray<ReverbFilter> reverbFilters;
	OscFixedArray<EqFilter, 10> oscEqFilters;
	std::array<EqFilter, 10> eqFilters;
	CompressorFilter compressorFilter;
	ExpanderFilter expanderFilter;
	TinyDenoiserFilter tinyDenoiserFilter;
	PeakMeter peakMeter;

	OscVariable<int32_t> delay;
	OscArray<float> oscVolume;
	std::array<float, 2> volume;
	OscVariable<float> masterVolume;
	OscVariable<bool> mute;
	OscVariable<bool> reverseAudioSignal;
};
