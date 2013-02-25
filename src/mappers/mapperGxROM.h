/*
 * mapperGXROM.h
 *
 *  Created on: 16/lug/2011
 *      Author: fhorse
 */

#ifndef MAPPERGXROM_H_
#define MAPPERGXROM_H_

#include "common.h"

void map_init_GxROM(void);
void extcl_cpu_wr_mem_GxROM(WORD address, BYTE value);

#endif /* MAPPERGXROM_H_ */
