/*
 * mapper_Vs.h
 *
 *  Created on: 22/set/2010
 *      Author: fhorse
 */

#ifndef MAPPER_VS_H_
#define MAPPER_VS_H_

#include "common.h"

void map_init_Vs(void);
void extcl_cpu_wr_mem_Vs(WORD address, BYTE value);
void extcl_cpu_wr_r4016_Vs(BYTE value);

#endif /* MAPPER_VS_H_ */
