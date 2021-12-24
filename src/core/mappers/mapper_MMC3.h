/*
 *  Copyright (C) 2010-2022 Fabio Cavallo (aka FHorse)
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

#define swap_chr_bank_1k(src, dst)\
{\
	BYTE *chr_bank_1k = chr.bank_1k[src];\
	chr.bank_1k[src] = chr.bank_1k[dst];\
	chr.bank_1k[dst] = chr_bank_1k;\
}

enum {
	NAMCO3413,
	NAMCO3414,
	NAMCO3415,
	NAMCO3416,
	NAMCO3417,
	NAMCO3451,
	TKROM,
	KT008,
	SMB2EREZA,
	SMB2JSMB1,
	RADRACER2,
	MMC3_ALTERNATE,
	MMC6
};

typedef struct _mmc3 {
	BYTE prg_ram_protect;
	BYTE bank_to_update;
	BYTE prg_rom_cfg;
	BYTE chr_rom_cfg;
} _mmc3;
typedef struct _kt008 {
	BYTE value;
} _kt008;

extern _mmc3 mmc3;
extern _kt008 kt008;

void map_init_MMC3(void);
void extcl_cpu_wr_mem_MMC3(WORD address, BYTE value);
BYTE extcl_save_mapper_MMC3(BYTE mode, BYTE slot, FILE *fp);
void extcl_cpu_every_cycle_MMC3(void);
void extcl_ppu_000_to_34x_MMC3(void);
void extcl_ppu_000_to_255_MMC3(void);
void extcl_ppu_256_to_319_MMC3(void);
void extcl_ppu_320_to_34x_MMC3(void);
void extcl_update_r2006_MMC3(WORD new_r2006, WORD old_r2006);
void extcl_irq_A12_clock_MMC3_alternate(void);

void extcl_cpu_wr_mem_KT008(WORD address, BYTE value);
BYTE extcl_save_mapper_KT008(BYTE mode, BYTE slot, FILE *fp);

#endif /* MAPPER_MMC3_H_ */
