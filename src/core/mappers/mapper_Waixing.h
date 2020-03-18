/*
 *  Copyright (C) 2010-2020 Fabio Cavallo (aka FHorse)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef MAPPER_WAIXING_H_
#define MAPPER_WAIXING_H_

#include "common.h"

enum {
	WPSX,
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
void extcl_update_r2006_Waixing_SH2(WORD new_r2006, WORD old_r2006);
void extcl_after_rd_chr_Waixing_SH2(WORD address);
void extcl_wr_chr_Waixing_SH2(WORD address, BYTE value);

#endif /* MAPPER_WAIXING_H_ */
