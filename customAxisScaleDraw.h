#pragma once

#include <qwt/qwt_scale_draw.h>
#include "commonFunctions.h"

class TimebaseScaleDraw : public QwtScaleDraw
{
public:
	TimebaseScaleDraw() { }
	virtual QwtText label(double value) const override
	{
		return QwtText(CommonFunctions::toSiValue(value, "s"));
	}
};

class VoltageScaleDraw : public QwtScaleDraw
{
public:
	VoltageScaleDraw() { }
	virtual QwtText label(double value) const override
	{
		return QwtText(CommonFunctions::toSiValue(value, "V"));
	}
};
