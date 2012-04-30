/*
 * mapperKasing.h
 *
 *  Created on: 28/set/2011
 *      Author: fhorse
 */

#ifndef MAPPERKASING_H_
#define MAPPERKASING_H_

#include "common.h"

struct _kasing {
	BYTE prgMode;
	BYTE prgHigh;
	WORD prgRomBank[4];
	BYTE chrHigh;
	WORD chrRomBank[8];
} kasing;

void mapInit_Kasing(void);
void extclCpuWrMem_Kasing(WORD address, BYTE value);
BYTE extclSaveMapper_Kasing(BYTE mode, BYTE slot, FILE *fp);

#endif /* MAPPERKASING_H_ */
