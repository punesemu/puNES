/*
 * linear.h
 *
 *  Created on: 11/lug/2012
 *      Author: fhorse
 */

#ifndef AUDIO_FILTER_LINEAR_H_
#define AUDIO_FILTER_LINEAR_H_

#include "common.h"

void audio_filter_init_linear(void);
void audio_filter_apu_tick_linear(void);
SWORD audio_filter_apu_mixer_linear(void);

#endif /* AUDIO_FILTER_LINEAR_H_ */
