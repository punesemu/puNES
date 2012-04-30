/*
 * sdlsnd.c
 *
 *  Created on: 08/mar/2011
 *      Author: fhorse
 */

#include <SDL.h>
#include <SDL_audio.h>
#include "sdlsnd.h"
#include "clock.h"
#include "fps.h"
#include "apu.h"
#include "gui.h"
#include "cfgfile.h"

#define PLAY_FRAMES  1
#define CACHE_FRAMES 1
#define TOTAL_FRAMES 8
#define sndHz()\
{\
	float coeff_filled;\
	coeff_filled = ((float) (cache->filled * snd.factor) -\
			(3.25 - (machine.type == NTSC ? 1 : 2))) / 10;\
	snd.frequency = (((fps.nominal + coeff_filled) * machine.cpuCyclesFrame) / dev->freq);\
}

typedef struct {
	SWORD *start;
	SBYTE *end;
	SBYTE *read;
	SWORD *write;
	SWORD filled;
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
	WORD samples = 1024;

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

	sndWmEvent(1000);

	/* samplarate */
	switch (cfg->samplerate) {
		case S44100:
			dev->freq = 44100;
			samples = 1024;
			break;
		case S22050:
			dev->freq = 22050;
			samples = 512;
			break;
		case S11025:
			dev->freq = 11025;
			samples = 512;
			break;
	}
	/* il formato dei samples (16 bit signed) */
	dev->format = AUDIO_S16SYS;
	/* il numero dei canali (1 = mono) */
	dev->channels = cfg->channels;
	/* il numero dei samples da passare al device */
	dev->samples = samples;
	/* la funzione di callback */
	dev->callback = sndOutput;
	/* la struttura dei dati */
	dev->userdata = cache;
	/* apro il device audio */
	if (SDL_OpenAudio(dev, NULL) < 0) {
		fprintf(stderr, "Unable to open audio device: %s\n", SDL_GetError());
		sndStop();
		return (EXIT_ERROR);
	}
	/*
	 * ho notato che (sotto linux) nelle sdl 1.2.13 i samples
	 * che passo al device vengono moltiplicati per 2 dalle
	 * sdl stesse, mentre nelle 1.2.14 questo non avviene.
	 * Quindi se voglio che si comportino allo stesso
	 * modo devo controllarne quanti ne ha settanti dopo
	 * aver aperto il device e se sono la meta' di quelli
	 * richiesti chiudo e riapro con i samples moltiplicati
	 * per 2 (per intenderci, se passo come parametro
	 * sndDevice.samples = 1472, nelle sdl 1.2.13 il device
	 * creato avra' un buffer di 2944 samples, mentre le
	 * 1.2.14 ne avra' uno di 1472 ergo, per uno di 2944
	 * devo passare come parametro sndDevice.samples = 2944).
	 */
	if (dev->samples == (samples / 2 + 56)) {

		SDL_Delay(3000);

		/* chiudo il device */
		SDL_CloseAudio();
		/* ricalcolo il numero dei samples */
		dev->samples = samples * 2;
		/* riapro il device */
		if (SDL_OpenAudio(dev, NULL) < 0) {
			fprintf(stderr, "Unable to open audio device: %s\n", SDL_GetError());
			sndStop();
			return (EXIT_ERROR);
		}
	}

	if (dev->channels == STEREO) {
		WORD ch_size = dev->size >> 1;

		snd.channel_buffer[CH_LEFT] = malloc(ch_size);
		snd.channel_buffer[CH_RIGHT] = malloc(ch_size);

		memset(snd.channel_buffer[CH_LEFT], 0, ch_size);
		memset(snd.channel_buffer[CH_RIGHT], 0, ch_size);

		snd.channel_ptr[CH_LEFT] = snd.channel_buffer[CH_LEFT];
		snd.channel_ptr[CH_RIGHT] = snd.channel_buffer[CH_RIGHT];
	}

	/* valorizzo il flag di apertura device */
	snd.opened = TRUE;

	snd.last_sample = dev->silence;

	sndHz();

	{
		DBWORD buffer_size;

		/* dimensione in bytes del buffer */
		buffer_size = TOTAL_FRAMES * ((dev->samples * 2) * dev->channels);
		/* inizializzo a zero il buffer */
		cache->start = NULL;
		/* alloco il buffer in memoria */
		cache->start = malloc(buffer_size);

		if (!cache->start) {
			fprintf(stderr, "Out of memory\n");
			sndStop();
			return (EXIT_ERROR);
		}

		/* inizializzo il frame di scrittura */
		cache->write = cache->start;
		/* inizializzo il frame di lettura */
		cache->read = (SBYTE *) cache->start;
		/* punto alla fine del buffer */
		cache->end = cache->read + buffer_size;

		/* punto all'inizio del frame di scrittura */
		snd.position = snd.last_position = 0;
		/* azzero completamente il buffer */
		memset(cache->start, 0, buffer_size);
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
BYTE sndCtrl(void) {
	SDL_AudioSpec *dev = snd.dev;
	_callbackData *cache = snd.cache;

	if (!cfg->audio) {
		return (FALSE);
	}

	/*
	 * calcolo qundo scrivere il dato nel frame
	 * di scrittura trattato.
	 */
	snd.position = snd.cycles++ / snd.frequency;

	if (snd.last_position == snd.position) {
		return (FALSE);
	}

	/*
	 * se la posizione e' maggiore o uguale al numero
	 * di samples che compongono il frame, vuol dire che
	 * sono passato nel frame successivo.
	 */
	if (snd.position >= dev->samples) {
		/*
		 * incremento il contatore dei frames pieni
		 * non ancora 'suonati'.
		 */
		SDL_LockAudio();
		cache->filled++;

		if (cache->filled <= 2) {
			snd.factor = 0.35;
			sndHz();
		} else if (cache->filled == 3) {
			snd.factor = 1.40;
			sndHz();
		} else if (cache->filled == 4) {
			snd.factor = 1.75;
			sndHz();
		} else {
			snd.on_play = FALSE;
			cache->filled = 0;
			cache->read = (SBYTE *) cache->start;
			cache->write = cache->start;
			snd.too_fast_sync++;
		}
		SDL_UnlockAudio();
		/* azzero posizione e contatore dei cicli del frame audio */
		snd.position = snd.cycles = 0;
		/*
		 * dopo aver portato il puntatore del frame
		 * trattato al frame successivo, controllo di
		 * non aver gia' riempito il buffer completo
		 * e se si, azzero il puntatore.
		 */
		/* swappo i buffers dei canali */
		if (dev->channels == STEREO) {
			SWORD *tmp = snd.channel_ptr[CH_RIGHT];
			snd.channel_ptr[CH_RIGHT] = snd.channel_ptr[CH_LEFT];
			snd.channel_ptr[CH_LEFT] = tmp;
		}
	}

	return (TRUE);
}
void sndWrite(SWORD data) {
	SDL_AudioSpec *dev = snd.dev;
	_callbackData *cache = snd.cache;

	/* mono or left*/
	(*cache->write++) = data;

	/* stereo */
	if (dev->channels == STEREO) {
		/* salvo il dato nel buffer del canale sinistro */
		snd.channel_ptr[CH_LEFT][snd.position] = data;
		/* scrivo nel nel frame audio il canale destro ritardato di un frame */
		(*cache->write++) = snd.channel_ptr[CH_RIGHT][snd.position];
	}

	if (cache->write >= (SWORD *) cache->end) {
		cache->write = cache->start;
	}

	snd.last_position = snd.position;
}
void sndOutput(void *udata, BYTE *stream, int len) {
	_callbackData *cache = udata;

#ifndef RELEASE
	SDL_AudioSpec *dev = snd.dev;

	if (info.no_rom) {
		return;
	}

	fprintf(stderr, "snd : %d %f %d %2d %2d %2d %f %d %d %.0f %4s\r", dev->samples, fps.avarage,
	        fps.total_frames_skipped, cache->filled,
	        (BYTE) ((cache->write - cache->start) / dev->samples),
	        (BYTE) (((SWORD *) cache->read - cache->start) / dev->samples), snd.frequency,
	        snd.total_out_of_sync, snd.too_fast_sync, snd.wmMs, "");

	/*
	 * in caso di spostamento della finestra o un resize
	 * problemi di sincronizzazzione sono normali, quindi
	 * non li considero per un tempo di X millisecondi.
	 */
	if (snd.wmMs) {
		if ((snd.wmMs != SNDNOSYNC) && ((guiGetMs() - snd.wmStart) > snd.wmMs)) {
			snd.wmMs = FALSE;
		}
		snd.out_of_sync = FALSE;
	}
#endif

	/*
	 * se non ci sono sufficienti frames pieni, allora
	 * blocco la riproduzione.
	 */
	if (snd.on_play) {
		if (cache->filled < PLAY_FRAMES) {
			memset(stream, snd.last_sample, len);
			/*
			 * attivo il flag che indica un potenziale
			 * errore di sicronizzazione.
			 */
			snd.out_of_sync = TRUE;
			/* disabilito l'output */
			snd.on_play = FALSE;
			return;
		}
	} else {
		/*
		 * non verra' riprodotto alcun suono fino a quando
		 * non si sara' riempita la cache.
		 */
		if (cache->filled < CACHE_FRAMES) {
			memset(stream, snd.last_sample, len);
			return;
		}
		/* gestione out of sync */
		if (snd.out_of_sync) {
			snd.total_out_of_sync++;
			snd.out_of_sync = FALSE;
		}
		snd.on_play = TRUE;
	}

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
	if ((cache->filled -= PLAY_FRAMES) < 0) {
		cache->filled = 0;
	}
}
void sndStop(void) {
	snd.on_play = FALSE;

	if (snd.opened) {
		SDL_PauseAudio(TRUE);
		SDL_CloseAudio();
		snd.opened = FALSE;
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
		free(snd.cache);
		snd.cache = NULL;
	}

	{
		BYTE i;

		for (i = 0; i < STEREO; i++) {
			if (snd.channel_buffer[i]) {
				free(snd.channel_buffer[i]);
				snd.channel_buffer[i] = NULL;
			}
			snd.channel_ptr[i] = NULL;
		}
	}
}
void sndQuit(void) {
	sndStop();
#ifndef RELEASE
	fprintf(stderr, "\n");
#endif
}
