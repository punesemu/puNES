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

#include "audio/snd.h"
#include "audio/mono.h"
#include "audio/channels.h"
#if defined (WITH_FFMPEG)
#include "recording.h"
#endif

BYTE ch_mono_init(void) {
	audio_channels_quit = ch_mono_quit;
	audio_channels_reset = ch_mono_reset;
	audio_channels_tick = ch_mono_tick;

	snd.channels = 1;

	return (EXIT_OK);
}
void ch_mono_quit(void) {}
void ch_mono_reset(void) {}
void ch_mono_tick(SWORD value) {
	(*snd.cache->write++) = value;

	snd.cache->samples_available++;
	snd.cache->bytes_available += sizeof(*snd.cache->write);

#if defined (WITH_FFMPEG)
	if (info.recording_on_air) {
		recording_audio_tick(&value);
	}
#endif
}
