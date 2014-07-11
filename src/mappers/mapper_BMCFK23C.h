/*
 * mapper_BMCFK23C.h
 *
 *  Created on: 03/lug/2014
 *      Author: fhorse
 */

#ifndef MAPPER_BMCFK23C_H_
#define MAPPER_BMCFK23C_H_

#include "common.h"

enum bmcfk23c_types { NOBMCFK23C, BMCFK23C_0 = 1, BMCFK23C_1 = 2, BMCFK23CA = 8 };

struct _bmcfk23c {
	uint32_t dipswitch;
	BYTE unromchr;
	BYTE A000;
	BYTE A001;
	BYTE reg[8];
	BYTE mmc3[8];
	BYTE chr_map[8];
	/* questo posso tranquillamente non salvarlo */
	BYTE prg_mask;
} bmcfk23c;

void map_init_BMCFK23C(void);
void extcl_cpu_wr_mem_BMCFK23C(WORD address, BYTE value);
void extcl_wr_chr_BMCFK23C(WORD address, BYTE value);
BYTE extcl_save_mapper_BMCFK23C(BYTE mode, BYTE slot, FILE *fp);

#endif /* MAPPER_BMCFK23C_H_ */
