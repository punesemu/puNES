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
#include "snd.h"
#include "audio/channels.h"
#include "audio/mono.h"
#include "audio/delay.h"
#include "audio/panning.h"

void audio_channels(BYTE channels) {
	if (audio_channels_quit) {
		audio_channels_quit();
	}

	audio_channels_init = NULL;
	audio_channels_quit = NULL;
	audio_channels_tick = NULL;

	switch (channels) {
		default:
		case CH_MONO:
			snd.channels = 1;
			audio_channels_init = ch_mono_init;
			break;
		case CH_STEREO_DELAY:
			snd.channels = 2;
			audio_channels_init = ch_stereo_delay_init;
			break;
		case CH_STEREO_PANNING:
			snd.channels = 2;
			audio_channels_init = ch_stereo_panning_init;
			break;
	}
}
void audio_channels_init_mode(void) {
	if (audio_channels_init()) {
		/* fallback */
		audio_channels(CH_MONO);
		audio_channels_init();
	}
}
