/*
 * mapperWhirlwind.h
 *
 *  Created on: 15/set/2011
 *      Author: fhorse
 */

#ifndef MAPPERWHIRLWIND_H_
#define MAPPERWHIRLWIND_H_

#include "common.h"

struct _whirlwind {
	uint32_t prgRam;
} whirlwind;

void mapInit_Whirlwind(void);
void extclCpuWrMem_Whirlwind(WORD address, BYTE value);
BYTE extclCpuRdMem_Whirlwind(WORD address, BYTE openbus, BYTE before);
BYTE extclSaveMapper_Whirlwind(BYTE mode, BYTE slot, FILE *fp);

#endif /* MAPPERWHIRLWIND_H_ */
