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

#ifndef MAPPER_JYASIC_H_
#define MAPPER_JYASIC_H_

#include "common.h"

enum _jyasic_types {
	MAP90,  MAP209, MAP211, MAP281,
	MAP282, MAP295, MAP358, MAP386,
	MAP387, MAP388, MAP394, MAP397 };

typedef struct _jyasic {
	BYTE mul[2];
	BYTE single_byte_ram;
	BYTE add;
	BYTE mode[4];
	BYTE prg[4];
	struct _jyasic_chr {
		BYTE latch[2];
		BYTE low[8];
		BYTE high[8];
	} chr;
	struct _jyasic_nmt {
		BYTE extended_mode;
		WORD reg[4];
	} nmt;
	struct _jyasic_irq {
		BYTE active;
		BYTE mode;
		BYTE prescaler;
		BYTE count;
		BYTE xor_value;
		BYTE pre_size;
		BYTE premask;
	} irq;
} _jyasic;

extern _jyasic jyasic;

void map_init_JYASIC(BYTE model);
void extcl_after_mapper_init_JYASIC(void);
void extcl_cpu_wr_mem_JYASIC(WORD address, BYTE value);
BYTE extcl_cpu_rd_mem_JYASIC(WORD address, BYTE openbus);
BYTE extcl_save_mapper_JYASIC(BYTE mode, BYTE slot, FILE *fp);
void extcl_cpu_every_cycle_JYASIC(void);
void extcl_rd_ppu_mem_JYASIC(WORD address);
BYTE extcl_rd_chr_JYASIC(WORD address);
void extcl_ppu_000_to_255_JYASIC(void);
void extcl_ppu_256_to_319_JYASIC(void);
void extcl_ppu_320_to_34x_JYASIC(void);
void extcl_update_r2006_JYASIC(WORD new_r2006, WORD old_r2006);

#endif /* MAPPER_JYASIC_H_ */
