/*
 *  Copyright (C) 2010-2022 Fabio Cavallo (aka FHorse)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <stdlib.h>
#include <string.h>
#include "audio/snd.h"
#include "mappers.h"
#include "mappers/mapper_VRC7_snd.h"
#include "fds.h"
#include "nsf.h"
#include "conf.h"
#include "clock.h"
#include "fps.h"
#include "audio/channels.h"
#include "audio/blip_buf.h"
#include "audio/blipbuf.h"
#include "info.h"
#include "rewind.h"

enum blbuf_misc { master_vol = 65536 / 15 };

#define _ch_gain(index, f) ((double)(f * cfg->apu.volume[index]))
#define ch_gain_ptnd(index) _ch_gain(index, 1.0f)
#define _ch_gain_ext(f) (master_vol * ((double)(f * cfg->apu.volume[APU_EXTRA])) / 100)
#define ch_gain_ext(out, f) (extra_out(out) * _ch_gain_ext(f))

#define _update_tick_blbuf(type, restart)\
	blipbuf.delta = blipbuf.output - blipbuf.type.amp;\
	blipbuf.type.time += blipbuf.type.period;\
	blipbuf.type.amp += blipbuf.delta;\
	blip_add_delta(blipbuf.wave, blipbuf.type.time, blipbuf.delta);\
	blipbuf.type.period = restart
#define update_tick_ptnd_blbuf(restart) _update_tick_blbuf(ptnd, restart)
#define update_tick_extra_blbuf(extra, restart) _update_tick_blbuf(extra, restart)

typedef struct {
	int time; /* clock time of next delta */
	int amp; /* current amplitude in delta buffer */
	int period;
	int min_period;
} _blipbuf_group;

static struct _blipbuf {
	blip_buffer_t *wave;

	_blipbuf_group ptnd;
	_blipbuf_group fds;
	_blipbuf_group mmc5;
	_blipbuf_group n163;
	_blipbuf_group fm7;
	_blipbuf_group vrc6;
	_blipbuf_group vrc7;

	struct _blipbuf_samples {
		int count;
		SWORD *data;
	} samples;

	DBWORD counter;

	SWORD output;
	int delta;
} blipbuf;

static void (*extra_apu_tick_blipbuf)(void);
static void apu_tick_blipbuf_NSF(void);
static void apu_tick_blipbuf_FDS(void);
static void apu_tick_blipbuf_MMC5(void);
static void apu_tick_blipbuf_Namco_N163(void);
static void apu_tick_blipbuf_Sunsoft_FM7(void);
static void apu_tick_blipbuf_VRC6(void);
static void apu_tick_blipbuf_VRC7(void);
static void (*extra_audio_end_frame_blipbuf)(void);
static void extra_audio_end_frame_blipbuf_NSF(void);
static void extra_audio_end_frame_blipbuf_FDS(void);
static void extra_audio_end_frame_blipbuf_MMC5(void);
static void extra_audio_end_frame_blipbuf_Namco_N163(void);
static void extra_audio_end_frame_blipbuf_Sunsoft_FM7(void);
static void extra_audio_end_frame_blipbuf_VRC6(void);
static void extra_audio_end_frame_blipbuf_VRC7(void);

BYTE audio_init_blipbuf(void) {
	memset(&blipbuf, 0, sizeof(blipbuf));

	snd_apu_tick = audio_apu_tick_blipbuf;
	snd_end_frame = audio_end_frame_blipbuf;

	init_nla_table(500, 500)

	if (!(blipbuf.samples.data = (SWORD *)malloc(snd.samplerate))) {
		return (EXIT_ERROR);
	}

	blipbuf.wave = blip_new(snd.samplerate / 10);

	if (blipbuf.wave == NULL) {
		free(blipbuf.samples.data);
		blipbuf.samples.data = NULL;
		return (EXIT_ERROR);
	}

	blip_set_rates(blipbuf.wave, machine.cpu_hz, snd.samplerate);

	switch (info.mapper.id) {
		case NSF_MAPPER:
			extra_apu_tick_blipbuf = apu_tick_blipbuf_NSF;
			extra_audio_end_frame_blipbuf = extra_audio_end_frame_blipbuf_NSF;
			if (nsf.sound_chips.vrc7) {
				blipbuf.vrc7.min_period = snd.frequency;
			}
			if (nsf.sound_chips.namco163) {
				blipbuf.n163.min_period = snd.frequency;
			}
			break;
		case FDS_MAPPER:
			/* FDS */
			extra_apu_tick_blipbuf = apu_tick_blipbuf_FDS;
			extra_audio_end_frame_blipbuf = extra_audio_end_frame_blipbuf_FDS;
			break;
		case 5:
			/* MMC5 */
			extra_apu_tick_blipbuf = apu_tick_blipbuf_MMC5;
			extra_audio_end_frame_blipbuf = extra_audio_end_frame_blipbuf_MMC5;
			break;
		case 19:
			/* Namcot N163 */
			extra_apu_tick_blipbuf = apu_tick_blipbuf_Namco_N163;
			extra_audio_end_frame_blipbuf = extra_audio_end_frame_blipbuf_Namco_N163;
			blipbuf.n163.min_period = snd.frequency;
			break;
		case 69:
			/* Sunsoft FM7 */
			extra_apu_tick_blipbuf = apu_tick_blipbuf_Sunsoft_FM7;
			extra_audio_end_frame_blipbuf = extra_audio_end_frame_blipbuf_Sunsoft_FM7;
			break;
		case 24:
		case 26:
			/* VRC6 */
			extra_apu_tick_blipbuf = apu_tick_blipbuf_VRC6;
			extra_audio_end_frame_blipbuf = extra_audio_end_frame_blipbuf_VRC6;
			break;
		case 85:
			/* VRC7 */
			extra_apu_tick_blipbuf = apu_tick_blipbuf_VRC7;
			extra_audio_end_frame_blipbuf = extra_audio_end_frame_blipbuf_VRC7;
			blipbuf.vrc7.min_period = snd.frequency;
			break;
		default:
			extra_apu_tick_blipbuf = NULL;
			extra_audio_end_frame_blipbuf = NULL;
			break;
	}

	return (EXIT_OK);
}
void audio_quit_blipbuf(void) {
	if (blipbuf.samples.data) {
		free(blipbuf.samples.data);
		blipbuf.samples.data = NULL;
		blipbuf.samples.count = 0;
	}
	if (blipbuf.wave) {
		blip_delete(blipbuf.wave);
		blipbuf.wave = NULL;
	}
}
void audio_reset_blipbuf(void) {
	blip_clear(blipbuf.wave);
	memset(blipbuf.samples.data, 0x00, snd.samplerate);
	blipbuf.samples.count = 0;
}
void audio_apu_tick_blipbuf(void) {
	if ((!blipbuf.wave) | fps_fast_forward_enabled() | rwnd.active) {
		return;
	}

	if (apu.clocked) {
		apu.clocked = FALSE;
		blipbuf.output = (pulse_output() + tnd_output()) * (master_vol / 100);
		update_tick_ptnd_blbuf(1);
	} else {
		blipbuf.ptnd.period++;
	}

	if (extra_apu_tick_blipbuf) {
		extra_apu_tick_blipbuf();
	}

	blipbuf.counter++;
}
void audio_end_frame_blipbuf(void) {
	if ((!blipbuf.wave) | fps_fast_forward_enabled() | rwnd.active) {
		if (snd.cache) {
			snd.cache->write = snd.cache->start;
			snd.cache->read = (SBYTE *)snd.cache->start;
			snd.cache->bytes_available = snd.cache->samples_available = 0;
		}
		snd.buffer.start = FALSE;
		return;
	}

	blipbuf.ptnd.time -= blipbuf.counter;

	// se esiste un canale extra allora...
	if (extra_audio_end_frame_blipbuf) {
		extra_audio_end_frame_blipbuf();
	}

	blip_end_frame(blipbuf.wave, blipbuf.counter);
	blipbuf.counter = 0;

	{
		int i;

		blipbuf.samples.count = blip_samples_avail(blipbuf.wave);

		blip_read_samples(blipbuf.wave, (SWORD *)blipbuf.samples.data, blipbuf.samples.count, 0);

		if (snd_handler() == EXIT_ERROR) {
			return;
		}

		snd_thread_lock();

		if (extcl_audio_samples_mod) {
			extcl_audio_samples_mod(blipbuf.samples.data, blipbuf.samples.count);
		}

		for (i = 0; i < blipbuf.samples.count; i++) {
			static SWORD data = 0;

			if (cfg->apu.channel[APU_MASTER]) {
				data = ((blipbuf.samples.data[i] * apu_pre_amp) * cfg->apu.volume[APU_MASTER]);
			}
			audio_channels_tick(data);

			if (snd.cache->write == (SWORD *)snd.cache->end) {
				snd.cache->write = snd.cache->start;
			}
		}

		if (snd.cache->samples_available >= (snd.samplerate / 10)) {
			snd.buffer.start = TRUE;
		}

		snd_thread_unlock();
	}
}
int audio_buffer_blipbuf(SWORD **buffer) {
	(*buffer) = blipbuf.samples.data;
	return (blipbuf.samples.count);
}

/* --------------------------------------------------------------------------------------- */
/*                                    Extra APU Tick                                       */
/* --------------------------------------------------------------------------------------- */
static void apu_tick_blipbuf_NSF(void) {
	if (nsf.sound_chips.vrc6) {
		apu_tick_blipbuf_VRC6();
	}
	if (nsf.sound_chips.vrc7) {
		apu_tick_blipbuf_VRC7();
	}
	if (nsf.sound_chips.fds) {
		apu_tick_blipbuf_FDS();
	}
	if (nsf.sound_chips.mmc5) {
		apu_tick_blipbuf_MMC5();
	}
	if (nsf.sound_chips.namco163) {
		apu_tick_blipbuf_Namco_N163();
	}
	if (nsf.sound_chips.sunsoft5b) {
		apu_tick_blipbuf_Sunsoft_FM7();
	}
}
static void apu_tick_blipbuf_FDS(void) {
	if (fds.snd.wave.clocked) {
		fds.snd.wave.clocked = FALSE;
		blipbuf.output = extra_out(fds.snd.main.output) * (1.0f * cfg->apu.volume[APU_EXTRA]);
		update_tick_extra_blbuf(fds, 1);
	} else {
		blipbuf.fds.period++;
	}
}
static void apu_tick_blipbuf_MMC5(void) {
	if (mmc5.clocked) {
		mmc5.clocked = FALSE;
		blipbuf.output = ch_gain_ext(mmc5.S3.output, 10.0f) + ch_gain_ext(mmc5.S4.output, 10.0f) +
			ch_gain_ext(mmc5.pcm.output, 2.0f);
		update_tick_extra_blbuf(mmc5, 1);
	} else {
		blipbuf.mmc5.period++;
	}
}
static void apu_tick_blipbuf_Namco_N163(void) {
	BYTE i;

	blipbuf.output = 0;

	if (++blipbuf.n163.period == blipbuf.n163.min_period) {
		double gain = 2.5f / (8 - n163.snd_ch_start);

		for (i = n163.snd_ch_start; i < 8; i++) {
			if (n163.ch[i].active) {
				blipbuf.output += n163.ch[i].output * gain;
			}
		}
		blipbuf.output = ch_gain_ext(blipbuf.output, 1.0f);
		update_tick_extra_blbuf(n163, 0);
	}
}
static void apu_tick_blipbuf_Sunsoft_FM7(void) {
	if (fm7.clocked) {
		fm7.clocked = FALSE;
		blipbuf.output = ch_gain_ext(fm7.square[0].output, 5.0f) +
			ch_gain_ext(fm7.square[1].output, 5.0f) + ch_gain_ext(fm7.square[2].output, 5.0f);
		update_tick_extra_blbuf(fm7, 1);
	} else {
		blipbuf.fm7.period++;
	}
}
static void apu_tick_blipbuf_VRC6(void) {
	if (vrc6.clocked) {
		vrc6.clocked = FALSE;
		blipbuf.output = ch_gain_ext(vrc6.S3.output, 5.0f) + ch_gain_ext(vrc6.S4.output, 5.0f) + ch_gain_ext(vrc6.saw.output, 0.7f);
		update_tick_extra_blbuf(vrc6, 1);
	} else {
		blipbuf.vrc6.period++;
	}
}
static void apu_tick_blipbuf_VRC7(void) {
	if (++blipbuf.vrc7.period == blipbuf.vrc7.min_period) {
		blipbuf.output = extra_out(opll_calc()) * (32.0f * cfg->apu.volume[APU_EXTRA]);
		update_tick_extra_blbuf(vrc7, 0);
	}
}

/* --------------------------------------------------------------------------------------- */
/*                            Extra Audio QUality End Frame                                */
/* --------------------------------------------------------------------------------------- */
static void extra_audio_end_frame_blipbuf_NSF(void) {
	if (nsf.sound_chips.vrc6) {
		extra_audio_end_frame_blipbuf_VRC6();
	}
	if (nsf.sound_chips.vrc7) {
		extra_audio_end_frame_blipbuf_VRC7();
	}
	if (nsf.sound_chips.fds) {
		extra_audio_end_frame_blipbuf_FDS();
	}
	if (nsf.sound_chips.mmc5) {
		extra_audio_end_frame_blipbuf_MMC5();
	}
	if (nsf.sound_chips.namco163) {
		extra_audio_end_frame_blipbuf_Namco_N163();
	}
	if (nsf.sound_chips.sunsoft5b) {
		extra_audio_end_frame_blipbuf_Sunsoft_FM7();
	}
}
static void extra_audio_end_frame_blipbuf_FDS(void) {
	blipbuf.fds.time -= blipbuf.counter;
}
static void extra_audio_end_frame_blipbuf_MMC5(void) {
	blipbuf.mmc5.time -= blipbuf.counter;
}
static void extra_audio_end_frame_blipbuf_Namco_N163(void) {
	blipbuf.n163.time -= blipbuf.counter;
}
static void extra_audio_end_frame_blipbuf_Sunsoft_FM7(void) {
	blipbuf.fm7.time -= blipbuf.counter;
}
static void extra_audio_end_frame_blipbuf_VRC6(void) {
	blipbuf.vrc6.time -= blipbuf.counter;
}
static void extra_audio_end_frame_blipbuf_VRC7(void) {
	blipbuf.vrc7.time -= blipbuf.counter;
}
