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

#include <string.h>
#include "mappers.h"
#include "mem_map.h"
#include "save_slot.h"

INLINE static void prg_fix_447(void);
INLINE static void chr_fix_447(void);

static const SBYTE dipswitch_447[][4] = {
	{  0, -1, -1, -1 }, // 0
	{  0,  1,  2,  3 }, // 1
};
struct _m447 {
	BYTE reg;
	BYTE prg[2];
	WORD chr[8];
	BYTE swap;
} m447;
struct _m447tmp {
	BYTE select;
	BYTE index;
	WORD dipswitch;
} m447tmp;

void map_init_447(void) {
	map_init_VRC4(VRC4E);

	EXTCL_AFTER_MAPPER_INIT(447);
	EXTCL_CPU_WR_MEM(447);
	EXTCL_CPU_RD_MEM(447);
	EXTCL_SAVE_MAPPER(447);
	mapper.internal_struct[0] = (BYTE *)&m447;
	mapper.internal_struct_size[0] = sizeof(m447);
	mapper.internal_struct[1] = (BYTE *)&vrc4;
	mapper.internal_struct_size[1] = sizeof(vrc4);

	memset(&m447, 0x00, sizeof(m447));

	if (info.reset == RESET) {
		do {
			m447tmp.index = (m447tmp.index + 1) & 0x03;
		} while (dipswitch_447[m447tmp.select][m447tmp.index] < 0);
	} else if (((info.reset == CHANGE_ROM) || (info.reset == POWER_UP))) {
		if (info.crc32.prg == 0x8A929147) { // 1993 New 860-in-1 Over-Valued Golden Version Games.nes
			m447tmp.select = 1;
			m447tmp.index = 0;
		} else {
			m447tmp.select = 0;
			m447tmp.index = 0;
		}
	}

	m447tmp.dipswitch = dipswitch_447[m447tmp.select][m447tmp.index];

	info.mapper.extend_wr = info.mapper.extend_rd = TRUE;
}
void extcl_after_mapper_init_447(void) {
	prg_fix_447();
	chr_fix_447();
}
void extcl_cpu_wr_mem_447(WORD address, BYTE value) {
	WORD vrc4_address = address_VRC4(address);

	if ((address >= 0x6000) && (address <= 0x7FFF)) {
		if (!(m447.reg & 0x01)) {
			m447.reg = address & 0xFF;
			prg_fix_447();
			chr_fix_447();
			return;
		}
	}
	if (address >= 0x8000) {
		switch (vrc4_address) {
			case 0x8000:
				m447.prg[0] = value;
				break;
			case 0xA000:
				m447.prg[1] = value;
				break;
			case 0x9002:
			case 0x9003:
				m447.swap = value & 0x02;
				break;
			case 0xB000:
			case 0xB001:
			case 0xB002:
			case 0xB003:
			case 0xC000:
			case 0xC001:
			case 0xC002:
			case 0xC003:
			case 0xD000:
			case 0xD001:
			case 0xD002:
			case 0xD003:
			case 0xE000:
			case 0xE001:
			case 0xE002:
			case 0xE003: {
				BYTE reg = ((vrc4_address - 0xB000) >> 11) | ((vrc4_address & 0x0003) >> 1);

				m447.chr[reg] = vrc4_address & 0x0001 ?
					(m447.chr[reg] & 0x000F) | (value << 4) :
					(m447.chr[reg] & 0x0FF0) | (value & 0x0F);
				break;
			}
			default:
				extcl_cpu_wr_mem_VRC4(address, value);
				break;
		}
		prg_fix_447();
		chr_fix_447();
	}
}
BYTE extcl_cpu_rd_mem_447(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	if ((address >= 0x8000) && m447tmp.select && (m447.reg & 0x08)) {
		return (prg_rom_rd(((address & 0xFFFC) | m447tmp.dipswitch)));
	}
	return (openbus);
}
BYTE extcl_save_mapper_447(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m447.reg);
	save_slot_ele(mode, slot, m447.prg);
	save_slot_ele(mode, slot, m447.chr);
	save_slot_ele(mode, slot, m447.swap);
	save_slot_ele(mode, slot, m447tmp.index);
	save_slot_ele(mode, slot, m447tmp.dipswitch);
	extcl_save_mapper_VRC4(mode, slot, fp);

	return (EXIT_OK);
}

INLINE static void prg_fix_447(void) {
	WORD base = m447.reg << 4;
	WORD bank;

	if (m447.reg & 0x04) {
		WORD A14 = ~m447.reg & 0x02;

		bank = base | ((m447.prg[0] & ~A14) & 0x0F);
		_control_bank(bank, info.prg.rom.max.banks_8k)
		map_prg_rom_8k(1, 0, bank);

		bank = base | ((m447.prg[1] & ~A14) & 0x0F);
		_control_bank(bank, info.prg.rom.max.banks_8k)
		map_prg_rom_8k(1, 1, bank);

		bank = base | ((m447.prg[0] | A14) & 0x0F);
		_control_bank(bank, info.prg.rom.max.banks_8k)
		map_prg_rom_8k(1, 2, bank);

		bank = base | ((m447.prg[1] | A14) & 0x0F);
		_control_bank(bank, info.prg.rom.max.banks_8k)
		map_prg_rom_8k(1, 3, bank);
	} else {
		bank = base | (m447.prg[0] & 0x0F);
		_control_bank(bank, info.prg.rom.max.banks_8k)
		map_prg_rom_8k(1, 0 ^ m447.swap, bank);

		bank = base | (m447.prg[1] & 0x0F);
		_control_bank(bank, info.prg.rom.max.banks_8k)
		map_prg_rom_8k(1, 1, bank);

		bank = base | (0xFE & 0x0F);
		_control_bank(bank, info.prg.rom.max.banks_8k)
		map_prg_rom_8k(1, 2 ^ m447.swap, bank);

		bank = base | (0xFF & 0x0F);
		_control_bank(bank, info.prg.rom.max.banks_8k)
		map_prg_rom_8k(1, 3, bank);
	}
	map_prg_rom_8k_update();
}
INLINE static void chr_fix_447(void) {
	WORD base = m447.reg << 7;
	WORD mask = 0x7F;
	DBWORD bank;

	bank = base | (m447.chr[0] & mask);
	_control_bank(bank, info.chr.rom.max.banks_1k)
	chr.bank_1k[0] = chr_pnt(bank << 10);

	bank = base | (m447.chr[1] & mask);
	_control_bank(bank, info.chr.rom.max.banks_1k)
	chr.bank_1k[1] = chr_pnt(bank << 10);

	bank = base | (m447.chr[2] & mask);
	_control_bank(bank, info.chr.rom.max.banks_1k)
	chr.bank_1k[2] = chr_pnt(bank << 10);

	bank = base | (m447.chr[3] & mask);
	_control_bank(bank, info.chr.rom.max.banks_1k)
	chr.bank_1k[3] = chr_pnt(bank << 10);

	bank = base | (m447.chr[4] & mask);
	_control_bank(bank, info.chr.rom.max.banks_1k)
	chr.bank_1k[4] = chr_pnt(bank << 10);

	bank = base | (m447.chr[5] & mask);
	_control_bank(bank, info.chr.rom.max.banks_1k)
	chr.bank_1k[5] = chr_pnt(bank << 10);

	bank = base | (m447.chr[6] & mask);
	_control_bank(bank, info.chr.rom.max.banks_1k)
	chr.bank_1k[6] = chr_pnt(bank << 10);

	bank = base | (m447.chr[7] & mask);
	_control_bank(bank, info.chr.rom.max.banks_1k)
	chr.bank_1k[7] = chr_pnt(bank << 10);
}
