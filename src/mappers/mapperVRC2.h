/*
 * mapperVRC2.h
 *
 *  Created on: 10/set/2011
 *      Author: fhorse
 */

#ifndef MAPPERVRC2_H_
#define MAPPERVRC2_H_

#include "common.h"

enum { VRC2B, VRC2A };

struct _vrc2 {
	BYTE chr_rom_bank[8];
} vrc2;

void map_init_VRC2(BYTE revision);
void extcl_cpu_wr_mem_VRC2(WORD address, BYTE value);
BYTE extcl_save_mapper_VRC2(BYTE mode, BYTE slot, FILE *fp);

#endif /* MAPPERVRC2_H_ */
