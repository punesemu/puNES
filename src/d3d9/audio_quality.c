/*
 * audio_quality.c
 *
 *  Created on: 30/lug/2012
 *      Author: fhorse
 */

#include <stddef.h>
#include "audio_quality.h"
#include "snd.h"
#include "filters/audio/original.h"
//#include "filters/audio/blip.h"
//#include "filters/audio/blip2.h"

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
			//audio_quality_init = audio_quality_init_original;
			break;
		case AQ_HIGH:
			audio_quality_init = audio_quality_init_original;
			//audio_quality_init = audio_quality_init_blip;
			//audio_quality_init = audio_quality_init_blip2;
			break;
	}

	if (audio_quality_init()) {
		/* fallback */
		audio_quality_init = audio_quality_init_original;
		audio_quality_init();
	}
}
