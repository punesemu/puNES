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
#include "mappers/mapper_GameGenie.h"

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

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC struct _gamegenie {
	BYTE phase;
	BYTE rom_present;
	BYTE counter;
	BYTE value;
	_cheat cheat[GG_CHEATS];
} gamegenie;

EXTERNC void gamegenie_init(void);
EXTERNC void gamegenie_reset(void);
EXTERNC void gamegenie_check_rom_present(BYTE print_message);
EXTERNC FILE *gamegenie_load_rom(FILE *fp);

#undef EXTERNC

#endif /* GAMEGENIE_H_ */
