#include "scope.h"
#include <gpib/ib.h>
#include <QDebug>
#include <QByteArray>
#include <string.h>
#include <QElapsedTimer>
#include <QThread>
#include <unistd.h>
#include <QtEndian>
#include <QMessageBox>
#include <QtMath>

Scope::Scope()
{
}

double Scope::nextLowerTimebaseRange()
{
	if(_sampleRateIndex <= 0)
		return maximumTimebaseRange();
	else
		return double(this->_points) / validSampleRates[_sampleRateIndex-1];
}

double Scope::maximumTimebaseRange()
{
	return double(this->_points) / validSampleRates[_sampleRateIndex];
}

QVector<ushort> Scope::getWaveformData()
{
	writeCmd(":WAVEFORM:DATA?");

	QByteArray data = readAllByteData();

	QVector<ushort> samples;
	samples.reserve(data.size()/2);

	for(int i = 0; i < data.size()/2; i++)
	{
		samples.append(qToBigEndian(*(((ushort*)data.data())+i)));
	}
	return samples;
}

int Scope::updateSampleRate()
{
	QString sampleRateWithSi = query(":TIMEBASE:SAMPLE:CLOCK?").remove("Sa/s");
	int newSampleRate = 0;
	if(sampleRateWithSi.contains("AUTO"))
	{
		setSampleRateByIndex(10); // 1 MSa/s
		newSampleRate = 1e6;
	}
	else if(sampleRateWithSi.endsWith(' '))
		newSampleRate = sampleRateWithSi.remove(' ').toInt();
	else if(sampleRateWithSi.endsWith(" k"))
		newSampleRate = sampleRateWithSi.remove(" k").toInt() * 1e3;
	else if(sampleRateWithSi.endsWith(" M"))
		newSampleRate = sampleRateWithSi.remove(" M").toInt() * 1e6;
	else if(sampleRateWithSi.endsWith(" G"))
		newSampleRate = sampleRateWithSi.remove(" G").toInt() * 1e9;

	if(newSampleRate != 0)
	{
		_sampleRateIndex = validSampleRates.indexOf(newSampleRate);
		return newSampleRate;
	}
	else
	{
		qCritical() << "[CRITICAL]: Sample rate could not be parsed! Setting it to 1MSa/s.";
		setSampleRateByIndex(10);
		return validSampleRates[10];
	}
}

bool Scope::setSampleRateByIndex(int m_sampleRateIndex)
{
	if(m_sampleRateIndex >= 0 && m_sampleRateIndex < validSampleRates.size())
	{
		_sampleRateIndex = m_sampleRateIndex;
		writeCmd(QString(":TIMEBASE:SAMPLE:CLOCK ") + QString::number(validSampleRates[m_sampleRateIndex]));
		return true;
	}
	else
	{
		qCritical() << "[CRITICAL]: setSampleRateIndex(): Index out of range!";
		return false;
	}
}

void Scope::digitizeAndGetPoints()
{
	digitize();
	auto preamble = getWaveformPreamble();
	writeCmd(":WAVEFORM:DATA?");

	QVector<ushort> yRawData = readAllWordData();
	WaveformPointsVector pointData;
	pointData.reserve(this->points());

	for(int x = 0; x < this->points(); x++)
	{
		pointData.append({	(double(x)-preamble["xreference"]) * preamble["xincrement"] + preamble["xorigin"],
							(double(yRawData[x])-preamble["yreference"]) * preamble["yincrement"] + preamble["yorigin"]	});
	}
	emit(waveformUpdated(pointData));
}

bool Scope::setPoints(QString newPoints)
{
	return setPoints(POINTS(newPoints.toInt()));
}

bool Scope::setPoints(POINTS newPoints)
{
	int oldPoints = _points;
	_points = newPoints;
	writeCmd(QString(":ACQUIRE:POINTS "), newPoints);

	if(oldPoints != -1)
	{
		int howOftenToZoom = log2(double(newPoints) / oldPoints);
		for(int i = 0; i < abs(howOftenToZoom); i++)
		{
			if(howOftenToZoom < 0)
				zoomOut();
			else
				zoomIn();
		}
	}
	return true;
}

bool Scope::setFormat(WAVEFORM_FORMAT m_format)
{
	this->_format = m_format;
	switch (this->_format) {
		case WAVEFORM_FORMAT_BYTE:
			writeCmd(":WAVEFORM:FORMAT BYTE");
			this->_bytesPerPoint = 1;
			break;
		case WAVEFORM_FORMAT_WORD:
			writeCmd(":WAVEFORM:FORMAT WORD");
			this->_bytesPerPoint = 2;
			break;
	}
	return true;
}

bool Scope::setSourceChannel(int m_channel)
{
	if(m_channel > 4 || m_channel < 1)
	{
		qWarning() << "Channel must be 1, 2, 3 or 4!";
		return false;
	}
	else
	{
		this->_sourceChannel = m_channel;
		writeCmd(QString(":WAVEFORM:SOURCE CHANNEL"), this->_sourceChannel);
		return true;
	}
}

bool Scope::setAcquireType(ACQUIRE_TYPE m_type)
{
	this->_acquireType = m_type;
	switch (this->_acquireType) {
		case ACQUIRE_TYPE_RAW:
			writeCmd(":ACQUIRE:TYPE RAWDATA");
			break;
		case ACQUIRE_TYPE_NORMAL:
			writeCmd(":ACQUIRE:TYPE NORMAL");
			break;
	}
	return true;
}

void Scope::setTimebaseReference(TIMEBASE_REFERENCE m_reference)
{
	this->_timebaseReference = m_reference;
	switch(this->_timebaseReference) {
		case LEFT:
			writeCmd(":TIMEBASE:REFERENCE LEFT");
			break;
		case CENTER:
			writeCmd(":TIMEBASE:REFERENCE CENTER");
			break;
		case RIGHT:
			writeCmd(":TIMEBASE:REFERENCE RIGHT");
			break;
	}
}

bool Scope::zoomIn()
{
	if(_sampleRateIndex > 0)
		return setSampleRateByIndex(_sampleRateIndex-1);
	else // This is a normal condition, maximum zoom in is reached.
		return false;
}

bool Scope::zoomOut()
{
	if(_sampleRateIndex < validSampleRates.size()-1)
		return setSampleRateByIndex(_sampleRateIndex+1);
	else // This is a normal condition, maximum zoom out is reached.
		return false;
}

double Scope::getChannelRange()
{
	return query(":CHANNEL1:RANGE?").toDouble();
}

double Scope::getChannelOffset()
{
	return query(":CHANNEL1:OFFSET?").toDouble();
}

void Scope::initializeScope()
{
	writeCmd(":DISPLAY:SCREEN OFF");
	writeCmd(":DISPLAY:CONNECT OFF");
	setFormat(WAVEFORM_FORMAT_WORD);
	setPoints(POINTS_1024);
	setAcquireType(ACQUIRE_TYPE_NORMAL);
	setTimebaseReference(CENTER);
	updateSampleRate();
}

void Scope::initializeThreadRelatedStuff()
{
	waveformUpdateTimer = new QTimer(this);
	connect(waveformUpdateTimer, &QTimer::timeout, this, &Scope::digitizeAndGetPoints);
}

void Scope::toggleAcCouplingAndLfReject(bool toggle)
{
	if(toggle)
	{
		writeCmd(":CHANNEL1:COUPLING AC");
		writeCmd(":CHANNEL1:LFREJECT ON");
	}
	else
	{
		writeCmd(":CHANNEL1:COUPLING DC");
		writeCmd(":CHANNEL1:LFREJECT OFF");
	}
}

void Scope::autoAdjustSampleRate(double newTimebaseRange)
{
	if(newTimebaseRange < nextLowerTimebaseRange())
		zoomIn();
	else if(newTimebaseRange > maximumTimebaseRange())
		zoomOut();
}

QMap<QString, double> Scope::getWaveformPreamble()
{
	QMap<QString, double> preamble;
	QStringList params = query(":WAVEFORM:PREAMBLE?").split(',');
	preamble["xincrement"] = params[4].toDouble();
	preamble["xorigin"] = params[5].toDouble();
	preamble["xreference"] = params[6].toDouble();
	preamble["yincrement"] = params[7].toDouble();
	preamble["yorigin"] = params[8].toDouble();
	preamble["yreference"] = params[9].toDouble();
	return preamble;
}

void Scope::autoscale()
{
	writeCmd(":AUTOSCALE");
	setPoints(_points);
	setTimebaseReference(_timebaseReference);
}

void Scope::singleWaveformUpdate()
{
	digitizeAndGetPoints();
}

bool Scope::digitize()
{
	writeCmd(QString(":DIGITIZE CHANNEL"), this->_sourceChannel);
	return true;
}
