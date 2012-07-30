/*
 * blip.c
 *
 *  Created on: 28/lug/2012
 *      Author: fhorse
 */

#include <string.h>
#include "audio_filter.h"
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

#define run_tick_channel(ch, blch, run)\
	if (ch.clocked && (ch.period >= blch.min_period)) {\
		ch.clocked = FALSE;\
		run(&ch, &blch, 1);\
	} else {\
		ch.period++;\
	}
#define run_end_frame_channel(ch, blch, run)\
	run(&ch, &blch, 0);\
	blch.time -= bl.counter

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
	_blip_chan ch[AFTOTCH];
} bl;

static void INLINE update_amp(_blip_chan *m, int new_amp);
static void INLINE update_amp(_blip_chan *m, int new_amp) {
	int delta = new_amp * m->gain - m->amp;
	m->amp += delta;
	blip_add_delta(bl.blip, m->time, delta);
}

static void INLINE run_square(_apuSquare *s, _blip_chan *m, WORD start);
static void INLINE run_square(_apuSquare *s, _blip_chan *m, WORD start) {
	m->time += s->period;
	update_amp(m, s->output);
	s->period = start;
}

static void INLINE run_noise(_apuNoise *n, _blip_chan *m, WORD start);
static void INLINE run_noise(_apuNoise *n, _blip_chan *m, WORD start) {
	m->time += n->period;
	update_amp(m, n->output);
	n->period = start;
}

static void INLINE run_triangle(_apuTriangle *t, _blip_chan *m, WORD start);
static void INLINE run_triangle(_apuTriangle *t, _blip_chan *m, WORD start) {
	/* phase only increments when volume is non-zero (volume is otherwise ignored)*/
	m->time += t->period;
	if (t->output != 0) {
		update_amp(m, t->output);
	}
	t->period = start;
}

void audio_filter_init_blip(void) {
	memset(&bl, 0, sizeof(bl));

	/* azzero l'ouput di tutti i canali */
	audio_filter_reset_output_channels();

	/* popolo la tabella di approssimazione */
	audio_filter_popolate_table_approx();

	audio_filter_apu_tick = audio_filter_apu_tick_blip;
	audio_filter_apu_mixer = audio_filter_apu_mixer_blip;
	audio_filter_end_frame = audio_filter_end_frame_blip;

	snd_write = audio_filter_snd_write_blip;

	{
		SDL_AudioSpec *dev = snd.dev;

		bl.blip = blip_new(dev->freq / 10);
		if (bl.blip == NULL )
			exit(EXIT_FAILURE); /* out of memory */

		blip_set_rates(bl.blip, machine.cpuHz, dev->freq);

		enum { master_vol = 65536 / 15 };
		bl.ch[AFS1].gain = master_vol * 26 / 100;
		bl.ch[AFS2].gain = master_vol * 26 / 100;
		bl.ch[AFTR].gain = master_vol * 30 / 100;
		bl.ch[AFNS].gain = master_vol * 18 / 100;

		/*
		bl.ch[AFS1].gain = master_vol * 24 / 100;
		bl.ch[AFS2].gain = master_vol * 24 / 100;
		bl.ch[AFTR].gain = master_vol * 18 / 100;
		bl.ch[AFNS].gain = master_vol * 18 / 100;

		bl.ch[AFS1].gain = master_vol * 0 / 100;
		bl.ch[AFS2].gain = master_vol * 0 / 100;
		bl.ch[AFTR].gain = master_vol * 30 / 100;
		bl.ch[AFNS].gain = master_vol * 0 / 100;
		*/

		bl.ch[AFS1].min_period = min_period;
		bl.ch[AFS2].min_period = min_period;
		bl.ch[AFTR].min_period = min_period / 2.5;
		bl.ch[AFNS].min_period = min_period / 2;
	}
}
void audio_filter_apu_tick_blip(void) {

	run_tick_channel(S1, bl.ch[AFS1], run_square)
	run_tick_channel(S2, bl.ch[AFS2], run_square)
	run_tick_channel(TR, bl.ch[AFTR], run_triangle)
	run_tick_channel(NS, bl.ch[AFNS], run_noise)

	bl.counter++;
}
SWORD audio_filter_apu_mixer_blip(void) {
	return (0);
}
BYTE audio_filter_snd_write_blip(void) {
	return(TRUE);
}
void audio_filter_end_frame_blip(void) {
	SDL_AudioSpec *dev = snd.dev;
	_callbackData *cache = snd.cache;

	run_end_frame_channel(S1, bl.ch[AFS1], run_square);
	run_end_frame_channel(S2, bl.ch[AFS2], run_square);
	run_end_frame_channel(TR, bl.ch[AFTR], run_triangle);
	run_end_frame_channel(NS, bl.ch[AFNS], run_noise);

	blip_end_frame(bl.blip, bl.counter);
	bl.counter = 0;

	{
		int i, count = blip_samples_avail(bl.blip);
		short temp[count];

		blip_read_samples(bl.blip, temp, count, 0);

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
				++cache->filled;
				SDL_mutexV(cache->lock);
			}
		}
	}
}
