/*
 *  Copyright (C) 2010-2023 Fabio Cavallo (aka FHorse)
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

#ifndef TAPE_DATA_RECORDER_H_
#define TAPE_DATA_RECORDER_H_

#include <stdio.h>
#include "common.h"
#include "vector.h"

enum _tape_data_recorder_mode {
	TAPE_DATA_NONE,
	TAPE_DATA_PLAY,
	TAPE_DATA_RECORD
};
enum _tape_data_recorder_file_types {
	TAPE_DATA_TYPE_TAP,
	TAPE_DATA_TYPE_VIRTUANES,
	TAPE_DATA_TYPE_NESTOPIA,
	TAPE_DATA_TYPE_WAV
};

typedef struct _tape_data_recorder {
	BYTE enabled;
	BYTE mode;
	BYTE type;
	BYTE in;
	BYTE out;
	size_t bits;
	size_t index;
	double cycles;
	_vector data;

	FILE *file;
	void (*tick)(void);
} _tape_data_recorder;

extern _tape_data_recorder tape_data_recorder;

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC BYTE tape_data_recorder_init(uTCHAR *file, BYTE type, BYTE mode);
EXTERNC void tape_data_recorder_quit(void);
EXTERNC void tape_data_recorder_stop(void);
EXTERNC void tape_data_recorder_tick(void);

#undef EXTERNC

#endif /* TAPE_DATA_RECORDER_H_ */
