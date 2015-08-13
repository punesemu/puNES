/*
 * blipbuf.c
 *
 *  Created on: 28/lug/2012
 *      Author: fhorse
 */

#include <string.h>
#include "audio_quality.h"
#include "apu.h"
#include "snd.h"
#include "mappers.h"
#include "mappers/mapper_VRC7_snd.h"
#include "fds.h"
#include "blipbuf.h"
#include "conf.h"
#include "clock.h"
#include "fps.h"
#include "blip_buf.h"
#include "info.h"

enum blbuf_misc { master_vol = 65536 / 15 , volume_fator = 1 };
enum blbuf_extern { BLBUF_EXT0, BLBUF_EXT1, BLBUF_EXT2 };

//#define pulse_output() nla_table.pulse[S1.output + S2.output]
//#define tnd_output() nla_table.tnd[(3 * TR.output) + (2 * NS.output) + DMC.output]
#define pulse_output() nla_table.pulse[s1_out + s2_out]
#define tnd_output() nla_table.tnd[(tr_out * 3) + (ns_out * 2) + dmc_out]

#define update_amp_blbuf(grp, new_amp)\
	blipbuf.delta = new_amp * blipbuf.grp.gain - blipbuf.grp.amp;\
	blipbuf.grp.amp += blipbuf.delta;\
	blip_add_delta(blipbuf.wave, blipbuf.grp.time, blipbuf.delta);
#define _update_group_blbuf(grp, restart, out)\
	blipbuf.output = out;\
	blipbuf.grp.time += blipbuf.grp.period;\
	update_amp_blbuf(grp, blipbuf.output)\
	blipbuf.grp.period = restart;

#define update_tick_extra_blbuf(ch, blch, out)\
	if (ch.clocked) {\
		ch.clocked = FALSE;\
		_update_group_blbuf(extra[blch], 1, out)\
	} else {\
		blipbuf.extra[blch].period++;\
	}

typedef struct blipbuf_group _blipbuf_group;

struct blipbuf_group {
	int gain; /* overall volume of channel */
	int time; /* clock time of next delta */
	int amp; /* current amplitude in delta buffer */
	int period;
	int min_period;
};
struct _blipbuf {
	DBWORD counter;
	blip_buffer_t *wave;
	_blipbuf_group ptnd;
	_blipbuf_group extra[3];

	BYTE min;
	BYTE max;

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

void (*extra_end_frame_blipbuf)(void);
void end_frame_blipbuf_FDS(void);
void end_frame_blipbuf_MMC5(void);
void end_frame_blipbuf_Namco_N163(void);
void end_frame_blipbuf_Sunsoft_FM7(void);
void end_frame_blipbuf_VRC6(void);
void end_frame_blipbuf_VRC7(void);

BYTE audio_quality_init_blipbuf(void) {
	memset(&blipbuf, 0, sizeof(blipbuf));

	audio_quality_quit = audio_quality_quit_blipbuf;

	snd_apu_tick = audio_quality_apu_tick_blipbuf;
	snd_end_frame = audio_quality_end_frame_blipbuf;

	init_nla_table(502, 522)

	blipbuf.max = ((snd.buffer.count >> 1) + 1);
	blipbuf.min = (((snd.buffer.count >> 1) + 1) < 3 ? 3 : ((snd.buffer.count >> 1) + 1));

	blipbuf.wave = blip_new(snd.samplerate / 10);

	if (blipbuf.wave == NULL) {
		/* out of memory */
		return (EXIT_ERROR);
	}

	blip_set_rates(blipbuf.wave, machine.cpu_hz, snd.samplerate);

	blipbuf.ptnd.gain = master_vol * (1.2 * volume_fator) / 100;

	switch (info.mapper.id) {
		case FDS_MAPPER:
			/* FDS */
			extra_apu_tick_blipbuf = apu_tick_blipbuf_FDS;
			extra_end_frame_blipbuf = end_frame_blipbuf_FDS;

			blipbuf.extra[BLBUF_EXT0].gain = 1;
			break;
		case 5:
			/* MMC5 */
			extra_apu_tick_blipbuf = apu_tick_blipbuf_MMC5;
			extra_end_frame_blipbuf = end_frame_blipbuf_MMC5;

			blipbuf.extra[BLBUF_EXT0].gain = master_vol * (10.0 * volume_fator) / 100;
			blipbuf.extra[BLBUF_EXT1].gain = master_vol * (10.0 * volume_fator) / 100;
			blipbuf.extra[BLBUF_EXT2].gain = master_vol * (2.0 * volume_fator) / 100;
			break;
		case 19:
			/* Namcot N163 */
			extra_apu_tick_blipbuf = apu_tick_blipbuf_Namco_N163;
			extra_end_frame_blipbuf = end_frame_blipbuf_Namco_N163;

			blipbuf.extra[BLBUF_EXT0].gain = master_vol * (2.0 * volume_fator) / 100;

			blipbuf.extra[BLBUF_EXT0].min_period = snd.frequency;
			break;
		case 69:
			/* Sunsoft FM7 */
			extra_apu_tick_blipbuf = apu_tick_blipbuf_Sunsoft_FM7;
			extra_end_frame_blipbuf = end_frame_blipbuf_Sunsoft_FM7;

			blipbuf.extra[BLBUF_EXT0].gain = master_vol * (5.0 * volume_fator) / 100;
			blipbuf.extra[BLBUF_EXT1].gain = master_vol * (5.0 * volume_fator) / 100;
			blipbuf.extra[BLBUF_EXT2].gain = master_vol * (5.0 * volume_fator) / 100;
			break;
		case 24:
		case 26:
			/* VRC6 */
			extra_apu_tick_blipbuf = apu_tick_blipbuf_VRC6;
			extra_end_frame_blipbuf = end_frame_blipbuf_VRC6;

			blipbuf.extra[BLBUF_EXT0].gain = master_vol * (5.0 * volume_fator) / 100;
			blipbuf.extra[BLBUF_EXT1].gain = master_vol * (5.0 * volume_fator) / 100;
			blipbuf.extra[BLBUF_EXT2].gain = master_vol * (0.7 * volume_fator) / 100;
			break;
		case 85:
			/* VRC7 */
			extra_apu_tick_blipbuf = apu_tick_blipbuf_VRC7;
			extra_end_frame_blipbuf = end_frame_blipbuf_VRC7;

			blipbuf.extra[BLBUF_EXT0].gain = 5;

			blipbuf.extra[BLBUF_EXT0].min_period = snd.frequency;
			break;
		default:
			extra_apu_tick_blipbuf = NULL;
			extra_end_frame_blipbuf = NULL;
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
		blipbuf.delta = blipbuf.output * blipbuf.ptnd.gain - blipbuf.ptnd.amp;
		blipbuf.ptnd.time += blipbuf.ptnd.period;
		blipbuf.ptnd.amp += blipbuf.delta;
		blip_add_delta(blipbuf.wave, blipbuf.ptnd.time, blipbuf.delta);
		blipbuf.ptnd.period = 1;
	} else {
		blipbuf.ptnd.period++;
	}

	if (extra_apu_tick_blipbuf) {
		extra_apu_tick_blipbuf();
	}

	blipbuf.counter++;
}
void audio_quality_end_frame_blipbuf(void) {
	_callback_data *cache = (_callback_data *) snd.cache;

	if (!blipbuf.wave) {
		return;
	}

	blipbuf.ptnd.time -= blipbuf.counter;

	if (extra_end_frame_blipbuf) {
		extra_end_frame_blipbuf();
	}

	blip_end_frame(blipbuf.wave, blipbuf.counter);
	blipbuf.counter = 0;

	{
		int i, count = blip_samples_avail(blipbuf.wave);
		short sample[count];

		blip_read_samples(blipbuf.wave, sample, count, 0);

		if (snd.brk) {
			if (cache->filled < 3) {
				snd.brk = FALSE;
			} else {
				return;
			}
		}

		snd_lock_cache(cache);

		for (i = 0; i < count; i++) {
			SWORD data = (sample[i] * apu_pre_amp) * cfg->apu.volume[APU_MASTER];

			/* mono o canale sinistro */
			(*cache->write++) = data;
			/* incremento il contatore dei bytes disponibili */
			cache->bytes_available += sizeof(*cache->write);

			/* stereo */
			if (cfg->channels == STEREO) {
				/* salvo il dato nel buffer del canale sinistro */
				snd.channel.ptr[CH_LEFT][snd.channel.pos] = data;
				/* scrivo nel nel frame audio il canale destro ritardato di un frame */
				(*cache->write++) = snd.channel.ptr[CH_RIGHT][snd.channel.pos];
				/* incremento il contatore dei bytes disponibili */
				cache->bytes_available += sizeof(*cache->write);

				/* swappo i buffers dei canali */
				if (++snd.channel.pos >= snd.channel.max_pos) {
					SWORD *swap = snd.channel.ptr[CH_RIGHT];

					snd.channel.ptr[CH_RIGHT] = snd.channel.ptr[CH_LEFT];
					snd.channel.ptr[CH_LEFT] = swap;
					snd.channel.pos = 0;
				}

				(*snd.channel.bck.write++) = data;

				if (snd.channel.bck.write >= (SWORD *) snd.channel.bck.end) {
					snd.channel.bck.write = snd.channel.bck.start;
				}
			}

			if (cache->write >= (SWORD *) cache->end) {
				cache->write = cache->start;
			}

			if (++snd.pos.current >= snd.samples) {
				snd.pos.current = 0;

				/* incremento il contatore dei frames pieni non ancora 'riprodotti' */
				if (++cache->filled >= (SDBWORD) snd.buffer.count) {
					snd.brk = TRUE;
				} else if (cache->filled == 1) {
					snd_frequency(snd_factor[apu.type][SND_FACTOR_SPEED])
				} else if (cache->filled >= blipbuf.max) {
					snd_frequency(snd_factor[apu.type][SND_FACTOR_SLOW])
				} else if (cache->filled < blipbuf.min) {
					snd_frequency(snd_factor[apu.type][SND_FACTOR_NORMAL])
				}
			}
		}

		snd_unlock_cache(cache);
	}
}

/* --------------------------------------------------------------------------------------- */
/*                                    Extra APU Tick                                       */
/* --------------------------------------------------------------------------------------- */
void apu_tick_blipbuf_FDS(void) {
	if (fds.snd.wave.clocked) {
		fds.snd.wave.clocked = FALSE;
		blipbuf.extra[BLBUF_EXT0].time += blipbuf.extra[BLBUF_EXT0].period;
		update_amp_blbuf(extra[BLBUF_EXT0], extra_out(fds.snd.main.output))
		blipbuf.extra[BLBUF_EXT0].period = 1;
	} else {
		blipbuf.extra[BLBUF_EXT0].period++;
	}
}
void apu_tick_blipbuf_MMC5(void) {
	update_tick_extra_blbuf(mmc5.S3, BLBUF_EXT0, extra_out(mmc5.S3.output))
	update_tick_extra_blbuf(mmc5.S4, BLBUF_EXT1, extra_out(mmc5.S4.output))
	update_tick_extra_blbuf(mmc5.pcm, BLBUF_EXT2, extra_out(mmc5.pcm.output))
}
void apu_tick_blipbuf_Namco_N163(void) {
	BYTE i;

	blipbuf.output = 0;
	if (++blipbuf.extra[BLBUF_EXT0].period == blipbuf.extra[BLBUF_EXT0].min_period) {
		for (i = n163.snd_ch_start; i < 8; i++) {
			if (n163.ch[i].active) {
				blipbuf.output += ((n163.ch[i].output * 1.5) * (n163.ch[i].volume >> 2));
			}
		}
		blipbuf.extra[BLBUF_EXT0].time += blipbuf.extra[BLBUF_EXT0].period;
		update_amp_blbuf(extra[BLBUF_EXT0], extra_out(blipbuf.output))
		blipbuf.extra[BLBUF_EXT0].period = 0;
	}
}
void apu_tick_blipbuf_Sunsoft_FM7(void) {
	update_tick_extra_blbuf(fm7.square[0], BLBUF_EXT0, extra_out(extra_out(fm7.square[0].output)))
	update_tick_extra_blbuf(fm7.square[1], BLBUF_EXT1, extra_out(extra_out(fm7.square[1].output)))
	update_tick_extra_blbuf(fm7.square[2], BLBUF_EXT2, extra_out(extra_out(fm7.square[2].output)))
}
void apu_tick_blipbuf_VRC6(void) {
	update_tick_extra_blbuf(vrc6.S3, BLBUF_EXT0, extra_out(vrc6.S3.output))
	update_tick_extra_blbuf(vrc6.S4, BLBUF_EXT1, extra_out(vrc6.S4.output))
	update_tick_extra_blbuf(vrc6.saw, BLBUF_EXT2, extra_out(vrc6.saw.output))
}
void apu_tick_blipbuf_VRC7(void) {
	if (++blipbuf.extra[BLBUF_EXT0].period == blipbuf.extra[BLBUF_EXT0].min_period) {
		blipbuf.extra[BLBUF_EXT0].time += blipbuf.extra[BLBUF_EXT0].period;
		update_amp_blbuf(extra[BLBUF_EXT0], extra_out(opll_calc()))
		blipbuf.extra[BLBUF_EXT0].period = 0;
	}
}
/* --------------------------------------------------------------------------------------- */
/*                                   Extra End Frame                                       */
/* --------------------------------------------------------------------------------------- */
void end_frame_blipbuf_FDS(void) {
	blipbuf.extra[BLBUF_EXT0].time -= blipbuf.counter;
}
void end_frame_blipbuf_MMC5(void) {
	blipbuf.extra[BLBUF_EXT0].time -= blipbuf.counter;
	blipbuf.extra[BLBUF_EXT1].time -= blipbuf.counter;
	blipbuf.extra[BLBUF_EXT2].time -= blipbuf.counter;
}
void end_frame_blipbuf_Namco_N163(void) {
	blipbuf.extra[BLBUF_EXT0].time -= blipbuf.counter;
}
void end_frame_blipbuf_Sunsoft_FM7(void) {
	blipbuf.extra[BLBUF_EXT0].time -= blipbuf.counter;
	blipbuf.extra[BLBUF_EXT1].time -= blipbuf.counter;
	blipbuf.extra[BLBUF_EXT2].time -= blipbuf.counter;
}
void end_frame_blipbuf_VRC6(void) {
	blipbuf.extra[BLBUF_EXT0].time -= blipbuf.counter;
	blipbuf.extra[BLBUF_EXT1].time -= blipbuf.counter;
	blipbuf.extra[BLBUF_EXT2].time -= blipbuf.counter;
}
void end_frame_blipbuf_VRC7(void) {
	blipbuf.extra[BLBUF_EXT0].time -= blipbuf.counter;
}
