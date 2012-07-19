/*
 * linear.c
 *
 *  Created on: 11/lug/2012
 *      Author: fhorse
 */

#include <string.h>
#include "audio_filter.h"
#include "apu.h"
#include "sdlsnd.h"
#include "mappers.h"
#include "mappers/mapperVRC7snd.h"
#include "fds.h"
#include "linear.h"

void (*extra_tick_linear)(void);
void tick_linear_FDS(void);
void tick_linear_MMC5(void);
void tick_linear_Namco_N163(void);
void tick_linear_Sunsoft_FM7(void);
void tick_linear_VRC6(void);

SDBWORD (*extra_mixer_linear)(SDBWORD mixer);
SDBWORD mixer_linear_FDS(SDBWORD mixer);
SDBWORD mixer_linear_MMC5(SDBWORD mixer);
SDBWORD mixer_linear_Namco_N163(SDBWORD mixer);
SDBWORD mixer_linear_Sunsoft_FM7(SDBWORD mixer);
SDBWORD mixer_linear_VRC6(SDBWORD mixer);
SDBWORD mixer_linear_VRC7(SDBWORD mixer);

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

struct _af_linear {
	DBWORD divider;
	DBWORD ch[AFTOTCH];
} af_linear;

void audio_filter_init_linear(void) {
	memset(&af_linear, 0, sizeof(af_linear));

	/* azzero l'ouput di tutti i canali */
	audio_filter_reset_output_channels();

	/* popolo la tabella di approssimazione */
	audio_filter_popolate_table_approx();

	audio_filter_apu_tick = audio_filter_apu_tick_linear;
	audio_filter_apu_mixer = audio_filter_apu_mixer_linear;

	extra_tick_linear = NULL;
	extra_mixer_linear = NULL;

	switch (info.mapper) {
		case FDS_MAPPER:
			/* FDS */
			extra_tick_linear = tick_linear_FDS;
			extra_mixer_linear = mixer_linear_FDS;
			break;
		case 5:
			/* MMC5 */
			extra_tick_linear = tick_linear_MMC5;
			extra_mixer_linear = mixer_linear_MMC5;
			break;
		case 19:
			/* Namcot N163 */
			extra_tick_linear = tick_linear_Namco_N163;
			extra_mixer_linear = mixer_linear_Namco_N163;
			break;
		case 69:
			/* Sunsoft FM7 */
			extra_tick_linear = tick_linear_Sunsoft_FM7;
			extra_mixer_linear = mixer_linear_Sunsoft_FM7;
			break;
		case 24:
		case 26:
			/* VRC6 */
			extra_tick_linear = tick_linear_VRC6;
			extra_mixer_linear = mixer_linear_VRC6;
			break;
		case 85:
			/* VRC7 */
			extra_mixer_linear = mixer_linear_VRC7;
			break;
	}
}
void audio_filter_apu_tick_linear(void) {
	af_linear.ch[AFS1] += S1.output;
	af_linear.ch[AFS2] += S2.output;
	af_linear.ch[AFTR] += TR.output;
	af_linear.ch[AFNS] += NS.output;
	af_linear.ch[AFDMC] += DMC.output;

	if (extra_tick_linear) {
		extra_tick_linear();
	}

	af_linear.divider++;

	return;
}
SWORD audio_filter_apu_mixer_linear(void) {
	DBWORD p, t;
	SDBWORD mixer;

	p  = (af_linear.ch[AFS1] / af_linear.divider) + (af_linear.ch[AFS2] / af_linear.divider);
	t  = (3 * (af_linear.ch[AFTR] / af_linear.divider));
	t += (2 * (af_linear.ch[AFNS] / af_linear.divider));
	t += (af_linear.ch[AFDMC] / af_linear.divider);

	mixer = af_table_approx.pulse[p] + af_table_approx.tnd[t];

	if (extra_mixer_linear) {
		mixer = extra_mixer_linear(mixer);
	}

	mixer *= 1.5;

	if (mixer > 0x7FFF) {
		mixer = 0x7FFF;
	} else if (mixer < -0x7FFF) {
		mixer = -0x7FFF;
	}

	{
		BYTE i;

		for (i = 0; i < AFTOTCH; i++) {
			af_linear.ch[i] = 0;
		}
		af_linear.divider = 0;
	}

	return (mixer);
}

/* --------------------------------------------------------------------------------------- */
/*                                   Extra Audio Tick                                      */
/* --------------------------------------------------------------------------------------- */
void tick_linear_FDS(void) {
	af_linear.ch[AFEXT0] += fds.snd.main.output;
}
void tick_linear_MMC5(void) {
	af_linear.ch[AFEXT0] += mmc5.S3.output;
	af_linear.ch[AFEXT1] += mmc5.S4.output;
	af_linear.ch[AFEXT2] += mmc5.pcmSample;
}
void tick_linear_Namco_N163(void) {
	BYTE i;

	for (i = 0; i < 8; i++) {
		if (n163.ch[i].active) {
			af_linear.ch[AFEXT0 + i] += (n163.ch[i].output * n163.ch[i].volume);
		}
	}
}
void tick_linear_Sunsoft_FM7(void) {
	af_linear.ch[AFEXT0] += fm7.square[0].output;
	af_linear.ch[AFEXT1] += fm7.square[1].output;
	af_linear.ch[AFEXT2] += fm7.square[2].output;
}
void tick_linear_VRC6(void) {
	af_linear.ch[AFEXT0] += vrc6.S3.output;
	af_linear.ch[AFEXT1] += vrc6.S4.output;
	af_linear.ch[AFEXT2] += vrc6.saw.output;
}

/* --------------------------------------------------------------------------------------- */
/*                                   Extra Audio Mixer                                     */
/* --------------------------------------------------------------------------------------- */
SDBWORD mixer_linear_FDS(SDBWORD mixer) {
	return (mixer + (af_linear.ch[AFEXT0] / af_linear.divider));
}
SDBWORD mixer_linear_MMC5(SDBWORD mixer) {
	DBWORD p = (af_linear.ch[AFEXT0] / af_linear.divider)
	        + (af_linear.ch[AFEXT1] / af_linear.divider);
	DBWORD t = (af_linear.ch[AFEXT2] / af_linear.divider);

	return (mixer + af_table_approx.pulse[p] + af_table_approx.tnd[t]);
}
SDBWORD mixer_linear_Namco_N163(SDBWORD mixer) {
	BYTE i;
	DBWORD a = 0;

	for (i = n163.sndChStart; i < 8; i++) {
		if (n163.ch[i].active) {
			//a += (n163.ch[i].output * n163.ch[i].volume);
			a += (af_linear.ch[AFEXT0 + i] / af_linear.divider);
		}
	}

	if ((a <<= 4) > 0x3FFF) {
		a = 0x3FFF;
	}

	return (mixer + a);
}
SDBWORD mixer_linear_Sunsoft_FM7(SDBWORD mixer) {
	DBWORD p;

	p = (af_linear.ch[AFEXT0] / af_linear.divider);
	p += (af_linear.ch[AFEXT1] / af_linear.divider);
	p += (af_linear.ch[AFEXT2] / af_linear.divider);

	return (mixer + af_table_approx.pulse[p]);
}
SDBWORD mixer_linear_VRC6(SDBWORD mixer) {
	DBWORD p = (af_linear.ch[AFEXT0] / af_linear.divider)
	        + (af_linear.ch[AFEXT1] / af_linear.divider);
	DBWORD t = (af_linear.ch[AFEXT2] / af_linear.divider);

	return (mixer + (af_table_approx.pulse[p] * 1.35) + (af_table_approx.tnd[t] * 0.20));
}
SDBWORD mixer_linear_VRC7(SDBWORD mixer) {
	return (mixer + (opll_calc() << 2));
}
