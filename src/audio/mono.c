/*
 * mono.c
 *
 *  Created on: 05 set 2015
 *      Author: fhorse
 */

#include <math.h>
#include "snd.h"
#include "audio/mono.h"
#include "audio/channels.h"

BYTE ch_mono_init(void) {
	audio_channels_quit = ch_mono_quit;
	audio_channels_tick = ch_mono_tick;

	snd.channels = 1;

	return (EXIT_OK);
}
void ch_mono_quit(void) {}
void ch_mono_tick(SWORD value) {
	(*SNDCACHE->write++) = value;

	SNDCACHE->samples_available++;
	SNDCACHE->bytes_available += sizeof(*SNDCACHE->write);
}
