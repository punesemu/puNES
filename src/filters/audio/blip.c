/*
 * blip.c
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
#include "blip.h"

#include "cfg_file.h"
#include "clock.h"
#include "fps.h"
#include "blip_buf.h"

enum bl_misc { master_vol = 65536 / 15 , volume_fator = 4, min_period = 20 };
enum bl_extern { BL_EXT0 = 5, BL_EXT1, BL_EXT2 };

#define update_amp(blch, new_amp)\
{\
	int delta = new_amp * blch.gain - blch.amp;\
	blch.amp += delta;\
	blip_add_delta(bl.blip, blch.time, delta);\
}
#define update_general_channel(ch, blch, restart, out)\
{\
	SWORD output = out;\
	blch.time += blch.period;\
	update_amp(blch, output)\
	blch.period = restart;\
}
#define update_tick_channel(ch, blch, out)\
	if (ch.clocked && (blch.period >= blch.min_period)) {\
		ch.clocked = FALSE;\
		update_general_channel(ch, blch, 1, out)\
	} else {\
		blch.period++;\
	}
#define update_end_frame_channel(ch, blch, out)\
{\
	update_general_channel(ch, blch, 0, out);\
	blch.time -= bl.counter;\
}

typedef struct blip_chan _blip_chan;
typedef struct af_blip _af_blip;

struct blip_chan {
	int gain; /* overall volume of channel */
	int time; /* clock time of next delta */
	int phase; /* position within waveform */
	int amp; /* current amplitude in delta buffer */

	int period;
	int min_period;
};
struct af_blip {
	DBWORD counter;
	blip_buffer_t *blip;
	_blip_chan ch[5 + 3];
} bl;

void (*extra_apu_tick_blip)(void);
void apu_tick_blip_FDS(void);
void apu_tick_blip_MMC5(void);
void apu_tick_blip_Namco_N163(void);
void apu_tick_blip_Sunsoft_FM7(void);
void apu_tick_blip_VRC6(void);
void apu_tick_blip_VRC7(void);

void (*extra_end_frame_blip)(void);
void end_frame_blip_FDS(void);
void end_frame_blip_MMC5(void);
void end_frame_blip_Namco_N163(void);
void end_frame_blip_Sunsoft_FM7(void);
void end_frame_blip_VRC6(void);
void end_frame_blip_VRC7(void);

BYTE audio_quality_init_blip(void) {
	memset(&bl, 0, sizeof(bl));

	audio_quality_quit = audio_quality_quit_blip;

	snd_apu_tick = audio_quality_apu_tick_blip;
	snd_end_frame = audio_quality_end_frame_blip;

	bl.blip = blip_new(snd.samplerate / 10);

	if (bl.blip == NULL) {
		 /* out of memory */
		return (EXIT_ERROR);
	}

	blip_set_rates(bl.blip, machine.cpu_hz, snd.samplerate);

	bl.ch[APU_S1].gain = master_vol  * (1.8 * volume_fator) / 100;
	bl.ch[APU_S2].gain = master_vol  * (1.8 * volume_fator) / 100;
	bl.ch[APU_TR].gain = master_vol  * (2.2 * volume_fator) / 100;
	bl.ch[APU_NS].gain = master_vol  * (1.8 * volume_fator) / 100;
	bl.ch[APU_DMC].gain = master_vol * (1.2 * volume_fator) / 100;

	bl.ch[APU_S1].min_period  = min_period;
	bl.ch[APU_S2].min_period  = min_period;
	bl.ch[APU_TR].min_period  = min_period / 2.5;
	bl.ch[APU_NS].min_period  = min_period / 2;
	bl.ch[APU_DMC].min_period = min_period;

	switch (info.mapper) {
		case FDS_MAPPER:
			/* FDS */
			extra_apu_tick_blip = apu_tick_blip_FDS;
			extra_end_frame_blip = end_frame_blip_FDS;

			bl.ch[BL_EXT0].gain = 1;

			bl.ch[BL_EXT0].min_period = min_period;
			break;
		case 5:
			/* MMC5 */
			extra_apu_tick_blip = apu_tick_blip_MMC5;
			extra_end_frame_blip = end_frame_blip_MMC5;

			bl.ch[BL_EXT0].gain = master_vol * (2.6 * volume_fator) / 100;
			bl.ch[BL_EXT1].gain = master_vol * (2.6 * volume_fator) / 100;
			bl.ch[BL_EXT2].gain = master_vol * (1.0 * volume_fator) / 100;

			bl.ch[BL_EXT0].min_period = min_period;
			bl.ch[BL_EXT1].min_period = min_period;
			bl.ch[BL_EXT2].min_period = min_period;
			break;
		case 19:
			/* Namcot N163 */
			extra_apu_tick_blip = apu_tick_blip_Namco_N163;
			extra_end_frame_blip = end_frame_blip_Namco_N163;

			bl.ch[BL_EXT0].gain = master_vol * (1.0 * volume_fator) / 100;

			bl.ch[BL_EXT0].min_period = snd.frequency;
			break;
		case 69:
			/* Sunsoft FM7 */
			extra_apu_tick_blip = apu_tick_blip_Sunsoft_FM7;
			extra_end_frame_blip = end_frame_blip_Sunsoft_FM7;

			bl.ch[BL_EXT0].gain = master_vol * (1.6 * volume_fator) / 100;
			bl.ch[BL_EXT1].gain = master_vol * (1.6 * volume_fator) / 100;
			bl.ch[BL_EXT2].gain = master_vol * (1.6 * volume_fator) / 100;

			bl.ch[BL_EXT0].min_period = min_period;
			bl.ch[BL_EXT1].min_period = min_period;
			bl.ch[BL_EXT2].min_period = min_period;
			break;
		case 24:
		case 26:
			/* VRC6 */
			extra_apu_tick_blip = apu_tick_blip_VRC6;
			extra_end_frame_blip = end_frame_blip_VRC6;

			bl.ch[BL_EXT0].gain = master_vol * (1.60 * volume_fator) / 100;
			bl.ch[BL_EXT1].gain = master_vol * (1.60 * volume_fator) / 100;
			bl.ch[BL_EXT2].gain = master_vol * (0.15 * volume_fator) / 100;

			bl.ch[BL_EXT0].min_period = min_period;
			bl.ch[BL_EXT1].min_period = min_period;
			bl.ch[BL_EXT2].min_period = min_period;
			break;
		case 85:
			/* VRC7 */
			extra_apu_tick_blip = apu_tick_blip_VRC7;
			extra_end_frame_blip = end_frame_blip_VRC7;

			bl.ch[BL_EXT0].gain = 5;

			bl.ch[BL_EXT0].min_period = snd.frequency;
			break;
		default:
			extra_apu_tick_blip = NULL;
			extra_end_frame_blip = NULL;
			break;
	}

	return (EXIT_OK);
}
void audio_quality_quit_blip(void) {
	if (bl.blip) {
		blip_delete(bl.blip);
		bl.blip = NULL;
	}
}
void audio_quality_apu_tick_blip(void) {
	if (!bl.blip) {
		return;
	}

	update_tick_channel(S1, bl.ch[APU_S1], S1.output)
	update_tick_channel(S2, bl.ch[APU_S2], S2.output)
	update_tick_channel(TR, bl.ch[APU_TR], TR.output)
	update_tick_channel(NS, bl.ch[APU_NS], NS.output)
	update_tick_channel(DMC, bl.ch[APU_DMC], DMC.output)

	if (extra_apu_tick_blip) {
		extra_apu_tick_blip();
	}

	bl.counter++;
}
void audio_quality_end_frame_blip(void) {
	_callback_data *cache = snd.cache;

	if (!bl.blip) {
		return;
	}

	update_end_frame_channel(S1, bl.ch[APU_S1], S1.output)
	update_end_frame_channel(S2, bl.ch[APU_S2], S2.output)
	update_end_frame_channel(TR, bl.ch[APU_TR], TR.output)
	update_end_frame_channel(NS, bl.ch[APU_NS], NS.output)
	update_end_frame_channel(DMC, bl.ch[APU_DMC], DMC.output)

	if (extra_end_frame_blip) {
		extra_end_frame_blip();
	}

	blip_end_frame(bl.blip, bl.counter);
	bl.counter = 0;

	{
		int i, count = blip_samples_avail(bl.blip);
		short temp[count];

		blip_read_samples(bl.blip, temp, count, 0);

		if (snd.brk) {
			if (cache->filled < 3) {
				snd.brk = FALSE;
			} else {
				return;
			}
		}

		for (i = 0; i < count; i++) {
			SWORD data = temp[i];

			/* mono or left*/
			(*cache->write++) = data;

			/* stereo */
			if (cfg->channels == STEREO) {
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

			if (++snd.pos.current >= snd.samples) {
				snd.pos.current = 0;

				snd_lock_cache(cache);

				/* incremento il contatore dei frames pieni non ancora 'riprodotti' */
				if (++cache->filled >= snd.buffer.count) {
					snd.brk = TRUE;
				} else if (cache->filled == 1) {
					snd_frequency(snd_factor[apu.type][SND_FACTOR_SPEED])
				} else if (cache->filled >= ((snd.buffer.count >> 1) + 1)) {
					snd_frequency(snd_factor[apu.type][SND_FACTOR_SLOW])
				} else if (cache->filled < 3) {
					snd_frequency(snd_factor[apu.type][SND_FACTOR_NORMAL])
				}

				snd_unlock_cache(cache);
			}
		}
	}
}

/* --------------------------------------------------------------------------------------- */
/*                                    Extra APU Tick                                       */
/* --------------------------------------------------------------------------------------- */
void apu_tick_blip_FDS(void) {
	if (fds.snd.wave.clocked && (bl.ch[BL_EXT0].period >= bl.ch[BL_EXT0].min_period)) {
		fds.snd.wave.clocked = FALSE;
		bl.ch[BL_EXT0].time += bl.ch[BL_EXT0].period;
		update_amp(bl.ch[BL_EXT0], fds.snd.main.output)
		bl.ch[BL_EXT0].period = 1;
	} else {
		bl.ch[BL_EXT0].period++;
	}
}
void apu_tick_blip_MMC5(void) {
	update_tick_channel(mmc5.S3, bl.ch[BL_EXT0], mmc5.S3.output)
	update_tick_channel(mmc5.S4, bl.ch[BL_EXT1], mmc5.S4.output)
	update_tick_channel(mmc5.pcm, bl.ch[BL_EXT2], mmc5.pcm.output)
}
void apu_tick_blip_Namco_N163(void) {
	BYTE i;
	SWORD output = 0;

	if (++bl.ch[BL_EXT0].period == bl.ch[BL_EXT0].min_period) {
		for (i = n163.snd_ch_start; i < 8; i++) {
			if (n163.ch[i].active) {
				output += ((n163.ch[i].output * 1.5) * (n163.ch[i].volume >> 2));
			}
		}
		bl.ch[BL_EXT0].time += bl.ch[BL_EXT0].period;
		update_amp(bl.ch[BL_EXT0], output)
		bl.ch[BL_EXT0].period = 0;
	}
}
void apu_tick_blip_Sunsoft_FM7(void) {
	update_tick_channel(fm7.square[0], bl.ch[BL_EXT0], fm7.square[0].output)
	update_tick_channel(fm7.square[1], bl.ch[BL_EXT1], fm7.square[1].output)
	update_tick_channel(fm7.square[2], bl.ch[BL_EXT2], fm7.square[2].output)
}
void apu_tick_blip_VRC6(void) {
	update_tick_channel(vrc6.S3, bl.ch[BL_EXT0], vrc6.S3.output)
	update_tick_channel(vrc6.S4, bl.ch[BL_EXT1], vrc6.S4.output)
	update_tick_channel(vrc6.saw, bl.ch[BL_EXT2], vrc6.saw.output)
}
void apu_tick_blip_VRC7(void) {
	if (++bl.ch[BL_EXT0].period == bl.ch[BL_EXT0].min_period) {
		bl.ch[BL_EXT0].time += bl.ch[BL_EXT0].period;
		update_amp(bl.ch[BL_EXT0], opll_calc())
		bl.ch[BL_EXT0].period = 0;
	}
}

/* --------------------------------------------------------------------------------------- */
/*                                   Extra End Frame                                       */
/* --------------------------------------------------------------------------------------- */
void end_frame_blip_FDS(void) {
	bl.ch[BL_EXT0].time += bl.ch[BL_EXT0].period;
	update_amp(bl.ch[BL_EXT0], fds.snd.main.output)
	bl.ch[BL_EXT0].period = 0;
	bl.ch[BL_EXT0].time -= bl.counter;
}
void end_frame_blip_MMC5(void) {
	update_end_frame_channel(mmc5.S3, bl.ch[BL_EXT0], mmc5.S3.output)
	update_end_frame_channel(mmc5.S4, bl.ch[BL_EXT1], mmc5.S4.output)
	update_end_frame_channel(mmc5.pcm, bl.ch[BL_EXT2], mmc5.pcm.output)
}
void end_frame_blip_Namco_N163(void) {
	BYTE i;
	SWORD output = 0;

	for (i = n163.snd_ch_start; i < 8; i++) {
		if (n163.ch[i].active) {
			output += ((n163.ch[i].output * 1.5) * (n163.ch[i].volume >> 2));
		}
	}
	bl.ch[BL_EXT0].time += bl.ch[BL_EXT0].period;
	update_amp(bl.ch[BL_EXT0], output)
	bl.ch[BL_EXT0].period = 0;
	bl.ch[BL_EXT0].time -= bl.counter;
}
void end_frame_blip_Sunsoft_FM7(void) {
	update_end_frame_channel(fm7.square[0], bl.ch[BL_EXT0], fm7.square[0].output)
	update_end_frame_channel(fm7.square[1], bl.ch[BL_EXT1], fm7.square[1].output)
	update_end_frame_channel(fm7.square[2], bl.ch[BL_EXT2], fm7.square[2].output)
}
void end_frame_blip_VRC6(void) {
	update_end_frame_channel(vrc6.S3, bl.ch[BL_EXT0], vrc6.S3.output)
	update_end_frame_channel(vrc6.S4, bl.ch[BL_EXT1], vrc6.S4.output)
	update_end_frame_channel(vrc6.saw, bl.ch[BL_EXT2], vrc6.saw.output)
}
void end_frame_blip_VRC7(void) {
	bl.ch[BL_EXT0].time += bl.ch[BL_EXT0].period;
	update_amp(bl.ch[BL_EXT0], opll_calc())
	bl.ch[BL_EXT0].period = 0;
	bl.ch[BL_EXT0].time -= bl.counter;
}
