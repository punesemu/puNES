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

#ifndef MAPPER_N118_H_
#define MAPPER_N118_H_

#include "common.h"

typedef struct _n118 {
	BYTE reg[8 + 1];
} _n118;

extern _n118 n118;

void extcl_after_mapper_init_N118(void);
void extcl_cpu_wr_mem_N118(WORD address, BYTE value);
BYTE extcl_save_mapper_N118(BYTE mode, BYTE slot, FILE *fp);

void init_N118(void);
void prg_fix_N118_base(void);
void prg_swap_N118_base(WORD address, WORD value);
void chr_fix_N118_base(void);
void chr_swap_N118_base(WORD address, WORD value);

extern void (*N118_prg_fix)(void);
extern void (*N118_prg_swap)(WORD address, WORD value);
extern void (*N118_chr_fix)(void);
extern void (*N118_chr_swap)(WORD address, WORD value);

#endif /* MAPPER_N118_H_ */
