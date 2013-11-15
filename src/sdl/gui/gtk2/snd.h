/*
 * snd.h
 *
 *  Created on: 08/mar/2011
 *      Author: fhorse
 */

#ifndef SND_H_
#define SND_H_

#include "common.h"

enum samplerate_mode { S44100, S22050, S11025 };
enum channel_mode { MONO = 1, STEREO };
enum channels { CH_LEFT, CH_RIGHT };
enum snd_factor_type { SND_FACTOR_SPEED, SND_FACTOR_NORMAL, SND_FACTOR_SLOW };

#define STEREO_DELAY_DEFAULT 0.3f
#define snd_frequency(a)\
	if (snd.factor != a) {\
		snd.factor = a;\
		fps_machine_ms(snd.factor)\
	}

typedef struct {
	SWORD *start;
	SBYTE *end;

	SBYTE *read;
	SWORD *write;

	SWORD filled;

	void *lock;
} _callback_data;
struct _snd {
	BYTE opened;
	BYTE brk;

	WORD samples;
	DBWORD samplerate;

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
		struct _bck {
			SWORD *write;
			SWORD *start;
			SWORD *middle;
			SBYTE *end;
		} bck;
	} channel;
	struct _buffer {
		DBWORD size;
		DBWORD count;
	} buffer;
} snd;

static const double snd_factor[3][3] = {
	//{ 0.967f, 0.998f, 1.1f }, { 0.967f, 1.0f, 1.1f }, { 0.967f, 1.0f, 1.1f }
	{ 0.967f, 0.998f, 1.1f }, { 0.960f, 0.992f, 1.1f }, { 0.960f, 0.992f, 1.1f }
};

BYTE snd_init(void);
BYTE snd_start(void);
void snd_output(void *udata, BYTE *stream, int len);
void snd_lock_cache(_callback_data *cache);
void snd_unlock_cache(_callback_data *cache);
void snd_stop(void);
void snd_quit(void);
void snd_stereo_delay(void);

void (*snd_apu_tick)(void);
void (*snd_end_frame)(void);

#endif /* SND_H_ */
