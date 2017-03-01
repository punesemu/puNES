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

#include <math.h>
#include "snd.h"
#include "audio/panning.h"
#include "audio/channels.h"

#define ANG 165.0f

static struct _panning {
	float angle;
	float sq;
	float cs;
	float si;
} panning;

BYTE ch_stereo_panning_init(void) {
	audio_channels_quit = ch_stereo_panning_quit;
	audio_channels_tick = ch_stereo_panning_tick;

	snd.channels = 2;

	panning.angle = (ANG * M_PI) / 180.0f;
	//panning.sq = sqrtf(2.0f) / 2.0f;
	panning.sq = 1.0f;
	panning.cs = cosf(panning.angle);
	panning.si = sinf(panning.angle);

	return (EXIT_OK);
}
void ch_stereo_panning_quit(void) {}
void ch_stereo_panning_tick(SWORD value) {
	float mixer = (float) value / 65535.0f;

	// sinistro
	(*SNDCACHE->write++) = (panning.sq * (panning.cs - panning.si) * mixer) * 65535.0f;
	// destro
	(*SNDCACHE->write++) = (panning.sq * (panning.cs + panning.si) * mixer) * 65535.0f;

	SNDCACHE->samples_available++;
	SNDCACHE->bytes_available += (2 * sizeof(*SNDCACHE->write));
}
