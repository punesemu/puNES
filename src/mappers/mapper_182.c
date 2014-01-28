/*
 * mapper_182.c
 *
 *  Created on: 8/ott/2011
 *      Author: fhorse
 */

#include <stdlib.h>
#include <string.h>
#include "mappers.h"
#include "mem_map.h"
#include "irqA12.h"
#include "save_slot.h"

WORD prg_rom_8k_before_last, chr_rom_2k_max, chr_rom_1k_max;

void map_init_182(void) {
	prg_rom_8k_before_last = info.prg.rom.banks_8k - 2;
	chr_rom_2k_max = (info.chr.rom.banks_1k >> 1) - 1;
	chr_rom_1k_max = info.chr.rom.banks_1k - 1;

	EXTCL_CPU_WR_MEM(182);
	EXTCL_SAVE_MAPPER(MMC3);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *) &mmc3;
	mapper.internal_struct_size[0] = sizeof(mmc3);

	if (info.reset >= HARD) {
		memset(&mmc3, 0x00, sizeof(mmc3));
		memset(&irqA12, 0x00, sizeof(irqA12));
	}

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_182(WORD address, BYTE value) {
	switch (address & 0xE001) {
		case 0x8001:
			extcl_cpu_wr_mem_MMC3(0xA000, value);
			return;
		case 0xA000:
			extcl_cpu_wr_mem_MMC3(0x8000, value);
			return;
		case 0xC000: {
			switch (mmc3.bank_to_update) {
				case 0: {
					DBWORD bank;

					value >>= 1;
					control_bank(chr_rom_2k_max)
					bank = value << 11;
					chr.bank_1k[mmc3.chr_rom_cfg] = &chr.data[bank];
					chr.bank_1k[mmc3.chr_rom_cfg | 0x01] = &chr.data[bank | 0x0400];
					break;
				}
				case 1:
					control_bank(chr_rom_1k_max)
					chr.bank_1k[(mmc3.chr_rom_cfg ^ 0x04) | 0x01] = &chr.data[value << 10];
					break;
				case 2: {
					DBWORD bank;

					value >>= 1;
					control_bank(chr_rom_2k_max)
					bank = value << 11;
					chr.bank_1k[mmc3.chr_rom_cfg | 0x02] = &chr.data[bank];
					chr.bank_1k[mmc3.chr_rom_cfg | 0x03] = &chr.data[bank | 0x0400];
					break;
				}
				case 3:
					control_bank(chr_rom_1k_max)
					chr.bank_1k[(mmc3.chr_rom_cfg ^ 0x04) | 0x03] = &chr.data[value << 10];
					break;
				case 4:
					control_bank(info.prg.rom.max.banks_8k)
					map_prg_rom_8k(1, mmc3.prg_rom_cfg, value);
					map_prg_rom_8k_update();
					break;
				case 5:
					control_bank(info.prg.rom.max.banks_8k)
					map_prg_rom_8k(1, 1, value);
					map_prg_rom_8k_update();
					break;
				case 6:
					control_bank(chr_rom_1k_max)
					chr.bank_1k[mmc3.chr_rom_cfg ^ 0x04] = &chr.data[value << 10];
					break;
				case 7:
					control_bank(chr_rom_1k_max)
					chr.bank_1k[(mmc3.chr_rom_cfg ^ 0x04) | 0x02] = &chr.data[value << 10];
					break;
			}
			return;
		}
		case 0xC001:
			irqA12.latch = value;
			irqA12.reload = TRUE;
			irqA12.counter = 0;
			return;
		case 0xE000:
		case 0xE001:
			extcl_cpu_wr_mem_MMC3(address, value);
			return;
	}
}
