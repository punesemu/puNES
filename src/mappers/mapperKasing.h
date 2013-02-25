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
void extcl_cpu_wr_mem_Kasing(WORD address, BYTE value);
BYTE extcl_save_mapper_Kasing(BYTE mode, BYTE slot, FILE *fp);

#endif /* MAPPERKASING_H_ */
