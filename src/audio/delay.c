/*
 *  Copyright (C) 2010-2022 Fabio Cavallo (aka FHorse)
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

#include <stdlib.h>
#include <string.h>
#include "audio/snd.h"
#include "audio/delay.h"
#include "audio/channels.h"
#include "conf.h"
#if defined (WITH_FFMPEG)
#include "recording.h"
#endif

enum delay_channels { CH_LEFT, CH_RIGHT };

static struct _delay {
	DBWORD samples;
	DBWORD max_pos;
	DBWORD pos;
	DBWORD size;
	SWORD *ptr[2];
	SWORD *buf[2];

	struct {
		SWORD *write;
		SWORD *start;
		SWORD *middle;
		SBYTE *end;
	} bck;
} delay;

BYTE ch_stereo_delay_init(void) {
	BYTE i;

	audio_channels_quit = ch_stereo_delay_quit;
	audio_channels_reset = ch_stereo_delay_reset;
	audio_channels_tick = ch_stereo_delay_tick;

	switch (snd.samplerate) {
		case 48000:
			delay.samples = ((snd.samplerate / (11025 / 128)) * 8) / 2;
			break;
		case 44100:
			delay.samples = (512 * 8) / 2;
			break;
		case 22050:
			delay.samples = (256 * 8) / 2;
			break;
		case 11025:
			delay.samples = (128 * 8) / 2;
			break;
	}

	delay.max_pos = (DBWORD)((double)delay.samples * cfg->stereo_delay);
	delay.pos = 0;
	delay.size = delay.samples * sizeof(*snd.cache->write);

	for (i = 0; i < 2; i++) {
		delay.buf[i] = (SWORD *)malloc(delay.size);
		memset(delay.buf[i], 0x00, delay.size);
		delay.ptr[i] = delay.buf[i];
	}

	delay.bck.start = (SWORD *)malloc(delay.size * 2);
	memset(delay.bck.start, 0x00, delay.size * 2);
	delay.bck.write = delay.bck.start;
	delay.bck.middle = delay.bck.start + delay.samples;
	delay.bck.end = (SBYTE *)delay.bck.start + (delay.size * 2);

	return (EXIT_OK);
}
void ch_stereo_delay_quit(void) {
	BYTE i;

	if (delay.bck.start) {
		free(delay.bck.start);
		delay.bck.start = NULL;
	}

	for (i = 0; i < 2; i++) {
		// rilascio la memoria
		if (delay.buf[i]) {
			free(delay.buf[i]);
		}

		// azzero i puntatori
		delay.ptr[i] = delay.buf[i] = NULL;
	}
}
void ch_stereo_delay_reset(void) {
	BYTE i;

	delay.pos = 0;

	for (i = 0; i < 2; i++) {
		memset(delay.buf[i], 0x00, delay.size);
		delay.ptr[i] = delay.buf[i];
	}

	memset(delay.bck.start, 0x00, delay.size * 2);
	delay.bck.write = delay.bck.start;
}
void ch_stereo_delay_tick(SWORD value) {
	SWORD actual[2] = { value, delay.ptr[CH_RIGHT][delay.pos] };

	// sinistro
	(*snd.cache->write++) = actual[0];

	// salvo il dato nel buffer del canale sinistro
	delay.ptr[CH_LEFT][delay.pos] = actual[0];

	// scrivo nel frame audio il canale destro ritardato rispetto al canale sinistro
	(*snd.cache->write++) = actual[1];

	// swappo i buffers dei canali
	if (++delay.pos >= delay.max_pos) {
		SWORD *swap = delay.ptr[CH_RIGHT];

		delay.ptr[CH_RIGHT] = delay.ptr[CH_LEFT];
		delay.ptr[CH_LEFT] = swap;
		delay.pos = 0;
	}

	(*delay.bck.write++) = actual[0];

	if (delay.bck.write >= (SWORD *)delay.bck.end) {
		delay.bck.write = delay.bck.start;
	}

	snd.cache->samples_available++;
	snd.cache->bytes_available += (2 * sizeof(*snd.cache->write));

#if defined (WITH_FFMPEG)
	if (info.recording_on_air) {
		recording_audio_tick(&actual[0]);
	}
#endif
}
void ch_stereo_delay_set(void) {
	SWORD *here;
	int i;

	delay.max_pos = (DBWORD)((double)delay.samples * cfg->stereo_delay);
	delay.pos = 0;

	for (i = 0; i < 2; i++) {
		delay.ptr[i] = delay.buf[i];
	}

	here = delay.bck.write - delay.max_pos;

	if (here >= delay.bck.start) {
		memcpy(delay.ptr[CH_RIGHT], here, delay.max_pos * sizeof(*snd.cache->write));
	} else {
		DBWORD step = delay.bck.start - here;
		SWORD *src1 = (SWORD *)delay.bck.end - step;
		SWORD *src2 = delay.bck.start;
		SWORD *dst1 = delay.ptr[CH_RIGHT];
		SWORD *dst2 = delay.ptr[CH_RIGHT] + step;

		memcpy(dst1, src1, step * sizeof(*snd.cache->write));
		memcpy(dst2, src2, (delay.max_pos - step) * sizeof(*snd.cache->write));
	}
}
