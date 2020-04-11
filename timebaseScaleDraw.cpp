#include "timebaseScaleDraw.h"
#include <timePoint.h>

QwtText TimebaseScaleDraw::label(double value) const
{
	TimePoint timePoint(value);
	return QwtText(timePoint.toTimebaseString());
}
