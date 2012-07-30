/*
 * audio_filter.h
 *
 *  Created on: 11/lug/2012
 *      Author: fhorse
 */

#ifndef AUDIO_FILTER_H_
#define AUDIO_FILTER_H_

#include "common.h"

enum {
	AF_ORIGINAL = 0,
	AF_APPROXIMATION,
	AF_LINEAR
};

enum {
	AFS1 = 0,
	AFS2,
	AFTR,
	AFNS,
	AFDMC,
	AFEXT0,
	AFEXT1,
	AFEXT2,
	AFEXT3,
	AFEXT4,
	AFEXT5,
	AFEXT6,
	AFEXT7,
	AFTOTCH
};

struct _af_table_approx {
	SWORD pulse[48];
	SWORD tnd[256];
} af_table_approx;

void audio_filter(BYTE filter);
void audio_filter_popolate_table_approx(void);
void audio_filter_reset_output_channels(void);
void audio_filter_end_frame_none(void);

/* funzioni virtuali */
void (*audio_filter_init)(void);
void (*audio_filter_apu_tick)(void);
void (*audio_filter_end_frame)(void);
SWORD (*audio_filter_apu_mixer)(void);

#endif /* AUDIO_FILTER_H_ */
