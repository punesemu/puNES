/*
 * tas.h
 *
 *  Created on: 30/gen/2012
 *      Author: fhorse
 */

#ifndef TAS_H_
#define TAS_H_

#include "common.h"

enum tas_types { NOTAS, FM2 };
enum tas_emulators { FCEUX, PUNES };
/* NTSC : 960 / 60 = 16 secondi */
enum tas_misc { TAS_CACHE = 960 };

typedef struct {
	BYTE state;
	BYTE port1[8];
	BYTE port2[8];
} _tas_input_log;
struct _tas {
	FILE *fp;
	char file[1024];
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

BYTE tas_file(char *ext, char *file);
void tas_quit(void);

void tas_header_FM2(char *file);
void tas_read_FM2(void);
void tas_frame_FM2(void);

void (*tas_header)(char *file);
void (*tas_read)(void);
void (*tas_frame)(void);

#endif /* TAS_H_ */
