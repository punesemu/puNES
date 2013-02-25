/*
 * mapperVs.h
 *
 *  Created on: 22/set/2010
 *      Author: fhorse
 */

#ifndef MAPPERVS_H_
#define MAPPERVS_H_

#include "common.h"

void map_init_Vs(void);
void extcl_cpu_wr_mem_Vs(WORD address, BYTE value);
void extcl_cpu_wr_r4016_Vs(BYTE value);

#endif /* MAPPERVS_H_ */
