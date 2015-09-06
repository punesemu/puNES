/*
 * audio_quality.c
 *
 *  Created on: 30/lug/2012
 *      Author: fhorse
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
