/*
 * sdlsnd.c
 *
 *  Created on: 08/mar/2011
 *      Author: fhorse
 */

#include <SDL.h>
#include <SDL_audio.h>
#include <SDL_thread.h>
#include "sdlsnd.h"
#include "clock.h"
#include "fps.h"
#include "apu.h"
#include "gui.h"
#include "cfgfile.h"

typedef struct {
	SWORD *start;
	SBYTE *end;

	SBYTE *read;
	SWORD *write;

	SWORD filled;

	SDL_mutex *lock;
} _callbackData;

BYTE sndInit(void) {
	/* inizializzo il comparto audio dell'sdl */
	if (SDL_Init(SDL_INIT_AUDIO) < 0) {
		fprintf(stderr, "SDL sound initialization failed: %s\n", SDL_GetError());
		return (EXIT_ERROR);
	}

	memset(&snd, 0x00, sizeof(snd));

	/* apro e avvio la riproduzione */
	if (sndStart()) {
		return (EXIT_ERROR);
	}

	return (EXIT_OK);
}
BYTE sndStart(void) {
	SDL_AudioSpec *dev;
	_callbackData *cache;

	if (!cfg->audio) {
		return (EXIT_OK);
	}

	/* come prima cosa blocco eventuali riproduzioni */
	sndStop();

	memset(&snd, 0, sizeof(snd));

	dev = malloc(sizeof(SDL_AudioSpec));
	memset(dev, 0, sizeof(SDL_AudioSpec));
	snd.dev = dev;

	cache = malloc(sizeof(_callbackData));
	memset(cache, 0, sizeof(_callbackData));
	snd.cache = cache;

	/* samplarate */
#if defined MINGW32 || defined MINGW64
	WORD factor = 4;
#else
	WORD factor = 4;
#endif

	switch (cfg->samplerate) {
		case S44100:
			dev->freq = 44100;
			snd.buffer.size = 512 * factor;
			break;
		case S22050:
			dev->freq = 22050;
			snd.buffer.size = 256 * factor;
			break;
		case S11025:
			dev->freq = 11025;
			snd.buffer.size = 128 * factor;
			break;
	}

	{
		WORD latency;
		long sample_latency;

		if (dev->channels == STEREO) {
			latency = 200;
		} else {
			latency = 400;
		}

		sample_latency = latency * dev->freq * cfg->channels / 1000;
		snd.buffer.count = sample_latency / snd.buffer.size;
	}

	/* il formato dei samples (16 bit signed) */
	dev->format = AUDIO_S16SYS;
	/* il numero dei canali (1 = mono) */
	dev->channels = cfg->channels;
	/* il valore del silenzio */
	dev->silence = 0;
	/* il numero dei samples da passare al device */
	dev->samples = snd.buffer.size / dev->channels;
	/* la funzione di callback */
	dev->callback = sndOutput;
	/* la struttura dei dati */
	dev->userdata = cache;

	if (SDL_OpenAudio(dev, NULL) < 0) {
		fprintf(stderr, "Unable to open audio device: %s\n", SDL_GetError());
		sndStop();
		return (EXIT_ERROR);
	}

	/* valorizzo il flag di apertura device */
	snd.opened = TRUE;

	if (dev->channels == STEREO) {
		BYTE i;

		for (i = 0; i < 2; i++) {
			snd.channel.buf[i] = malloc(dev->size);
			memset(snd.channel.buf[i], 0, dev->size);
			snd.channel.ptr[i] = snd.channel.buf[i];
		}
	}

	if (machine.type == NTSC) {
		snd.frequency = ((fps.nominal * machine.cpuCyclesFrame) -
			((machine.cpuCyclesFrame / 100.0) * 10.5)) / dev->freq;
	} else {
		snd.frequency = ((fps.nominal * machine.cpuCyclesFrame) -
			((machine.cpuCyclesFrame / 100.0) * 1.9)) / dev->freq;
	}

	{
		DBWORD total_buffer_size;

		/* dimensione in bytes del buffer */
		total_buffer_size = snd.buffer.size * snd.buffer.count * sizeof(*cache->write);

		/* alloco il buffer in memoria */
		cache->start = malloc(total_buffer_size);

		if (!cache->start) {
			fprintf(stderr, "Out of memory\n");
			sndStop();
			return (EXIT_ERROR);
		}

		/* inizializzo il frame di scrittura */
		cache->write = cache->start + dev->channels;
		/* inizializzo il frame di lettura */
		cache->read = (SBYTE *) cache->start;
		/* punto alla fine del buffer */
		cache->end = cache->read + total_buffer_size;
		/* creo il lock */
		cache->lock = SDL_CreateMutex();
		/* azzero completamente il buffer */
		memset(cache->start, 0, total_buffer_size);

		/* punto all'inizio del frame di scrittura */
		snd.pos.current = snd.pos.last = 0;
	}

	if (extclSndStart) {
		extclSndStart(dev->freq);
	}

#ifdef DEBUG
	return (EXIT_OK);
#endif

	/* avvio la riproduzione */
	SDL_PauseAudio(FALSE);

	return (EXIT_OK);
}
BYTE sndWrite(void) {
	SDL_AudioSpec *dev = snd.dev;
	_callbackData *cache = snd.cache;

	if (!cfg->audio || ((snd.pos.current = snd.cycles++ / snd.frequency) == snd.pos.last)) {
		return (FALSE);
	}

	SDL_mutexP(cache->lock);

	/*
	 * se la posizione e' maggiore o uguale al numero
	 * di samples che compongono il frame, vuol dire che
	 * sono passato nel frame successivo.
	 */
	if (snd.pos.current == dev->samples) {
		/* incremento il contatore dei frames pieni non ancora 'riprodotti' */
		if (++cache->filled > snd.buffer.count) {
			cache->filled = snd.buffer.count;
		}

		/* azzero posizione e contatore dei cicli del frame audio */
		snd.pos.current = snd.cycles = 0;

		/* swappo i buffers dei canali */
		if (dev->channels == STEREO) {
			SWORD *swap = snd.channel.ptr[CH_RIGHT];
			snd.channel.ptr[CH_RIGHT] = snd.channel.ptr[CH_LEFT];
			snd.channel.ptr[CH_LEFT] = swap;
		}
	}

	{
		SWORD data = apuMixer();

		/* mono or left*/
		(*cache->write++) = data;

		/* stereo */
		if (dev->channels == STEREO) {
			/* salvo il dato nel buffer del canale sinistro */
			snd.channel.ptr[CH_LEFT][snd.pos.current] = data;
			/* scrivo nel nel frame audio il canale destro ritardato di un frame */
			(*cache->write++) = snd.channel.ptr[CH_RIGHT][snd.pos.current];
		}

		if (cache->write >= (SWORD *) cache->end) {
			cache->write = cache->start;
		}

		snd.pos.last = snd.pos.current;
	}

	SDL_mutexV(cache->lock);

	return (TRUE);
}
void sndOutput(void *udata, BYTE *stream, int len) {
	_callbackData *cache = udata;

	if (info.no_rom) {
		return;
	}

	SDL_mutexP(cache->lock);

	if (cache->filled == 0) {
		memset(stream, 0, len);

		snd.out_of_sync++;
	} else {
#ifndef RELEASE
		fprintf(stderr, "snd : %d %d %2d %d %f %4s\r", len, fps.total_frames_skipped,
				cache->filled, snd.out_of_sync, snd.frequency, "");
#endif

		if (!snd.started && (cache->filled < 2)) {
			memset(stream, 0, len);

		} else {
			snd.started = TRUE;

			/* invio i samples richiesti alla scheda sonora */
			memcpy(stream, cache->read, len);

			/*
		 	 * mi preparo per i prossimi frames da inviare, sempre
		 	 * che non abbia raggiunto la fine del buffer, nel
		 	 * qual caso devo puntare al suo inizio.
		 	 */
			if ((cache->read += len) >= cache->end) {
				cache->read = (SBYTE *) cache->start;
			}

			{
				SDL_AudioSpec *dev = snd.dev;

				/* decremento il numero dei frames pieni */
				cache->filled -= (((len / dev->channels) / 2) / dev->samples);
			}
		}
	}

	SDL_mutexV(cache->lock);
}
void sndStop(void) {
	if (snd.opened) {
		snd.opened = snd.started = FALSE;
		SDL_PauseAudio(TRUE);
		SDL_CloseAudio();
	}

	if (snd.dev) {
		free(snd.dev);
		snd.dev = NULL;
	}

	if (snd.cache) {
		_callbackData *cache = snd.cache;

		if (cache->start) {
			free(cache->start);
		}

		if (cache->lock) {
			SDL_DestroyMutex(cache->lock);
		}

		free(snd.cache);
		snd.cache = NULL;
	}

	{
		BYTE i;

		for (i = 0; i < STEREO; i++) {
			/* rilascio la memoria */
			if (snd.channel.buf[i]) {
				free(snd.channel.buf[i]);
			}
			/* azzero i puntatori */
			snd.channel.ptr[i] =  snd.channel.buf[i] = NULL;
		}
	}
}
void sndQuit(void) {
	sndStop();
#ifndef RELEASE
	fprintf(stderr, "\n");
#endif
}
