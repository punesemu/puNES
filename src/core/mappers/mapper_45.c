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

void prg_swap_45(WORD address, WORD value);
void chr_swap_45(WORD address, WORD value);

INLINE static void tmp_fix_45(BYTE max, BYTE index, const BYTE *ds);

struct _m45 {
	BYTE index;
	BYTE reg[4];
} m45;
struct _m45tmp {
	BYTE ds_used;
	BYTE max;
	BYTE index;
	const BYTE *dipswitch;
} m45tmp;

void map_init_45(void) {
	EXTCL_AFTER_MAPPER_INIT(MMC3);
	EXTCL_CPU_WR_MEM(45);
	EXTCL_CPU_RD_MEM(45);
	EXTCL_SAVE_MAPPER(45);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *)&m45;
	mapper.internal_struct_size[0] = sizeof(m45);
	mapper.internal_struct[1] = (BYTE *)&mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);

	memset(&irqA12, 0x00, sizeof(irqA12));
	memset(&m45, 0x00, sizeof(m45));

	m45.reg[2] = 0x0F;

	init_MMC3();
	MMC3_prg_swap = prg_swap_45;
	MMC3_chr_swap = chr_swap_45;

	if (info.reset == RESET) {
		if (m45tmp.ds_used) {
			m45tmp.index = (m45tmp.index + 1) % m45tmp.max;
		}
	} else if (((info.reset == CHANGE_ROM) || (info.reset == POWER_UP))) {
		memset(&m45tmp, 0x00, sizeof(m45tmp));
		if (info.crc32.prg == 0x2011376B) { // 98+1800000-in-1.nes
			static const BYTE ds[] = { 0,  4,  3,  2 };

			tmp_fix_45(LENGTH(ds), 0, &ds[0]);
		} else if (
			(info.crc32.prg == 0x70EE2D30) || // Super 19-in-1 (329-JY818).nes
			(info.crc32.prg == 0x07FDD349) || // Super 1000000-in-1 [p1][!].nes
			(info.crc32.prg == 0x899AEB47)) { // Super 19-in-1 (329-JY819).nes
			static const BYTE ds[] = { 0,  1,  2 };

			tmp_fix_45(LENGTH(ds), 0, &ds[0]);
		} else if (info.crc32.prg == 0x28E0CF3B) { // Brain Series 12-in-1 [p1][!].nes
			static const BYTE ds[] = { 0,  2 };

			tmp_fix_45(LENGTH(ds), 0, &ds[0]);
		}
	}

	info.mapper.extend_wr = info.mapper.extend_rd = TRUE;

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_45(WORD address, BYTE value) {
	if ((address >= 0x6000) && (address <= 0x7FFF)) {
		if (cpu.prg_ram_wr_active && !(m45.reg[3] & 0x40)) {
			m45.reg[m45.index] = value;
			m45.index = (m45.index + 1) & 0x03;
			MMC3_prg_fix();
			MMC3_chr_fix();
		}
		return;
	}
	if (address >= 0x8000) {
		extcl_cpu_wr_mem_MMC3(address, value);
	}
}
BYTE extcl_cpu_rd_mem_45(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	switch (address & 0xF000) {
		case 0x5000:
			if (!cpu.prg_ram_rd_active || !m45tmp.ds_used) {
				return (openbus);
			}
			if (~m45tmp.dipswitch[m45tmp.index] & address) {
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
			if (!m45tmp.ds_used) {
				return (openbus);
			}
			switch (m45tmp.dipswitch[m45tmp.index]) {
				case 1:
					return (m45.reg[1] & 0x80 ? 0xFF : openbus);
				case 2:
					return (m45.reg[2] & 0x40 ? 0xFF : openbus);
				case 3:
					return (m45.reg[1] & 0x40 ? 0xFF : openbus);
				case 4:
					return (m45.reg[2] & 0x20 ? 0xFF : openbus);
				default:
					return (openbus);
			}
			break;
		default:
			return (openbus);
	}
}
BYTE extcl_save_mapper_45(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m45.index);
	save_slot_ele(mode, slot, m45.reg);
	save_slot_ele(mode, slot, m45tmp.index);
	save_slot_ele(mode, slot, m45tmp.dipswitch);
	extcl_save_mapper_MMC3(mode, slot, fp);

	if (mode == SAVE_SLOT_READ) {
		MMC3_chr_fix();
	}

	return (EXIT_OK);
}

void prg_swap_45(WORD address, WORD value) {
	WORD base = m45.reg[1] | ((m45.reg[2] & 0xC0) << 2);
	WORD mask = ~m45.reg[3] & 0x3F;

	value = base | (value & mask);
	control_bank(info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, (address >> 13) & 0x03, value);
	map_prg_rom_8k_update();
}
void chr_swap_45(WORD address, WORD value) {
	if (mapper.write_vram && (info.chr.rom.max.banks_8k == 1)) {
		value = address >> 10;
		chr.bank_1k[address >> 10] = chr_pnt(value << 10);
	} else {
		WORD base = m45.reg[0] | ((m45.reg[2] & 0xF0) << 4);
		WORD mask = 0xFF >> (~m45.reg[2] & 0x0F);

		value = base | (value & mask);
		control_bank(info.chr.rom.max.banks_1k)
		chr.bank_1k[address >> 10] = chr_pnt(value << 10);
	}
}

INLINE static void tmp_fix_45(BYTE max, BYTE index, const BYTE *ds) {
	m45tmp.ds_used = TRUE;
	m45tmp.max = max;
	m45tmp.index = index;
	m45tmp.dipswitch = ds;
}