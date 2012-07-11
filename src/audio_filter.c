/*
 * audio_filter.c
 *
 *  Created on: 11/lug/2012
 *      Author: fhorse
 */

#include "audio_filter.h"
#include "cfgfile.h"
#include "filters/audio/none.h"
#include "filters/audio/simple.h"

void audio_filter(void) {
	switch (cfg->audio_filter) {
		default:
		case AF_NONE:
			audio_filter_init = audio_filter_init_none;
			break;
		case AF_SIMPLE:
			audio_filter_init = audio_filter_init_simple;
			break;
	}
	audio_filter_init();
}


