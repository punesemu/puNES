/*
 * panning.h
 *
 *  Created on: 05 set 2015
 *      Author: fhorse
 */

#ifndef CHANNELS_STEREO_PANNING_H_
#define CHANNELS_STEREO_PANNING_H_

#include "common.h"

BYTE ch_stereo_panning_init(void);
void ch_stereo_panning_quit(void);
void ch_stereo_panning_tick(SWORD value);

#endif /* CHANNELS_STEREO_PANNING_H_ */
