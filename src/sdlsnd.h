/*
 * sdlsnd.h
 *
 *  Created on: 08/mar/2011
 *      Author: fhorse
 */

#ifndef SDLSND_H_
#define SDLSND_H_

#include <SDL.h>
#include <SDL_audio.h>
#include <SDL_thread.h>
#include "common.h"

enum { S44100, S22050, S11025 };
enum { MONO = 1, STEREO };
enum { CH_LEFT, CH_RIGHT };

struct _snd {
	BYTE opened;
	BYTE brk;

	SWORD last_sample;

	DBWORD cycles;
	DBWORD out_of_sync;

	WORD freq;

	double frequency;
	double factor;

	void *cache;
	void *dev;

	struct _position {
		DBWORD current;
		DBWORD last;
	} pos;

	struct _channel {
		DBWORD max_pos;
		DBWORD pos;
		SWORD *ptr[2];
		SWORD *buf[2];
	} channel;

	struct _buffer {
		DBWORD size;
		DBWORD count;
	} buffer;
} snd;

typedef struct {
	SWORD *start;
	SBYTE *end;

	SBYTE *read;
	SWORD *write;

	SWORD filled;

	SDL_mutex *lock;
} _callbackData;

enum { FCNORMAL, FCNONE };

#define snd_frequency(a)\
	if (snd.factor != a) {\
		snd.factor = a;\
		fps_machine_ms(snd.factor)\
	}

static const double sndFactor[3][2] = {
	{ 0.997, 1.1 }, { 1.0, 1.1 }, { 1.0, 1.1 }
};

BYTE sndInit(void);
BYTE sndStart(void);
void sndOutput(void *udata, BYTE *stream, int len);
void sndStop(void);
void sndQuit(void);

void (*snd_apu_tick)(void);
void (*snd_end_frame)(void);

#endif /* SDLSND_H_ */
