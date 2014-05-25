/*
 * mapper_A65AS.h
 *
 *  Created on: 20/mag/2014
 *      Author: fhorse
 */

#ifndef MAPPER_A65AS_H_
#define MAPPER_A65AS_H_

#include "common.h"

void map_init_A65AS(void);
void extcl_cpu_wr_mem_A65AS(WORD address, BYTE value);

#endif /* MAPPER_A65AS_H_ */
