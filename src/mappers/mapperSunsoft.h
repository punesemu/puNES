/*
 * mapperSunsoft.h
 *
 *  Created on: 18/set/2011
 *      Author: fhorse
 */

#ifndef MAPPERSUNSOFT_H_
#define MAPPERSUNSOFT_H_

#include "common.h"

enum {
	SUN1 = 2,
	SUN2A = 3,
	SUN2B = 4,
	SUN3 = 5,
	SUN4 = 6,
	FM7 = 7,
	MAHARAJA = 8,
	BARCODEWORLD = 9,
	DODGEDANPEI2 = 10
};

struct _sunsoft3 {
	BYTE enable;
	BYTE toggle;
	WORD count;
	BYTE delay;
} s3;
struct _sunsoft4 {
	uint32_t chrNmt[2];
	BYTE mode;
	BYTE mirroring;
} s4;

typedef struct {
	BYTE disable;
	BYTE step;
	WORD frequency;
	WORD timer;
	WORD volume;
	SWORD output;

/* ------------------------------------------------------- */
/* questi valori non e' necessario salvarli nei savestates */
/* ------------------------------------------------------- */
/* */ BYTE clocked;                                     /* */
/* ------------------------------------------------------- */
} _squareFM7;

struct sunsoftFm {
	BYTE address;
	BYTE prgRamEnable;
	BYTE prgRamMode;
	uint32_t prgRamAddress;
	BYTE irqEnableTrig;
	BYTE irqEnableCount;
	WORD irqCount;
	BYTE irqDelay;
	BYTE sndReg;
	_squareFM7 square[3];
} fm7;

void map_init_Sunsoft(BYTE model);

void extcl_cpu_wr_mem_Sunsoft_S1(WORD address, BYTE value);

void extcl_cpu_wr_mem_Sunsoft_S2(WORD address, BYTE value);

void extcl_cpu_wr_mem_Sunsoft_S3(WORD address, BYTE value);
BYTE extcl_save_mapper_Sunsoft_S3(BYTE mode, BYTE slot, FILE *fp);
void extcl_cpu_every_cycle_Sunsoft_S3(void);

void extcl_cpu_wr_mem_Sunsoft_S4(WORD address, BYTE value);
BYTE extcl_save_mapper_Sunsoft_S4(BYTE mode, BYTE slot, FILE *fp);

void extcl_cpu_wr_mem_Sunsoft_FM7(WORD address, BYTE value);
BYTE extcl_cpu_rd_mem_Sunsoft_FM7(WORD address, BYTE openbus, BYTE before);
BYTE extcl_save_mapper_Sunsoft_FM7(BYTE mode, BYTE slot, FILE *fp);
void extcl_cpu_every_cycle_Sunsoft_FM7(void);
void extcl_apu_tick_Sunsoft_FM7(void);

#endif /* MAPPERSUNSOFT_H_ */
