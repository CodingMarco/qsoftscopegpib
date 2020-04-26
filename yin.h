#pragma once

#include "scopeNamespace.h"

class Yin
{

public:
	Yin();
	Yin(float sampleRate, int bufferSize);
	void initialize(float sampleRate,int bufferSize);
	float getPitch(WaveformPointsVector &buffer);
	float getProbability();

private:
	float* yinBuffer;
	float parabolicInterpolation(int tauEstimate);
	int absoluteThreshold();
	void cumulativeMeanNormalizedDifference();
	void difference(WaveformPointsVector &buffer);
	float threshold;
	int bufferSize;
	int halfBufferSize;
	float sampleRate;
	float probability;

};
