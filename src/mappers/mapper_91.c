/*
 * mapper_91.c
 *
 *  Created on: 03/gen/2014
 *      Author: fhorse
 */

#include <string.h>
#include "mappers.h"
#include "mem_map.h"
#include "irqA12.h"

WORD prg_rom_8k_max, chr_rom_2k_max;

void map_init_91(void) {
	prg_rom_8k_max = info.prg_rom_8k_count - 1;
	chr_rom_2k_max = (info.chr_rom_1k_count >> 1) - 1;

	EXTCL_CPU_WR_MEM(91);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);

	if (info.reset >= HARD) {
		memset(&irqA12, 0x00, sizeof(irqA12));
	}

	irqA12.present = TRUE;
	irqA12_delay = 1;

	info.mapper_extend_wr = TRUE;
}
void extcl_cpu_wr_mem_91(WORD address, BYTE value) {
	if (address < 0x6000) {
		return;
	}
	if (address <= 0x6FFF) {
		DBWORD bank;

		control_bank(chr_rom_2k_max)
		bank = value << 11;

		switch (address & 0x0003) {
			case 0:
				chr.bank_1k[0] = &chr.data[bank];
				chr.bank_1k[1] = &chr.data[bank | 0x0400];
				return;
			case 1:
				chr.bank_1k[2] = &chr.data[bank];
				chr.bank_1k[3] = &chr.data[bank | 0x0400];
				return;
			case 2:
				chr.bank_1k[4] = &chr.data[bank];
				chr.bank_1k[5] = &chr.data[bank | 0x0400];
				return;
			case 3:
				chr.bank_1k[6] = &chr.data[bank];
				chr.bank_1k[7] = &chr.data[bank | 0x0400];
				return;
		}
	}
	if (address < 0x7FFF) {
		switch (address & 0x0003) {
			case 0:
				control_bank(prg_rom_8k_max)
				map_prg_rom_8k(1, 0, value);
				map_prg_rom_8k_update();
				return;
			case 1:
				control_bank(prg_rom_8k_max)
				map_prg_rom_8k(1, 1, value);
				map_prg_rom_8k_update();
				return;
			case 2:
				extcl_cpu_wr_mem_MMC3(0xE000, value);
				return;
			case 3:
				extcl_cpu_wr_mem_MMC3(0xC000, 0x07);
				extcl_cpu_wr_mem_MMC3(0xC001, value);
				extcl_cpu_wr_mem_MMC3(0xE001, value);
				return;
		}
	}
}
