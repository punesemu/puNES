/*
 * blip.h
 *
 *  Created on: 28/lug/2012
 *      Author: fhorse
 */

#ifndef AUDIO_QUALITY_BLIP_H_
#define AUDIO_QUALITY_BLIP_H_

#include "common.h"

BYTE audio_quality_init_blip(void);
void audio_quality_quit_blip(void);
void audio_quality_apu_tick_blip(void);
void audio_quality_end_frame_blip(void);

#endif /* AUDIO_QUALITY_BLIP_H_ */
