/*
 * mapperMMC3.h
 *
 *  Created on: 24/feb/2011
 *      Author: fhorse
 */

#ifndef MAPPERMMC3_H_
#define MAPPERMMC3_H_

#include "common.h"

#define swapChrBank1k(src, dst)\
{\
	BYTE *chrBank1k = chr.bank1k[src];\
	chr.bank1k[src] = chr.bank1k[dst];\
	chr.bank1k[dst] = chrBank1k;\
}

enum {
	NAMCO3413 = 2,
	NAMCO3414 = 3,
	NAMCO3415 = 4,
	NAMCO3416 = 5,
	NAMCO3417 = 6,
	NAMCO3451 = 7,
	TKROM     = 8,
	SMB2EREZA = 9,
	SMB2JSMB1 = 10,
	RADRACER2 = 11
};

struct _mmc3 {
	BYTE prgRamProtect;
	BYTE bankToUpdate;
	BYTE prgRomCfg;
	BYTE chrRomCfg;
} mmc3;

void mapInit_MMC3(void);
void extclCpuWrMem_MMC3(WORD address, BYTE value);
BYTE extclSaveMapper_MMC3(BYTE mode, BYTE slot, FILE *fp);
void extclPPU000to34x_MMC3(void);
void extclPPU000to255_MMC3(void);
void extclPPU256to319_MMC3(void);
void extclPPU320to34x_MMC3(void);
void extcl2006Update_MMC3(WORD r2006Old);

void extclCPUEveryCycle_MMC3(void);

#endif /* MAPPERMMC3_H_ */
