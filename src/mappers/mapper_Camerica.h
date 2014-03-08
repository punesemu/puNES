/*
 * mapper_Camerica.h
 *
 *  Created on: 12/lug/2011
 *      Author: fhorse
 */

#ifndef MAPPER_CAMERICA_H_
#define MAPPER_CAMERICA_H_

#include "common.h"

enum {
	BF9093      = 2,
	BF9096      = 3,
	BF9097      = 4,
	GOLDENFIVE  = 5,
	PEGASUS4IN1 = 6
};

void map_init_Camerica(void);
void extcl_cpu_wr_mem_Camerica_BF9093(WORD address, BYTE value);
void extcl_cpu_wr_mem_Camerica_BF9096(WORD address, BYTE value);
void extcl_cpu_wr_mem_Camerica_BF9097(WORD address, BYTE value);
void extcl_cpu_wr_mem_Camerica_GoldenFive(WORD address, BYTE value);

#endif /* MAPPER_CAMERICA_H_ */
