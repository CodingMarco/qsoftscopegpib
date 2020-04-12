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

double Scope::updateTimebaseRange()
{
	updateSampleRate();
	this->_timebaseRange = this->_points / this->_sampleRate;
	emit timebaseRangeUpdated(this->_timebaseRange);
	return this->_timebaseRange;
}

int Scope::updateSampleRate()
{
	QString sampleRateWithSi = query(":TIMEBASE:SAMPLE:CLOCK?").remove("Sa/s");
	int m_sampleRate = -1;
	if(sampleRateWithSi.endsWith(' '))
	{
		m_sampleRate = sampleRateWithSi.remove(' ').toInt();
	}
	else if(sampleRateWithSi.endsWith(" k"))
	{
		m_sampleRate = sampleRateWithSi.remove(" k").toInt() * 1e3;
	}
	else if(sampleRateWithSi.endsWith(" M"))
	{
		m_sampleRate = sampleRateWithSi.remove(" M").toInt() * 1e6;
	}
	else if(sampleRateWithSi.endsWith(" G"))
	{
		m_sampleRate = sampleRateWithSi.remove(" G").toInt() * 1e9;
	}

	if(m_sampleRate != -1 && m_sampleRate != 0)
	{
		_sampleRate = m_sampleRate;
		return m_sampleRate;
	}
	else
	{
		qCritical() << "[CRITICAL]: Sample rate could not be parsed!";
		return -1;
	}
}

bool Scope::setTimebaseRange(double range)
{
	writeCmd(QString(":TIMEBASE:RANGE ") + QString::number(range));
	_timebaseRange = updateTimebaseRange();
	emit timebaseRangeUpdated(_timebaseRange);
	return true;
}

bool Scope::setSampleRate(int m_sampleRate)
{
	if((!QString::number(m_sampleRate).startsWith("10") &&
		!QString::number(m_sampleRate).startsWith("25") &&
		!QString::number(m_sampleRate).startsWith("50") &&
		m_sampleRate != 2e9) || m_sampleRate > 2e9)
	{
		qCritical() << "[CRITICAL]: setSampleRate(): Invalid sample rate " << QString::number(m_sampleRate);
		return false;
	}
	else
	{
		writeCmd(QString(":TIMEBASE:SAMPLE:CLOCK ") + QString::number(m_sampleRate));
		_sampleRate = m_sampleRate;
		return true;
	}
}

QVector<QPointF> Scope::digitizeAndGetPoints()
{
	if(_timebaseRange == -1)
		updateTimebaseRange();
	digitize();
	auto preamble = getWaveformPreamble();
	writeCmd(":WAVEFORM:DATA?");

	QVector<ushort> yRawData = readAllWordData();
	QVector<QPointF> pointData;
	pointData.reserve(this->points());

	for(int x = 0; x < this->points(); x++)
	{
		pointData.append({	(double(x)-preamble["xreference"]) * preamble["xincrement"] + preamble["xorigin"],
							(double(yRawData[x])-preamble["yreference"]) * preamble["yincrement"] + preamble["yorigin"]	});
	}
	return pointData;
}

bool Scope::setPoints(POINTS newPoints)
{
	int oldPoints = _points;
	_points = newPoints;
	writeCmd(QString(":ACQUIRE:POINTS "), newPoints);
	updateTimebaseRange();

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
	if(_sampleRate == 1e9) // After 1 GSa/s comes 2, not 2.5
		return setSampleRate(2e9);
	else if(_sampleRate < 2e9)
	{
		int currentFirstDigit = (int)QString::number(_sampleRate, 'e', 1).at(0).digitValue();
		return setSampleRate(_sampleRate * (currentFirstDigit == 1 ? 2.5 : 2));
	}
	else // Maximum zoom in reached
		return false;
}

bool Scope::zoomOut()
{
	if(_sampleRate == 2e9) // Before 2 GSa/s comes 1
		return setSampleRate(1e9);
	else if(_sampleRate > 1e3)
	{
		int currentFirstDigit = (int)QString::number(double(_sampleRate), 'e', 1).at(0).digitValue();
		return setSampleRate(_sampleRate * (currentFirstDigit == 2 ? 0.4 : 0.5));
	}
	else // Maximum zoom out reached
		return false;
}

void Scope::initializeParameters()
{
	updateTimebaseRange();
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
	writeCmd(":autoscale");
	setPoints(_points);
	setTimebaseReference(_timebaseReference);
}

bool Scope::digitize()
{
	writeCmd(QString(":DIGITIZE CHANNEL"), this->_sourceChannel);
	return true;
}
