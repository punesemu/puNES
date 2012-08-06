/*
 * blip2.c
 *
 *  Created on: 28/lug/2012
 *      Author: fhorse
 */

#include <string.h>
#include "audio_quality.h"
#include "apu.h"
#include "sdlsnd.h"
#include "mappers.h"
#include "mappers/mapperVRC7snd.h"
#include "fds.h"
#include "blip2.h"

#include "cfgfile.h"
#include "clock.h"
#include "fps.h"
#include "blip_buf.h"

enum { master_vol = 65536 / 15 , volume_fator = 4, min_period = 20 };

#define update_amp_2(blch, new_amp)\
{\
	int delta = new_amp * blch.gain - blch.amp;\
	blch.amp += delta;\
	blip_add_delta(bl2.blip, blch.time, delta);\
}
#define update_general_channel_2(ch, blch, restart, out)\
{\
	SWORD output = out;\
	blch.time += blch.period;\
	update_amp_2(blch, output)\
	blch.period = restart;\
}
#define update_tick_channel_2(ch, blch, out)\
	if (ch.clocked && (blch.period >= blch.min_period)) {\
		ch.clocked = FALSE;\
		update_general_channel_2(ch, blch, 1, out)\
	} else {\
		blch.period++;\
	}
#define update_end_frame_channel_2(ch, blch, out)\
{\
	update_general_channel_2(ch, blch, 0, out);\
	blch.time -= bl2.counter;\
}

typedef struct blip2_chan _blip2_chan;
typedef struct af_blip2 _af_blip2;

struct blip2_chan {
	int gain; /* overall volume of channel */
	int time; /* clock time of next delta */
	int phase; /* position within waveform */
	int amp; /* current amplitude in delta buffer */

	int period;
	int min_period;
};
struct af_blip2 {
	DBWORD counter;
	blip_buffer_t *blip;
	_blip2_chan ch[APU_TOT_CH];
} bl2;


BYTE audio_quality_init_blip2(void) {
	memset(&bl2, 0, sizeof(bl2));

	audio_quality_quit = audio_quality_quit_blip2;

	snd_apu_tick = audio_quality_apu_tick_blip2;
	snd_end_frame = audio_quality_end_frame_blip2;

	{
		SDL_AudioSpec *dev = snd.dev;

		bl2.blip = blip_new(dev->freq / 10);

		if (bl2.blip == NULL ) {
			 /* out of memory */
			return (EXIT_ERROR);
		}

		blip_set_rates(bl2.blip, machine.cpuHz, dev->freq);

		bl2.ch[APU_S1].gain = master_vol  * (2.6 * volume_fator) / 100;
		//bl2.ch[APU_S2].gain = master_vol  * (2.6 * volume_fator) / 100;
		//bl2.ch[APU_TR].gain = master_vol  * (2.2 * volume_fator) / 100;
		//bl2.ch[APU_NS].gain = master_vol  * (1.8 * volume_fator) / 100;
		//bl2.ch[APU_DMC].gain = master_vol * (1.0 * volume_fator) / 100;

		bl2.ch[APU_S1].min_period  = min_period;
		bl2.ch[APU_S2].min_period  = min_period;
		bl2.ch[APU_TR].min_period  = min_period / 2.5;
		bl2.ch[APU_NS].min_period  = min_period / 2;
		bl2.ch[APU_DMC].min_period = min_period;
	}

	return (EXIT_OK);
}
void audio_quality_quit_blip2(void) {
	if (bl2.blip) {
		blip_delete(bl2.blip);
		bl2.blip = NULL;
	}
}
void audio_quality_apu_tick_blip2(void) {
	if (!bl2.blip) {
		return;
	}

	update_tick_channel_2(S1, bl2.ch[APU_S1], S1.output)
	update_tick_channel_2(S2, bl2.ch[APU_S2], S2.output)
	update_tick_channel_2(TR, bl2.ch[APU_TR], TR.output)
	update_tick_channel_2(NS, bl2.ch[APU_NS], NS.output)
	update_tick_channel_2(DMC, bl2.ch[APU_DMC], DMC.output)

	bl2.counter++;
}
void audio_quality_end_frame_blip2(void) {
	SDL_AudioSpec *dev = snd.dev;
	_callbackData *cache = snd.cache;

	if (!bl2.blip) {
		return;
	}

	update_end_frame_channel_2(S1, bl2.ch[APU_S1], S1.output)
	update_end_frame_channel_2(S2, bl2.ch[APU_S2], S2.output)
	update_end_frame_channel_2(TR, bl2.ch[APU_TR], TR.output)
	update_end_frame_channel_2(NS, bl2.ch[APU_NS], NS.output)
	update_end_frame_channel_2(DMC, bl2.ch[APU_DMC], DMC.output)

	blip_end_frame(bl2.blip, bl2.counter);
	bl2.counter = 0;

	{
		int i, count = blip_samples_avail(bl2.blip);
		short temp[count];

		blip_read_samples(bl2.blip, temp, count, 0);

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
				} else if (cache->filled >= ((snd.buffer.count >> 1) + 1)) {
					snd_frequency(sndFactor[apu.type][FCNONE])
				} else if (cache->filled < 3) {
					snd_frequency(sndFactor[apu.type][FCNORMAL])
				}

				SDL_mutexV(cache->lock);
			}
		}
	}
}
