/*
 * sdlsnd.h
 *
 *  Created on: 08/mar/2011
 *      Author: fhorse
 */

#ifndef SDLSND_H_
#define SDLSND_H_

#include "common.h"

#define SNDNOSYNC 0xFFFF
#define sndWmEvent(ms)\
	snd.wmMs = ms;\
	snd.wmStart = guiGetMs()

enum { S44100, S22050, S11025 };
enum { MONO = 1, STEREO };
enum { CH_LEFT, CH_RIGHT, CH_TOTAL };

struct _snd {
	BYTE opened;
	BYTE on_play;
	BYTE out_of_sync;
	WORD cycles;
	WORD position;
	WORD last_position;
	SWORD last_sample;
	SWORD *channel_buffer[2];
	SWORD *channel_ptr[2];
	DBWORD total_out_of_sync;
	DBWORD too_fast_sync;
	float frequency;
	float factor;
	double wmMs;
	double wmStart;
	void *cache;
	void *dev;
} snd;

BYTE sndInit(void);
BYTE sndStart(void);
BYTE sndCtrl(void);
void sndWrite(SWORD data);
void sndOutput(void *udata, BYTE *stream, int len);
void sndStop(void);
void sndQuit(void);

#endif /* SDLSND_H_ */
