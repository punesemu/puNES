/*
 * simple.c
 *
 *  Created on: 11/lug/2012
 *      Author: fhorse
 */

#include <string.h>
#include "audio_filter.h"
#include "apu.h"
#include "mappers.h"
#include "mappers/mapperVRC7snd.h"
#include "fds.h"
#include "simple.h"

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

SWORD (*extra_audio_mixer_simple)(SWORD mixer);
SWORD mixer_simple_FDS(SWORD mixer);
SWORD mixer_simple_MMC5(SWORD mixer);
SWORD mixer_simple_Namco_N163(SWORD mixer);
SWORD mixer_simple_Sunsoft_FM7(SWORD mixer);
SWORD mixer_simple_VRC6(SWORD mixer);
SWORD mixer_simple_VRC7(SWORD mixer);

void (*extra_audio_tick_simple)(void);
void tick_simple_FDS(void);
void tick_simple_MMC5(void);
void tick_simple_Namco_N163(void);
void tick_simple_Sunsoft_FM7(void);
void tick_simple_VRC6(void);

struct _af_simple {
	SDBWORD ch[AFTOTCH];
	DBWORD divider;
} af_simple;

void audio_filter_init_simple(void) {

	memset(&af_simple, 0, sizeof(af_simple));

	audio_filter_apu_tick = audio_filter_apu_tick_simple;
	audio_filter_apu_mixer = audio_filter_apu_mixer_simple;

	extra_audio_mixer_simple = NULL;
	extra_audio_tick_simple = NULL;

	switch (info.mapper) {
		case FDS_MAPPER:
			/* FDS */
			extra_audio_tick_simple = tick_simple_FDS;
			extra_audio_mixer_simple = mixer_simple_FDS;
			break;
		case 5:
			/* MMC5 */
			extra_audio_tick_simple = tick_simple_MMC5;
			extra_audio_mixer_simple = mixer_simple_MMC5;
			break;
		case 19:
			/* Namcot N163 */
			extra_audio_tick_simple = tick_simple_Namco_N163;
			extra_audio_mixer_simple = mixer_simple_Namco_N163;
			break;
		case 69:
			/* Sunsoft FM7 */
			extra_audio_tick_simple = tick_simple_Sunsoft_FM7;
			extra_audio_mixer_simple = mixer_simple_Sunsoft_FM7;
			break;
		case 24:
		case 26:
			/* VRC6 */
			extra_audio_tick_simple = tick_simple_VRC6;
			extra_audio_mixer_simple = mixer_simple_VRC6;
			break;
		case 85:
			/* VRC7 */
			/* il tick non lo uso perche' l'audio del VRC7 e' gia' filtrato */
			extra_audio_mixer_simple = mixer_simple_VRC7;
			break;
	}
	return;
}
void audio_filter_apu_tick_simple(void) {
	af_simple.ch[AFS1]  += S1.output;
	af_simple.ch[AFS2]  += S2.output;
	af_simple.ch[AFTR]  += TR.output;
	af_simple.ch[AFNS]  += NS.output;
	af_simple.ch[AFDMC] += DMC.output;

	if (extra_audio_tick_simple) {
		extra_audio_tick_simple();
	}

	af_simple.divider++;

	return;
}
SWORD audio_filter_apu_mixer_simple(void) {
	SWORD mixer =
			(af_simple.ch[AFS1]  / af_simple.divider) +
	        (af_simple.ch[AFS2]  / af_simple.divider) +
	        (af_simple.ch[AFTR]  / af_simple.divider) +
	        (af_simple.ch[AFNS]  / af_simple.divider) +
	        (af_simple.ch[AFDMC] / af_simple.divider);

	if (extra_audio_mixer_simple) {
		mixer = extra_audio_mixer_simple(mixer);
	} else {
		apuMixerCutAndHigh();
	}

	{
		BYTE i;

		for (i = 0; i < AFTOTCH; i++) {
			af_simple.ch[i] = 0;
		}
		af_simple.divider = 0;
	}

	return (mixer);
}

/* --------------------------------------------------------------------------------------- */
/*                                   Extra Audio Tick                                      */
/* --------------------------------------------------------------------------------------- */
void tick_simple_FDS(void) {
	af_simple.ch[AFEXT0] += (fds.snd.main.output + (fds.snd.main.output >> 1));
}
void tick_simple_MMC5(void) {
	af_simple.ch[AFEXT0] += mmc5.S3.output;
	af_simple.ch[AFEXT1] += mmc5.S4.output;
	af_simple.ch[AFEXT2] += mmc5.pcmSample;
}
void tick_simple_Sunsoft_FM7(void) {
	af_simple.ch[AFEXT0] += fm7.square[0].output;
	af_simple.ch[AFEXT1] += fm7.square[1].output;
	af_simple.ch[AFEXT2] += fm7.square[2].output;
}
void tick_simple_Namco_N163(void) {
	BYTE i;

	for (i = 0; i < 8; i++) {
		if (n163.ch[i].active) {
			af_simple.ch[AFEXT0 + i] += (n163.ch[i].output * (n163.ch[i].volume >> 2));
		}
	}
}
void tick_simple_VRC6(void) {
	af_simple.ch[AFEXT0] += vrc6.S3.output;
	af_simple.ch[AFEXT1] += vrc6.S4.output;
	af_simple.ch[AFEXT2] += vrc6.saw.output;
}

/* --------------------------------------------------------------------------------------- */
/*                                   Extra Audio Mixer                                     */
/* --------------------------------------------------------------------------------------- */
SWORD mixer_simple_FDS(SWORD mixer) {
	apuMixerCutAndHigh();

	return (mixer + (af_simple.ch[AFEXT0] / af_simple.divider));
}
SWORD mixer_simple_MMC5(SWORD mixer) {
	mixer += (af_simple.ch[AFEXT0] / af_simple.divider) +
			(af_simple.ch[AFEXT1] / af_simple.divider) +
			(af_simple.ch[AFEXT2] / af_simple.divider);

	apuMixerCutAndHigh();

	return (mixer);
}
SWORD mixer_simple_Namco_N163(SWORD mixer) {
	BYTE i;

	for (i = 0; i < 8; i++) {
		if (n163.ch[i].active) {
			mixer += (af_simple.ch[AFEXT0 + i] / af_simple.divider);
		}
	}

	apuMixerCutAndHigh();

	return (mixer);
}
SWORD mixer_simple_Sunsoft_FM7(SWORD mixer) {
	mixer += (af_simple.ch[AFEXT0] / af_simple.divider) +
			(af_simple.ch[AFEXT1] / af_simple.divider) +
			(af_simple.ch[AFEXT2] / af_simple.divider);

	apuMixerCutAndHigh();

	return (mixer);
}
SWORD mixer_simple_VRC6(SWORD mixer) {
	mixer += (af_simple.ch[AFEXT0] / af_simple.divider) +
			(af_simple.ch[AFEXT1] / af_simple.divider) +
			(af_simple.ch[AFEXT2] / af_simple.divider);

	apuMixerCutAndHigh();

	return (mixer);
}
SWORD mixer_simple_VRC7(SWORD mixer) {
	apuMixerCutAndHigh();

	return (mixer + (opll_calc() << 2));
}
