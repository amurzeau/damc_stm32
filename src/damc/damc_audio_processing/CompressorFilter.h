#pragma once

#include "LoudnessMeter.h"
#include <Osc/OscContainer.h>
#include <Osc/OscReadOnlyVariable.h>
#include <Osc/OscVariable.h>
#include <array>
#include <stddef.h>

class CompressorFilter : public OscContainer {
protected:
public:
	CompressorFilter(OscContainer* parent);
	void init(size_t numChannel);
	void reset();
	void processSamples(float** samples, size_t count);

	void onFastTimer();

protected:
	float doCompression(float levelPeak, float levelLoudness);
	float gainComputer(float sample) const;
	void levelDetectorSmoothing(float dbCompression);
	void levelDetectorSilenceSmoothing(float dbCompression);
	float levelToDbPeak(float sample);
	float levelToDbLoudnessLUFS(float sample);
	float levelDetectorPeak(float sample);
	float levelDetectorLoudnessLUFS(LoudnessMeter* loudnessMeter, float sample);

private:
	static constexpr size_t numChannel = 2;

	// Level detector state
	float y1;
	float yL;

	// Loudness LUFS level detector
	std::array<LoudnessMeter, 2> loudnessMeters;

	OscVariable<bool> enablePeak;
	OscVariable<bool> enableLoudness;
	float alphaR;
	float alphaA;
	OscVariable<float> attackTime;
	OscVariable<float> releaseTime;
	OscVariable<float> threshold;
	OscVariable<float> makeUpGain;
	OscVariable<float> ratio;
	float gainDiffRatio = 0;
	float gainDiffRatioInKnee = 0;
	OscVariable<float> kneeWidth;
	OscVariable<float> lufsTarget;
	OscVariable<float> lufsIntegrationTime;
	OscVariable<float> lufsGate;

	float lufsRealtimeLevel;
	OscReadOnlyVariable<float> lufsMeter;
};
