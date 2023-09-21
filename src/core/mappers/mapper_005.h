/*
 *  Copyright (C) 2010-2023 Fabio Cavallo (aka FHorse)
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

#ifndef MAPPER_005_H_
#define MAPPER_005_H_

#include "common.h"
#include "apu.h"

typedef struct _m005 {
	BYTE prg_mode;
	BYTE chr_mode;
	BYTE ext_mode;

	BYTE prg[5];
	WORD chr[12];
	WORD chr_high;
	BYTE chr_last;
	BYTE wram_protect[2];
	BYTE nmt;

	BYTE ext_ram[S1K];
	BYTE fill_table[S1K];
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
	DBWORD split_bank;
	BYTE factor[2];

	BYTE timer_running;
	BYTE timer_irq;
	WORD timer_count;

	struct _m005_snd {
		_apuSquare S3;
		_apuSquare S4;
		struct _m005_pcm {
			BYTE enabled;
			BYTE output;
			BYTE amp;
		} pcm;
		// questi valori non e' necessario salvarli nei savestates
		BYTE clocked;
	} snd;
} _m005;

extern _m005 m005;

void map_init_005(void);
void map_init_NSF_005(void);
void extcl_after_mapper_init_005(void);
void extcl_cpu_wr_mem_005(BYTE cidx, WORD address, BYTE value);
BYTE extcl_cpu_rd_mem_005(BYTE cidx, WORD address, BYTE openbus);
BYTE extcl_save_mapper_005(BYTE mode, BYTE slot, FILE *fp);
void extcl_ppu_256_to_319_005(BYTE cidx);
void extcl_ppu_320_to_34x_005(BYTE cidx);
void extcl_rd_r2007_005(BYTE cidx);
void extcl_after_rd_chr_005(BYTE cidx, WORD address);
BYTE extcl_rd_chr_005(BYTE cidx, WORD address);
BYTE extcl_rd_nmt_005(BYTE cidx, WORD address);
void extcl_cpu_every_cycle_005(BYTE cidx);
void extcl_length_clock_005(void);
void extcl_envelope_clock_005(void);
void extcl_apu_tick_005(void);

#endif /* MAPPER_005_H_ */
