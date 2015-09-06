/*
 * channels.h
 *
 *  Created on: 05 set 2015
 *      Author: fhorse
 */

#ifndef CHANNELS_H_
#define CHANNELS_H_

#include "common.h"

enum types_of_audio_channels { CH_MONO, CH_STEREO_DELAY, CH_STEREO_PANNING };

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC void audio_channels(BYTE channels);
EXTERNC void audio_channels_init_mode(void);
EXTERNC BYTE (*audio_channels_init)(void);
EXTERNC void (*audio_channels_quit)(void);
EXTERNC void (*audio_channels_tick)(SWORD value);

#undef EXTERNC

#endif /* CHANNELS_H_ */
