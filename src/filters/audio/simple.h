/*
 * simple.h
 *
 *  Created on: 11/lug/2012
 *      Author: fhorse
 */

#ifndef AUDIO_FILTER_SIMPLE_H_
#define AUDIO_FILTER_SIMPLE_H_

#include "common.h"

void audio_filter_init_simple(void);
void audio_filter_apu_tick_simple(void);
SWORD audio_filter_apu_mixer_simple(void);

#endif /* AUDIO_FILTER_SIMPLE_H_ */
