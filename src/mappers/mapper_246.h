/*
 * mapper_246.h
 *
 *  Created on: 24/apr/2012
 *      Author: fhorse
 */

#ifndef MAPPER_246_H_
#define MAPPER_246_H_

#include "common.h"

void map_init_246(void);
void extcl_cpu_wr_mem_246(WORD address, BYTE value);
BYTE extcl_cpu_rd_mem_246(WORD address, BYTE openbus, BYTE before);

#endif /* MAPPER_246_H_ */
