/*
 * mapper221.h
 *
 *  Created on: 14/feb/2012
 *      Author: fhorse
 */

#ifndef MAPPER221_H_
#define MAPPER221_H_

#include "common.h"

struct _m221 {
	BYTE reg[2];
} m221;

void mapInit_221(void);
void extcl_cpu_wr_mem_221(WORD address, BYTE value);
BYTE extcl_save_mapper_221(BYTE mode, BYTE slot, FILE *fp);

#endif /* MAPPER221_H_ */
