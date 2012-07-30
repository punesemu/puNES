/*
 * audio_quality.c
 *
 *  Created on: 30/lug/2012
 *      Author: fhorse
 */

#include "audio_quality.h"
#include "sdlsnd.h"
#include "filters/audio/original.h"
#include "filters/audio/blip.h"

void audio_quality(BYTE quality) {

	snd_write = sndWrite;

	audio_quality_end_frame = NULL;

	switch (quality) {
		default:
		case AQ_LOW:
			audio_quality_init = audio_quality_init_original;
			break;
		case AQ_HIGH:
			audio_quality_init = audio_quality_init_blip;
			break;
	}

	audio_quality_init();
}
