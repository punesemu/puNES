/*
 * mapper_CNROM.h
 *
 *  Created on: 19/mag/2010
 *      Author: fhorse
 */

#ifndef MAPPER_CNROM_H_
#define MAPPER_CNROM_H_

#include "common.h"

enum {
	CNROM,
	CNROM_CNFL,
	CNROM_26CE27CE,
	CNROM_26CE27NCE,
	CNROM_26NCE27CE,
	CNROM_26NCE27NCE
};

struct _cnrom_2627 {
	BYTE chr_rd_enable;
} cnrom_2627;

void map_init_CNROM(BYTE model);
void extcl_cpu_wr_mem_CNROM(WORD address, BYTE value);
BYTE extcl_save_mapper_CNROM(BYTE mode, BYTE slot, FILE *fp);
BYTE extcl_rd_chr_CNROM(WORD address);

#endif /* MAPPER_CNROM_H_ */
