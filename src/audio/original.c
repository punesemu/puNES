/*
 * original.c
 *
 *  Created on: 11/lug/2012
 *      Author: fhorse
 */

#include <string.h>
#include "apu.h"
#include "snd.h"
#include "audio/quality.h"
#include "audio/channels.h"
#include "audio/original.h"
#include "mappers.h"
#include "mappers/mapper_VRC7_snd.h"
#include "fds.h"
#include "conf.h"
#include "clock.h"
#include "fps.h"
#include "info.h"

#define _ch_gain(index, f) ((double) (f * cfg->apu.volume[index]))
#define ch_gain_ptnd(index) _ch_gain(index, 1.0f)
#define mixer_cut_and_high() mixer *= 45

static struct _original {
	DBWORD cycles;
	struct {
		DBWORD current;
		DBWORD last;
	} pos;
} original;

SWORD (*extra_mixer_original)(SWORD mixer);
SWORD mixer_original_FDS(SWORD mixer);
SWORD mixer_original_MMC5(SWORD mixer);
SWORD mixer_original_Namco_N163(SWORD mixer);
SWORD mixer_original_Sunsoft_FM7(SWORD mixer);
SWORD mixer_original_VRC6(SWORD mixer);
SWORD mixer_original_VRC7(SWORD mixer);

BYTE audio_quality_init_original(void) {
	audio_quality_quit = audio_quality_quit_original;
	snd_apu_tick = audio_quality_apu_tick_original;

	memset(&original, 0x00, sizeof(original));

	init_nla_table(502, 522)

	switch (info.mapper.id) {
		case FDS_MAPPER:
			/* FDS */
			extra_mixer_original = mixer_original_FDS;
			break;
		case 5:
			/* MMC5 */
			extra_mixer_original = mixer_original_MMC5;
			break;
		case 19:
			/* Namcot N163 */
			extra_mixer_original = mixer_original_Namco_N163;
			break;
		case 69:
			/* Sunsoft FM7 */
			extra_mixer_original = mixer_original_Sunsoft_FM7;
			break;
		case 24:
		case 26:
			/* VRC6 */
			extra_mixer_original = mixer_original_VRC6;
			break;
		case 85:
			/* VRC7 */
			extra_mixer_original = mixer_original_VRC7;
			break;
		default:
			extra_mixer_original = NULL;
			break;
	}

	return (EXIT_OK);
}
void audio_quality_quit_original(void) {
	return;
}
void audio_quality_apu_tick_original(void) {
	if (!cfg->apu.channel[APU_MASTER] || fps.fast_forward) {
		if (SNDCACHE) {
			SNDCACHE->write = SNDCACHE->start;
			SNDCACHE->read = (SBYTE *) SNDCACHE->start;
			SNDCACHE->bytes_available = SNDCACHE->samples_available = 0;
		}
		snd.buffer.start = FALSE;
		return;
	}

	if ((original.pos.current = original.cycles++ / snd.frequency) == original.pos.last) {
		return;
	}

	if (snd_handler() == EXIT_ERROR) {
		return;
	}

	snd_lock_cache(SNDCACHE);

	{
		SWORD mixer = 0;

		mixer = pulse_output() + tnd_output();
		mixer *= cfg->apu.volume[APU_MASTER];

		if (extra_mixer_original) {
			mixer = extra_mixer_original(mixer);
		} else {
			mixer_cut_and_high();
		}

		audio_channels_tick(mixer);

		if (SNDCACHE->write == (SWORD *) SNDCACHE->end) {
			SNDCACHE->write = SNDCACHE->start;
		}

		original.pos.last = original.pos.current;

		if (original.pos.current >= snd.samples) {
			snd.buffer.start = TRUE;
			// azzero posizione e contatore dei cicli del frame audio
			original.pos.current = original.cycles = 0;
		}
	}

	snd_unlock_cache(SNDCACHE);
}

/* --------------------------------------------------------------------------------------- */
/*                                   Extra Audio Mixer                                     */
/* --------------------------------------------------------------------------------------- */
SWORD mixer_original_FDS(SWORD mixer) {
	mixer_cut_and_high();

	mixer += (extra_out(fds.snd.main.output) * ch_gain_ptnd(APU_EXTRA));
	return (mixer);
}
SWORD mixer_original_MMC5(SWORD mixer) {
	mixer += ((SWORD) (extra_out(mmc5.S3.output) * ch_gain_ptnd(APU_EXTRA)) << 3);
	mixer += ((SWORD) (extra_out(mmc5.S4.output) * ch_gain_ptnd(APU_EXTRA)) << 3);
	mixer += ((SWORD) (extra_out(mmc5.pcm.output) * ch_gain_ptnd(APU_EXTRA)) << 3);

	mixer_cut_and_high();

	return (mixer);
}
SWORD mixer_original_Namco_N163(SWORD mixer) {
	BYTE i;
	SWORD a = 0;

	for (i = n163.snd_ch_start; i < 8; i++) {
		if (n163.ch[i].active) {
			a += ((n163.ch[i].output << 1) * (n163.ch[i].volume >> 2));
		}
	}

	mixer += (extra_out(a) * ch_gain_ptnd(APU_EXTRA));

	mixer_cut_and_high();

	return (mixer);
}
SWORD mixer_original_Sunsoft_FM7(SWORD mixer) {
	mixer += ((SWORD) (extra_out(fm7.square[0].output) * ch_gain_ptnd(APU_EXTRA)) << 3);
	mixer += ((SWORD) (extra_out(fm7.square[1].output) * ch_gain_ptnd(APU_EXTRA)) << 3);
	mixer += ((SWORD) (extra_out(fm7.square[2].output) * ch_gain_ptnd(APU_EXTRA)) << 3);

	mixer_cut_and_high();

	return (mixer);
}
SWORD mixer_original_VRC6(SWORD mixer) {
	mixer += ((SWORD) (extra_out(vrc6.S3.output) * ch_gain_ptnd(APU_EXTRA)) << 2);
	mixer += ((SWORD) (extra_out(vrc6.S4.output) * ch_gain_ptnd(APU_EXTRA)) << 2);
	mixer += ((SWORD) (extra_out((vrc6.saw.output / 5)) * ch_gain_ptnd(APU_EXTRA)) << 2);

	mixer_cut_and_high();

	return (mixer);
}
SWORD mixer_original_VRC7(SWORD mixer) {
	mixer_cut_and_high();

	mixer += (extra_out((opll_calc() << 2)) * ch_gain_ptnd(APU_EXTRA));
	return (mixer);
}
