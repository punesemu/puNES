/*
 *  Copyright (C) 2010-2026 Fabio Cavallo (aka FHorse)
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

enum snd_samplerate_mode { S44100, S22050, S11025, S48000, S96000, S192000 };
enum snd_thread_actions { ST_UNINITIALIZED, ST_RUN, ST_STOP, ST_PAUSE };

typedef struct _snd_dev {
	uTCHAR *desc;
	void *id;
} _snd_dev;
typedef struct _snd_list_dev {
	int count;
	_snd_dev *devices;
} _snd_list_dev;
typedef struct _callback_data {
	SWORD *silence;

	SWORD *start;
	SBYTE *end;

	SBYTE *read;
	SWORD *write;

	int32_t samples_available;
	int32_t bytes_available;
} _callback_data;
typedef struct _snd {
	BYTE initialized;

	int32_t samplerate;
	BYTE channels;

	DBWORD overlap;
	DBWORD out_of_sync;

	double frequency;
	double factor;

	_callback_data *cache;

	int32_t pause_calls;

	struct _period {
#if defined (_WIN32)
		uint32_t samples;
#else
		int32_t samples;
#endif
		uint32_t size;
	} period;

	struct _buffer {
		BYTE start;

		int32_t size;

		struct {
			DBWORD low;
			DBWORD high;
		} limit;
	} buffer;
} _snd;
typedef struct _snd_list {
	_snd_list_dev playback;
	_snd_list_dev capture;
} _snd_list;

extern _snd snd;
extern _snd_list snd_list;

extern void (*snd_apu_tick)(void);
extern void (*snd_end_frame)(void);

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC BYTE snd_init(void);
EXTERNC void snd_quit(void);

EXTERNC void snd_reset_buffers(void);

EXTERNC void snd_thread_pause(void);
EXTERNC void snd_thread_continue(void);

EXTERNC void snd_thread_lock(void);
EXTERNC void snd_thread_unlock(void);

EXTERNC BYTE snd_playback_start(void);
EXTERNC void snd_playback_stop(void);

EXTERNC void snd_playback_pause(void);
EXTERNC void snd_playback_continue(void);

EXTERNC uTCHAR *snd_playback_device_desc(int dev);
EXTERNC uTCHAR *snd_playback_device_id(int dev);

EXTERNC uTCHAR *snd_capture_device_desc(int dev);
EXTERNC uTCHAR *snd_capture_device_id(int dev);

EXTERNC void snd_list_devices(void);

EXTERNC BYTE snd_handler(void);

#undef EXTERNC

#endif /* SND_H_ */
