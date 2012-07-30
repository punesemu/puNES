/*
 * blip.h
 *
 *  Created on: 28/lug/2012
 *      Author: fhorse
 */

#ifndef AUDIO_FILTER_BLIP_H_
#define AUDIO_FILTER_BLIP_H_

#include "common.h"

void audio_filter_init_blip(void);
void audio_filter_apu_tick_blip(void);
void audio_filter_end_frame_blip(void);
SWORD audio_filter_apu_mixer_blip(void);

BYTE audio_filter_snd_write_blip(void);

#endif /* AUDIO_FILTER_BLIP_H_ */
