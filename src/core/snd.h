/*
 *  Copyright (C) 2010-2017 Fabio Cavallo (aka FHorse)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef SND_H_
#define SND_H_

#include "common.h"

enum samplerate_mode { S44100, S22050, S11025, S48000 };

#define SNDCACHE ((_callback_data *) snd.cache)

typedef struct {
#if defined (__WIN32__)
	void *xa2buffer;
	void *xa2source;
#endif
	SWORD *silence;

	SWORD *start;
	SBYTE *end;

	SBYTE *read;
	SWORD *write;

	int32_t bytes_available;
	int32_t samples_available;

	void *lock;
} _callback_data;
typedef struct {
	DBWORD samplerate;
	BYTE channels;

	WORD samples;
	DBWORD out_of_sync;

	double frequency;
	double factor;

	void *cache;

	struct _buffer {
		BYTE start;

		DBWORD size;

		struct {
			DBWORD low;
			DBWORD high;
		} limit;
	} buffer;
} _snd;

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC _snd snd;

EXTERNC BYTE snd_init(void);
EXTERNC BYTE snd_start(void);
EXTERNC void snd_lock_cache(_callback_data *cache);
EXTERNC void snd_unlock_cache(_callback_data *cache);
EXTERNC void snd_stop(void);
EXTERNC void snd_quit(void);

EXTERNC BYTE snd_handler(void);

EXTERNC void (*snd_apu_tick)(void);
EXTERNC void (*snd_end_frame)(void);

#undef EXTERNC

#endif /* SND_H_ */
