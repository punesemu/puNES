/*
 * mapper_malee.h
 *
 *  Created on: 11/lug/2014
 *      Author: fhorse
 */

#ifndef MAPPER_MALEE_H_
#define MAPPER_MALEE_H_

#include "common.h"

void map_init_malee(void);
void extcl_cpu_wr_mem_malee(WORD address, BYTE value);
BYTE extcl_cpu_rd_mem_malee(WORD address, BYTE openbus, BYTE before);

#endif /* MAPPER_MALEE_H_ */
