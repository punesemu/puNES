/*
 * audio_quality.h
 *
 *  Created on: 30/lug/2012
 *      Author: fhorse
 */

#ifndef AUDIO_QUALITY_H_
#define AUDIO_QUALITY_H_

#include "common.h"

enum types_of_audio_quality { AQ_LOW, AQ_HIGH };

void audio_quality(BYTE quality);
BYTE (*audio_quality_init)(void);
void (*audio_quality_quit)(void);

#endif /* AUDIO_QUALITY_H_ */
