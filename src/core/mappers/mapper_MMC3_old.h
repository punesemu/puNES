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

#ifndef MAPPER_MMC3_OLD_H_
#define MAPPER_MMC3_OLD_H_

#include "common.h"

typedef struct _mmc3_old {
	BYTE prg_ram_protect;
	BYTE bank_to_update;
	BYTE prg_rom_cfg;
	BYTE chr_rom_cfg;
} _mmc3_old;

extern _mmc3_old mmc3_old;

void map_init_MMC3_old(void);
void extcl_cpu_wr_mem_MMC3_old(WORD address, BYTE value);
BYTE extcl_save_mapper_MMC3_old(BYTE mode, BYTE slot, FILE *fp);
void extcl_cpu_every_cycle_MMC3_old(void);
void extcl_ppu_000_to_34x_MMC3_old(void);
void extcl_ppu_000_to_255_MMC3_old(void);
void extcl_ppu_256_to_319_MMC3_old(void);
void extcl_ppu_320_to_34x_MMC3_old(void);
void extcl_update_r2006_MMC3_old(WORD new_r2006, WORD old_r2006);
void extcl_irq_A12_clock_MMC3_alternate_old(void);

#endif /* MAPPER_MMC3_OLD_H_ */
