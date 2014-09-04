/*
 * snd.c
 *
 *  Created on: 08/mar/2011
 *      Author: fhorse
 */

#include <SDL.h>
#include <SDL_audio.h>
#include <SDL_thread.h>
#include "info.h"
#include "snd.h"
#include "clock.h"
#include "fps.h"
#include "apu.h"
#include "gui.h"
#include "cfg_file.h"
#include "audio_quality.h"

BYTE snd_init(void) {
	/* inizializzo il comparto audio dell'sdl */
	if (SDL_Init(SDL_INIT_AUDIO) < 0) {
		fprintf(stderr, "SDL sound initialization failed: %s\n", SDL_GetError());
		return (EXIT_ERROR);
	}

	memset(&snd, 0x00, sizeof(_snd));

	snd_apu_tick = NULL;
	snd_end_frame = NULL;

	/* apro e avvio la riproduzione */
	if (snd_start()) {
		return (EXIT_ERROR);
	}

	return (EXIT_OK);
}
BYTE snd_start(void) {
	SDL_AudioSpec *dev;
	_callback_data *cache;

	if (!cfg->apu.channel[APU_MASTER]) {
		return (EXIT_OK);
	}

	/* come prima cosa blocco eventuali riproduzioni */
	snd_stop();

	memset(&snd, 0, sizeof(_snd));

	dev = (SDL_AudioSpec *) malloc(sizeof(SDL_AudioSpec));
	memset(dev, 0, sizeof(SDL_AudioSpec));
	snd.dev = dev;

	cache = (_callback_data *) malloc(sizeof(_callback_data));
	memset(cache, 0, sizeof(_callback_data));
	snd.cache = cache;

 	{
		double latency = 200.0f;
 		double sample_latency;

#if defined (MINGW32) || defined (MINGW64)
 		/*
 		 * Anche se ormai non uso più l'SDL per l'audio nella versione windows,
 		 * conservo i parametri con cui arginavo il problema del delay audio
 		 * riscontrato da molti utenti.
 		 */
		BYTE factor = 4, bs = 32;

		switch (cfg->samplerate) {
			case S44100:
				snd.samplerate = 44100;
				snd.buffer.size = (bs * 21) * factor;
				break;
			case S22050:
				snd.samplerate = 22050;
				snd.buffer.size = (bs * 11) * factor;
				break;
			case S11025:
				snd.samplerate = 11025;
				snd.buffer.size = (bs * 6) * factor;
				break;
 		}
#else
		switch (cfg->samplerate) {
			case S48000:
				snd.samplerate = 48000;
				snd.buffer.size = (48000 / (11025 / 128)) * 8;
				break;
			case S44100:
				snd.samplerate = 44100;
				snd.buffer.size = 512 * 8;
				break;
			case S22050:
				snd.samplerate = 22050;
				snd.buffer.size = 256 * 8;
				break;
			case S11025:
				snd.samplerate = 11025;
				snd.buffer.size = 128 * 8;
				break;
		}
#endif

		if (cfg->channels == MONO) {
			latency *= 2.0f;
		}

		sample_latency = latency * (double) snd.samplerate * (double) cfg->channels / 1000.0f;
		snd.buffer.count = sample_latency / snd.buffer.size;
 	}

	/* il formato dei samples (16 bit signed) */
	dev->format = AUDIO_S16SYS;
	/* il numero dei canali (1 = mono) */
	dev->channels = cfg->channels;
	/* il samplerate */
	dev->freq = snd.samplerate;
	/* il valore del silenzio */
	dev->silence = 0;
	/* il numero dei samples da passare al device */
	dev->samples = snd.buffer.size / cfg->channels;
	/* la funzione di callback */
	dev->callback = snd_output;
	/* la struttura dei dati */
	dev->userdata = cache;

	if (SDL_OpenAudio(dev, NULL) < 0) {
		fprintf(stderr, "Unable to open audio device: %s\n", SDL_GetError());
		snd_stop();
		return (EXIT_ERROR);
	}

	snd.frequency = (fps.nominal * (double) machine.cpu_cycles_frame) / (double) snd.samplerate;
	snd.samples = dev->samples;
	snd.opened = TRUE;
	//snd.last_sample = dev->silence;
	snd.last_sample = 0;

	if (cfg->channels == STEREO) {
		BYTE i;

		snd.channel.max_pos = snd.samples * cfg->stereo_delay;
		snd.channel.pos = 0;

		for (i = 0; i < 2; i++) {
			DBWORD size = snd.samples * sizeof(*cache->write);

			snd.channel.buf[i] = (SWORD *) malloc(size);
			memset(snd.channel.buf[i], 0x00, size);
			snd.channel.ptr[i] = snd.channel.buf[i];

			snd.channel.bck.start = (SWORD *) malloc(size * 2);
			memset(snd.channel.bck.start, 0x00, size * 2);
			snd.channel.bck.write = snd.channel.bck.start;
			snd.channel.bck.middle = snd.channel.bck.start + snd.samples;
			snd.channel.bck.end = (SBYTE *) snd.channel.bck.start + (size * 2);
		}
	}

	snd_frequency(snd_factor[apu.type][SND_FACTOR_SPEED])

	{
		/* dimensione in bytes del buffer */
		DBWORD total_buffer_size = snd.buffer.size * snd.buffer.count * sizeof(*cache->write);

		/* alloco il buffer in memoria */
		if (!(cache->start = (SWORD *) malloc(total_buffer_size))) {
			fprintf(stderr, "Unable to allocate audio buffers\n");
			snd_stop();
			return (EXIT_ERROR);
		}

		/* inizializzo il frame di scrittura */
		cache->write = cache->start;
		/* inizializzo il frame di lettura */
		cache->read = (SBYTE *) cache->start;
		/* punto alla fine del buffer */
		cache->end = cache->read + total_buffer_size;
		/* creo il lock */
		cache->lock = SDL_CreateMutex();
		/* azzero completamente il buffer */
		memset(cache->start, 0x00, total_buffer_size);

		/* punto all'inizio del frame di scrittura */
		snd.pos.current = snd.pos.last = 0;
	}

	if (extcl_snd_start) {
		extcl_snd_start(snd.samplerate);
	}

	audio_quality(cfg->audio_quality);

#if defined (DEBUG)
	return (EXIT_OK);
#endif

	/* avvio la riproduzione */
	SDL_PauseAudio(FALSE);

	return (EXIT_OK);
}
void snd_stereo_delay(void) {
	int i;
	_callback_data *cache = (_callback_data *) snd.cache;
	SWORD *here;

	snd.channel.max_pos = snd.samples * cfg->stereo_delay;
	snd.channel.pos = 0;

	for (i = 0; i < 2; i++) {
		snd.channel.ptr[i] = snd.channel.buf[i];
	}

	here = snd.channel.bck.write - snd.channel.max_pos;

	if (here >= snd.channel.bck.start) {
		memcpy(snd.channel.ptr[CH_RIGHT], here, snd.channel.max_pos * sizeof(*cache->write));
	} else {
		DBWORD step = snd.channel.bck.start - here;
		SWORD *src1 = (SWORD *) snd.channel.bck.end - step;
		SWORD *src2 = snd.channel.bck.start;
		SWORD *dst1 = snd.channel.ptr[CH_RIGHT];
		SWORD *dst2 = snd.channel.ptr[CH_RIGHT] + step;

		memcpy(dst1, src1, step * sizeof(*cache->write));
		memcpy(dst2, src2, (snd.channel.max_pos - step) * sizeof(*cache->write));
	}
}
void snd_output(void *udata, BYTE *stream, int len) {
	_callback_data *cache = (_callback_data *) udata;

	if (info.no_rom) {
		return;
	}

	snd_lock_cache(cache);

#if !defined (RELEASE)
	/*
	fprintf(stderr, "snd : %d %d %d %d %2d %d %f %f %4s\r",
			len,
			snd.buffer.count,
			snd.brk,
			fps.total_frames_skipped,
			cache->filled,
			snd.out_of_sync,
			snd.frequency,
			machine.ms_frame,
			"");
	*/
#endif

	if (!cache->filled) {
		memset(stream, snd.last_sample, len);

		snd.out_of_sync++;
	} else {
		/* invio i samples richiesti alla scheda sonora */
		memcpy(stream, cache->read, len);

		/* salvo l'ultimo sample inviato */
		snd.last_sample = (SWORD) cache->read[len - 1];

		/*
	 	 * mi preparo per i prossimi frames da inviare, sempre
	 	 * che non abbia raggiunto la fine del buffer, nel
	 	 * qual caso devo puntare al suo inizio.
	 	 */
		if ((cache->read += len) >= cache->end) {
			cache->read = (SBYTE *) cache->start;
		}

		/* decremento il numero dei frames pieni */
		cache->filled -= (((len / cfg->channels) / sizeof(*cache->write)) / snd.samples);
	}

	snd_unlock_cache(cache);
}
void snd_lock_cache(_callback_data *cache) {
	SDL_mutexP((SDL_mutex *) cache->lock);
}
void snd_unlock_cache(_callback_data *cache) {
	SDL_mutexV((SDL_mutex *) cache->lock);
}
void snd_stop(void) {
	if (snd.opened) {
		snd.opened = FALSE;
		SDL_PauseAudio(TRUE);
		SDL_CloseAudio();
	}

	if (snd.dev) {
		free(snd.dev);
		snd.dev = NULL;
	}

	if (snd.cache) {
		_callback_data *cache = (_callback_data *) snd.cache;

		if (cache->start) {
			free(cache->start);
		}

		if (cache->lock) {
			SDL_DestroyMutex((SDL_mutex *) cache->lock);
		}

		free(snd.cache);
		snd.cache = NULL;
	}

	{
		BYTE i;

		if (snd.channel.bck.start) {
			free(snd.channel.bck.start);
			snd.channel.bck.start = NULL;
		}

		for (i = 0; i < STEREO; i++) {
			/* rilascio la memoria */
			if (snd.channel.buf[i]) {
				free(snd.channel.buf[i]);
			}
			/* azzero i puntatori */
			snd.channel.ptr[i] =  snd.channel.buf[i] = NULL;
		}
	}

	if (audio_quality_quit) {
		audio_quality_quit();
	}
}
void snd_quit(void) {
	snd_stop();

#if !defined (RELEASE)
	fprintf(stderr, "\n");
#endif
}
