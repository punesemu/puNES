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

void prg_swap_mmc3_045(WORD address, WORD value);
void chr_swap_mmc3_045(WORD address, WORD value);

INLINE static void tmp_fix_045(BYTE max, BYTE index, const BYTE *ds);

struct _m045 {
	BYTE index;
	BYTE reg[4];
} m045;
struct _m045tmp {
	BYTE ds_used;
	BYTE max;
	BYTE index;
	const BYTE *dipswitch;
} m045tmp;

void map_init_045(void) {
	EXTCL_AFTER_MAPPER_INIT(MMC3);
	EXTCL_CPU_WR_MEM(045);
	EXTCL_CPU_RD_MEM(045);
	EXTCL_SAVE_MAPPER(045);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *)&m045;
	mapper.internal_struct_size[0] = sizeof(m045);
	mapper.internal_struct[1] = (BYTE *)&mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);

	memset(&irqA12, 0x00, sizeof(irqA12));
	memset(&m045, 0x00, sizeof(m045));

	m045.reg[2] = 0x0F;

	init_MMC3();
	MMC3_prg_swap = prg_swap_mmc3_045;
	MMC3_chr_swap = chr_swap_mmc3_045;

	if (info.reset == RESET) {
		if (m045tmp.ds_used) {
			m045tmp.index = (m045tmp.index + 1) % m045tmp.max;
		}
	} else if (((info.reset == CHANGE_ROM) || (info.reset == POWER_UP))) {
		memset(&m045tmp, 0x00, sizeof(m045tmp));
		if (info.crc32.prg == 0x2011376B) { // 98+1800000-in-1.nes
			static const BYTE ds[] = { 0,  4,  3,  2 };

			tmp_fix_045(LENGTH(ds), 0, &ds[0]);
		} else if (
			(info.crc32.prg == 0x70EE2D30) || // Super 19-in-1 (329-JY818).nes
			(info.crc32.prg == 0x07FDD349) || // Super 1000000-in-1 [p1][!].nes
			(info.crc32.prg == 0x899AEB47)) { // Super 19-in-1 (329-JY819).nes
			static const BYTE ds[] = { 0,  1,  2 };

			tmp_fix_045(LENGTH(ds), 0, &ds[0]);
		} else if (info.crc32.prg == 0x28E0CF3B) { // Brain Series 12-in-1 [p1][!].nes
			static const BYTE ds[] = { 0,  2 };

			tmp_fix_045(LENGTH(ds), 0, &ds[0]);
		}
	}

	info.mapper.extend_wr = TRUE;
	info.mapper.extend_rd = TRUE;

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_045(WORD address, BYTE value) {
	if ((address >= 0x6000) && (address <= 0x7FFF)) {
		if (!(m045.reg[3] & 0x40) && memmap_adr_is_writable(address)) {
			m045.reg[m045.index] = value;
			m045.index = (m045.index + 1) & 0x03;
			MMC3_prg_fix();
			MMC3_chr_fix();
		}
		return;
	}
	if (address >= 0x8000) {
		extcl_cpu_wr_mem_MMC3(address, value);
	}
}
BYTE extcl_cpu_rd_mem_045(WORD address, BYTE openbus) {
	switch (address & 0xF000) {
		case 0x5000:
			if (!m045tmp.ds_used || !memmap_adr_is_readable(address)) {
				return (openbus);
			}
			if (~m045tmp.dipswitch[m045tmp.index] & address) {
				return (0x01);
			} else {
				return (0x00);
			}
			break;
		case 0x8000:
		case 0x9000:
		case 0xA000:
		case 0xB000:
		case 0xC000:
		case 0xD000:
		case 0xE000:
		case 0xF000:
			if (!m045tmp.ds_used) {
				return (openbus);
			}
			switch (m045tmp.dipswitch[m045tmp.index]) {
				case 1:
					return (m045.reg[1] & 0x80 ? 0xFF : openbus);
				case 2:
					return (m045.reg[2] & 0x40 ? 0xFF : openbus);
				case 3:
					return (m045.reg[1] & 0x40 ? 0xFF : openbus);
				case 4:
					return (m045.reg[2] & 0x20 ? 0xFF : openbus);
				default:
					return (openbus);
			}
			break;
		default:
			return (openbus);
	}
}
BYTE extcl_save_mapper_045(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m045.index);
	save_slot_ele(mode, slot, m045.reg);
	save_slot_ele(mode, slot, m045tmp.index);
	save_slot_ele(mode, slot, m045tmp.dipswitch);
	extcl_save_mapper_MMC3(mode, slot, fp);

	if (mode == SAVE_SLOT_READ) {
		MMC3_chr_fix();
	}

	return (EXIT_OK);
}

void prg_swap_mmc3_045(WORD address, WORD value) {
	WORD base = m045.reg[1] | ((m045.reg[2] & 0xC0) << 2);
	WORD mask = ~m045.reg[3] & 0x3F;

	prg_swap_MMC3_base(address, (base | (value & mask)));
}
void chr_swap_mmc3_045(WORD address, WORD value) {
	if (mapper.write_vram && (info.chr.rom.max.banks_8k == 1)) {
		value = address >> 10;
		chr.bank_1k[address >> 10] = chr_pnt(value << 10);
	} else {
		WORD base = m045.reg[0] | ((m045.reg[2] & 0xF0) << 4);
		WORD mask = 0xFF >> (~m045.reg[2] & 0x0F);

		chr_swap_MMC3_base(address, (base | (value & mask)));
	}
}

INLINE static void tmp_fix_045(BYTE max, BYTE index, const BYTE *ds) {
	m045tmp.ds_used = TRUE;
	m045tmp.max = max;
	m045tmp.index = index;
	m045tmp.dipswitch = ds;
}