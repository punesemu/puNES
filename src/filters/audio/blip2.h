/*
 * blip2.h
 *
 *  Created on: 28/lug/2012
 *      Author: fhorse
 */

#ifndef AUDIO_QUALITY_BLIP2_H_
#define AUDIO_QUALITY_BLIP2_H_

#include "common.h"

BYTE audio_quality_init_blip2(void);
void audio_quality_quit_blip2(void);
void audio_quality_apu_tick_blip2(void);
void audio_quality_end_frame_blip2(void);

#endif /* AUDIO_QUALITY_BLIP2_H_ */
