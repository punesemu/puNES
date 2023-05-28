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
#include "info.h"
#include "mem_map.h"
#include "irqA12.h"
#include "save_slot.h"

void prg_swap_mmc3_134(WORD address, WORD value);
void chr_swap_mmc3_134(WORD address, WORD value);

INLINE static void tmp_fix_134(BYTE max, BYTE index, const BYTE *ds);

struct _m134 {
	BYTE reg[4];
} m134;
struct _m134tmp {
	BYTE ds_used;
	BYTE max;
	BYTE index;
	const BYTE *dipswitch;
} m134tmp;

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
	mapper.internal_struct[0] = (BYTE *)&m134;
	mapper.internal_struct_size[0] = sizeof(m134);
	mapper.internal_struct[1] = (BYTE *)&mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);

	memset(&irqA12, 0x00, sizeof(irqA12));
	memset(&m134, 0x00, sizeof(m134));

	init_MMC3();
	MMC3_prg_swap = prg_swap_mmc3_134;
	MMC3_chr_swap = chr_swap_mmc3_134;

	if (info.reset == RESET) {
		if (m134tmp.ds_used) {
			m134tmp.index = (m134tmp.index + 1) % m134tmp.max;
		}
	} else if ((info.reset == CHANGE_ROM) || (info.reset == POWER_UP)) {
		memset(&m134tmp, 0x00, sizeof(m134tmp));
		if (info.crc32.prg == 0x7A9405C1) { // 2-in-1 - Family Kid & Aladdin 4 (Ch) [!].nes
			static const BYTE ds[] = { 0, 1 };

			tmp_fix_134(LENGTH(ds), 0, &ds[0]);
		} else if (info.crc32.prg == 0x08C80066) { // 7500-in-1.nes
			static const BYTE ds[] = {
				1, 6, 5, 7, 8, 4, 10, 11, 0,
				3, 12, 13, 14, 9, 15, 2
			};

			tmp_fix_134(LENGTH(ds), 8, &ds[0]);
		} else if (info.crc32.prg == 0x331113AB) { // 27-in-1.nes
			static const BYTE ds[] = {0, 2, 3, 1 };

			tmp_fix_134(LENGTH(ds), 0, &ds[0]);
		} else if (info.crc32.prg == 0xC03FE986) { // 9700-in-1.nes
			static const BYTE ds[] = {0, 1, 2, 3 };

			tmp_fix_134(LENGTH(ds), 0, &ds[0]);
		} else if (info.crc32.prg == 0xCC4E95E0) { // 2200000-in-1.nes
			static const BYTE ds[] = {
					4, 5, 6, 3, 2, 7, 8, 9, 10,
					11, 12, 13, 1, 14, 0, 15
			};

			tmp_fix_134(LENGTH(ds), 0, &ds[0]);
		}
	}

	info.mapper.extend_wr = TRUE;
	info.mapper.extend_rd = TRUE;

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_134(WORD address, BYTE value) {
	if ((address >= 0x6000) && (address <= 0x7FFF)) {
		if (memmap_adr_is_writable(MMCPU(address))) {
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
					extcl_cpu_wr_mem_MMC3(address, value);
					return;
			}
		}
		extcl_cpu_wr_mem_MMC3(address, value);
	}
}
BYTE extcl_cpu_rd_mem_134(WORD address, BYTE openbus) {
	if (m134tmp.ds_used && (address >= 0x8000) && (m134.reg[0] & 0x40)) {
		return (m134tmp.dipswitch[m134tmp.index]);
	}
	return (openbus);
}
BYTE extcl_save_mapper_134(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m134.reg);
	extcl_save_mapper_MMC3(mode, slot, fp);

	return (EXIT_OK);
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

INLINE static void tmp_fix_134(BYTE max, BYTE index, const BYTE *ds) {
	m134tmp.ds_used = TRUE;
	m134tmp.max = max;
	m134tmp.index = index;
	m134tmp.dipswitch = ds;
}