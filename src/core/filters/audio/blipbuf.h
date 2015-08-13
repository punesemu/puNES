/*
 * blipbuf.h
 *
 *  Created on: 28/lug/2012
 *      Author: fhorse
 */

#ifndef AUDIO_QUALITY_BLIPBUF_H_
#define AUDIO_QUALITY_BLIPBUF_H_

#include "common.h"

BYTE audio_quality_init_blipbuf(void);
void audio_quality_quit_blipbuf(void);
void audio_quality_apu_tick_blipbuf(void);
void audio_quality_end_frame_blipbuf(void);

#endif /* AUDIO_QUALITY_BLIPBUF_H_ */
