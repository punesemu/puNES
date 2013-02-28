/*
 * mapper_UxROM.h
 *
 *  Created on: 18/mag/2010
 *      Author: fhorse
 */

#ifndef MAPPER_UXROM_H_
#define MAPPER_UXROM_H_

#include "common.h"

enum { UXROM = 2, UNL1XROM, UNROM180, UNLROM, BAD_INES_BOTBE };

void map_init_UxROM(BYTE model);

void extcl_cpu_wr_mem_UxROM(WORD address, BYTE value);

void extcl_cpu_wr_mem_Unl1xROM(WORD address, BYTE value);

void extcl_cpu_wr_mem_UNROM_180(WORD address, BYTE value);

void extcl_cpu_wr_mem_UnlROM(WORD address, BYTE value);

#endif /* MAPPER_UXROM_H_ */
