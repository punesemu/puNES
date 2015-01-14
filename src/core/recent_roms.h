/*
 * recent_roms.h
 *
 *  Created on: 06/nov/2013
 *      Author: fhorse
 */

#ifndef RECENT_ROMS_H_
#define RECENT_ROMS_H_

#include "common.h"

enum recent_roms_misc {
	RECENT_ROMS_MAX = 15,
	RECENT_ROMS_LINE = 1024
};

typedef struct {
	BYTE count;
	char item[RECENT_ROMS_MAX][RECENT_ROMS_LINE];
	char current[RECENT_ROMS_LINE];
} _recent_roms;

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC _recent_roms recent_roms_list;

EXTERNC void recent_roms_init(void);
EXTERNC void recent_roms_add(char *rom);
EXTERNC void recent_roms_parse(void);
EXTERNC void recent_roms_save(void);

#undef EXTERNC

#endif /* RECENT_ROMS_H_ */
