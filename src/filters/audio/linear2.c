/*
 * linear2.c
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
#include "linear2.h"

#include "cfgfile.h"
#include "clock.h"
#include "fps.h"

#define cycles_divider 5

struct _af_linear2 {
	WORD counter;

	float samples_to_run;
	float position_within_sample;
	float remaining_within_sample;
	float accumulated_volume[AFTOTCH];
	float old_volume[AFTOTCH];

	float volume[AFTOTCH];

	SWORD old_value;

	BYTE flag;
} l2;

void audio_filter_init_linear2(void) {
	memset(&l2, 0, sizeof(l2));

	/* azzero l'ouput di tutti i canali */
	audio_filter_reset_output_channels();

	/* popolo la tabella di approssimazione */
	audio_filter_popolate_table_approx();

	audio_filter_apu_tick = audio_filter_apu_tick_linear2;
	audio_filter_apu_mixer = audio_filter_apu_mixer_linear2;
}
void audio_filter_apu_tick_linear2(void) {
	return;
}
SWORD audio_filter_apu_mixer_linear2(void) {
	return (0);

	//mixer = af_table_approx.pulse[p] + af_table_approx.tnd[t];

	/* low-pass filter */
	//mixer = l2.old_value + (0.47 * (mixer - l2.old_value));
	/* hi-pass filter */
	//mixer = mixer - (l2.old_value + 0.35 * (mixer - l2.old_value));

	/*mixer *= 2;

	l2.old_value = mixer;

	l2.old_volume[AFS1] = S1.output;
	l2.old_volume[AFS2] = S2.output;
	l2.old_volume[AFTR] = TR.output;
	l2.old_volume[AFNS] = NS.output;
	l2.old_volume[AFDMC] = DMC.output;

	if (mixer > 0x7FFF) {
		mixer = 0x7FFF;
	} else if (mixer < -0x7FFF) {
		mixer = -0x7FFF;
	}

	return(mixer);*/
}











BYTE audio_filter_snd_write_linear2(void) {
	SDL_AudioSpec *dev = snd.dev;
	_callbackData *cache = snd.cache;
	BYTE elaborate = FALSE;

	if (!cfg->audio) {
		return (FALSE);
	}

	if (++l2.counter == cycles_divider) {
		l2.counter = 0;

		l2.samples_to_run = (float) cycles_divider / snd.frequency;

		if (l2.position_within_sample > 0.0) {
			if (l2.position_within_sample + l2.samples_to_run >= 1.0) {
				l2.remaining_within_sample = 1.0 - l2.position_within_sample;
				l2.samples_to_run -= l2.remaining_within_sample;

				l2.accumulated_volume[AFS1] += l2.remaining_within_sample * l2.old_volume[AFS1];
				l2.accumulated_volume[AFS2] += l2.remaining_within_sample * l2.old_volume[AFS2];
				l2.accumulated_volume[AFTR] += l2.remaining_within_sample * l2.old_volume[AFTR];
				l2.accumulated_volume[AFNS] += l2.remaining_within_sample * l2.old_volume[AFNS];
				l2.accumulated_volume[AFDMC] += l2.remaining_within_sample * l2.old_volume[AFDMC];

				elaborate = TRUE;

				l2.volume[AFS1] = l2.accumulated_volume[AFS1];
				l2.volume[AFS2] = l2.accumulated_volume[AFS2];
				l2.volume[AFTR] = l2.accumulated_volume[AFTR];
				l2.volume[AFNS] = l2.accumulated_volume[AFNS];
				l2.volume[AFDMC] = l2.accumulated_volume[AFDMC];
			} else {
				l2.accumulated_volume[AFS1] += l2.samples_to_run * l2.old_volume[AFS1];
				l2.accumulated_volume[AFS2] += l2.samples_to_run * l2.old_volume[AFS2];
				l2.accumulated_volume[AFTR] += l2.samples_to_run * l2.old_volume[AFTR];
				l2.accumulated_volume[AFNS] += l2.samples_to_run * l2.old_volume[AFNS];
				l2.accumulated_volume[AFDMC] += l2.samples_to_run * l2.old_volume[AFDMC];

				l2.position_within_sample += l2.samples_to_run;

				l2.old_volume[AFS1] = S1.output;
				l2.old_volume[AFS2] = S2.output;
				l2.old_volume[AFTR] = TR.output;
				l2.old_volume[AFNS] = NS.output;
				l2.old_volume[AFDMC] = DMC.output;

				return (FALSE);
			}
		}

		WORD whole_samples_to_run = (WORD) l2.samples_to_run;

		if (whole_samples_to_run) {
			elaborate = TRUE;
			l2.volume[AFS1] = l2.old_volume[AFS1];
			l2.volume[AFS2] = l2.old_volume[AFS2];
			l2.volume[AFTR] = l2.old_volume[AFTR];
			l2.volume[AFNS] = l2.old_volume[AFNS];
			l2.volume[AFDMC] = l2.old_volume[AFDMC];
		}
		l2.samples_to_run -= whole_samples_to_run;

		if (l2.samples_to_run > 0.0) {
			l2.accumulated_volume[AFS1] = l2.samples_to_run * l2.old_volume[AFS1];
			l2.accumulated_volume[AFS2] = l2.samples_to_run * l2.old_volume[AFS2];
			l2.accumulated_volume[AFTR] = l2.samples_to_run * l2.old_volume[AFTR];
			l2.accumulated_volume[AFNS] = l2.samples_to_run * l2.old_volume[AFNS];
			l2.accumulated_volume[AFDMC] = l2.samples_to_run * l2.old_volume[AFDMC];

			l2.position_within_sample = l2.samples_to_run;
		} else {
			l2.position_within_sample = 0.0;
		}

		l2.old_volume[AFS1] = S1.output;
		l2.old_volume[AFS2] = S2.output;
		l2.old_volume[AFTR] = TR.output;
		l2.old_volume[AFNS] = NS.output;
		l2.old_volume[AFDMC] = DMC.output;
	}

	if (snd.brk) {
		if (cache->filled < 3) {
			snd.brk = FALSE;
		} else {
			return (FALSE);
		}
	}

	if (!elaborate) {
		return(FALSE);
	}

	{
		WORD p, t;
		SWORD data;

		p = l2.volume[AFS1] + l2.volume[AFS2];
		t = (3 * l2.volume[AFTR]) + (2 * l2.volume[AFNS]) + l2.volume[AFDMC];

		data = af_table_approx.pulse[p] + af_table_approx.tnd[t];

		data *= 2;

		if (data > 0x7FFF) {
			data = 0x7FFF;
		} else if (data < -0x7FFF) {
			data = -0x7FFF;
		}

		//p = l2.volume[AFS1];
		//t = l2.volume[AFTR] + l2.volume[AFNS] + l2.volume[AFDMC];
		//data = af_table_approx.pulse[p];// + af_table_approx.tnd[t];

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

	return(TRUE);
}
