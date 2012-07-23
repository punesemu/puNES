/*
 * linear2.h
 *
 *  Created on: 11/lug/2012
 *      Author: fhorse
 */

#ifndef AUDIO_FILTER_LINEAR2_H_
#define AUDIO_FILTER_LINEAR2_H_

#include "common.h"

void audio_filter_init_linear2(void);
void audio_filter_apu_tick_linear2(void);
SWORD audio_filter_apu_mixer_linear2(void);

BYTE audio_filter_snd_write_linear2(void);

#endif /* AUDIO_FILTER_LINEAR2_H_ */
