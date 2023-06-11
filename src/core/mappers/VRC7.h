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

#ifndef MAPPER_VRC7_H_
#define MAPPER_VRC7_H_

#include "common.h"

typedef struct _vrc7 {
	BYTE reg;
	WORD prg[3];
	WORD chr[8];
	struct _vrc7_irq {
		BYTE enabled;
		BYTE reload;
		BYTE mode;
		BYTE acknowledge;
		BYTE count;
		BYTE delay;
		WORD prescaler;
	} irq;
} _vrc7;

extern _vrc7 vrc7;

void extcl_after_mapper_init_VRC7(void);
void extcl_cpu_wr_mem_VRC7(WORD address, BYTE value);
BYTE extcl_save_mapper_VRC7(BYTE mode, BYTE slot, FILE *fp);
void extcl_cpu_every_cycle_VRC7(void);
void extcl_apu_tick_VRC7(void);

void init_NSF_VRC7(WORD A0, WORD A1);
void init_VRC7(WORD A0, WORD A1);
void prg_fix_VRC7_base(void);
void prg_swap_VRC7_base(WORD address, WORD value);
void chr_fix_VRC7_base(void);
void chr_swap_VRC7_base(WORD address, WORD value);
void wram_fix_VRC7_base(void);
void mirroring_fix_VRC7_base(void);

extern void (*VRC7_prg_fix)(void);
extern void (*VRC7_prg_swap)(WORD address, WORD value);
extern void (*VRC7_chr_fix)(void);
extern void (*VRC7_chr_swap)(WORD address, WORD value);
extern void (*VRC7_wram_fix)(void);
extern void (*VRC7_mirroring_fix)(void);

#endif /* MAPPER_VRC7_H_ */
