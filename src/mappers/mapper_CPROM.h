/*
 * mapper_CPROM.h
 *
 *  Created on: 14/lug/2011
 *      Author: fhorse
 */

#ifndef MAPPER_CPROM_H_
#define MAPPER_CPROM_H_

#include "common.h"

void map_init_CPROM(void);
void extcl_cpu_wr_mem_CPROM(WORD address, BYTE value);

#endif /* MAPPER_CPROM_H_ */
