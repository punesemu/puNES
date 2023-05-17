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

#ifndef MAPPER_KS202_H_
#define MAPPER_KS202_H_

#include "common.h"

typedef struct _ks202 {
	BYTE index;
	BYTE reg[5];
	struct _ks202_irq {
		BYTE enabled;
		WORD count;
		WORD reload;
	} irq;
} _ks202;

extern _ks202 ks202;

void extcl_after_mapper_init_KS202(void);
void extcl_cpu_wr_mem_KS202(WORD address, BYTE value);
BYTE extcl_save_mapper_KS202(BYTE mode, BYTE slot, FILE *fp);
void extcl_cpu_every_cycle_KS202(void);

void init_KS202(void);
void prg_fix_KS202_base(void);
void prg_swap_KS202_base(WORD address, WORD value);
void wram_fix_KS202_base(void);
void wram_swap_KS202_base(WORD address, WORD value);

extern void (*KS202_prg_fix)(void);
extern void (*KS202_prg_swap)(WORD address, WORD value);
extern void (*KS202_wram_fix)(void);
extern void (*KS202_wram_swap)(WORD address, WORD value);

#endif /* MAPPER_KS202_H_ */
