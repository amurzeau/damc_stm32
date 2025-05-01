#include "PeakMeter.h"
#include <OscRoot.h>
#include <algorithm>
#include <math.h>
#include <spdlog/spdlog.h>

PeakMeter::PeakMeter(OscContainer* parent,
                     OscReadOnlyVariable<int32_t>* oscNumChannel,
                     OscReadOnlyVariable<int32_t>* oscSampleRate)
    : oscRoot(parent->getRoot()),
      oscSampleRate(oscSampleRate),
      oscPeakGlobal(parent, "meter"),
      oscPeakPerChannel(parent, "meter_per_channel", false),
      samplesInPeaks(0),
      peaksPerChannel{0.0f, 0.0f},
      oscEnablePeakUpdate(parent, "meter_enable_per_channel", false) {
	oscNumChannel->addChangeCallback([this](int32_t newValue) {
		levelsDb.resize(newValue, -192);

		// peakMutex.lock();
		// peaksPerChannel.resize(newValue, 0);
		// peaksPerChannelToSend.resize(newValue, 0);
		// loudnessMeters.resize(newValue);
		// peakMutex.unlock();
		//		for(auto& loudnessMeter : loudnessMeters) {
		//			loudnessMeter.reset(this->oscSampleRate->get());
		//		}
	});

	// oscSampleRate->addChangeCallback([this](int32_t newValue) {
	//		for(auto& loudnessMeter : loudnessMeters) {
	//			loudnessMeter.reset(newValue);
	//		}
	// });
}

PeakMeter::~PeakMeter() {}

void PeakMeter::processSamples(const float* peaks, size_t numChannels, size_t samplesInPeaks) {
	// peakMutex.lock();
	this->samplesInPeaks += samplesInPeaks;
	for(size_t i = 0; i < numChannels; i++) {
		this->peaksPerChannel[i] = fmaxf(peaks[i], this->peaksPerChannel[i]);
	}
	// peakMutex.unlock();
}

void PeakMeter::onFastTimer() {
	int samples;
	int32_t sampleRate = oscSampleRate->get();

	// peakMutex.lock();
	samples = this->samplesInPeaks;
	for(size_t i = 0; i < peaksPerChannelToSend.size(); i++) {
		peaksPerChannelToSend[i] = peaksPerChannel[i];
		peaksPerChannel[i] = 0;
	}
	this->samplesInPeaks = 0;
	// peakMutex.unlock();

	if(sampleRate == 0)
		return;

	float deltaT = (float) samples / sampleRate;
	float maxLevel = 0;

	for(size_t channel = 0; channel < peaksPerChannelToSend.size(); channel++) {
		// float peakDb = this->loudnessMeters[channel].getLoudness();
		float peakDb =
		    peaksPerChannelToSend[channel] != 0.f ? 20.f * log10f(peaksPerChannelToSend[channel]) : -INFINITY;

		float decayAmount = 11.76470588235294f * deltaT;  // -20dB / 1.7s
		// float levelDb = peakDb;
		float levelDb = std::max(levelsDb[channel] - decayAmount, peakDb);
		levelsDb[channel] = levelDb > -192 ? levelDb : -192;
		if(channel == 0 || levelsDb[channel] > maxLevel)
			maxLevel = levelsDb[channel];
	}

	if(oscEnablePeakUpdate.get()) {
		oscPeakGlobal.set(maxLevel);
	}

	oscPeakPerChannel.setData(levelsDb);
}
