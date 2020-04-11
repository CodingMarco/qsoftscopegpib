#pragma once
#include <QString>

class TimePoint
{
private:
	double seconds = 0;
public:
	TimePoint();
	TimePoint(double m_seconds);
	double getSeconds() const { return seconds; }
	double getMilliseconds() const { return seconds * 1e3; }
	double getMicroseconds() const { return seconds * 1e6; }
	double getNanoseconds() const  { return seconds * 1e9; }
	double getPicoseconds() const  { return seconds * 1e12; }
	QString toTimebaseString() const;
};
