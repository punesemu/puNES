/*
 *  Copyright (C) 2010-2019 Fabio Cavallo (aka FHorse)
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

typedef struct _snd_dev {
	uTCHAR *desc;
	void *id;
} _snd_dev;
typedef struct _snd_list_dev {
	int count;
	_snd_dev *devices;
} _snd_list_dev;
typedef struct _callback_data {
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
typedef struct _snd {
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
EXTERNC struct _snd_list {
	_snd_list_dev playback;
	_snd_list_dev capture;
} snd_list;

EXTERNC BYTE snd_init(void);
EXTERNC void snd_quit(void);

EXTERNC BYTE snd_playback_start(void);
EXTERNC void snd_playback_lock(_callback_data *cache);
EXTERNC void snd_playback_unlock(_callback_data *cache);
EXTERNC void snd_playback_stop(void);
EXTERNC uTCHAR *snd_playback_device_desc(int dev);
EXTERNC uTCHAR *snd_playback_device_id(int dev);

EXTERNC uTCHAR *snd_capture_device_desc(int dev);
EXTERNC uTCHAR *snd_capture_device_id(int dev);

EXTERNC void snd_list_devices(void);

EXTERNC BYTE snd_handler(void);

EXTERNC void (*snd_apu_tick)(void);
EXTERNC void (*snd_end_frame)(void);

#undef EXTERNC

#endif /* SND_H_ */
