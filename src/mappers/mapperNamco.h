/*
 * mapperNamco.h
 *
 *  Created on: 13/set/2011
 *      Author: fhorse
 */

#ifndef MAPPERNAMCO_H_
#define MAPPERNAMCO_H_

#include "common.h"

enum {
	N163 = 2,
	N3416 = 3,
	N3425 = 4,
	N3433 = 5,
	N3446 = 6,
	N3453 = 7,
	NHARDWIREDV = 8,
	NHARDWIREDH = 9,
	MINDSEEKER = 10
};

typedef struct {
	BYTE enabled;
	BYTE active;
	WORD address;
	DBWORD freq;
	DBWORD cyclesReload;
	DBWORD cycles;
	BYTE length;
	BYTE step;
	WORD volume;
	SWORD output;
} _n163SndCh;
struct _n163 {
	uint32_t nmtBank[4][2];
	BYTE irqDelay;
	DBWORD irqCount;
	BYTE sndRam[0x80];
	BYTE sndAdr;
	BYTE sndAutoInc;
	BYTE sndChStart;
	BYTE sndWave[0x100];
	_n163SndCh ch[8];
} n163;
struct _n3425 {
	BYTE bankToUpdate;
} n3425;
struct _n3446 {
	BYTE bankToUpdate;
	BYTE prgRomMode;
} n3446;

void mapInit_Namco(BYTE model);

void extcl_cpu_wr_mem_Namco_163(WORD address, BYTE value);
BYTE extcl_cpu_rd_mem_Namco_163(WORD address, BYTE openbus, BYTE before);
BYTE extcl_save_mapper_Namco_163(BYTE mode, BYTE slot, FILE *fp);
void extcl_cpu_every_cycle_Namco_163(void);
void extcl_apu_tick_Namco_163(void);

void extcl_cpu_wr_mem_Namco_3425(WORD address, BYTE value);
BYTE extcl_save_mapper_Namco_3425(BYTE mode, BYTE slot, FILE *fp);

void extcl_cpu_wr_mem_Namco_3446(WORD address, BYTE value);
BYTE extcl_save_mapper_Namco_3446(BYTE mode, BYTE slot, FILE *fp);

#endif /* MAPPERNAMCO_H_ */
