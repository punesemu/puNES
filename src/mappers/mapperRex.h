/*
 * mapperRex.h
 *
 *  Created on: 11/set/2011
 *      Author: fhorse
 */

#ifndef MAPPERREX_H_
#define MAPPERREX_H_

#include "common.h"

enum { DBZ = 2 };

struct _rexDbz {
	WORD chrRomBank[8];
	BYTE chrHigh;
} rexDbz;

void mapInit_Rex(BYTE model);
void extclCpuWrMem_Rex_dbz(WORD address, BYTE value);
BYTE extclCpuRdMem_Rex_dbz(WORD address, BYTE openbus, BYTE before);
BYTE extclSaveMapper_Rex_dbz(BYTE mode, BYTE slot, FILE *fp);

#endif /* MAPPERREX_H_ */
