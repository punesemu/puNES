/*
 * mapperVRC2.h
 *
 *  Created on: 10/set/2011
 *      Author: fhorse
 */

#ifndef MAPPERVRC2_H_
#define MAPPERVRC2_H_

#include "common.h"

enum { VRC2B, VRC2A };

struct _vrc2 {
	BYTE chrRomBank[8];
} vrc2;

void mapInit_VRC2(BYTE revision);
void extclCpuWrMem_VRC2(WORD address, BYTE value);
BYTE extclSaveMapper_VRC2(BYTE mode, BYTE slot, FILE *fp);

#endif /* MAPPERVRC2_H_ */
