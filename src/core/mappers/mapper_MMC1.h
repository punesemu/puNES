/*
 * mapper_MMC1.h
 *
 *  Created on: 18/mag/2010
 *      Author: fhorse
 */

#ifndef MAPPER_MMC1_H_
#define MAPPER_MMC1_H_

#include "common.h"

enum MMC1_types { SNROM, SOROM, SUROM, SXROM, BAD_YOSHI_U };

struct _mmc1 {
	BYTE reg;
	BYTE pos;
	BYTE prg_mode;
	BYTE chr_mode;
	BYTE ctrl;
	BYTE chr0;
	BYTE chr1;
	BYTE prg0;
	BYTE reset;
	BYTE prg_upper;
} mmc1;

void map_init_MMC1(void);
void extcl_cpu_wr_mem_MMC1(WORD address, BYTE value);
BYTE extcl_save_mapper_MMC1(BYTE mode, BYTE slot, FILE *fp);

#endif /* MAPPER_MMC1_H_ */
