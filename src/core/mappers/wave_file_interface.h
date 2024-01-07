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

#ifndef WAVE_FILE_INTERFACE_H_
#define WAVE_FILE_INTERFACE_H_

#include <stdio.h>
#include "common.h"
#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

typedef void *wfiles;

EXTERNC void wavefiles_clear(void);
EXTERNC void wavefiles_restart(int index);
EXTERNC int wavefiles_get_next_sample(int index);
EXTERNC BYTE wavefiles_is_finished(int index);

#undef EXTERNC

#endif /* WAVE_FILE_INTERFACE_H_ */
