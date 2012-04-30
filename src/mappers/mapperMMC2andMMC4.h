/*
 * mapperMMC2andMMC4.h
 *
 *  Created on: 10/lug/2011
 *      Author: fhorse
 */

#ifndef MAPPERMMC2ANDMMC4_H_
#define MAPPERMMC2ANDMMC4_H_

#include "common.h"

/* MMC4 */
enum { BADINESFWJ = 2 };

struct _mmc2and4 {
	BYTE regs[4];
	BYTE latch0;
	BYTE latch1;
} mmc2and4;

void mapInit_MMC2and4(void);
void extclCpuWrMem_MMC2and4(WORD address, BYTE value);
BYTE extclSaveMapper_MMC2and4(BYTE mode, BYTE slot, FILE *fp);
void extclRdChrAfter_MMC2and4(WORD address);

#endif /* MAPPERMMC2ANDMMC4_H_ */
