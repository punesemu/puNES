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

_recent_roms recent_roms_list;

void recent_roms_init(void);
void recent_roms_add(char *rom);
void recent_roms_parse(void);
void recent_roms_save(void);

#endif /* RECENT_ROMS_H_ */
