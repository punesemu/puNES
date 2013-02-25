/*
 * mapper217.h
 *
 *  Created on: 23/mar/2012
 *      Author: fhorse
 */

#ifndef MAPPER217_H_
#define MAPPER217_H_

#include "common.h"

struct _m217 {
	BYTE reg[4];
	WORD prg8kBank[4];
} m217;

void mapInit_217(void);
void extcl_cpu_wr_mem_217(WORD address, BYTE value);
BYTE extcl_save_mapper_217(BYTE mode, BYTE slot, FILE *fp);

#endif /* MAPPER217_H_ */
