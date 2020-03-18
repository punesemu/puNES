/*
 *  Copyright (C) 2010-2020 Fabio Cavallo (aka FHorse)
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

#ifndef TAS_H_
#define TAS_H_

#include <stdio.h>
#include "common.h"
#include "input.h"

enum tas_types { NOTAS, FM2 };
enum tas_emulators { FCEUX, PUNES };
// NTSC : 960 / 60 = 16 secondi
enum tas_misc { TAS_CACHE = 960 };

typedef struct _tas_input_log {
	BYTE state;
	BYTE port[PORT_MAX][8];
} _tas_input_log;
typedef struct _tas {
	uTCHAR file[LENGTH_FILE_NAME_LONG];
	FILE *fp;
	uint8_t emulator;
	uint8_t type;
	uint8_t lag_next_frame;
	uint8_t lag_actual_frame;
	int32_t start_frame;
	uint32_t emu_version;
	int32_t index;
	int32_t count;
	int32_t total;
	int32_t frame;
	uint32_t total_lag_frames;
	_tas_input_log il[TAS_CACHE];
} _tas;

extern _tas tas;

extern void (*tas_header)(uTCHAR *file);
extern void (*tas_read)(void);
extern void (*tas_frame)(void);
extern void (*tas_rewind)(int32_t frames_to_rewind);

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC BYTE tas_file(uTCHAR *ext, uTCHAR *file);
EXTERNC void tas_quit(void);

EXTERNC void tas_header_FM2(uTCHAR *file);
EXTERNC void tas_read_FM2(void);
EXTERNC void tas_frame_FM2(void);
EXTERNC void tas_rewind_FM2(int32_t frames_to_rewind);

#undef EXTERNC

#endif /* TAS_H_ */
