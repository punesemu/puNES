/*
 * mapper_MMC2andMMC4.h
 *
 *  Created on: 10/lug/2011
 *      Author: fhorse
 */

#ifndef MAPPER_MMC2ANDMMC4_H_
#define MAPPER_MMC2ANDMMC4_H_

#include "common.h"

/* MMC4 */
enum { BAD_INES_FWJ };

struct _mmc2and4 {
	BYTE regs[4];
	BYTE latch0;
	BYTE latch1;
} mmc2and4;

void map_init_MMC2and4(void);
void extcl_cpu_wr_mem_MMC2and4(WORD address, BYTE value);
BYTE extcl_save_mapper_MMC2and4(BYTE mode, BYTE slot, FILE *fp);
void extcl_after_rd_chr_MMC2and4(WORD address);

#endif /* MAPPER_MMC2ANDMMC4_H_ */
