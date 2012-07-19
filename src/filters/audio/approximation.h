/*
 * approximation.h
 *
 *  Created on: 11/lug/2012
 *      Author: fhorse
 */

#ifndef AUDIO_FILTER_APPROXIMATION_H_
#define AUDIO_FILTER_APPROXIMATION_H_

#include "common.h"

void audio_filter_init_approximation(void);
void audio_filter_apu_tick_approximation(void);
SWORD audio_filter_apu_mixer_approximation(void);

#endif /* AUDIO_FILTER_APPROXIMATION_H_ */
