/*
 * original.h
 *
 *  Created on: 11/lug/2012
 *      Author: fhorse
 */

#ifndef AUDIO_FILTER_ORIGINAL_H_
#define AUDIO_FILTER_ORIGINAL_H_

#include "common.h"

void audio_filter_init_original(void);
void audio_filter_apu_tick_original(void);
SWORD audio_filter_apu_mixer_original(void);

#endif /* AUDIO_FILTER_ORIGINAL_H_ */
