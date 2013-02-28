/*
 * mapperWaixing.h
 *
 *  Created on: 11/set/2011
 *      Author: fhorse
 */

#ifndef MAPPERWAIXING_H_
#define MAPPERWAIXING_H_

#include "common.h"

enum {
	WPSX = 2,
	WTA,
	WTB,
	WTC,
	WTD,
	WTE,
	WTG,
	WTH,
	SH2,
	BAD_SUGOROQUEST
};

struct _waixing {
	WORD prg_map[4];
	WORD chr_map[8];
	BYTE chr_ram[0x2000];
	BYTE reg;
	WORD ctrl[8];
} waixing;

void map_init_Waixing(BYTE model);

void extcl_cpu_wr_mem_Waixing_PSx(WORD address, BYTE value);

void extcl_cpu_wr_mem_Waixing_type_ACDE(WORD address, BYTE value);
BYTE extcl_save_mapper_Waixing_type_ACDE(BYTE mode, BYTE slot, FILE *fp);
void extcl_wr_chr_Waixing_type_ACDE(WORD address, BYTE value);

void extcl_cpu_wr_mem_Waixing_type_B(WORD address, BYTE value);
BYTE extcl_save_mapper_Waixing_type_B(BYTE mode, BYTE slot, FILE *fp);
void extcl_wr_chr_Waixing_type_B(WORD address, BYTE value);

void extcl_cpu_wr_mem_Waixing_type_G(WORD address, BYTE value);
BYTE extcl_save_mapper_Waixing_type_G(BYTE mode, BYTE slot, FILE *fp);
void extcl_wr_chr_Waixing_type_G(WORD address, BYTE value);

void extcl_cpu_wr_mem_Waixing_type_H(WORD address, BYTE value);
BYTE extcl_save_mapper_Waixing_type_H(BYTE mode, BYTE slot, FILE *fp);

void extcl_cpu_wr_mem_Waixing_SH2(WORD address, BYTE value);
BYTE extcl_save_mapper_Waixing_SH2(BYTE mode, BYTE slot, FILE *fp);
void extcl_update_r2006_Waixing_SH2(WORD old_r2006);
void extcl_after_rd_chr_Waixing_SH2(WORD address);
void extcl_wr_chr_Waixing_SH2(WORD address, BYTE value);

#endif /* MAPPERWAIXING_H_ */
