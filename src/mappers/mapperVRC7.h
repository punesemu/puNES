/*
 * mapperVRC7.h
 *
 *  Created on: 24/gen/2012
 *      Author: fhorse
 */

#ifndef MAPPERVRC7_H_
#define MAPPERVRC7_H_

#include "common.h"

enum { VRC7A, VRC7B };

struct _vrc7 {
	BYTE reg;
	BYTE enabled;
	BYTE reload;
	BYTE mode;
	BYTE acknowledge;
	BYTE count;
	BYTE delay;
	WORD prescaler;
} vrc7;

void mapInit_VRC7(BYTE revision);
void extcl_cpu_wr_mem_VRC7(WORD address, BYTE value);
BYTE extcl_save_mapper_VRC7(BYTE mode, BYTE slot, FILE *fp);
void extcl_cpu_every_cycle_VRC7(void);
void extcl_snd_start_VRC7(WORD samplarate);

#endif /* MAPPERVRC7_H_ */
