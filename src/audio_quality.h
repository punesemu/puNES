/*
 * audio_quality.h
 *
 *  Created on: 30/lug/2012
 *      Author: fhorse
 */

#ifndef AUDIO_QUALITY_H_
#define AUDIO_QUALITY_H_

#include "common.h"

enum {
	AQ_LOW = 0,
	AQ_HIGH
};

enum {
	AQ_S1 = 0,
	AQ_S2,
	AQ_TR,
	AQ_NS,
	AQ_DMC,
	AQ_EXT0,
	AQ_EXT1,
	AQ_EXT2,
	AQ_EXT3,
	AQ_EXT4,
	AQ_EXT5,
	AQ_EXT6,
	AQ_EXT7,
	AQ_TOT_CH
};

void audio_quality(BYTE quality);

/* funzioni virtuali */
void  (*audio_quality_init)(void);
void  (*audio_quality_apu_tick)(void);
void  (*audio_quality_end_frame)(void);
SWORD (*audio_quality_apu_mixer)(void);

#endif /* AUDIO_QUALITY_H_ */
