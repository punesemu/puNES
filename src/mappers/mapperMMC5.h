/*
 * mapperMMC5.h
 *
 *  Created on: 26/ago/2011
 *      Author: fhorse
 */

#ifndef MAPPERMMC5_H_
#define MAPPERMMC5_H_

#include <stdio.h>
#include "common.h"
#include "apu.h"

enum {
	EKROM = 2,
	ELROM = 3,
	ETROM = 4,
	EWROM = 5
};

struct _mmc5 {
	BYTE prgMode;
	BYTE chrMode;
	BYTE extMode;
	BYTE nmtMode[4];
	BYTE prgRamWrite[2];
	BYTE prgBank[4];
	uint32_t prgRamBank[4][2];
	BYTE chrLast;
	WORD chrHigh;
	WORD chrS[8];
	WORD chrB[4];
	BYTE extRam[0x400];
	BYTE fillTable[0x400];
	BYTE fillTile;
	BYTE fillAttr;
	BYTE split;
	BYTE splitStTile;
	BYTE splitSide;
	BYTE splitScrl;
	BYTE splitInReg;
	BYTE splitX;
	BYTE splitY;
	WORD splitTile;
	uint32_t splitBank;
	BYTE factor[2];
	WORD product;
	_apuSquare S3, S4;
	BYTE pcmEnabled;
	BYTE pcmSample;
	BYTE pcmAmp;
	BYTE filler[50];
} mmc5;

void mapInit_MMC5(void);
void extclCpuWrMem_MMC5(WORD address, BYTE value);
BYTE extclCpuRdMem_MMC5(WORD address, BYTE openbus, BYTE before);
BYTE extclSaveMapper_MMC5(BYTE mode, BYTE slot, FILE *fp);
void extclPPU256to319_MMC5(void);
void extclPPU320to34x_MMC5(void);
void extclRdChrAfter_MMC5(WORD address);
BYTE extclRdChr_MMC5(WORD address);
BYTE extclRdNmt_MMC5(WORD address);
void extclLengthClock_MMC5(void);
void extclEnvelopeClock_MMC5(void);
void extclApuTick_MMC5(void);

#endif /* MAPPERMMC5_H_ */
