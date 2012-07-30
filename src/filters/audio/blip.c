/*
 * blip.c
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
#include "blip.h"

#include "cfgfile.h"
#include "clock.h"
#include "fps.h"
#include "blip_buf.h"

enum { min_period = 10 };

#define update_amp(blch, new_amp)\
{\
	int delta = new_amp * blch.gain - blch.amp;\
	blch.amp += delta;\
	blip_add_delta(bl.blip, blch.time, delta);\
}
#define update_general_channel(ch, blch, restart)\
	blch.time += ch.period;\
	update_amp(blch, ch.output)\
	ch.period = restart
#define update_tick_channel(ch, blch)\
	if (ch.clocked && (ch.period >= blch.min_period)) {\
		ch.clocked = FALSE;\
		update_general_channel(ch, blch, 1);\
	} else {\
		ch.period++;\
	}
#define update_end_frame_channel(ch, blch)\
{\
	update_general_channel(ch, blch, 0);\
	blch.time -= bl.counter;\
}

typedef struct blip_chan _blip_chan;
typedef struct af_blip _af_blip;

struct blip_chan {
	int gain; /* overall volume of channel */
	int time; /* clock time of next delta */
	int phase; /* position within waveform */
	int amp; /* current amplitude in delta buffer */
	int min_period;
};
struct af_blip {
	DBWORD counter;
	blip_buffer_t *blip;
	_blip_chan ch[AQ_TOT_CH];
} bl;

void audio_quality_init_blip(void) {
	memset(&bl, 0, sizeof(bl));

	audio_quality_apu_tick = audio_quality_apu_tick_blip;
	audio_quality_apu_mixer = audio_quality_apu_mixer_blip;
	audio_quality_end_frame = audio_quality_end_frame_blip;

	snd_write = audio_quality_snd_write_blip;

	{
		SDL_AudioSpec *dev = snd.dev;

		bl.blip = blip_new(dev->freq / 10);

		if (bl.blip == NULL ) {
			exit(EXIT_FAILURE); /* out of memory */
		}

		blip_set_rates(bl.blip, machine.cpuHz, dev->freq);

		enum { master_vol = 65536 / 15 };

		/**/
		bl.ch[AQ_S1].gain = master_vol * 26 / 100;
		bl.ch[AQ_S2].gain = master_vol * 26 / 100;
		bl.ch[AQ_TR].gain = master_vol * 30 / 100;
		bl.ch[AQ_NS].gain = master_vol * 18 / 100;
		bl.ch[AQ_DMC].gain = master_vol * 10 / 100;
		/**/

		/*
		bl.ch[AQ_S1].gain = master_vol * 24 / 100;
		bl.ch[AQ_S2].gain = master_vol * 24 / 100;
		bl.ch[AQ_TR].gain = master_vol * 18 / 100;
		bl.ch[AQ_NS].gain = master_vol * 18 / 100;
		bl.ch[AQ_DMC].gain = master_vol * 10 / 100;
		*/

		/*
		bl.ch[AQ_S1].gain = master_vol * 0 / 100;
		bl.ch[AQ_S2].gain = master_vol * 0 / 100;
		bl.ch[AQ_TR].gain = master_vol * 30 / 100;
		bl.ch[AQ_NS].gain = master_vol * 0 / 100;
		bl.ch[AQ_DMC].gain = master_vol * 0 / 100;
		*/

		bl.ch[AQ_S1].min_period = min_period;
		bl.ch[AQ_S2].min_period = min_period;
		bl.ch[AQ_TR].min_period = min_period / 2.5;
		bl.ch[AQ_NS].min_period = min_period / 2;
		bl.ch[AQ_DMC].min_period = min_period;
	}
}
void audio_quality_apu_tick_blip(void) {

	update_tick_channel(S1, bl.ch[AQ_S1])
	update_tick_channel(S2, bl.ch[AQ_S2])
	update_tick_channel(TR, bl.ch[AQ_TR])
	update_tick_channel(NS, bl.ch[AQ_NS])
	update_tick_channel(DMC, bl.ch[AQ_DMC])

	bl.counter++;
}
SWORD audio_quality_apu_mixer_blip(void) {
	return (0);
}
BYTE audio_quality_snd_write_blip(void) {
	return(TRUE);
}
void audio_quality_end_frame_blip(void) {
	SDL_AudioSpec *dev = snd.dev;
	_callbackData *cache = snd.cache;

	update_end_frame_channel(S1, bl.ch[AQ_S1])
	update_end_frame_channel(S2, bl.ch[AQ_S2])
	update_end_frame_channel(TR, bl.ch[AQ_TR])
	update_end_frame_channel(NS, bl.ch[AQ_NS])
	update_end_frame_channel(DMC, bl.ch[AQ_DMC])

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
