/*
 * approximation.c
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
#include "approximation.h"

SWORD (*extra_mixer_approximation)(SWORD mixer);
SWORD mixer_approximation_FDS(SWORD mixer);
SWORD mixer_approximation_MMC5(SWORD mixer);
SWORD mixer_approximation_Namco_N163(SWORD mixer);
SWORD mixer_approximation_Sunsoft_FM7(SWORD mixer);
SWORD mixer_approximation_VRC6(SWORD mixer);
SWORD mixer_approximation_VRC7(SWORD mixer);

void audio_filter_init_approximation(void) {
	/* azzero l'ouput di tutti i canali */
	audio_filter_reset_output_channels();

	/* popolo la tabella di approssimazione */
	audio_filter_popolate_table_approx();

	audio_filter_apu_tick = audio_filter_apu_tick_approximation;
	audio_filter_apu_mixer = audio_filter_apu_mixer_approximation;

	extra_mixer_approximation = NULL;

	switch (info.mapper) {
		case FDS_MAPPER:
			/* FDS */
			extra_mixer_approximation = mixer_approximation_FDS;
			break;
		case 5:
			/* MMC5 */
			extra_mixer_approximation = mixer_approximation_MMC5;
			break;
		case 19:
			/* Namcot N163 */
			extra_mixer_approximation = mixer_approximation_Namco_N163;
			break;
		case 69:
			/* Sunsoft FM7 */
			extra_mixer_approximation = mixer_approximation_Sunsoft_FM7;
			break;
		case 24:
		case 26:
			/* VRC6 */
			extra_mixer_approximation = mixer_approximation_VRC6;
			break;
		case 85:
			/* VRC7 */
			extra_mixer_approximation = mixer_approximation_VRC7;
			break;
	}
}
void audio_filter_apu_tick_approximation(void) {
	return;
}
SWORD audio_filter_apu_mixer_approximation(void) {
	WORD p  = S1.output + S2.output;
	WORD t  = (3 * TR.output) + (2 * NS.output) + DMC.output;
	SWORD mixer = af_table_approx.pulse[p] + af_table_approx.tnd[t];

	if (extra_mixer_approximation) {
		mixer = extra_mixer_approximation(mixer);
	}

	mixer *= 1.6;

	if (mixer > 0x7FFF) {
		mixer = 0x7FFF;
	} else if (mixer < -0x7FFF) {
		mixer = -0x7FFF;
	}

	return (mixer);
}

/* --------------------------------------------------------------------------------------- */
/*                                   Extra Audio Mixer                                     */
/* --------------------------------------------------------------------------------------- */
SWORD mixer_approximation_FDS(SWORD mixer) {
	return (mixer + fds.snd.main.output);
}
SWORD mixer_approximation_MMC5(SWORD mixer) {
	WORD p = mmc5.S3.output + mmc5.S4.output;
	WORD t = mmc5.pcmSample;

	return (mixer + af_table_approx.pulse[p] + af_table_approx.tnd[t]);
}
SWORD mixer_approximation_Namco_N163(SWORD mixer) {
	BYTE i;
	WORD a = 0;

	for (i = n163.sndChStart; i < 8; i++) {
		if (n163.ch[i].active) {
			a += (n163.ch[i].output * n163.ch[i].volume);
		}
	}

	if ((a <<= 4) > 0x3FFF) {
		a = 0x3FFF;
	}

	return (mixer + a);
}
SWORD mixer_approximation_Sunsoft_FM7(SWORD mixer) {
	WORD p  = fm7.square[0].output + fm7.square[1].output + fm7.square[2].output;

	return (mixer + af_table_approx.pulse[p]);
}
SWORD mixer_approximation_VRC6(SWORD mixer) {
	WORD p = vrc6.S3.output + vrc6.S4.output;
	WORD t = vrc6.saw.output;

	return (mixer + (af_table_approx.pulse[p] * 1.35) + (af_table_approx.tnd[t] * 0.20));
}
SWORD mixer_approximation_VRC7(SWORD mixer) {
	return (mixer + (opll_calc() << 2));
}
