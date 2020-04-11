#include "timePoint.h"
//#include <QtMath>

TimePoint::TimePoint(double m_seconds)
	: seconds(m_seconds) { }

QString TimePoint::toTimebaseString() const
{
	double absSeconds = seconds < 0 ? -seconds : seconds;

	if(absSeconds >= 1 || absSeconds == 0)
		return QString::number(seconds) + " s";

	else if(absSeconds < 1 && absSeconds >= 1e-3)
		return QString::number(getMilliseconds()) + " ms";

	else if(absSeconds < 1e-3 && absSeconds >= 9999e-10)
		return QString::number(getMicroseconds()) + " us";

	else if(absSeconds < 1e-6 && absSeconds >= 1e-9)
		return QString::number(getNanoseconds()) + " ns";

	else
		return QString::number(getPicoseconds()) + " ps";
}
