/*
 * panning.c - constant power panning
 *
 *  Created on: 05 set 2015
 *      Author: fhorse
 */

#include <math.h>
#include "snd.h"
#include "audio/panning.h"
#include "audio/channels.h"

#define ANG 165.0f

struct _panning {
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
