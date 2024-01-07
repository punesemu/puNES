/*
 *  Copyright (C) 2010-2024 Fabio Cavallo (aka FHorse)
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

typedef struct _jyasic {
	BYTE prg[4];
	BYTE mul[2];
	BYTE single_byte_ram;
	BYTE add;
	BYTE mode[4];
	struct _jyasic_chr {
		WORD reg[8];
		BYTE latch[2];
	} chr;
	struct _jyasic_nmt {
		WORD reg[4];
		BYTE extended_mode;
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

void extcl_after_mapper_init_JYASIC(void);
void extcl_cpu_wr_mem_JYASIC(BYTE nidx, WORD address, BYTE value);
BYTE extcl_cpu_rd_mem_JYASIC(BYTE nidx, WORD address, BYTE openbus);
BYTE extcl_save_mapper_JYASIC(BYTE mode, BYTE slot, FILE *fp);
void extcl_cpu_every_cycle_JYASIC(BYTE nidx);
void extcl_rd_ppu_mem_JYASIC(BYTE nidx, WORD address);
BYTE extcl_rd_chr_JYASIC(BYTE nidx, WORD address);
void extcl_ppu_000_to_255_JYASIC(BYTE nidx);
void extcl_ppu_256_to_319_JYASIC(BYTE nidx);
void extcl_ppu_320_to_34x_JYASIC(BYTE nidx);
void extcl_update_r2006_JYASIC(BYTE nidx, WORD new_r2006, WORD old_r2006);

void init_JYASIC(BYTE extended_mode, BYTE reset);
void prg_fix_JYASIC_base(void);
void prg_swap_JYASIC_base(WORD address, DBWORD value);
void chr_fix_JYASIC_base(void);
void chr_swap_JYASIC_base(WORD address, DBWORD value);
void wram_fix_JYASIC_base(void);
void wram_swap_JYASIC_base(WORD address, DBWORD value);
void mirroring_fix_JYASIC_base(void);
void mirroring_swap_JYASIC_base(WORD address, DBWORD value);

extern void (*JYASIC_prg_fix)(void);
extern void (*JYASIC_prg_swap)(WORD address, DBWORD value);
extern void (*JYASIC_chr_fix)(void);
extern void (*JYASIC_chr_swap)(WORD address, DBWORD value);
extern void (*JYASIC_wram_fix)(void);
extern void (*JYASIC_wram_swap)(WORD address, DBWORD value);
extern void (*JYASIC_mirroring_fix)(void);
extern void (*JYASIC_mirroring_swap)(WORD address, DBWORD value);

#endif /* MAPPER_JYASIC_H_ */
