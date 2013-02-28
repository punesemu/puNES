/*
 * mapper_0.h
 *
 *  Created on: 09/mag/2010
 *      Author: fhorse
 */

#ifndef MAPPER_0_H_
#define MAPPER_0_H_

#include "common.h"

enum  { UNK_HORIZONTAL, UNK_VERTICAL };

void map_init_0(void);
void extcl_cpu_wr_mem_0(WORD address, BYTE value);
BYTE extcl_cpu_rd_mem_0(WORD address, BYTE openbus, BYTE before);

#endif /* MAPPER_0_H_ */
