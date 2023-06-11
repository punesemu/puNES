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

#ifndef MAPPER_VRC2and4_H_
#define MAPPER_VRC2and4_H_

#include "common.h"

enum _vrc24_types { VRC24_VRC2 = 20, VRC24_VRC4 };

typedef struct _vrc2and4 {
	WORD prg[2];
	WORD chr[8];
	BYTE mirroring;
	BYTE swap_mode;
	BYTE wram_protect;
	BYTE wired;
	struct _vrc2and4_irq {
		BYTE enabled;
		BYTE reload;
		BYTE mode;
		BYTE acknowledge;
		BYTE count;
		WORD prescaler;
	} irq;
} _vrc2and4;

extern _vrc2and4 vrc2and4;

void extcl_after_mapper_init_VRC2and4(void);
void extcl_cpu_wr_mem_VRC2and4(WORD address, BYTE value);
BYTE extcl_cpu_rd_mem_VRC2and4(WORD address, BYTE openbus);
BYTE extcl_save_mapper_VRC2and4(BYTE mode, BYTE slot, FILE *fp);
void extcl_cpu_every_cycle_VRC2and4(void);

void init_VRC2and4(BYTE type, WORD A0, WORD A1, BYTE irq_repeated);
void prg_fix_VRC2and4_base(void);
void prg_swap_VRC2and4_base(WORD address, WORD value);
void chr_fix_VRC2and4_base(void);
void chr_swap_VRC2and4_base(WORD address, WORD value);
void wram_fix_VRC2and4_base(void);
void mirroring_fix_VRC2and4_base(void);
void wired_fix_VRC2and4_base(void);
void misc_03_VRC2and4_base(WORD address, BYTE value);

extern void (*VRC2and4_prg_fix)(void);
extern void (*VRC2and4_prg_swap)(WORD address, WORD value);
extern void (*VRC2and4_chr_fix)(void);
extern void (*VRC2and4_chr_swap)(WORD address, WORD value);
extern void (*VRC2and4_wram_fix)(void);
extern void (*VRC2and4_mirroring_fix)(void);
extern void (*VRC2and4_wired_fix)(void);
extern void (*VRC2and4_misc_03)(WORD address, BYTE value);

#endif /* MAPPER_VRC2and4_H_ */
