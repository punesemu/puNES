/*
 *  Copyright (C) 2010-2016 Fabio Cavallo (aka FHorse)
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

#ifndef MAPPER_MMC5_H_
#define MAPPER_MMC5_H_

#include "apu.h"

enum { EKROM, ELROM, ETROM, EWROM };

struct _mmc5 {
	BYTE prg_mode;
	BYTE chr_mode;
	BYTE ext_mode;
	BYTE nmt_mode[4];
	BYTE prg_ram_write[2];
	BYTE prg_bank[4];
	uint32_t prg_ram_bank[4][2];
	BYTE chr_last;
	WORD chr_high;
	WORD chr_s[8];
	WORD chr_b[4];
	BYTE ext_ram[0x400];
	BYTE fill_table[0x400];
	BYTE fill_tile;
	BYTE fill_attr;
	BYTE split;
	BYTE split_st_tile;
	BYTE split_side;
	BYTE split_scrl;
	BYTE split_in_reg;
	BYTE split_x;
	BYTE split_y;
	WORD split_tile;
	uint32_t split_bank;
	BYTE factor[2];
	WORD product;
	_apuSquare S3, S4;
	struct {
		BYTE enabled;
		BYTE output;
		BYTE amp;

	/* ------------------------------------------------------- */
	/* questi valori non e' necessario salvarli nei savestates */
	/* ------------------------------------------------------- */
	/* */ BYTE clocked;                                     /* */
	/* ------------------------------------------------------- */
	} pcm;
	BYTE filler[50];
} mmc5;

void map_init_MMC5(void);
void extcl_cpu_wr_mem_MMC5(WORD address, BYTE value);
BYTE extcl_cpu_rd_mem_MMC5(WORD address, BYTE openbus, BYTE before);
BYTE extcl_save_mapper_MMC5(BYTE mode, BYTE slot, FILE *fp);
void extcl_ppu_256_to_319_MMC5(void);
void extcl_ppu_320_to_34x_MMC5(void);
void extcl_after_rd_chr_MMC5(WORD address);
BYTE extcl_rd_chr_MMC5(WORD address);
BYTE extcl_rd_nmt_MMC5(WORD address);
void extcl_length_clock_MMC5(void);
void extcl_envelope_clock_MMC5(void);
void extcl_apu_tick_MMC5(void);

#endif /* MAPPER_MMC5_H_ */
