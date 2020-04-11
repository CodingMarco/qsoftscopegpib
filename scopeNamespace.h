#pragma once

#include <QString>

typedef unsigned char byte;

enum POINTS {
	POINTS_512 = 512,
	POINTS_1024 = 1024,
	POINTS_2048 = 2048,
	POINTS_4096 = 4096,
	POINTS_8192 = 8192,
	POINTS_16384 = 16384,
	POINTS_32768 = 32768
};

enum WAVEFORM_FORMAT {
	WAVEFORM_FORMAT_WORD,
	WAVEFORM_FORMAT_BYTE
};

enum ACQUIRE_TYPE {
	ACQUIRE_TYPE_NORMAL,
	ACQUIRE_TYPE_RAW
};

enum TIMEBASE_REFERENCE {
	LEFT,
	CENTER,
	RIGHT
};

struct ScopeSettings
{
	POINTS points;
	WAVEFORM_FORMAT waveformFormat;
	ACQUIRE_TYPE acquireType;
	TIMEBASE_REFERENCE timebaseReference;
};
