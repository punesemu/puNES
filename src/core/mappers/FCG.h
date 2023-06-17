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

#ifndef MAPPER_FCG_H_
#define MAPPER_FCG_H_

#include "common.h"

typedef struct _fcg {
	BYTE prg;
	BYTE chr[8];
	BYTE mirroring;
	struct _fcg_irq {
		BYTE enabled;
		WORD count;
		BYTE delay;
	} irq;
} _fcg;

extern _fcg fcg;

void extcl_after_mapper_init_FCG(void);
void extcl_cpu_wr_mem_FCG(WORD address, BYTE value);
BYTE extcl_save_mapper_FCG(BYTE mode, BYTE slot, FILE *fp);
void extcl_cpu_every_cycle_FCG(void);

void init_FCG(BYTE reset);
void prg_fix_FCG_base(void);
void prg_swap_FCG_base(WORD address, WORD value);
void chr_fix_FCG_base(void);
void chr_swap_FCG_base(WORD address, WORD value);
void mirroring_fix_FCG_base(void);

extern void (*FCG_prg_fix)(void);
extern void (*FCG_prg_swap)(WORD address, WORD value);
extern void (*FCG_chr_fix)(void);
extern void (*FCG_chr_swap)(WORD address, WORD value);
extern void (*FCG_mirroring_fix)(void);

#endif /* MAPPER_FCG_H_ */
