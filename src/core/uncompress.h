/*
 *  Copyright (C) 2010-2021 Fabio Cavallo (aka FHorse)
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

enum uncompress_type {
	UNCOMPRESS_TYPE_ROM,
	UNCOMPRESS_TYPE_PATCH,
	UNCOMPRESS_TYPE_ALL
};
enum uncompress_exit_states {
	UNCOMPRESS_EXIT_OK = EXIT_OK,
	UNCOMPRESS_EXIT_ERROR_ON_UNCOMP = EXIT_ERROR,
	UNCOMPRESS_EXIT_IS_NOT_COMP,
	UNCOMPRESS_EXIT_IS_COMP_BUT_NOT_SELECTED,
	UNCOMPRESS_EXIT_IS_COMP_BUT_NO_ITEMS
};
enum uncompress_misc {
	UNCOMPRESS_NO_FILE_SELECTED = 0xFFFF
};

typedef struct _uncompress_extension {
	BYTE type;
	uTCHAR e[10];
} _uncompress_extension;

typedef struct _uncompress_archive_item {
	BYTE type;
	uint32_t index;
} _uncompress_archive_item;
typedef struct _uncompress_archive_items_list {
	uint32_t count;
	_uncompress_archive_item *item;
} _uncompress_archive_items_list;
typedef struct _uncompress_archive_type_info {
	uint32_t count;
	uint32_t storage_index;
} _uncompress_archive_type_info;
typedef struct _uncompress_archive {
	uTCHAR *file;
	_uncompress_archive_items_list list;
	_uncompress_archive_type_info rom;
	_uncompress_archive_type_info patch;
} _uncompress_archive;

typedef struct _uncompress_storage_item {
	BYTE type;
	uTCHAR *archive;
	uTCHAR *file;
	uint32_t index;
} _uncompress_storage_item;
typedef struct _uncompress_storage {
	uint32_t count;
	_uncompress_storage_item *item;
} _uncompress_storage;

static const _uncompress_extension uncompress_exts[] = {
	{ UNCOMPRESS_TYPE_ROM, uL(".nes")  },
	{ UNCOMPRESS_TYPE_ROM, uL(".fds")  },
	{ UNCOMPRESS_TYPE_ROM, uL(".unf")  },
	{ UNCOMPRESS_TYPE_ROM, uL(".unif") },
	{ UNCOMPRESS_TYPE_ROM, uL(".nsf")  },
	{ UNCOMPRESS_TYPE_ROM, uL(".nsfe") },
	{ UNCOMPRESS_TYPE_PATCH, uL(".ips") },
	{ UNCOMPRESS_TYPE_PATCH, uL(".bps") },
	{ UNCOMPRESS_TYPE_PATCH, uL(".xdelta") }
};

extern _uncompress_storage uncstorage;

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC BYTE uncompress_init(void);
EXTERNC void uncompress_quit(void);

EXTERNC _uncompress_archive *uncompress_archive_alloc(uTCHAR *file, BYTE *rc);
EXTERNC void uncompress_archive_free(_uncompress_archive *archive) ;
EXTERNC uint32_t uncompress_archive_counter(_uncompress_archive *archive, BYTE type);
EXTERNC BYTE uncompress_archive_extract_file(_uncompress_archive *archive, BYTE type);
EXTERNC _uncompress_archive_item *uncompress_archive_find_item(_uncompress_archive *archive, uint32_t selected, BYTE type);
EXTERNC uTCHAR *uncompress_archive_extracted_file_name(_uncompress_archive *archive, BYTE type);
EXTERNC uTCHAR *uncompress_archive_file_name(_uncompress_archive *archive, uint32_t selected, BYTE type);

EXTERNC uTCHAR *uncompress_storage_archive_name(uTCHAR *file);
EXTERNC uint32_t uncompress_storage_add_to_list(_uncompress_archive *archive, _uncompress_archive_item *aitem, uTCHAR *file);

#undef EXTERNC

#endif /* UNCOMPRESS_H_ */
