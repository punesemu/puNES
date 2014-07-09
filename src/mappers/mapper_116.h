/*
 * mapper_116.h
 *
 *  Created on: 24/apr/2012
 *      Author: fhorse
 */

#ifndef MAPPER_116_H_
#define MAPPER_116_H_

#include "common.h"

enum {
	MAP116_TYPE_A,
	MAP116_TYPE_B,
	MAP116_TYPE_C
};

struct _m166 {
	BYTE mode;

	struct {
		WORD chr[8];
		WORD prg[4];
		BYTE nmt;
		BYTE padding;
	} mode0;
	struct {
		WORD banks[10];
		BYTE ctrl;
		BYTE nmt;
	} mode1;
	struct {
		BYTE reg[4];
		BYTE buffer;
		BYTE shifter;
		BYTE padding[2];
	} mode2;

	WORD prg_map[4];
	WORD chr_map[8];
	BYTE chr_ram[0x2000];
} m116;

void map_init_116(void);

void extcl_cpu_wr_mem_116_type_A(WORD address, BYTE value);
BYTE extcl_save_mapper_116_type_A(BYTE mode, BYTE slot, FILE *fp);

void extcl_cpu_wr_mem_116_type_B(WORD address, BYTE value);
BYTE extcl_save_mapper_116_type_B(BYTE mode, BYTE slot, FILE *fp);
void extcl_wr_chr_116_type_B(WORD address, BYTE value);

void extcl_cpu_wr_mem_116_type_C(WORD address, BYTE value);
BYTE extcl_save_mapper_116_type_C(BYTE mode, BYTE slot, FILE *fp);

#endif /* MAPPER_116_H_ */
