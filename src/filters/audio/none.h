/*
 * none.h
 *
 *  Created on: 11/lug/2012
 *      Author: fhorse
 */

#ifndef AUDIO_FILTER_NONE_H_
#define AUDIO_FILTER_NONE_H_

#include "common.h"

void audio_filter_init_none(void);
void audio_filter_apu_tick_none(void);
SWORD audio_filter_apu_mixer_none(void);

#endif /* AUDIO_FILTER_NONE_H_ */
