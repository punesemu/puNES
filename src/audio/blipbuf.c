/*
 * blipbuf.c
 *
 *  Created on: 28/lug/2012
 *      Author: fhorse
 */

#include <string.h>
#include "apu.h"
#include "snd.h"
#include "mappers.h"
#include "mappers/mapper_VRC7_snd.h"
#include "fds.h"
#include "conf.h"
#include "clock.h"
#include "fps.h"
#include "audio/quality.h"
#include "audio/channels.h"
#include "audio/blip_buf.h"
#include "audio/blipbuf.h"
#include "info.h"

enum blbuf_misc { master_vol = 65536 / 15 };

#define _ch_gain(index, f) (master_vol * ((double) (f * cfg->apu.volume[index])) / 100)
#define ch_gain_ptnd(index) _ch_gain(index, 1.0f)
#define ch_gain_ext(out, f) (extra_out(out) * _ch_gain(APU_EXTRA, f))

#define _update_tick_blbuf(type, restart)\
	blipbuf.delta = blipbuf.output - blipbuf.type.amp;\
	blipbuf.type.time += blipbuf.type.period;\
	blipbuf.type.amp += blipbuf.delta;\
	blip_add_delta(blipbuf.wave, blipbuf.type.time, blipbuf.delta);\
	blipbuf.type.period = restart
#define update_tick_ptnd_blbuf(restart) _update_tick_blbuf(ptnd, restart)
#define update_tick_extra_blbuf(restart) _update_tick_blbuf(extra, restart)

typedef struct {
	int time; /* clock time of next delta */
	int amp; /* current amplitude in delta buffer */
	int period;
	int min_period;
} _blipbuf_group;

static struct _blipbuf {
	blip_buffer_t *wave;

	_blipbuf_group ptnd;
	_blipbuf_group extra;

	DBWORD counter;

	SWORD output;
	int delta;
} blipbuf;

void (*extra_apu_tick_blipbuf)(void);
void apu_tick_blipbuf_FDS(void);
void apu_tick_blipbuf_MMC5(void);
void apu_tick_blipbuf_Namco_N163(void);
void apu_tick_blipbuf_Sunsoft_FM7(void);
void apu_tick_blipbuf_VRC6(void);
void apu_tick_blipbuf_VRC7(void);

BYTE audio_quality_init_blipbuf(void) {
	memset(&blipbuf, 0, sizeof(blipbuf));

	audio_quality_quit = audio_quality_quit_blipbuf;

	snd_apu_tick = audio_quality_apu_tick_blipbuf;
	snd_end_frame = audio_quality_end_frame_blipbuf;

	init_nla_table(502, 522)

	blipbuf.wave = blip_new(snd.samplerate / 10);

	if (blipbuf.wave == NULL) {
		/* out of memory */
		return (EXIT_ERROR);
	}

	blip_set_rates(blipbuf.wave, machine.cpu_hz, snd.samplerate);

	switch (info.mapper.id) {
		case FDS_MAPPER:
			/* FDS */
			extra_apu_tick_blipbuf = apu_tick_blipbuf_FDS;
			break;
		case 5:
			/* MMC5 */
			extra_apu_tick_blipbuf = apu_tick_blipbuf_MMC5;
			break;
		case 19:
			/* Namcot N163 */
			extra_apu_tick_blipbuf = apu_tick_blipbuf_Namco_N163;
			blipbuf.extra.min_period = snd.frequency;
			break;
		case 69:
			/* Sunsoft FM7 */
			extra_apu_tick_blipbuf = apu_tick_blipbuf_Sunsoft_FM7;
			break;
		case 24:
		case 26:
			/* VRC6 */
			extra_apu_tick_blipbuf = apu_tick_blipbuf_VRC6;
			break;
		case 85:
			/* VRC7 */
			extra_apu_tick_blipbuf = apu_tick_blipbuf_VRC7;
			blipbuf.extra.min_period = snd.frequency;
			break;
		default:
			extra_apu_tick_blipbuf = NULL;
			break;
	}

	return (EXIT_OK);
}
void audio_quality_quit_blipbuf(void) {
	if (blipbuf.wave) {
		blip_delete(blipbuf.wave);
		blipbuf.wave = NULL;
	}
}
void audio_quality_apu_tick_blipbuf(void) {
	if (!blipbuf.wave) {
		return;
	}

	if (S1.clocked | S2.clocked | TR.clocked | NS.clocked | DMC.clocked ) {
		S1.clocked = S2.clocked = TR.clocked = NS.clocked = DMC.clocked = FALSE;
		blipbuf.output = pulse_output() + tnd_output();
		update_tick_ptnd_blbuf(1);
	} else {
		blipbuf.ptnd.period++;
	}

	if (extra_apu_tick_blipbuf) {
		extra_apu_tick_blipbuf();
	}

	blipbuf.counter++;
}
void audio_quality_end_frame_blipbuf(void) {
	if (!blipbuf.wave) {
		return;
	}

	blipbuf.ptnd.time -= blipbuf.counter;

	// se esiste un canale extra allora...
	if (extra_apu_tick_blipbuf) {
		blipbuf.extra.time -= blipbuf.counter;
	}

	blip_end_frame(blipbuf.wave, blipbuf.counter);
	blipbuf.counter = 0;

	{
		int i, count = blip_samples_avail(blipbuf.wave);
		short sample[count];

		blip_read_samples(blipbuf.wave, sample, count, 0);

		if (snd_handler() == EXIT_ERROR) {
			return;
		}

		snd_lock_cache(SNDCACHE);

		for (i = 0; i < count; i++) {
			SWORD data = (sample[i] * apu_pre_amp) * cfg->apu.volume[APU_MASTER];

			audio_channels_tick(data);

			if (SNDCACHE->write == (SWORD *) SNDCACHE->end) {
				SNDCACHE->write = SNDCACHE->start;
			}
		}

		if ((SNDCACHE->samples_available > snd.samples)) {
			snd.buffer.start = TRUE;
		}

		snd_unlock_cache(SNDCACHE);
	}
}

/* --------------------------------------------------------------------------------------- */
/*                                    Extra APU Tick                                       */
/* --------------------------------------------------------------------------------------- */
void apu_tick_blipbuf_FDS(void) {
	if (fds.snd.wave.clocked) {
		fds.snd.wave.clocked = FALSE;
		blipbuf.output = extra_out(fds.snd.main.output) * (1.0f * cfg->apu.volume[APU_EXTRA]);
		update_tick_extra_blbuf(1);
	} else {
		blipbuf.extra.period++;
	}
}
void apu_tick_blipbuf_MMC5(void) {
	if (mmc5.S3.clocked | mmc5.S4.clocked | mmc5.pcm.clocked) {
		mmc5.S3.clocked = mmc5.S4.clocked = mmc5.pcm.clocked = FALSE;
		blipbuf.output = ch_gain_ext(mmc5.S3.output, 10.0f) + ch_gain_ext(mmc5.S4.output, 10.0f) +
				ch_gain_ext(mmc5.pcm.output, 2.0f);
		update_tick_extra_blbuf(1);
	} else {
		blipbuf.extra.period++;
	}
}
void apu_tick_blipbuf_Namco_N163(void) {
	BYTE i;

	blipbuf.output = 0;

	if (++blipbuf.extra.period == blipbuf.extra.min_period) {
		for (i = n163.snd_ch_start; i < 8; i++) {
			if (n163.ch[i].active) {
				blipbuf.output += ((n163.ch[i].output * 1.5) * (n163.ch[i].volume >> 2));
			}
		}
		blipbuf.output = ch_gain_ext(blipbuf.output, 2.0f);
		update_tick_extra_blbuf(0);
	}
}
void apu_tick_blipbuf_Sunsoft_FM7(void) {
	if (fm7.square[0].clocked | fm7.square[1].clocked | fm7.square[2].clocked) {
		fm7.square[0].clocked = fm7.square[1].clocked = fm7.square[2].clocked = FALSE;
		blipbuf.output = ch_gain_ext(fm7.square[0].output, 5.0f) +
				ch_gain_ext(fm7.square[1].output, 5.0f) + ch_gain_ext(fm7.square[2].output, 5.0f);
		update_tick_extra_blbuf(1);
	} else {
		blipbuf.extra.period++;
	}
}
void apu_tick_blipbuf_VRC6(void) {
	if (vrc6.S3.clocked | vrc6.S4.clocked | vrc6.saw.clocked) {
		vrc6.S3.clocked = vrc6.S4.clocked = vrc6.saw.clocked = FALSE;
		blipbuf.output = ch_gain_ext(vrc6.S3.output, 5.0f) + ch_gain_ext(vrc6.S4.output, 5.0f) +
				ch_gain_ext(vrc6.saw.output, 0.7f);
		update_tick_extra_blbuf(1);
	} else {
		blipbuf.extra.period++;
	}
}
void apu_tick_blipbuf_VRC7(void) {
	if (++blipbuf.extra.period == blipbuf.extra.min_period) {
		blipbuf.output = extra_out(opll_calc()) * (5.0f * cfg->apu.volume[APU_EXTRA]);
		update_tick_extra_blbuf(0);
	}
}
