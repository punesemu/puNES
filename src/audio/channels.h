/*
 *  Copyright (C) 2010-2022 Fabio Cavallo (aka FHorse)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef CHANNELS_H_
#define CHANNELS_H_

#include "common.h"

enum types_of_audio_channels { CH_MONO, CH_STEREO_DELAY, CH_STEREO_PANNING };

extern BYTE (*audio_channels_init)(void);
extern void (*audio_channels_quit)(void);
extern void (*audio_channels_reset)(void);
extern void (*audio_channels_tick)(SWORD value);

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC void audio_channels(BYTE channels);
EXTERNC void audio_channels_init_mode(void);

#undef EXTERNC

#endif /* CHANNELS_H_ */
