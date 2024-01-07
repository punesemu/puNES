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

#ifndef MAPPER_MMC4_H_
#define MAPPER_MMC4_H_

#include "common.h"

typedef struct _mmc4 {
	BYTE prg;
	BYTE chr[4];
	BYTE latch[2];
	BYTE mirroring;
} _mmc4;

extern _mmc4 mmc4;

void extcl_after_mapper_init_MMC4(void);
void extcl_cpu_wr_mem_MMC4(BYTE nidx, WORD address, BYTE value);
BYTE extcl_save_mapper_MMC4(BYTE mode, BYTE slot, FILE *fp);
void extcl_after_rd_chr_MMC4(BYTE nidx, WORD address);
void extcl_update_r2006_MMC4(BYTE nidx, WORD new_r2006, WORD old_r2006);

void init_MMC4(BYTE reset);
void prg_fix_MMC4_base(void);
void prg_swap_MMC4_base(WORD address, WORD value);
void chr_fix_MMC4_base(void);
void chr_swap_MMC4_base(WORD address, WORD value);
void mirroring_fix_MMC4_base(void);

extern void (*MMC4_prg_fix)(void);
extern void (*MMC4_prg_swap)(WORD address, WORD value);
extern void (*MMC4_chr_fix)(void);
extern void (*MMC4_chr_swap)(WORD address, WORD value);
extern void (*MMC4_mirroring_fix)(void);

#endif /* MAPPER_MMC4_H_ */
