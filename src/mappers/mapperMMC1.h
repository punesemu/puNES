/*
 * mapperMMC1.h
 *
 *  Created on: 18/mag/2010
 *      Author: fhorse
 */

#ifndef MAPPERMMC1_H_
#define MAPPERMMC1_H_

#include <stdio.h>
#include "common.h"

enum {
	SNROM = 2,
	SOROM = 3,
	SUROM = 4,
	SXROM = 5
};

struct _mmc1 {
	BYTE reg;
	BYTE pos;
	BYTE prgMode;
	BYTE chrMode;
	BYTE ctrl;
	BYTE chr0;
	BYTE chr1;
	BYTE prg0;
	BYTE reset;
	BYTE prgUpper;
} mmc1;

void mapInit_MMC1(void);
void extclCpuWrMem_MMC1(WORD address, BYTE value);
BYTE extclSaveMapper_MMC1(BYTE mode, BYTE slot, FILE *fp);

#endif /* MAPPERMMC1_H_ */
