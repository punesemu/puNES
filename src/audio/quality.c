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

#include <stddef.h>
#include "audio/quality.h"
#include "snd.h"
#include "audio/original.h"
#include "audio/blipbuf.h"

void audio_quality(BYTE quality) {
	if (audio_quality_quit) {
		audio_quality_quit();
	}

	audio_quality_init = NULL;
	audio_quality_quit = NULL;

	snd_apu_tick = NULL;
	snd_end_frame = NULL;

	switch (quality) {
		default:
		case AQ_LOW:
			audio_quality_init = audio_quality_init_original;
			break;
		case AQ_HIGH:
			audio_quality_init = audio_quality_init_blipbuf;
			break;
	}

	if (audio_quality_init()) {
		/* fallback */
		audio_quality_init = audio_quality_init_original;
		audio_quality_init();
	}
}
