/*
 * gamegenie.h
 *
 *  Created on: 16/apr/2012
 *      Author: fhorse
 */

#ifndef GAMEGENIE_H_
#define GAMEGENIE_H_

#include <stdio.h>
#include "common.h"
#include "mappers/mapperGameGenie.h"

#define GG_CHEATS 3

enum GG_DATA {
	GG_ADDRESS_HIGH,
	GG_ADDRESS_LOW,
	GG_COMPARE,
	GG_REPLACE
};
enum GG_PHASE {
	GG_LOAD_ROM = 1,
	GG_EXECUTE,
	GG_FINISH,
	GG_LOAD_GAMEGENIE
};

typedef struct {
	BYTE disabled;
	BYTE enabled_compare;
	WORD address;
	BYTE replace;
	BYTE compare;
} _cheat;

struct {
	BYTE phase;
	BYTE rom_present;
	BYTE counter;
	BYTE print;
	BYTE value;
	_cheat cheat[GG_CHEATS];
} gamegenie;

void gamegenie_init(void);
void gamegenie_reset(BYTE print_message);
void gamegenie_check_rom_present(BYTE print_message);
FILE *gamegenie_load_rom(FILE *fp);

#endif /* GAMEGENIE_H_ */
