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

#ifndef MAPPER_MMC3_H_
#define MAPPER_MMC3_H_

#include "common.h"

typedef struct _mmc3 {
	WORD reg[8];
	BYTE bank_to_update;
	BYTE mirroring;
	BYTE wram_protect;
} _mmc3;

extern _mmc3 mmc3;

void extcl_after_mapper_init_MMC3(void);
void extcl_cpu_wr_mem_MMC3(BYTE nidx, WORD address, BYTE value);
BYTE extcl_save_mapper_MMC3(BYTE mode, BYTE slot, FILE *fp);
void extcl_cpu_every_cycle_MMC3(BYTE nidx);
void extcl_ppu_000_to_34x_MMC3(BYTE nidx);
void extcl_ppu_000_to_255_MMC3(BYTE nidx);
void extcl_ppu_256_to_319_MMC3(BYTE nidx);
void extcl_ppu_320_to_34x_MMC3(BYTE nidx);
void extcl_update_r2006_MMC3(BYTE nidx, WORD new_r2006, WORD old_r2006);
void extcl_irq_A12_clock_MMC3_NEC(BYTE nidx);

void init_MMC3(BYTE reset);
void prg_fix_MMC3_base(void);
void prg_swap_MMC3_base(WORD address, WORD value);
void chr_fix_MMC3_base(void);
void chr_swap_MMC3_base(WORD address, WORD value);
void wram_fix_MMC3_base(void);
void wram_swap_MMC3_base(WORD address, WORD value);
void mirroring_fix_MMC3_base(void);

extern void (*MMC3_prg_fix)(void);
extern void (*MMC3_prg_swap)(WORD address, WORD value);
extern void (*MMC3_chr_fix)(void);
extern void (*MMC3_chr_swap)(WORD address, WORD value);
extern void (*MMC3_wram_fix)(void);
extern void (*MMC3_wram_swap)(WORD address, WORD value);
extern void (*MMC3_mirroring_fix)(void);

#endif /* MAPPER_MMC3_H_ */
