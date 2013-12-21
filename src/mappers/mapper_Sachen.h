/*
 * mapper_Sachen.h
 *
 *  Created on: 27/set/2011
 *      Author: fhorse
 */

#ifndef MAPPER_SACHEN_H_
#define MAPPER_SACHEN_H_

#include "common.h"

enum {
	SA0036 = 2,
	SA0037,
	SA8259A,
	SA8259B,
	SA8259C,
	SA8259D,
	TCA01,
	TCU01,
	TCU02,
	SA72007,
	SA72008,
	SA74374A,
	SA74374B,
};

struct _sa8259 {
	BYTE ctrl;
	BYTE reg[8];
} sa8259;
struct _tcu02 {
	BYTE reg;
} tcu02;
struct _sa74374x {
	BYTE reg;
	BYTE chr_rom_8k_bank;
} sa74374x;

static const char pokeriiichr[2][41] = {
	"5066c2d12ff2ac45ef395d3a4353e897fce19f78",
	"c6bf926ed14c21f1a5b64fbccf3288005ff54be5"
};

void map_init_Sachen(BYTE model);

void extcl_cpu_wr_mem_Sachen_sa0036(WORD address, BYTE value);

void extcl_cpu_wr_mem_Sachen_sa0037(WORD address, BYTE value);

void extcl_cpu_wr_mem_Sachen_sa8259x(WORD address, BYTE value);
BYTE extcl_save_mapper_Sachen_sa8259x(BYTE mode, BYTE slot, FILE *fp);

void extcl_cpu_wr_mem_Sachen_tca01(WORD address, BYTE value);
BYTE extcl_cpu_rd_mem_Sachen_tca01(WORD address, BYTE openbus, BYTE before);

void extcl_cpu_wr_mem_Sachen_tcu01(WORD address, BYTE value);

void extcl_cpu_wr_mem_Sachen_tcu02(WORD address, BYTE value);
BYTE extcl_cpu_rd_mem_Sachen_tcu02(WORD address, BYTE openbus, BYTE before);
BYTE extcl_save_mapper_Sachen_tcu02(BYTE mode, BYTE slot, FILE *fp);

void extcl_cpu_wr_mem_Sachen_sa72007(WORD address, BYTE value);

void extcl_cpu_wr_mem_Sachen_sa72008(WORD address, BYTE value);

void extcl_cpu_wr_mem_Sachen_sa74374a(WORD address, BYTE value);
void extcl_cpu_wr_mem_Sachen_sa74374b(WORD address, BYTE value);
BYTE extcl_save_mapper_Sachen_sa74374x(BYTE mode, BYTE slot, FILE *fp);

#endif /* MAPPER_SACHEN_H_ */
