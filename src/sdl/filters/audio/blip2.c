/*
 * blip2.c
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
#include "blip2.h"

#include "cfg_file.h"
#include "clock.h"
#include "fps.h"
#include "blip_buf.h"

enum bl2_misc { master_vol = 65536 / 15 , volume_fator = 1, min_period = 20 };
enum bl2_group { PULSE, TND, EXTRA };
enum bl2_extern { BL2_EXT0, BL2_EXT1, BL2_EXT2 };

#define pulse_output() nla_table.pulse[S1.output + S2.output]
#define tnd_output() nla_table.tnd[(3 * TR.output) + (2 * NS.output) + DMC.output]
#define update_amp_bl2(grp, new_amp)\
{\
	int delta = new_amp * bl2.grp.gain - bl2.grp.amp;\
	bl2.grp.amp += delta;\
	blip_add_delta(bl2.wave, bl2.grp.time, delta);\
}
#define _update_group_bl2(grp, restart, out)\
{\
	SWORD output = out;\
	bl2.grp.time += bl2.grp.period;\
	update_amp_bl2(grp, output)\
	bl2.grp.period = restart;\
}
#define update_group_pulse_bl2(restart)\
	_update_group_bl2(group[PULSE], restart, pulse_output())
#define update_group_tnd_bl2(restart)\
	_update_group_bl2(group[TND], restart, tnd_output())
#define update_end_frame_group_pulse_bl2()\
{\
	update_group_pulse_bl2(0)\
	bl2.group[PULSE].time -= bl2.counter;\
}
#define update_end_frame_group_tnd_bl2()\
{\
	update_group_tnd_bl2(0)\
	bl2.group[TND].time -= bl2.counter;\
}

#define update_tick_extra_bl2(ch, blch, out)\
	if (ch.clocked && (bl2.extra[blch].period >= bl2.extra[blch].min_period)) {\
		ch.clocked = FALSE;\
		_update_group_bl2(extra[blch], 1, out)\
	} else {\
		bl2.extra[blch].period++;\
	}
#define update_end_frame_extra_bl2(blch, out)\
{\
	_update_group_bl2(extra[blch], 0, out)\
	bl2.extra[blch].time -= bl2.counter;\
}

typedef struct blip2_group _blip2_group;
typedef struct blip2_chan _blip2_chan;
typedef struct bl2 _bl2;

struct blip2_group {
	int gain; /* overall volume of channel */
	int time; /* clock time of next delta */
	int phase; /* position within waveform */
	int amp; /* current amplitude in delta buffer */
	int period;
	int min_period;
};
struct blip2_chan {
	int period;
	int min_period;
};
struct _bl2 {
	DBWORD counter;
	blip_buffer_t *wave;
	_blip2_group group[2];
	_blip2_group extra[3];
	_blip2_chan ch[5];

	BYTE min;
	BYTE max;
} bl2;

void (*extra_apu_tick_blip2)(void);
void apu_tick_blip2_FDS(void);
void apu_tick_blip2_MMC5(void);
void apu_tick_blip2_Namco_N163(void);
void apu_tick_blip2_Sunsoft_FM7(void);
void apu_tick_blip2_VRC6(void);
void apu_tick_blip2_VRC7(void);

void (*extra_end_frame_blip2)(void);
void end_frame_blip2_FDS(void);
void end_frame_blip2_MMC5(void);
void end_frame_blip2_Namco_N163(void);
void end_frame_blip2_Sunsoft_FM7(void);
void end_frame_blip2_VRC6(void);
void end_frame_blip2_VRC7(void);

BYTE audio_quality_init_blip2(void) {
	memset(&bl2, 0, sizeof(bl2));

	audio_quality_quit = audio_quality_quit_blip2;

	snd_apu_tick = audio_quality_apu_tick_blip2;
	snd_end_frame = audio_quality_end_frame_blip2;

	init_nla_table(502, 522)

	bl2.max = ((snd.buffer.count >> 1) + 1);
	bl2.min = (((snd.buffer.count >> 1) + 1) < 3 ? 3 : ((snd.buffer.count >> 1) + 1));

	{
		SDL_AudioSpec *dev = snd.dev;

		bl2.wave = blip_new(dev->freq / 10);

		if (bl2.wave == NULL) {
			 /* out of memory */
			return (EXIT_ERROR);
		}

		blip_set_rates(bl2.wave, machine.cpu_hz, dev->freq);

		bl2.group[PULSE].gain = master_vol * (1.2 * volume_fator) / 100;
		bl2.group[TND].gain = master_vol * (1.2 * volume_fator) / 100;

		bl2.ch[APU_S1].min_period  = min_period;
		bl2.ch[APU_S2].min_period  = min_period;
		bl2.ch[APU_TR].min_period  = min_period; // / 2.5;
		bl2.ch[APU_NS].min_period  = min_period; // / 2;
		bl2.ch[APU_DMC].min_period = min_period;
	}

	switch (info.mapper) {
		case FDS_MAPPER:
			/* FDS */
			extra_apu_tick_blip2 = apu_tick_blip2_FDS;
			extra_end_frame_blip2 = end_frame_blip2_FDS;

			bl2.extra[BL2_EXT0].gain = 1;

			bl2.extra[BL2_EXT0].min_period = min_period;
			break;
		case 5:
			/* MMC5 */
			extra_apu_tick_blip2 = apu_tick_blip2_MMC5;
			extra_end_frame_blip2 = end_frame_blip2_MMC5;

			bl2.extra[BL2_EXT0].gain = master_vol * (10.0 * volume_fator) / 100;
			bl2.extra[BL2_EXT1].gain = master_vol * (10.0 * volume_fator) / 100;
			bl2.extra[BL2_EXT2].gain = master_vol * (2.0 * volume_fator) / 100;

			bl2.extra[BL2_EXT0].min_period = min_period;
			bl2.extra[BL2_EXT1].min_period = min_period;
			bl2.extra[BL2_EXT2].min_period = min_period;
			break;
		case 19:
			/* Namcot N163 */
			extra_apu_tick_blip2 = apu_tick_blip2_Namco_N163;
			extra_end_frame_blip2 = end_frame_blip2_Namco_N163;

			bl2.extra[BL2_EXT0].gain = master_vol * (2.0 * volume_fator) / 100;

			bl2.extra[BL2_EXT0].min_period = snd.frequency;
			break;
		case 69:
			/* Sunsoft FM7 */
			extra_apu_tick_blip2 = apu_tick_blip2_Sunsoft_FM7;
			extra_end_frame_blip2 = end_frame_blip2_Sunsoft_FM7;

			bl2.extra[BL2_EXT0].gain = master_vol * (5.0 * volume_fator) / 100;
			bl2.extra[BL2_EXT1].gain = master_vol * (5.0 * volume_fator) / 100;
			bl2.extra[BL2_EXT2].gain = master_vol * (5.0 * volume_fator) / 100;

			bl2.extra[BL2_EXT0].min_period = min_period;
			bl2.extra[BL2_EXT1].min_period = min_period;
			bl2.extra[BL2_EXT2].min_period = min_period;
			break;
		case 24:
		case 26:
			/* VRC6 */
			extra_apu_tick_blip2 = apu_tick_blip2_VRC6;
			extra_end_frame_blip2 = end_frame_blip2_VRC6;

			bl2.extra[BL2_EXT0].gain = master_vol * (5.0 * volume_fator) / 100;
			bl2.extra[BL2_EXT1].gain = master_vol * (5.0 * volume_fator) / 100;
			bl2.extra[BL2_EXT2].gain = master_vol * (0.7 * volume_fator) / 100;

			bl2.extra[BL2_EXT0].min_period = min_period;
			bl2.extra[BL2_EXT1].min_period = min_period;
			bl2.extra[BL2_EXT2].min_period = min_period;
			break;
		case 85:
			/* VRC7 */
			extra_apu_tick_blip2 = apu_tick_blip2_VRC7;
			extra_end_frame_blip2 = end_frame_blip2_VRC7;

			bl2.extra[BL2_EXT0].gain = 5;

			bl2.extra[BL2_EXT0].min_period = snd.frequency;
			break;
		default:
			extra_apu_tick_blip2 = NULL;
			extra_end_frame_blip2 = NULL;
			break;
	}

	return (EXIT_OK);
}
void audio_quality_quit_blip2(void) {
	if (bl2.wave) {
		blip_delete(bl2.wave);
		bl2.wave = NULL;
	}
}
void audio_quality_apu_tick_blip2(void) {
	if (!bl2.wave) {
		return;
	}

	if (S1.clocked && (bl2.ch[APU_S1].period >= bl2.ch[APU_S1].min_period)) {
		S1.clocked = FALSE;
		update_group_pulse_bl2(1)
		bl2.ch[APU_S1].period = 1;
		bl2.ch[APU_S2].period++;
	} else if (S2.clocked && (bl2.ch[APU_S2].period >= bl2.ch[APU_S2].min_period)) {
		S2.clocked = FALSE;
		update_group_pulse_bl2(1)
		bl2.ch[APU_S1].period++;
		bl2.ch[APU_S2].period = 1;
	} else {
		bl2.ch[APU_S1].period++;
		bl2.ch[APU_S2].period++;
		bl2.group[PULSE].period++;
	}

	if (TR.clocked && (bl2.ch[APU_TR].period >= bl2.ch[APU_TR].min_period)) {
		TR.clocked = FALSE;
		update_group_tnd_bl2(1)
		bl2.ch[APU_TR].period = 1;
		bl2.ch[APU_NS].period++;
		bl2.ch[APU_DMC].period++;
	} else if (NS.clocked && (bl2.ch[APU_NS].period >= bl2.ch[APU_NS].min_period)) {
		NS.clocked = FALSE;
		update_group_tnd_bl2(1)
		bl2.ch[APU_TR].period++;
		bl2.ch[APU_NS].period = 1;
		bl2.ch[APU_DMC].period++;
	} else if (DMC.clocked && (bl2.ch[APU_NS].period >= bl2.ch[APU_NS].min_period)) {
		DMC.clocked = FALSE;
		update_group_tnd_bl2(1)
		bl2.ch[APU_TR].period++;
		bl2.ch[APU_NS].period++;
		bl2.ch[APU_DMC].period = 1;
	} else {
		bl2.ch[APU_TR].period++;
		bl2.ch[APU_NS].period++;
		bl2.ch[APU_DMC].period++;
		bl2.group[TND].period++;
	}

	if (extra_apu_tick_blip2) {
		extra_apu_tick_blip2();
	}

	bl2.counter++;
}
void audio_quality_end_frame_blip2(void) {
	SDL_AudioSpec *dev = snd.dev;
	_callback_data *cache = snd.cache;

	if (!bl2.wave) {
		return;
	}

	update_end_frame_group_pulse_bl2()
	update_end_frame_group_tnd_bl2()

	if (extra_end_frame_blip2) {
		extra_end_frame_blip2();
	}

	blip_end_frame(bl2.wave, bl2.counter);
	bl2.counter = 0;

	{
		int i, count = blip_samples_avail(bl2.wave);
		short sample[count];

		blip_read_samples(bl2.wave, sample, count, 0);

		if (snd.brk) {
			if (cache->filled < 3) {
				snd.brk = FALSE;
			} else {
				return;
			}
		}

		for (i = 0; i < count; i++) {
			SWORD data = sample[i];

			/* mono or left*/
			(*cache->write++) = data;

			/* stereo */
			if (dev->channels == STEREO) {
				/* salvo il dato nel buffer del canale sinistro */
				snd.channel.ptr[CH_LEFT][snd.channel.pos] = data;
				/* scrivo nel nel frame audio il canale destro ritardato di un frame */
				(*cache->write++) = snd.channel.ptr[CH_RIGHT][snd.channel.pos];
				/* swappo i buffers dei canali */
				if (++snd.channel.pos >= snd.channel.max_pos) {
					SWORD *swap = snd.channel.ptr[CH_RIGHT];

					snd.channel.ptr[CH_RIGHT] = snd.channel.ptr[CH_LEFT];
					snd.channel.ptr[CH_LEFT] = swap;
					snd.channel.pos = 0;
				}
			}

			if (cache->write >= (SWORD *) cache->end) {
				cache->write = cache->start;
			}

			if (++snd.pos.current >= dev->samples) {
				snd.pos.current = 0;

				SDL_mutexP(cache->lock);

				/* incremento il contatore dei frames pieni non ancora 'riprodotti' */
				if (++cache->filled >= snd.buffer.count) {
					snd.brk = TRUE;
				} else if (cache->filled >= bl2.max) {
					snd_frequency(snd_factor[apu.type][SND_FACTOR_NONE])
				} else if (cache->filled < bl2.min) {
					snd_frequency(snd_factor[apu.type][SND_FACTOR_NORMAL])
				}

				SDL_mutexV(cache->lock);
			}
		}
	}
}

/* --------------------------------------------------------------------------------------- */
/*                                    Extra APU Tick                                       */
/* --------------------------------------------------------------------------------------- */
void apu_tick_blip2_FDS(void) {
	if (fds.snd.wave.clocked && (bl2.extra[BL2_EXT0].period >= bl2.extra[BL2_EXT0].min_period)) {
		fds.snd.wave.clocked = FALSE;
		bl2.extra[BL2_EXT0].time += bl2.extra[BL2_EXT0].period;
		update_amp_bl2(extra[BL2_EXT0], fds.snd.main.output)
		bl2.extra[BL2_EXT0].period = 1;
	} else {
		bl2.extra[BL2_EXT0].period++;
	}
}
void apu_tick_blip2_MMC5(void) {
	update_tick_extra_bl2(mmc5.S3, BL2_EXT0, mmc5.S3.output)
	update_tick_extra_bl2(mmc5.S4, BL2_EXT1, mmc5.S4.output)
	update_tick_extra_bl2(mmc5.pcm, BL2_EXT2, mmc5.pcm.output)
}
void apu_tick_blip2_Namco_N163(void) {
	BYTE i;
	SWORD output = 0;

	if (++bl2.extra[BL2_EXT0].period == bl2.extra[BL2_EXT0].min_period) {
		for (i = n163.snd_ch_start; i < 8; i++) {
			if (n163.ch[i].active) {
				output += ((n163.ch[i].output * 1.5) * (n163.ch[i].volume >> 2));
			}
		}
		bl2.extra[BL2_EXT0].time += bl2.extra[BL2_EXT0].period;
		update_amp_bl2(extra[BL2_EXT0], output)
		bl2.extra[BL2_EXT0].period = 0;
	}
}
void apu_tick_blip2_Sunsoft_FM7(void) {
	update_tick_extra_bl2(fm7.square[0], BL2_EXT0, fm7.square[0].output)
	update_tick_extra_bl2(fm7.square[1], BL2_EXT1, fm7.square[1].output)
	update_tick_extra_bl2(fm7.square[2], BL2_EXT2, fm7.square[2].output)
}
void apu_tick_blip2_VRC6(void) {
	update_tick_extra_bl2(vrc6.S3, BL2_EXT0, vrc6.S3.output)
	update_tick_extra_bl2(vrc6.S4, BL2_EXT1, vrc6.S4.output)
	update_tick_extra_bl2(vrc6.saw, BL2_EXT2, vrc6.saw.output)
}
void apu_tick_blip2_VRC7(void) {
	if (++bl2.extra[BL2_EXT0].period == bl2.extra[BL2_EXT0].min_period) {
		bl2.extra[BL2_EXT0].time += bl2.extra[BL2_EXT0].period;
		update_amp_bl2(extra[BL2_EXT0], opll_calc())
		bl2.extra[BL2_EXT0].period = 0;
	}
}
/* --------------------------------------------------------------------------------------- */
/*                                   Extra End Frame                                       */
/* --------------------------------------------------------------------------------------- */
void end_frame_blip2_FDS(void) {
	bl2.extra[BL2_EXT0].time += bl2.extra[BL2_EXT0].period;
	update_amp_bl2(extra[BL2_EXT0], fds.snd.main.output)
	bl2.extra[BL2_EXT0].period = 0;
	bl2.extra[BL2_EXT0].time -= bl2.counter;
}
void end_frame_blip2_MMC5(void) {
	update_end_frame_extra_bl2(BL2_EXT0, mmc5.S3.output)
	update_end_frame_extra_bl2(BL2_EXT1, mmc5.S4.output)
	update_end_frame_extra_bl2(BL2_EXT2, mmc5.pcm.output)
}
void end_frame_blip2_Namco_N163(void) {
	BYTE i;
	SWORD output = 0;

	for (i = n163.snd_ch_start; i < 8; i++) {
		if (n163.ch[i].active) {
			output += ((n163.ch[i].output * 1.5) * (n163.ch[i].volume >> 2));
		}
	}
	bl2.extra[BL2_EXT0].time += bl2.extra[BL2_EXT0].period;
	update_amp_bl2(extra[BL2_EXT0], output)
	bl2.extra[BL2_EXT0].period = 0;
	bl2.extra[BL2_EXT0].time -= bl2.counter;
}
void end_frame_blip2_Sunsoft_FM7(void) {
	update_end_frame_extra_bl2(BL2_EXT0, fm7.square[0].output)
	update_end_frame_extra_bl2(BL2_EXT1, fm7.square[1].output)
	update_end_frame_extra_bl2(BL2_EXT2, fm7.square[2].output)
}
void end_frame_blip2_VRC6(void) {
	update_end_frame_extra_bl2(BL2_EXT0, vrc6.S3.output)
	update_end_frame_extra_bl2(BL2_EXT1, vrc6.S4.output)
	update_end_frame_extra_bl2(BL2_EXT2, vrc6.saw.output)
}
void end_frame_blip2_VRC7(void) {
	bl2.extra[BL2_EXT0].time += bl2.extra[BL2_EXT0].period;
	update_amp_bl2(extra[BL2_EXT0], opll_calc())
	bl2.extra[BL2_EXT0].period = 0;
	bl2.extra[BL2_EXT0].time -= bl2.counter;
}
