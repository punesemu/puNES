/*
 * delay.h
 *
 *  Created on: 06 set 2015
 *      Author: fhorse
 */

#ifndef CHANNELS_STEREO_DELAY_H_
#define CHANNELS_STEREO_DELAY_H_

#include "common.h"

#define STEREO_DELAY_DEFAULT 0.3f

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC BYTE ch_stereo_delay_init(void);
EXTERNC void ch_stereo_delay_quit(void);
EXTERNC void ch_stereo_delay_tick(SWORD value);
EXTERNC void ch_stereo_delay_set(void);

#undef EXTERNC

#endif /* CHANNELS_STEREO_DELAY_H_ */
