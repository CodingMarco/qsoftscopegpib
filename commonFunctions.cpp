#include "commonFunctions.h"

#include <QDebug>

QString CommonFunctions::toSiValue(double value, QString unit)
{
	double absValue = value < 0 ? -value : value;

	if((absValue >= 1 && absValue < 1e3) || absValue == 0)
	{
		return QString::number(value) + " " + unit;
	}
	else if(absValue >= 1e3)
	{
		if(absValue < 1e6)
			return QString::number(value * 1e-3) + " k" + unit;

		else if(absValue >= 1e6 && absValue < 1e9)
			return QString::number(value * 1e-6) + " M" + unit;

		else
			return QString::number(value * 1e-9) + " G" + unit;
	}
	else
	{
		if(absValue >= 1e-3)
			return QString::number(value * 1e3) + " m" + unit;

		else if(absValue < 1e-3 && absValue >= 9999e-10)
			return QString::number(value * 1e6) + " u" + unit;

		else if(absValue < 1e-6 && absValue >= 9999e-13)
			return QString::number(value * 1e9) + " n" + unit;

		else
			return QString::number(value * 1e12) + " p" + unit;
	}
}

WaveformPointsVector CommonFunctions::getFunctionWaveform(WaveformPointsVector &op1,
														  WaveformPointsVector &op2,
														  FUNCTION_OPERATOR functionOperator)
{
	WaveformPointsVector result;
	if(op1.size() != op2.size())
	{
		qWarning() << "[WARNING]: Function operands are not the same size!";
		return result;
	}

	int resultSize = op1.size();
	result.reserve(resultSize);

	for(int i = 0; i < resultSize; i++)
	{
		QPointF point;
		point.setX(op1[i].x());
		double y = 0;
		switch (functionOperator) {
			case MINUS:
				y = op1[i].y() - op2[i].y();
				break;
			case PLUS:
				y = op1[i].y() + op2[i].y();
				break;
		}
		point.setY(y);
		result.append(point);
	}

	return result;
}

double CommonFunctions::spinBoxAndComboBoxToVoltage(int spinBoxValue, int comboBoxIndex)
{
	// 0 --> V
	// 1 --> mV
	switch (comboBoxIndex) {
		case 0:
			return double(spinBoxValue);
		case 1:
			return double(spinBoxValue) * 1e-3;
		default:
			return 0;
	}
}

double CommonFunctions::spinBoxAndComboBoxToTime(int spinBoxValue, int comboBoxIndex)
{
	// 0 --> s
	// 1 --> ms
	// 2 --> us
	// 3 --> ns

	switch (comboBoxIndex) {
		case 0:
			return double(spinBoxValue);
		case 1:
			return double(spinBoxValue) * 1e-3;
		case 2:
			return double(spinBoxValue) * 1e-6;
		case 3:
			return double(spinBoxValue) * 1e-9;
		default:
			return 0;
	}
}
