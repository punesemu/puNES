/*
 * mono.h
 *
 *  Created on: 05 set 2015
 *      Author: fhorse
 */

#ifndef CHANNELS_MONO_H_
#define CHANNELS_MONO_H_

#include "common.h"

BYTE ch_mono_init(void);
void ch_mono_quit(void);
void ch_mono_tick(SWORD value);

#endif /* CHANNELS_MONO_H_ */
