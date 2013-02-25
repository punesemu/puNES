/*
 * mapper183.h
 *
 *  Created on: 9/ott/2011
 *      Author: fhorse
 */

#ifndef MAPPER183_H_
#define MAPPER183_H_

#include "common.h"

struct _m183 {
	BYTE enabled;
	BYTE prescaler;
	BYTE count;
	BYTE delay;
	BYTE chrRomBank[8];
} m183;

void mapInit_183(void);
void extcl_cpu_wr_mem_183(WORD address, BYTE value);
BYTE extcl_cpu_rd_mem_183(WORD address, BYTE openbus, BYTE before);
BYTE extcl_save_mapper_183(BYTE mode, BYTE slot, FILE *fp);
void extcl_cpu_every_cycle_183(void);

#endif /* MAPPER183_H_ */
