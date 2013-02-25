/*
 * mapperBxROM.h
 *
 *  Created on: 16/lug/2011
 *      Author: fhorse
 */

#ifndef MAPPERBXROM_H_
#define MAPPERBXROM_H_

#include "common.h"

enum { AVENINA001 = 2, BXROMUNL = 3 };

void map_init_BxROM(void);

void extcl_cpu_wr_mem_BxROM(WORD address, BYTE value);

void extcl_cpu_wr_mem_BxROM_UNL(WORD address, BYTE value);

void extcl_cpu_wr_mem_AveNina001(WORD address, BYTE value);

#endif /* MAPPERBXROM_H_ */
