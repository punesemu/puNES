/*
 * tas.h
 *
 *  Created on: 30/gen/2012
 *      Author: fhorse
 */

#ifndef TAS_H_
#define TAS_H_

#include <stdio.h>
#include "common.h"
#include "input.h"

enum tas_types { NOTAS, FM2 };
enum tas_emulators { FCEUX, PUNES };
/* NTSC : 960 / 60 = 16 secondi */
enum tas_misc { TAS_CACHE = 960 };

typedef struct {
	BYTE state;
	BYTE port[PORT_MAX][8];
} _tas_input_log;

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC struct _tas {
	FILE *fp;
	char file[LENGTH_FILE_NAME_MID];
	uint8_t emulator;
	uint8_t type;
	uint8_t add_fake_frame;
	uint8_t lag_frame;
	int32_t start_frame;
	uint32_t emu_version;
	uint32_t index;
	uint32_t count;
	uint32_t total;
	uint32_t frame;
	uint32_t total_lag_frames;
	_tas_input_log il[TAS_CACHE];
} tas;

EXTERNC BYTE tas_file(char *ext, char *file);
EXTERNC void tas_quit(void);

EXTERNC void tas_header_FM2(char *file);
EXTERNC void tas_read_FM2(void);
EXTERNC void tas_frame_FM2(void);

EXTERNC void (*tas_header)(char *file);
EXTERNC void (*tas_read)(void);
EXTERNC void (*tas_frame)(void);

#undef EXTERNC

#endif /* TAS_H_ */
