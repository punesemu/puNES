/*
 * sdlsnd.h
 *
 *  Created on: 08/mar/2011
 *      Author: fhorse
 */

#ifndef SDLSND_H_
#define SDLSND_H_

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

	float frequency;
	float factor;

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

BYTE sndInit(void);
BYTE sndStart(void);
BYTE sndWrite(void);
void sndOutput(void *udata, BYTE *stream, int len);
void sndStop(void);
void sndQuit(void);

#endif /* SDLSND_H_ */
