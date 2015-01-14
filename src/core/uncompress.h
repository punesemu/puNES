/*
 * uncompress.h
 *
 *  Created on: 17/dic/2013
 *      Author: fhorse
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

typedef struct {
	char ext[10];
	BYTE id;
} _format_supported;
typedef struct {
	uint32_t num;
	BYTE format;
} _uncomp_file_data;

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

static const _format_supported format_supported[] = {
	{ ".nes", FMT_NES },
	{ ".fds", FMT_FDS }
	//{ ".fm2", FMT_FM2 }
};

EXTERNC struct _uncomp {
	int files_founded;
	_uncomp_file_data *file;
	char compress_archive[LENGTH_FILE_NAME_MID];
	char uncompress_file[LENGTH_FILE_NAME_MID];
	char buffer[LENGTH_FILE_NAME_MID];
} uncomp;

EXTERNC BYTE uncomp_init(void);
EXTERNC void uncomp_quit(void);
EXTERNC BYTE uncomp_ctrl(char *ext);
EXTERNC BYTE uncomp_name_file(_uncomp_file_data *file);
EXTERNC void uncomp_remove(void);

#undef EXTERNC

#endif /* UNCOMPRESS_H_ */
