/*
 * none.c
 *
 *  Created on: 11/lug/2012
 *      Author: fhorse
 */

#include "audio_filter.h"
#include "apu.h"
#include "sdlsnd.h"
#include "mappers.h"
#include "mappers/mapperVRC7snd.h"
#include "fds.h"
#include "none2.h"

SWORD (*extra_audio_mixer_none2)(SWORD mixer);
SWORD audio_filters_none2_FDS(SWORD mixer);
SWORD audio_filters_none2_MMC5(SWORD mixer);
SWORD audio_filters_none2_Namco_N163(SWORD mixer);
SWORD audio_filters_none2_Sunsoft_FM7(SWORD mixer);
SWORD audio_filters_none2_VRC6(SWORD mixer);
SWORD audio_filters_none2_VRC7(SWORD mixer);

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

struct {
	SWORD pulse[32];
	SWORD tnd[203];
	SWORD mmc5_pulse[32];
	SWORD mmc5_tnd[256];

	WORD cycles;
	DBWORD accumulators[AFTOTCH];

	BYTE flag;
} af_none2;

void audio_filter_init_none2(void) {

	{
		WORD i;

		/* 0x7FFF / 2 = 0x2AAA */
		for (i = 0; i < LENGTH(af_none2.pulse); i++) {
			float vl = 95.52 / (8128.0 / i + 100);
			af_none2.pulse[i] = (vl * 0x3FFF) - 0x2000;
		}

		for (i = 0; i < LENGTH(af_none2.tnd); i++) {
			float vl = 163.67 / (24329.0 / i + 100);
			af_none2.tnd[i] = (vl * 0x3FFF);
		}

		for (i = 0; i < LENGTH(af_none2.mmc5_pulse); i++) {
			float vl = 95.52 / (8128.0 / i + 100);
			//af_none2.mmc5_pulse[i] = (vl * (0x2AAA / 2));
			af_none2.mmc5_pulse[i] = (vl * 0x3FFF) - 0x2000;
		}

		for (i = 0; i < LENGTH(af_none2.mmc5_tnd); i++) {
			float vl = 163.67 / (24329.0 / i + 100);
			//af_none2.mmc5_tnd[i] = (vl * (0x2AAA / 2));
			af_none2.mmc5_tnd[i] = (vl * 0x3FFF);
		}
	}

	af_none2.cycles = 0;
	af_none2.accumulators[AFS1] = 0;
	af_none2.accumulators[AFS2] = 0;
	af_none2.accumulators[AFTR] = 0;
	af_none2.accumulators[AFNS] = 0;
	af_none2.accumulators[AFDMC] = 0;

	audio_filter_apu_tick = audio_filter_apu_tick_none2;
	audio_filter_apu_mixer = audio_filter_apu_mixer_none2;

	switch (info.mapper) {
		case FDS_MAPPER:
			/* FDS */
			extra_audio_mixer_none2 = audio_filters_none2_FDS;
			break;
		case 5:
			/* MMC5 */
			extra_audio_mixer_none2 = audio_filters_none2_MMC5;
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
			extra_audio_mixer_none2 = audio_filters_none2_VRC7;
			break;
		default:
			extra_audio_mixer_none2 = NULL;
			break;
	}
	return;
}
void audio_filter_apu_tick_none2(void) {
	af_none2.cycles++;
	af_none2.accumulators[AFS1] += S1.output;
	af_none2.accumulators[AFS2] += S2.output;
	af_none2.accumulators[AFTR] += TR.output;
	af_none2.accumulators[AFNS] += NS.output;
	af_none2.accumulators[AFDMC] += DMC.output;

	//S1.fc++;
	//NS.fc++;
	return;
}
SWORD audio_filter_apu_mixer_none2(void) {
	/*BYTE p = S1.output + S2.output;
	BYTE t = (3 * TR.output) + (2 * NS.output) + DMC.output;
	SWORD mixer = pulse[p] + tnd[t];
	*/

	SWORD mixer;

	mixer = af_none2.pulse[(af_none2.accumulators[AFS1] / af_none2.cycles)
	        + (af_none2.accumulators[AFS2] / af_none2.cycles)];

	mixer += af_none2.tnd[(3 * (af_none2.accumulators[AFTR] / af_none2.cycles))
	        + (2 * (af_none2.accumulators[AFNS] / af_none2.cycles))
	        + (af_none2.accumulators[AFDMC] / af_none2.cycles)];


	af_none2.cycles = 0;
	af_none2.accumulators[AFS1] = 0;
	af_none2.accumulators[AFS2] = 0;
	af_none2.accumulators[AFTR] = 0;
	af_none2.accumulators[AFNS] = 0;
	af_none2.accumulators[AFDMC] = 0;

	/*if (!af_none2.flag) {
		if (S1.output) {
			af_none2.flag = TRUE;
		}
	}

	mixer = S1.output;

	BYTE p = S1.prev + (S1.output - S1.prev) * ((float) S1.fc / (float) af_none2.cycles);

	mixer = af_none2.pulse[p];

	mixer = NS.output;

	BYTE t = 2 * (NS.prev + (NS.output - NS.prev) * ((float) NS.fc / (float) af_none2.cycles));
	mixer += af_none2.tnd[t];

	if (af_none2.flag) {
		printf("remain: %d %d %d - %d %d\n", af_none2.cycles, mixer, S1.output, S1.prev, S1.fc);
	}

	S1.fc = 0;
	NS.fc = 0;
	af_none2.cycles = 0;
	S1.prev = S1.output;
	NS.prev = NS.output;

	mixer <<= 7;
	*/
	return(mixer);

	if (extra_audio_mixer_none2) {
		mixer = extra_audio_mixer_none2(mixer);
	}

	return (mixer);
}

/* --------------------------------------------------------------------------------------- */
/* Mappers                                                                                 */
/* --------------------------------------------------------------------------------------- */
SWORD audio_filters_none2_FDS(SWORD mixer) {
	/*return (mixer + (fds.snd.main.output + (fds.snd.main.output >> 1)));*/
	return (mixer + fds.snd.main.output);
}
SWORD audio_filters_none2_MMC5(SWORD mixer) {
	return (mixer + af_none2.mmc5_pulse[mmc5.S3.output + mmc5.S4.output]
	        + af_none2.mmc5_tnd[mmc5.pcmSample]);
}
SWORD audio_filters_none2_Namco_N163(SWORD mixer) {
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
SWORD audio_filters_none2_Sunsoft_FM7(SWORD mixer) {
	mixer += (fm7.square[0].output + fm7.square[1].output + fm7.square[2].output);

	apuMixerCutAndHigh();

	return (mixer);
}
SWORD audio_filters_none2_VRC6(SWORD mixer) {
	mixer += (vrc6.S3.output + vrc6.S4.output) + vrc6.saw.output;

	apuMixerCutAndHigh();

	return (mixer);
}
SWORD audio_filters_none2_VRC7(SWORD mixer) {
	return (mixer + (opll_calc() << 2));
}
