#include "ExpanderFilter.h"

#include <MathUtils.h>
#include <algorithm>
#include <assert.h>
#include <fastapprox/fastexp.h>
#include <fastapprox/fastlog.h>
#include <float.h>
#include <math.h>
#include <string.h>

ExpanderFilter::ExpanderFilter(OscContainer* parent)
    : OscContainer(parent, "expanderFilter", 8),
      enable(this, "enable", false),
      attackTime(this, "attackTime", 0),
      releaseTime(this, "releaseTime", 8),
      threshold(this, "threshold", -50),
      makeUpGain(this, "makeUpGain", 0),
      ratio(this, "ratio", 4),
      kneeWidth(this, "kneeWidth", 0) {
	attackTime.addChangeCallback([this](float oscValue) { alphaA = oscValue != 0 ? expf(-1 / (oscValue * fs)) : 0; });
	releaseTime.addChangeCallback([this](float oscValue) { alphaR = oscValue != 0 ? expf(-1 / (oscValue * fs)) : 0; });
	ratio.addChangeCallback([this](float oscValue) { gainDiffRatio = oscValue - 1; });

	std::fill_n(previousLevelDetectorOutput.begin(), previousLevelDetectorOutput.size(), 0.0f);
	std::fill_n(previousPartialGainComputerOutput.begin(), previousPartialGainComputerOutput.size(), 0.0f);
}

void ExpanderFilter::init(size_t numChannel) {
	assert(this->numChannel == numChannel);
	// previousPartialGainComputerOutput.resize(numChannel);
	// previousLevelDetectorOutput.resize(numChannel);
}

void ExpanderFilter::reset(float fs) {
	this->fs = fs;
}

void ExpanderFilter::processSamples(float** samples, size_t count) {
	if(enable) {
		float makeUpGain = this->makeUpGain;

		for(size_t i = 0; i < count; i++) {
			float lowestCompressionDb = -FLT_MAX;

			for(size_t channel = 0; channel < numChannel; channel++) {
				float dbGain = doCompression(samples[channel][i],
				                             previousPartialGainComputerOutput[channel],
				                             previousLevelDetectorOutput[channel]);
				if(dbGain > lowestCompressionDb)
					lowestCompressionDb = dbGain;
			}

			// db to ratio
			float largerCompressionRatio;

			if(lowestCompressionDb > -FLT_MAX)
				largerCompressionRatio = fastpow2(LOG10_VALUE_DIV_20 * (lowestCompressionDb + makeUpGain));
			else
				largerCompressionRatio = 0;

			for(size_t channel = 0; channel < numChannel; channel++) {
				float value = largerCompressionRatio * samples[channel][i];

				samples[channel][i] = value;
			}
		}
	}
}

float ExpanderFilter::doCompression(float sample, float& y1, float& yL) {
	if(sample == 0)
		return -FLT_MAX;

	float dbSample = fastlog2(fabsf(sample)) / LOG10_VALUE_DIV_20;
	levelDetector(gainComputer(dbSample), y1, yL);
	return -yL;
}

float ExpanderFilter::gainComputer(float dbSample) {
	float zone = 2 * (dbSample - threshold);
	if(zone <= -kneeWidth) {
		return gainDiffRatio * (threshold - dbSample);
	} else if(zone >= kneeWidth) {
		return 0;
	} else {
		float a = threshold - dbSample + kneeWidth / 2;
		return gainDiffRatio * (a * a) / (2 * kneeWidth);
	}
}

void ExpanderFilter::levelDetector(float dbSample, float& y1, float& yL) {
	y1 = fminf(dbSample, alphaR * y1 + (1 - alphaR) * dbSample);
	yL = alphaA * yL + (1 - alphaA) * y1;
}
