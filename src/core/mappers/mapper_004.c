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

#include <string.h>
#include "mappers.h"
#include "info.h"

void wram_fix_004_mmc6(void);

void map_init_004(void) {
	if (info.mapper.submapper == 5) {
		map_init_249();
	} else {
		EXTCL_AFTER_MAPPER_INIT(MMC3);
		EXTCL_CPU_WR_MEM(MMC3);
		EXTCL_SAVE_MAPPER(MMC3);
		EXTCL_CPU_EVERY_CYCLE(MMC3);
		EXTCL_PPU_000_TO_34X(MMC3);
		EXTCL_PPU_000_TO_255(MMC3);
		EXTCL_PPU_256_TO_319(MMC3);
		EXTCL_PPU_320_TO_34X(MMC3);
		EXTCL_UPDATE_R2006(MMC3);
		map_internal_struct_init((BYTE *)&mmc3, sizeof(mmc3));

		if (info.reset >= HARD) {
			memset(&nes[0].irqA12, 0x00, sizeof(nes[0].irqA12));
		}

		init_MMC3(info.reset);

		// Mickey Mouse III - Dream Balloon (Japan) [T-En by NikcDC v1.1] [n].nes
		if ((info.crc32.total == 0x762653B1) && (wram_size() < S8K)) {
			wram_set_ram_size(S8K);
		}

		switch (info.mapper.submapper) {
			case 1:
				EXTCL_CPU_RD_MEM(004_mmc6);
				if ((info.reset == CHANGE_ROM) || (info.reset == POWER_UP)) {
					memmap_wram_region_init(0, S512B);
				}
				MMC3_wram_fix = wram_fix_004_mmc6;
				break;
			case 4:
				EXTCL_IRQ_A12_CLOCK(MMC3_NEC);
				break;
			default:
				break;
		}

		nes[0].irqA12.present = TRUE;
		irqA12_delay = 1;
	}
}
BYTE extcl_cpu_rd_mem_004_mmc6(BYTE nidx, WORD address, BYTE openbus) {
	if ((address >= 0x7000) && (address <= 0x7FFF)) {
		return (memmap_adr_is_readable(nidx, MMCPU(address))
			? wram_rd(nidx, address)
			: memmap_adr_is_readable(nidx, MMCPU((address ^ 0x200)) ? 0x00 : openbus));
	}
	return (wram_rd(nidx, address));
}

void wram_fix_004_mmc6(void) {
	BYTE rd = TRUE, wr = TRUE;

	memmap_disable_4k(0, MMCPU(0x6000));

	rd = !(mmc3.bank_to_update & 0x20) ? FALSE : (mmc3.wram_protect & 0x20) >> 5;
	wr = rd ? (mmc3.wram_protect & 0x10) >> 4 : FALSE;
	memmap_auto_wp_512b(0, MMCPU(0x7000), 0, rd, wr);
	memmap_auto_wp_512b(0, MMCPU(0x7400), 0, rd, wr);
	memmap_auto_wp_512b(0, MMCPU(0x7800), 0, rd, wr);
	memmap_auto_wp_512b(0, MMCPU(0x7C00), 0, rd, wr);

	rd = !(mmc3.bank_to_update & 0x20) ? FALSE : (mmc3.wram_protect & 0x80) >> 7;
	wr = rd ? (mmc3.wram_protect & 0x40) >> 6 : FALSE;
	memmap_auto_wp_512b(0, MMCPU(0x7200), 1, rd, wr);
	memmap_auto_wp_512b(0, MMCPU(0x7600), 1, rd, wr);
	memmap_auto_wp_512b(0, MMCPU(0x7A00), 1, rd, wr);
	memmap_auto_wp_512b(0, MMCPU(0x7E00), 1, rd, wr);
}
