/*
 * none.c
 *
 *  Created on: 11/lug/2012
 *      Author: fhorse
 */

#include "audio_filter.h"
#include "apu.h"
#include "mappers.h"
#include "mappers/mapperVRC7snd.h"
#include "fds.h"
#include "none.h"

SWORD (*extra_audio_mixer_none)(SWORD mixer);
SWORD audio_filters_none_FDS(SWORD mixer);
SWORD audio_filters_none_MMC5(SWORD mixer);
SWORD audio_filters_none_Namco_N163(SWORD mixer);
SWORD audio_filters_none_Sunsoft_FM7(SWORD mixer);
SWORD audio_filters_none_VRC6(SWORD mixer);
SWORD audio_filters_none_VRC7(SWORD mixer);

SWORD pulse[32];
SWORD tnd[203];

SWORD mmc5_pulse[32];
SWORD mmc5_tnd[256];


void audio_filter_init_none(void) {

	{
		WORD i;

		/* 0x7FFF / 2 = 0x2AAA */
		for (i = 0; i < LENGTH(pulse); i++) {
			float vl = 95.52 / (8128.0 / i + 100);
			pulse[i] = (vl * 0x3FFF) - 0x2000;
		}

		for (i = 0; i < LENGTH(tnd); i++) {
			float vl = 163.67 / (24329.0 / i + 100);
			tnd[i] = (vl * 0x3FFF);
		}

		for (i = 0; i < LENGTH(mmc5_pulse); i++) {
			float vl = 95.52 / (8128.0 / i + 100);
			//mmc5_pulse[i] = (vl * (0x2AAA / 2));
			mmc5_pulse[i] = (vl * 0x3FFF) - 0x2000;
		}

		for (i = 0; i < LENGTH(mmc5_tnd); i++) {
			float vl = 163.67 / (24329.0 / i + 100);
			//mmc5_tnd[i] = (vl * (0x2AAA / 2));
			mmc5_tnd[i] = (vl * 0x3FFF);
		}
	}

	audio_filter_apu_tick = audio_filter_apu_tick_none;
	audio_filter_apu_mixer = audio_filter_apu_mixer_none;

	switch (info.mapper) {
		case FDS_MAPPER:
			/* FDS */
			extra_audio_mixer_none = audio_filters_none_FDS;
			break;
		case 5:
			/* MMC5 */
			extra_audio_mixer_none = audio_filters_none_MMC5;
			break;
		case 19:
			/* Namcot N163 */
			//extra_audio_mixer_none = audio_filters_none_Namco_N163;
			break;
		case 69:
			/* Sunsoft FM7 */
			//extra_audio_mixer_none = audio_filters_none_Sunsoft_FM7;
			break;
		case 24:
		case 26:
			/* VRC6 */
			//extra_audio_mixer_none = audio_filters_none_VRC6;
			break;
		case 85:
			/* VRC7 */
			extra_audio_mixer_none = audio_filters_none_VRC7;
			break;
		default:
			extra_audio_mixer_none = NULL;
			break;
	}
	return;
}
void audio_filter_apu_tick_none(void) {
	return;
}
SWORD audio_filter_apu_mixer_none(void) {
	BYTE p = S1.output + S2.output;
	BYTE t = (3 * TR.output) + (2 * NS.output) + DMC.output;
	SWORD mixer = pulse[p] + tnd[t];

	if (extra_audio_mixer_none) {
		mixer = extra_audio_mixer_none(mixer);
	}

	return (mixer);
}

/* --------------------------------------------------------------------------------------- */
/* Mappers                                                                                 */
/* --------------------------------------------------------------------------------------- */
SWORD audio_filters_none_FDS(SWORD mixer) {
	/*return (mixer + (fds.snd.main.output + (fds.snd.main.output >> 1)));*/
	return (mixer + fds.snd.main.output);
}
SWORD audio_filters_none_MMC5(SWORD mixer) {
	return (mixer + mmc5_pulse[mmc5.S3.output + mmc5.S4.output] + mmc5_tnd[mmc5.pcmSample]);
}
SWORD audio_filters_none_Namco_N163(SWORD mixer) {
	BYTE i;
	SWORD a = 0;

	for (i = n163.sndChStart; i < 8; i++) {
		if (n163.ch[i].active) {
			a += (n163.ch[i].output * (n163.ch[i].volume >> 2));
		}
	}

	mixer += a;

	apuMixerCutAndHigh();

	return (mixer);
}
SWORD audio_filters_none_Sunsoft_FM7(SWORD mixer) {
	mixer += (fm7.square[0].output + fm7.square[1].output + fm7.square[2].output);

	apuMixerCutAndHigh();

	return (mixer);
}
SWORD audio_filters_none_VRC6(SWORD mixer) {
	mixer += (vrc6.S3.output + vrc6.S4.output) + vrc6.saw.output;

	apuMixerCutAndHigh();

	return (mixer);
}
SWORD audio_filters_none_VRC7(SWORD mixer) {
	return (mixer + (opll_calc() << 2));
}
