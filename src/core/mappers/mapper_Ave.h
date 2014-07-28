/*
 * mapper_Ave.h
 *
 *  Created on: 19/set/2011
 *      Author: fhorse
 */

#ifndef MAPPER_AVE_H_
#define MAPPER_AVE_H_

#include "common.h"

enum { NINA06, D1012, PUZZLEUNL };

struct _ave_d1012 {
	BYTE reg[3];
} ave_d1012;

void map_init_Ave(BYTE model);

void extcl_cpu_wr_mem_Ave_NINA06(WORD address, BYTE value);

void extcl_cpu_wr_mem_Ave_D1012(WORD address, BYTE value);
BYTE extcl_cpu_rd_mem_Ave_D1012(WORD address, BYTE openbus, BYTE before);
BYTE extcl_save_mapper_Ave_D1012(BYTE mode, BYTE slot, FILE *fp);

#endif /* MAPPER_AVE_H_ */
