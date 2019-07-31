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

#ifndef MAPPER_90_209_211_H_
#define MAPPER_90_209_211_H_

#include "common.h"

enum {
	MAP90,
	MAP209,
	MAP211
};

struct _m90_209_211 {
	BYTE mul[2];
	BYTE single_byte_ram;
	BYTE tekker;

	BYTE mode[4];

	BYTE prg[4];

	struct _m90_209_211_chr {
		BYTE latch[2];
		BYTE low[8];
		BYTE high[8];
	} chr;

	struct _m90_209_211_nmt {
		BYTE extended_mode;
		WORD reg[4];
		BYTE write[4];
	} nmt;

	struct _m90_209_211_irq {
		BYTE active;
		BYTE mode;
		BYTE prescaler;
		BYTE count;
		BYTE xor_value;
		BYTE pre_size;
		BYTE premask;
	} irq;

/*  questi non serve salvarli  */
	BYTE model;

	struct _m90_209_211_m6000 {
		WORD prg;
		BYTE *rom_8k;
	} m6000;
/*                             */
} m90_209_211;

void map_init_90_209_211(BYTE model);
void extcl_cpu_wr_mem_90_209_211(WORD address, BYTE value);
BYTE extcl_cpu_rd_mem_90_209_211(WORD address, BYTE openbus, BYTE before);
BYTE extcl_save_mapper_90_209_211(BYTE mode, BYTE slot, FILE *fp);
void extcl_cpu_every_cycle_90_209_211(void);
void extcl_rd_ppu_90_209_211(WORD address);
BYTE extcl_rd_chr_90_209_211(WORD address);
void extcl_wr_nmt_90_209_211(WORD address, BYTE value);

void extcl_ppu_000_to_255_90_209_211(void);
void extcl_ppu_256_to_319_90_209_211(void);
void extcl_ppu_320_to_34x_90_209_211(void);
void extcl_update_r2006_90_209_211(WORD new_r2006, WORD old_r2006);

#endif /* MAPPER_90_209_211_H_ */
