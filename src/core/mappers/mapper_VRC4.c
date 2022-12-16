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
#include "cpu.h"
#include "ines.h"
#include "save_slot.h"
#include "EE93Cx6.h"

#define _chr_rom_1k_update(slot, mask, shift)\
	value = (vrc4.chr_rom_bank[slot] & mask) | ((value & 0x0F) << shift);\
	tmp = vrc4.chr_rom_high_bank[slot] | value;\
	_control_bank(tmp, info.chr.rom.max.banks_1k)\
	chr.bank_1k[slot] = chr_pnt(tmp << 10);\
	vrc4.chr_rom_bank[slot] = value
#define chr_rom_1k_update_high(slot)\
	vrc4.chr_rom_high_bank[slot] = (value & 0x10) << 4;\
	_chr_rom_1k_update(slot, 0x0F, 4)
#define chr_rom_1k_update_low(slot)\
	_chr_rom_1k_update(slot, 0xF0, 0)

INLINE static void prg_ram_fix_VRC4T230(void);
INLINE static BYTE prg_ram_check_VRC4T230(void);

_vrc4 vrc4;
struct _vrc4tmp {
	BYTE type;
	BYTE *prg_6000;
	BYTE cc93c56;
} vrc4tmp;

const BYTE shift_VRC4[7] = { 0x01, 0x00, 0x06, 0x02, 0x02, 0x00, 0x0A };
const WORD mask_VRC4[7]  = { 0x0006, 0x0003, 0x00C0, 0x000C, 0x000C, 0x0003, 0x0C00 };
const WORD table_VRC4[7][4] = {
	{0x0000, 0x0001, 0x0002, 0x0003},
	{0x0000, 0x0002, 0x0001, 0x0003},
	{0x0000, 0x0001, 0x0002, 0x0003},
	{0x0000, 0x0002, 0x0001, 0x0003},
	{0x0000, 0x0001, 0x0002, 0x0003},
	{0x0000, 0x0001, 0x0002, 0x0003},
	{0x0000, 0x0001, 0x0002, 0x0003},
};

void map_init_VRC4(BYTE revision) {
	EXTCL_CPU_WR_MEM(VRC4);
	EXTCL_SAVE_MAPPER(VRC4);
	EXTCL_CPU_EVERY_CYCLE(VRC4);
	mapper.internal_struct[0] = (BYTE *)&vrc4;
	mapper.internal_struct_size[0] = sizeof(vrc4);

	if (info.reset >= HARD) {
		BYTE i;

		memset(&vrc4, 0x00, sizeof(vrc4));
		for (i = 0; i < 8; i++) {
			vrc4.chr_rom_high_bank[i] = 0;
			vrc4.chr_rom_bank[i] = i;
		}
	} else {
		vrc4.irq_enabled = 0;
		vrc4.irq_reload = 0;
		vrc4.irq_mode = 0;
		vrc4.irq_acknowledge = 0;
		vrc4.irq_count = 0;
		vrc4.irq_prescaler = 0;
	}

	vrc4tmp.type = revision;
	vrc4tmp.cc93c56 = FALSE;
}
void extcl_cpu_wr_mem_VRC4(WORD address, BYTE value) {
	WORD tmp;

	address = address_VRC4(address);

	switch (address) {
		case 0x8000:
			control_bank_with_AND(0x1F, info.prg.rom.max.banks_8k)
			map_prg_rom_8k(1, vrc4.swap_mode, value);
			map_prg_rom_8k(1, 0x02 >> vrc4.swap_mode, info.prg.rom.max.banks_8k_before_last);
			map_prg_rom_8k_update();
			return;
		case 0xA000:
			control_bank_with_AND(0x1F, info.prg.rom.max.banks_8k)
			map_prg_rom_8k(1, 1, value);
			map_prg_rom_8k_update();
			return;
		case 0x9000:
		case 0x9001: {
			switch (value & 0x03) {
				case 0:
					mirroring_V();
					break;
				case 1:
					mirroring_H();
					break;
				case 2:
					mirroring_SCR0();
					break;
				case 3:
					mirroring_SCR1();
					break;
			}
			return;
		}
		case 0x9002:
		case 0x9003:
			value &= 0x02;
			if (vrc4.swap_mode != value) {
				WORD swap = mapper.rom_map_to[0];

				mapper.rom_map_to[0] = mapper.rom_map_to[2];
				mapper.rom_map_to[2] = swap;
				map_prg_rom_8k_update();
				vrc4.swap_mode = value;
			}
			return;
		case 0xB000:
			chr_rom_1k_update_low(0);
			return;
		case 0xB001:
			chr_rom_1k_update_high(0);
			return;
		case 0xB002:
			chr_rom_1k_update_low(1);
			return;
		case 0xB003:
			chr_rom_1k_update_high(1);
			return;
		case 0xC000:
			chr_rom_1k_update_low(2);
			return;
		case 0xC001:
			chr_rom_1k_update_high(2);
			return;
		case 0xC002:
			chr_rom_1k_update_low(3);
			return;
		case 0xC003:
			chr_rom_1k_update_high(3);
			return;
		case 0xD000:
			chr_rom_1k_update_low(4);
			return;
		case 0xD001:
			chr_rom_1k_update_high(4);
			return;
		case 0xD002:
			chr_rom_1k_update_low(5);
			return;
		case 0xD003:
			chr_rom_1k_update_high(5);
			return;
		case 0xE000:
			chr_rom_1k_update_low(6);
			return;
		case 0xE001:
			chr_rom_1k_update_high(6);
			return;
		case 0xE002:
			chr_rom_1k_update_low(7);
			return;
		case 0xE003:
			chr_rom_1k_update_high(7);
			return;
		case 0xF000:
			vrc4.irq_reload = (vrc4.irq_reload & 0xF0) | (value & 0x0F);
			return;
		case 0xF001:
			vrc4.irq_reload = (vrc4.irq_reload & 0x0F) | ((value & 0x0F) << 4);
			return;
		case 0xF002:
			vrc4.irq_acknowledge = value & 0x01;
			vrc4.irq_enabled = value & 0x02;
			vrc4.irq_mode = value & 0x04;
			if (vrc4.irq_enabled) {
				vrc4.irq_prescaler = 0;
				vrc4.irq_count = vrc4.irq_reload;
			}
			irq.high &= ~EXT_IRQ;
			return;
		case 0xF003:
			vrc4.irq_enabled = vrc4.irq_acknowledge;
			irq.high &= ~EXT_IRQ;
			return;
		default:
			return;
	}
}
BYTE extcl_save_mapper_VRC4(BYTE mode, BYTE slot, FILE *fp) {
	if (save_slot.version >= 14) {
		save_slot_ele(mode, slot, vrc4.chr_rom_high_bank);
	}
	save_slot_ele(mode, slot, vrc4.chr_rom_bank);
	save_slot_ele(mode, slot, vrc4.swap_mode);
	save_slot_ele(mode, slot, vrc4.irq_enabled);
	save_slot_ele(mode, slot, vrc4.irq_reload);
	save_slot_ele(mode, slot, vrc4.irq_mode);
	save_slot_ele(mode, slot, vrc4.irq_acknowledge);
	save_slot_ele(mode, slot, vrc4.irq_count);
	save_slot_ele(mode, slot, vrc4.irq_prescaler);

	return (EXIT_OK);
}
void extcl_cpu_every_cycle_VRC4(void) {
	if (!vrc4.irq_enabled) {
		return;
	}

	if (!vrc4.irq_mode) {
		if (vrc4.irq_prescaler < 338) {
			vrc4.irq_prescaler += 3;
			return;
		}
		vrc4.irq_prescaler -= 338;
	}

	if (vrc4.irq_count != 0xFF) {
		vrc4.irq_count++;
		return;
	}

	vrc4.irq_count = vrc4.irq_reload;
	irq.delay = TRUE;
	irq.high |= EXT_IRQ;
}

void map_init_VRC4BMC(void) {
	map_init_VRC4(VRC4E);

	EXTCL_CPU_WR_MEM(VRC4BMC);
}
void extcl_cpu_wr_mem_VRC4BMC(WORD address, BYTE value) {
	if ((address >= 0x8000) && (address <= 0x8FFF)) {
		value = (mapper.rom_map_to[0] & 0x20) | (value & 0x1F);
		control_bank(info.prg.rom.max.banks_8k)
		map_prg_rom_8k(1, vrc4.swap_mode, value);
		map_prg_rom_8k_update();
		return;
	}
	if ((address >= 0xA000) && (address <= 0xAFFF)) {
		value = (mapper.rom_map_to[0] & 0x20) | (value & 0x1F);
		control_bank(info.prg.rom.max.banks_8k)
		map_prg_rom_8k(1, 1, value);
		map_prg_rom_8k_update();
		return;
	}
	if ((address >= 0xB000) && (address <= 0xEFFF)) {
		BYTE save = value << 2 & 0x20;

		value = (mapper.rom_map_to[0] & 0x1F) | save ;
		control_bank(info.prg.rom.max.banks_8k)
		map_prg_rom_8k(1, 0, value);

		value = (mapper.rom_map_to[1] & 0x1F) | save ;
		control_bank(info.prg.rom.max.banks_8k)
		map_prg_rom_8k(1, 1, value);

		value = (mapper.rom_map_to[2] & 0x1F) | save ;
		control_bank(info.prg.rom.max.banks_8k)
		map_prg_rom_8k(1, 2, value);

		value = (mapper.rom_map_to[3] & 0x1F) | save ;
		control_bank(info.prg.rom.max.banks_8k)
		map_prg_rom_8k(1, 3, value);

		map_prg_rom_8k_update();
		return;
	}

	extcl_cpu_wr_mem_VRC4(address, value);
}

void map_init_VRC4T230(void) {
	map_init_VRC4(VRC4E);

	EXTCL_AFTER_MAPPER_INIT(VRC4T230);
	EXTCL_CPU_WR_MEM(VRC4T230);
	EXTCL_CPU_RD_MEM(VRC4T230);

	if (prg_ram_check_VRC4T230()) {
		info.prg.ram.banks_8k_plus = 1;
		info.prg.ram.bat.banks = 1;
	}
}
void extcl_after_mapper_init_VRC4T230(void) {
	if ((vrc4tmp.cc93c56 = prg_ram_check_VRC4T230())) {
		ee93cx6_init(prg.ram_plus_8k, 256, 16);
	}

	prg_ram_fix_VRC4T230();

	info.mapper.ram_plus_op_controlled_by_mapper = vrc4tmp.prg_6000 != NULL;
}
void extcl_cpu_wr_mem_VRC4T230(WORD address, BYTE value) {
	if (address < 0x8000) {
		switch (address & 0xF000) {
			case 0x6000:
			case 0x7000:
				if (vrc4tmp.prg_6000) {
					if (vrc4tmp.cc93c56 && (address >= 0x7F00)) {
						return;
					}
					vrc4tmp.prg_6000[address & 0x1FFF] = value;
				}
				break;
		}
		return;
	}
	if (address & 0x0800) {
		if (vrc4tmp.cc93c56) {
			// D~[.... .ECD]
			//          ||+- Serial Data Input to 93C56 EEPROM
			//          |+-- Serial Clock to 93C56 EEPROM
			//          +--- Chip Select to 93C56 EEPROM
			ee93cx6_write((value & 0x04) >> 2, (value & 0x02) >> 1, value & 0x01);
			return;
		}
	}
	switch (address & 0xF000) {
		case 0x8000:
			break;
		case 0xA000:
			value = (mapper.rom_map_to[0] & 0x20) | ((value & 0x1F) << 1);
			control_bank(info.prg.rom.max.banks_8k)
			map_prg_rom_8k(1, vrc4.swap_mode, value);

			value = value | 0x01;
			control_bank(info.prg.rom.max.banks_8k)
			map_prg_rom_8k(1, 1, value);

			map_prg_rom_8k_update();
			return;
		case 0xB000:
		case 0xC000:
		case 0xD000:
		case 0xE000: {
			BYTE save = value << 2 & 0x20;

			value = (mapper.rom_map_to[0] & 0x3F) | save ;
			control_bank(info.prg.rom.max.banks_8k)
			map_prg_rom_8k(1, 0, value);

			value = (mapper.rom_map_to[1] & 0x3F) | save ;
			control_bank(info.prg.rom.max.banks_8k)
			map_prg_rom_8k(1, 1, value);

			value = (mapper.rom_map_to[2] & 0x3F) | save ;
			control_bank(info.prg.rom.max.banks_8k)
			map_prg_rom_8k(1, 2, value);

			value = (mapper.rom_map_to[3] & 0x3F) | save ;
			control_bank(info.prg.rom.max.banks_8k)
			map_prg_rom_8k(1, 3, value);

			map_prg_rom_8k_update();
			return;
		}
	}
	extcl_cpu_wr_mem_VRC4(address, value);
}
BYTE extcl_cpu_rd_mem_VRC4T230(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	switch (address & 0xF000) {
		case 0x5000:
			if (vrc4tmp.cc93c56) {
				return (ee93cx6_read() ? 0x01 : 0x00);
			}
			return (0x01);
		case 0x6000:
		case 0x7000:
			if (vrc4tmp.prg_6000) {
				if (vrc4tmp.cc93c56 && (address >= 0x7F00)) {
					return (0xFF);
				}
				return (vrc4tmp.prg_6000[address & 0x1FFF]);
			}
			break;
	}
	return (openbus);
}

INLINE static void prg_ram_fix_VRC4T230(void) {
	vrc4tmp.prg_6000 = prg.ram_plus_8k ? prg.ram_plus_8k + (vrc4tmp.cc93c56 ? 256 : 0) : NULL;
}
INLINE static BYTE prg_ram_check_VRC4T230(void) {
	if (info.format == NES_2_0) {
		size_t ee_size = (ines.flags[FL10] & 0xF0) ? (64 << (ines.flags[FL10] >> 4)): 0;

		if (ee_size == 256) {
			return (TRUE);
		}
	} else {
		if (info.prg.ram.banks_8k_plus && info.prg.ram.bat.banks) {
			return (TRUE);
		}
	}
	return (FALSE);
}

WORD address_VRC4(WORD address) {
	WORD tmp = address & 0xF000;

	if ((tmp == 0x8000) || (tmp == 0xA000)) {
		return (address & 0xF000);
	} else {
		return ((address & 0xF000) | table_VRC4[vrc4tmp.type][(address & mask_VRC4[vrc4tmp.type]) >> shift_VRC4[vrc4tmp.type]]);
	}
}
