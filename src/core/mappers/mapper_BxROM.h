/*
 * mapper_BxROM.h
 *
 *  Created on: 16/lug/2011
 *      Author: fhorse
 */

#ifndef MAPPER_BXROM_H_
#define MAPPER_BXROM_H_

#include "common.h"

enum bxroms_types { AVENINA001, BXROMUNL, BXROMBC };

void map_init_BxROM(void);

void extcl_cpu_wr_mem_BxROM(WORD address, BYTE value);

void extcl_cpu_wr_mem_BxROM_UNL(WORD address, BYTE value);

void extcl_cpu_wr_mem_AveNina001(WORD address, BYTE value);

#endif /* MAPPER_BXROM_H_ */
