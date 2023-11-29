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

#include <string.h>
#include "mappers.h"
#include "save_slot.h"

void prg_swap_mmc3_134(WORD address, WORD value);
void chr_swap_mmc3_134(WORD address, WORD value);

struct _m134 {
	BYTE reg[4];
} m134;

void map_init_134(void) {
	EXTCL_AFTER_MAPPER_INIT(MMC3);
	EXTCL_CPU_WR_MEM(134);
	EXTCL_CPU_RD_MEM(134);
	EXTCL_SAVE_MAPPER(134);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	map_internal_struct_init((BYTE *)&m134, sizeof(m134));
	map_internal_struct_init((BYTE *)&mmc3, sizeof(mmc3));

	if (info.reset >= HARD) {
		memset(&nes[0].irqA12, 0x00, sizeof(nes[0].irqA12));
	}

	memset(&m134, 0x00, sizeof(m134));

	init_MMC3(info.reset);
	MMC3_prg_swap = prg_swap_mmc3_134;
	MMC3_chr_swap = chr_swap_mmc3_134;

	info.mapper.extend_wr = TRUE;
	info.mapper.extend_rd = TRUE;

	nes[0].irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_134(BYTE nidx, WORD address, BYTE value) {
	if ((address >= 0x6000) && (address <= 0x7FFF)) {
		if (memmap_adr_is_writable(nidx, MMCPU(address))) {
			switch (address & 0x0003) {
				case 0:
					if (!(m134.reg[0] & 0x80)) {
						m134.reg[0] = value;
						MMC3_prg_fix();
						MMC3_chr_fix();
					}
					break;
				case 1:
					if (!(m134.reg[0] & 0x80)) {
						m134.reg[1] = value;
						MMC3_prg_fix();
						MMC3_chr_fix();
					}
					break;
				case 2:
					if (m134.reg[0] & 0x80) {
						value = (m134.reg[2] & 0xFC) | (value & 0x03);
					}
					m134.reg[2] = value;
					MMC3_chr_fix();
					break;
				case 3:
					if (!(m134.reg[0] & 0x80)) {
						m134.reg[3] = value;
					}
					break;
			}
		}
		return;
	}
	if (address >= 0x8000) {
		if ((address & 0xE001) == 0x8001) {
			switch (mmc3.bank_to_update & 0x07) {
				case 6:
					mmc3.reg[6] = value;

					if (m134.reg[1] & 0x80) {
						MMC3_prg_fix();
					} else {
						if (mmc3.bank_to_update & 0x40) {
							MMC3_prg_swap(0xC000, value);
						} else {
							MMC3_prg_swap(0x8000, value);
						}
					}
					return;
				default:
					extcl_cpu_wr_mem_MMC3(nidx, address, value);
					return;
			}
		}
		extcl_cpu_wr_mem_MMC3(nidx, address, value);
	}
}
BYTE extcl_cpu_rd_mem_134(BYTE nidx, WORD address, UNUSED(BYTE openbus)) {
	if (address >= 0x8000) {
		return (m134.reg[0] & 0x40 ? dipswitch.value : prgrom_rd(nidx, address));
	}
	return (wram_rd(nidx, address));
}
BYTE extcl_save_mapper_134(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m134.reg);
	return (extcl_save_mapper_MMC3(mode, slot, fp));
}

void prg_swap_mmc3_134(WORD address, WORD value) {
	WORD base = ((m134.reg[1] & 0x03) << 4) | ((m134.reg[0] & 0x10) << 2);
	WORD mask = m134.reg[1] & 0x04 ? 0x0F: 0x1F;

	// NROM mode
	if (m134.reg[1] & 0x80) {
		value = (mmc3.reg[6] & (m134.reg[1] & 0x08 ? 0xFE : 0xFC)) |
			((address >> 13) & (m134.reg[1] & 0x08 ? 0x01 : 0x03));
	}
	prg_swap_MMC3_base(address, ((base & ~mask) | (value & mask)));
}
void chr_swap_mmc3_134(WORD address, WORD value) {
	WORD base = ((m134.reg[1] & 0x30) << 3) | ((m134.reg[0] & 0x20) << 4);
	WORD mask = m134.reg[1] & 0x40 ? 0x7F: 0xFF;

	if (m134.reg[0] & 0x08) {
		value = ((m134.reg[2] & mask) << 3) | (address >> 10);
	}
	chr_swap_MMC3_base(address, ((base & ~mask) | (value & mask)));
}
