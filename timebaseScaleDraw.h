#pragma once
#include <qwt/qwt_scale_draw.h>

class TimebaseScaleDraw : public QwtScaleDraw
{
public:
	TimebaseScaleDraw() { }
	virtual QwtText label(double value) const override;
};
