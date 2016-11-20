/*
 *  Copyright (C) 2010-2016 Fabio Cavallo (aka FHorse)
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

#ifndef UNCOMPRESS_H_
#define UNCOMPRESS_H_

#include "common.h"

enum uncomp_formats {
	FMT_NES,
	FMT_FDS,
	FMT_FM2
};
enum uncomp_misc {
	UNCOMP_CTRL_FILE_COUNT_ROMS,
	UNCOMP_CTRL_FILE_SAVE_DATA,
	UNCOMP_NO_FILE_SELECTED = 0xFFFF
};

typedef struct _format_supported {
	uTCHAR ext[10];
	BYTE id;
} _format_supported;
typedef struct _uncomp_file_data {
	uint32_t num;
	BYTE format;
} _uncomp_file_data;

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

static const _format_supported format_supported[] = {
	{ uL(".nes"), FMT_NES },
	{ uL(".fds"), FMT_FDS }
	//{ uL(".fm2"), FMT_FM2 }
};

EXTERNC struct _uncomp {
	int files_founded;
	_uncomp_file_data *file;
	uTCHAR compress_archive[LENGTH_FILE_NAME_MID];
	uTCHAR uncompress_file[LENGTH_FILE_NAME_MID];
	uTCHAR buffer[LENGTH_FILE_NAME_MID];
} uncomp;

EXTERNC BYTE uncomp_init(void);
EXTERNC void uncomp_quit(void);
EXTERNC BYTE uncomp_ctrl(uTCHAR *ext);
EXTERNC BYTE uncomp_name_file(_uncomp_file_data *file);
EXTERNC void uncomp_remove(void);

#undef EXTERNC

#endif /* UNCOMPRESS_H_ */
