#include "commonFunctions.h"

QString CommonFunctions::toSiValue(double value, QString unit)
{
	double absSeconds = value < 0 ? -value : value;

	if(absSeconds >= 1 || absSeconds == 0)
		return QString::number(value) + " " + unit;

	else if(absSeconds < 1 && absSeconds >= 1e-3)
		return QString::number(value * 1e3) + " m" + unit;

	else if(absSeconds < 1e-3 && absSeconds >= 9999e-10)
		return QString::number(value * 1e6) + " u" + unit;

	else if(absSeconds < 1e-6 && absSeconds >= 9999e-13)
		return QString::number(value * 1e9) + " n" + unit;

	else
		return QString::number(value * 1e12) + " p" + unit;
}
