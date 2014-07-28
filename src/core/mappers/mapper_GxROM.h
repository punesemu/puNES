/*
 * mapperGXROM.h
 *
 *  Created on: 16/lug/2011
 *      Author: fhorse
 */

#ifndef MAPPER_GXROM_H_
#define MAPPER_GXROM_H_

#include "common.h"

void map_init_GxROM(void);
void extcl_cpu_wr_mem_GxROM(WORD address, BYTE value);

#endif /* MAPPER_GXROM_H_ */
