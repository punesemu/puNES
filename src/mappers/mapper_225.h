/*
 * mapper_225.h
 *
 *  Created on: 06/feb/2012
 *      Author: fhorse
 */

#ifndef MAPPER_225_H_
#define MAPPER_225_H_

#include "common.h"

enum { BMC52IN1 = 2 };

void map_init_225(void);
void extcl_cpu_wr_mem_225(WORD address, BYTE value);

#endif /* MAPPER_225_H_ */
