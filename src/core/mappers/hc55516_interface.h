/*
 *  Copyright (C) 2010-2024 Fabio Cavallo (aka FHorse)
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

#ifndef HC55516_INTERFACE_H_
#define HC55516_INTERFACE_H_

#include <stdio.h>
#include "common.h"
#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

typedef void *hc55516;

EXTERNC hc55516 hc55516_create(uint32_t clock);
EXTERNC void hc55516_free(hc55516 h);
EXTERNC void hc55516_reset(hc55516 h);
EXTERNC void hc55516_start(hc55516 h);
EXTERNC void hc55516_clock_w(hc55516 h, int state);
EXTERNC void hc55516_digit_w(hc55516 h, int digit);
EXTERNC void hc55516_sound_stream_update(hc55516 h, int16_t *buffer, int samples);

#undef EXTERNC

#endif /* HC55516_INTERFACE_H_ */
