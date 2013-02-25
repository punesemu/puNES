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
	struct {
		BYTE enabled;
		BYTE output;
		BYTE amp;

	/* ------------------------------------------------------- */
	/* questi valori non e' necessario salvarli nei savestates */
	/* ------------------------------------------------------- */
	/* */ BYTE clocked;                                     /* */
	/* ------------------------------------------------------- */
	} pcm;
	BYTE filler[50];
} mmc5;

void mapInit_MMC5(void);
void extcl_cpu_wr_mem_MMC5(WORD address, BYTE value);
BYTE extcl_cpu_rd_mem_MMC5(WORD address, BYTE openbus, BYTE before);
BYTE extcl_save_mapper_MMC5(BYTE mode, BYTE slot, FILE *fp);
void extcl_ppu_256_to_319_MMC5(void);
void extcl_ppu_320_to_34x_MMC5(void);
void extcl_after_rd_chr_MMC5(WORD address);
BYTE extcl_rd_chr_MMC5(WORD address);
BYTE extcl_rd_nmt_MMC5(WORD address);
void extcl_length_clock_MMC5(void);
void extcl_envelope_clock_MMC5(void);
void extcl_apu_tick_MMC5(void);

#endif /* MAPPERMMC5_H_ */
