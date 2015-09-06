/*
 * channels.c
 *
 *  Created on: 05 set 2015
 *      Author: fhorse
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
