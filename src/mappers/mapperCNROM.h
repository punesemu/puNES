/*
 * mapperCNROM.h
 *
 *  Created on: 19/mag/2010
 *      Author: fhorse
 */

#ifndef MAPPERCNROM_H_
#define MAPPERCNROM_H_

#include "common.h"

enum {
	CNROM = 2,
	CNROMCNFL = 3,
	CNROM26CE27CE = 4,
	CNROM26CE27NCE = 5,
	CNROM26NCE27CE = 6,
	CNROM26NCE27NCE = 7
};

struct _cnrom2627 {
	BYTE chrReadEnable;
} cnrom2627;

void map_init_CNROM(BYTE model);
void extcl_cpu_wr_mem_CNROM(WORD address, BYTE value);
BYTE extcl_save_mapper_CNROM(BYTE mode, BYTE slot, FILE *fp);
BYTE extcl_rd_chr_CNROM(WORD address);

#endif /* MAPPERCNROM_H_ */
