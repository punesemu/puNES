#pragma once
#include <vector>
#include <stddef.h>
#include "common.h"

class WaveFile {
	int bits;
	int rate;
	int fetchRate;
	int fetchCount;
	int channels;
	uint32_t position;
	bool finished;
	std::vector <int16_t> data;
public:
	WaveFile(uTCHAR *);
	WaveFile(const int16_t*, size_t, int);
	void setFetchRate(int);
	bool isFinished();
	void restart();
	int getNextSample();
};
typedef std::vector<WaveFile> WaveFiles;

extern WaveFiles waveFiles;

void loadWaveFiles (uTCHAR *, int);