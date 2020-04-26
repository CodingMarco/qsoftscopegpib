#pragma once

#include <QObject>
#include "scopeNamespace.h"

class Measurements : public QObject
{
	Q_OBJECT
public:
	explicit Measurements(QObject *parent = nullptr);
	int getFrequency(WaveformPointsVector &points, int sampleRate);

};
