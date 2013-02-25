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
void extcl_cpu_wr_mem_Rex_dbz(WORD address, BYTE value);
BYTE extcl_cpu_rd_mem_Rex_dbz(WORD address, BYTE openbus, BYTE before);
BYTE extcl_save_mapper_Rex_dbz(BYTE mode, BYTE slot, FILE *fp);

#endif /* MAPPERREX_H_ */
