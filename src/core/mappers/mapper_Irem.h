/*
 * mapper_Irem.h
 *
 *  Created on: 13/set/2011
 *      Author: fhorse
 */

#ifndef MAPPER_IREM_H_
#define MAPPER_IREM_H_

#include "common.h"

enum {
	G101,
	G101A,
	G101B,
	H3000,
	LROG017,
	TAMS1,
	MAJORLEAGUE
};

struct _irem_G101 {
	BYTE prg_mode;
	BYTE prg_reg;
} irem_G101;
struct _irem_H3000 {
	BYTE enable;
	WORD count;
	WORD reload;
	BYTE delay;
} irem_H3000;
struct _irem_LROG017 {
	BYTE filler;
} irem_LROG017;

void map_init_Irem(BYTE model);

void extcl_cpu_wr_mem_Irem_G101(WORD address, BYTE value);
BYTE extcl_save_mapper_Irem_G101(BYTE mode, BYTE slot, FILE *fp);

void extcl_cpu_wr_mem_Irem_H3000(WORD address, BYTE value);
BYTE extcl_save_mapper_Irem_H3000(BYTE mode, BYTE slot, FILE *fp);
void extcl_cpu_every_cycle_Irem_H3000(void);

void extcl_cpu_wr_mem_Irem_LROG017(WORD address, BYTE value);
BYTE extcl_save_mapper_Irem_LROG017(BYTE mode, BYTE slot, FILE *fp);
void extcl_wr_chr_Irem_LROG017(WORD address, BYTE value);

void extcl_cpu_wr_mem_Irem_TAMS1(WORD address, BYTE value);

#endif /* MAPPER_IREM_H_ */
