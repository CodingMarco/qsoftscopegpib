#pragma once

#include <QString>

#include "scopeNamespace.h"

class CommonFunctions
{
public:
	static QString toSiValue(double value, QString unit);
	static WaveformPointsVector getFunctionWaveform(WaveformPointsVector& op1,
													WaveformPointsVector& op2,
													FUNCTION_OPERATOR functionOperator);
	static double spinBoxAndComboBoxToVoltage(int spinBoxValue, int comboBoxIndex);
	static double spinBoxAndComboBoxToTime(int spinBoxValue, int comboBoxIndex);

private:
	CommonFunctions();
};
