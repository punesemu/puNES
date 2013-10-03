/*
 * mapper_MMC3.h
 *
 *  Created on: 24/feb/2011
 *      Author: fhorse
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
	NAMCO3413 = 2,
	NAMCO3414 = 3,
	NAMCO3415 = 4,
	NAMCO3416 = 5,
	NAMCO3417 = 6,
	NAMCO3451 = 7,
	TKROM     = 8,
	SMB2EREZA = 9,
	SMB2JSMB1 = 10,
	RADRACER2 = 11,
	MMC3_ALTERNATE = 12
};

struct _mmc3 {
	BYTE prg_ram_protect;
	BYTE bank_to_update;
	BYTE prg_rom_cfg;
	BYTE chr_rom_cfg;
} mmc3;

void map_init_MMC3(void);
void extcl_cpu_wr_mem_MMC3(WORD address, BYTE value);
BYTE extcl_save_mapper_MMC3(BYTE mode, BYTE slot, FILE *fp);
void extcl_cpu_every_cycle_MMC3(void);
void extcl_ppu_000_to_34x_MMC3(void);
void extcl_ppu_000_to_255_MMC3(void);
void extcl_ppu_256_to_319_MMC3(void);
void extcl_ppu_320_to_34x_MMC3(void);
void extcl_update_r2006_MMC3(WORD old_r2006);
void extcl_irqA12_clock_MMC3_alternate(void);

#endif /* MAPPER_MMC3_H_ */
