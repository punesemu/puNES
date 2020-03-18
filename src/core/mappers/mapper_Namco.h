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

#ifndef MAPPER_NAMCO_H_
#define MAPPER_NAMCO_H_

#include "common.h"

enum {
	N163,
	N3416,
	N3425,
	N3433,
	N3446,
	N3453,
	NAMCO_HARD_WIRED_V,
	NAMCO_HARD_WIRED_H,
	MINDSEEKER
};

typedef struct _n163_snd_ch {
	BYTE enabled;
	BYTE active;
	WORD address;
	DBWORD freq;
	DBWORD cycles_reload;
	DBWORD cycles;
	WORD length;
	WORD step;
	WORD volume;
	SWORD output;
} _n163_snd_ch;
typedef struct _n163 {
	uint32_t nmt_bank[4][2];
	BYTE irq_delay;
	DBWORD irq_count;
	BYTE snd_ram[0x80];
	BYTE snd_adr;
	BYTE snd_auto_inc;
	BYTE snd_ch_start;
	BYTE snd_wave[0x100];
	_n163_snd_ch ch[8];
} _n163;

extern _n163 n163;

void map_init_Namco(BYTE model);
void map_init_NSF_Namco(BYTE model);

void extcl_cpu_wr_mem_Namco_163(WORD address, BYTE value);
BYTE extcl_cpu_rd_mem_Namco_163(WORD address, BYTE openbus, BYTE before);
BYTE extcl_save_mapper_Namco_163(BYTE mode, BYTE slot, FILE *fp);
void extcl_cpu_every_cycle_Namco_163(void);
void extcl_apu_tick_Namco_163(void);

void extcl_cpu_wr_mem_Namco_3425(WORD address, BYTE value);
BYTE extcl_save_mapper_Namco_3425(BYTE mode, BYTE slot, FILE *fp);

void extcl_cpu_wr_mem_Namco_3446(WORD address, BYTE value);
BYTE extcl_save_mapper_Namco_3446(BYTE mode, BYTE slot, FILE *fp);

#endif /* MAPPER_NAMCO_H_ */
