#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>

#define MASTER_CLOCK 160000

#include "upd7756.h"
#include "butterworth.h"

namespace UPD7756 { // namespace UPD7756
static const int upd7756_step[16][16] ={
	{ 0,  0,  1,  2,  3,   5,   7,  10,  0,   0,  -1,  -2,  -3,   -5,   -7,  -10 },
	{ 0,  1,  2,  3,  4,   6,   8,  13,  0,  -1,  -2,  -3,  -4,   -6,   -8,  -13 },
	{ 0,  1,  2,  4,  5,   7,  10,  15,  0,  -1,  -2,  -4,  -5,   -7,  -10,  -15 },
	{ 0,  1,  3,  4,  6,   9,  13,  19,  0,  -1,  -3,  -4,  -6,   -9,  -13,  -19 },
	{ 0,  2,  3,  5,  8,  11,  15,  23,  0,  -2,  -3,  -5,  -8,  -11,  -15,  -23 },
	{ 0,  2,  4,  7, 10,  14,  19,  29,  0,  -2,  -4,  -7, -10,  -14,  -19,  -29 },
	{ 0,  3,  5,  8, 12,  16,  22,  33,  0,  -3,  -5,  -8, -12,  -16,  -22,  -33 },
	{ 1,  4,  7, 10, 15,  20,  29,  43, -1,  -4,  -7, -10, -15,  -20,  -29,  -43 },
	{ 1,  4,  8, 13, 18,  25,  35,  53, -1,  -4,  -8, -13, -18,  -25,  -35,  -53 },
	{ 1,  6, 10, 16, 22,  31,  43,  64, -1,  -6, -10, -16, -22,  -31,  -43,  -64 },
	{ 2,  7, 12, 19, 27,  37,  51,  76, -2,  -7, -12, -19, -27,  -37,  -51,  -76 },
	{ 2,  9, 16, 24, 34,  46,  64,  96, -2,  -9, -16, -24, -34,  -46,  -64,  -96 },
	{ 3, 11, 19, 29, 41,  57,  79, 117, -3, -11, -19, -29, -41,  -57,  -79, -117 },
	{ 4, 13, 24, 36, 50,  69,  96, 143, -4, -13, -24, -36, -50,  -69,  -96, -143 },
	{ 4, 16, 29, 44, 62,  85, 118, 175, -4, -16, -29, -44, -62,  -85, -118, -175 },
	{ 6, 20, 36, 54, 76, 104, 144, 214, -6, -20, -36, -54, -76, -104, -144, -214 },
};

static const int upd7756_state_table[16] = { -1, -1, 0, 0, 1, 2, 2, 3, -1, -1, 0, 0, 1, 2, 2, 3 };

int upd7756_masterROMSamples (const uint8_t* data, size_t size) {
	if (size >4 && data[1] ==0x5A && data[2] ==0xA5 && data[3] ==0x69 && data[4] ==0x55)
		// master ROM
		return (data[0] +1);
	else
	if (size >4 && data[0] ==0xFF && data[1] ==0x00 && data[2] ==0x00 && data[3] ==0x00 && data[4] ==0x00) {
		// Sega slave ROM
		int result =0;
		auto s =data;
		while (s +5 -data < (long long int)size) {
			if (s[0] ==0xFF && s[1] ==0x00 && s[2] ==0x00 && s[3] ==0x00 && s[4] ==0x00) result++;
			s++;
		}
		return result;
	}
	// neither
	return 0;		
}

const uint8_t* upd7756_findSample (const uint8_t* data, size_t size, int sampleNumber) {
	if (data[1] ==0x5A && data[2] ==0xA5 && data[3] ==0x69 && data[4] ==0x55)
		// master ROM
		return data +(data[sampleNumber *2 +5 +0] <<9 | data[sampleNumber *2 +5 +1] <<1) +1;
	else
	if (data[0] ==0xFF && data[1] ==0x00 && data[2] ==0x00 && data[3] ==0x00 && data[4] ==0x00) {
		// Sega slave ROM
		int count =0;
		auto s =data;
		while ((s +5 -data) < (long long int)size) {
			if (s[0] ==0xFF && s[1] ==0x00 && s[2] ==0x00 && s[3] ==0x00 && s[4] ==0x00 && count++ ==sampleNumber) return s +5;
			s++;
		}
	}
	return NULL;
}

std::vector<int16_t> upd7756_decodeSample (const uint8_t* data, int& sampleRate) {
	std::vector<int16_t> result;
	if (data ==NULL) return result;
	
	sampleRate =0;
	int16_t sample =0;
	int state =0;
	int repeatCount =0;
	int initialSilence =0;
	int divider =0;
	const uint8_t* repeatOffset =data;
	
	while (*data !=0x00) {
		int nibbles =0;
		int silence =0;
			
		uint8_t command =*data >>6;
		uint8_t parameter =*data++ &0x3F;
		switch (command) {
			case 0: // silence
				silence =256 *(parameter +1);
				sample =0;
				state =0;
				break;
			case 1:	// 256 nibbles
				divider =parameter +1;
				nibbles =256;
				break;
			case 2:	// n+1 nibbles
				divider =parameter +1;
				nibbles =*data++ +1;
				break;
			case 3:	// repeat
				repeatCount =(parameter &7) +1;
				repeatOffset =data;
				break;
		}
		for (int j =0; j <nibbles; j++) {
			int nibble =j &1? (*data++ &0x0F): (*data >>4);
			sample +=upd7756_step[state][nibble];
			state +=upd7756_state_table[nibble];
			if (state < 0) state =0;
			if (state >15) state =15;			
			result.push_back((sample <<7) | (sample &0x7F));
		}
		if (nibbles &1) data++;

		if (silence) {
			if (divider)
				for (int i =0; i <silence; i +=divider) result.push_back(0);
			else
				initialSilence +=silence;
		}
		
		if (repeatCount) {
			repeatCount--;
			data =repeatOffset;
		}
		if (divider) sampleRate =MASTER_CLOCK /divider;
	}
	if (initialSilence && divider) for (int i =0; i <initialSilence; i +=divider) result.insert(result.begin(), 1, 0);
	
	return result;
}

void loadSampleROM (unsigned char *data, size_t size) {
	butterworth lowPassFilter(20, 44100, 8000);
	
	unsigned int samples =upd7756_masterROMSamples(data, size);
	for (unsigned int sampleNumber =0; sampleNumber <samples; sampleNumber++) {
		const uint8_t* sampleStart =upd7756_findSample(data, size, sampleNumber);
		
		int sampleRate;
		std::vector<int16_t> output =upd7756_decodeSample(sampleStart, sampleRate);
		lowPassFilter.recalc(20, 44100, sampleRate /2);
		
		unsigned int filteredSize =44100 *output.size() /sampleRate +1;
		int count =0;
		unsigned int j =0;
		std::vector<int16_t> filteredOutput;
		while (filteredOutput.size() <filteredSize) {
			auto result =lowPassFilter.output(output[j] +1e-15);
			if (result >32767) result =32767;
			if (result<-32768) result =-32768;
			filteredOutput.push_back(result);
			count +=sampleRate;
			while (count >=44100) { count -=44100; j++; }
		}
		
		WaveFile wave(&filteredOutput[0], filteredOutput.size(), 44100);
		waveFiles.push_back(wave);
	}	
}

} // namespace UPD7756