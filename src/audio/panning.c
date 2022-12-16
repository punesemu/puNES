/*
 *  Copyright (C) 2010-2023 Fabio Cavallo (aka FHorse)
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

#include <math.h>
#include "audio/snd.h"
#include "audio/panning.h"
#include "audio/channels.h"
#if defined (WITH_FFMPEG)
#include "recording.h"
#endif

#define ANG 165.0f

static struct _panning {
	float angle;
	float sq;
	float cs;
	float si;
} panning;

BYTE ch_stereo_panning_init(void) {
	audio_channels_quit = ch_stereo_panning_quit;
	audio_channels_reset = ch_stereo_panning_reset;
	audio_channels_tick = ch_stereo_panning_tick;

	snd.channels = 2;

	panning.angle = (ANG * (float)M_PI) / 180.0f;
	//panning.sq = sqrtf(2.0f) / 2.0f;
	panning.sq = 1.0f;
	panning.cs = cosf(panning.angle);
	panning.si = sinf(panning.angle);

	return (EXIT_OK);
}
void ch_stereo_panning_quit(void) {}
void ch_stereo_panning_reset(void) {}
void ch_stereo_panning_tick(SWORD value) {
	float mixer = (float)value / 65535.0f;
	SWORD actual[2] = {
		(SWORD)((panning.sq * (panning.cs - panning.si) * mixer) * 65535.0f),
		(SWORD)((panning.sq * (panning.cs + panning.si) * mixer) * 65535.0f)
	};

	// sinistro
	(*snd.cache->write++) = actual[0];
	// destro
	(*snd.cache->write++) = actual[1];

	snd.cache->samples_available++;
	snd.cache->bytes_available += (2 * sizeof(*snd.cache->write));

#if defined (WITH_FFMPEG)
	if (info.recording_on_air) {
		recording_audio_tick(&actual[0]);
	}
#endif
}
