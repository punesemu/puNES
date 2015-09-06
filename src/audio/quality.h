/*
 * quality.h
 *
 *  Created on: 30/lug/2012
 *      Author: fhorse
 */

#ifndef AUDIO_QUALITY_H_
#define AUDIO_QUALITY_H_

#include "common.h"

enum types_of_audio_quality { AQ_LOW, AQ_HIGH };

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC void audio_quality(BYTE quality);
EXTERNC BYTE (*audio_quality_init)(void);
EXTERNC void (*audio_quality_quit)(void);

#undef EXTERNC

#endif /* AUDIO_QUALITY_H_ */
