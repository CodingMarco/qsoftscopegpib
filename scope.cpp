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
	this->_timebaseRange = query(":TIMEBASE:RANGE?").toDouble();
	emit timebaseRangeUpdated(this->_timebaseRange);
	return this->_timebaseRange;
}

bool Scope::setTimebaseRange(double range)
{
	writeCmd(QString(":TIMEBASE:RANGE ") + QString::number(range));
	_timebaseRange = updateTimebaseRange();
	emit timebaseRangeUpdated(_timebaseRange);
	return true;
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
	int currentFirstDigit = (int)QString::number(timebaseRange(), 'e', 0).at(0).digitValue();
	return setTimebaseRange(timebaseRange() * (currentFirstDigit == 5 ? 0.4 : 0.5));
}

bool Scope::zoomOut()
{
	int currentFirstDigit = (int)QString::number(timebaseRange(), 'e', 0).at(0).digitValue();
	return setTimebaseRange(timebaseRange() * (currentFirstDigit == 2 ? 2.5 : 2));
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
