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
#include "info.h"
#include "mem_map.h"
#include "irqA12.h"
#include "save_slot.h"

INLINE static void prg_fix_HP2018A(void);
INLINE static void chr_fix_HP2018A(void);
INLINE static void chr_swap_HP2018A(BYTE slot, WORD value);

struct _hp2018a {
	BYTE cpu5xxx[4];
	BYTE cpu8xxx[2];
	BYTE cnrom_chr_reg;
	BYTE mmc3[8];
	WORD prg_base;
} hp2018a;
struct _hp2018atmp {
	BYTE dipswitch;
} hp2018atmp;

void map_init_HP2018A(void) {
	EXTCL_AFTER_MAPPER_INIT(HP2018A);
	EXTCL_CPU_WR_MEM(HP2018A);
	EXTCL_CPU_RD_MEM(HP2018A);
	EXTCL_SAVE_MAPPER(HP2018A);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *)&hp2018a;
	mapper.internal_struct_size[0] = sizeof(hp2018a);

	memset(&hp2018a, 0x00, sizeof(hp2018a));
	memset(&irqA12, 0x00, sizeof(irqA12));

	if (info.reset == RESET) {
		hp2018atmp.dipswitch = !hp2018atmp.dipswitch ; //(hp2018atmp.dipswitch + 1) & 0x03;
	} else if (((info.reset == CHANGE_ROM) || (info.reset == POWER_UP))) {
		if (info.crc32.prg == 0x50A810A6) {
			hp2018atmp.dipswitch = 1;
		} else {
			hp2018atmp.dipswitch = 0;
		}
	}

	hp2018a.mmc3[0] = 0;
	hp2018a.mmc3[1] = 2;
	hp2018a.mmc3[2] = 4;
	hp2018a.mmc3[3] = 5;
	hp2018a.mmc3[4] = 6;
	hp2018a.mmc3[5] = 7;
	hp2018a.mmc3[6] = 0;
	hp2018a.mmc3[7] = 0;

	info.prg.ram.banks_8k_plus = 1;

	info.mapper.extend_wr = TRUE;

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_after_mapper_init_HP2018A(void) {
	prg_fix_HP2018A();
	chr_fix_HP2018A();
}
void extcl_cpu_wr_mem_HP2018A(WORD address, BYTE value) {
	if (address < 0x8000) {
		if ((address >= 0x5000) && (address <= 0x5FFF)) {
			if (hp2018a.cpu5xxx[0] & 0x80) {
				return;
			}
			switch (address & 0x03) {
				case 0:
					hp2018a.cpu5xxx[0] = value;
					prg_fix_HP2018A();
					chr_fix_HP2018A();
					break;
				case 1:
					hp2018a.cpu5xxx[1] = value;
					hp2018a.prg_base = (hp2018a.prg_base & ~0x3F) | (value & 0x3F);
					prg_fix_HP2018A();
					break;
				case 2:
					hp2018a.cpu5xxx[2] = value;
					chr_fix_HP2018A();
					break;
				case 3:
					hp2018a.cpu5xxx[3] = value;
					break;
			}
		}
	} else {
		if ((hp2018a.cpu5xxx[0] & 0x07) >= 6) {
			hp2018a.cnrom_chr_reg = value & 0x03;
			chr_fix_HP2018A();
		}
		switch (address & 0xE001) {
			case 0x8000:
				hp2018a.cpu8xxx[0] = value;
				prg_fix_HP2018A();
				chr_fix_HP2018A();
				break;
			case 0x8001:
				hp2018a.cpu8xxx[1] = value;
				hp2018a.mmc3[hp2018a.cpu8xxx[0] & 0x07] = value;
				prg_fix_HP2018A();
				chr_fix_HP2018A();
				break;
			case 0xA000:
			case 0xA001:
			case 0xC000:
			case 0xC001:
			case 0xE000:
			case 0xE001:
				extcl_cpu_wr_mem_MMC3(address, value);
				break;
		}
	}
}
BYTE extcl_cpu_rd_mem_HP2018A(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	if ((address >= 0x5000) && (address <= 0x5FFF)) {
		return ((openbus & ~0x03) | (hp2018atmp.dipswitch & 0x03));
	}
	return (openbus);
}
BYTE extcl_save_mapper_HP2018A(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, hp2018a.cpu5xxx);
	save_slot_ele(mode, slot, hp2018a.cpu8xxx);
	save_slot_ele(mode, slot, hp2018a.cnrom_chr_reg);
	save_slot_ele(mode, slot, hp2018a.mmc3);
	save_slot_ele(mode, slot, hp2018a.prg_base);

	return (EXIT_OK);
}

INLINE static void prg_fix_HP2018A(void) {
	WORD bank[4];

	switch (hp2018a.cpu5xxx[0] & 0x07) {
		case 0:
		case 1:
		case 2:
		case 3: {
			BYTE swap = (hp2018a.cpu8xxx[0] & 0x40) >> 5;
			WORD base = hp2018a.prg_base << 1;
			BYTE mask = hp2018a.cpu5xxx[0] & 0x02 ? 0x0F : 0x1F;

			base &= ~mask;

			bank[0] = base | (hp2018a.mmc3[6] & mask);
			bank[1] = base | (hp2018a.mmc3[7] & mask);
			bank[2] = base | (0xFE & mask);
			bank[3] = base | (0xFF & mask);

			_control_bank(bank[0], info.prg.rom.max.banks_8k)
			map_prg_rom_8k(1, 0 ^ swap, bank[0]);

			_control_bank(bank[1], info.prg.rom.max.banks_8k)
			map_prg_rom_8k(1, 1, bank[1]);

			_control_bank(bank[2], info.prg.rom.max.banks_8k)
			map_prg_rom_8k(1, 2 ^ swap, bank[2]);

			_control_bank(bank[3], info.prg.rom.max.banks_8k)
			map_prg_rom_8k(1, 3, bank[3]);
			break;
		}
		case 4:
			bank[0] = hp2018a.prg_base;
			_control_bank(bank[0], info.prg.rom.max.banks_16k)
			map_prg_rom_8k(2, 0, bank[0]);
			map_prg_rom_8k(2, 2, bank[0]);
			break;
		case 5:
		case 6:
		case 7:
			bank[0] = hp2018a.prg_base >> 1;
			_control_bank(bank[0], info.prg.rom.max.banks_32k)
			map_prg_rom_8k(4, 0, bank[0]);
			break;
	}
	map_prg_rom_8k_update();
}
INLINE static void chr_fix_HP2018A(void) {
	WORD mask, bank[8], base = hp2018a.cpu5xxx[2] << 3;
	BYTE swap = 0;

	switch (hp2018a.cpu5xxx[0] & 0x07) {
		default:
		case 0:
		case 1:
		case 2:
		case 3:
			swap = (hp2018a.cpu8xxx[0] & 0x80) >> 5;
			mask = hp2018a.cpu5xxx[0] & 0x01 ? 0x7F : 0xFF;
			base &= ~mask;

			bank[0] = base | ((hp2018a.mmc3[0] & 0xFE) & mask);
			bank[1] = base | ((hp2018a.mmc3[0] | 0x01) & mask);
			bank[2] = base | ((hp2018a.mmc3[1] & 0xFE) & mask);
			bank[3] = base | ((hp2018a.mmc3[1] | 0x01) & mask);
			bank[4] = base | (hp2018a.mmc3[2] & mask);
			bank[5] = base | (hp2018a.mmc3[3] & mask);
			bank[6] = base | (hp2018a.mmc3[4] & mask);
			bank[7] = base | (hp2018a.mmc3[5] & mask);
			break;
		case 4:
		case 5:
			bank[0] = base | 0;
			bank[1] = base | 1;
			bank[2] = base | 2;
			bank[3] = base | 3;
			bank[4] = base | 4;
			bank[5] = base | 5;
			bank[6] = base | 6;
			bank[7] = base | 7;
			break;
		case 6:
		case 7:
			mask = hp2018a.cpu5xxx[0] & 0x01 ? 0x03 : 0x01;
			base = ((hp2018a.cpu5xxx[2] & ~mask) | (hp2018a.cnrom_chr_reg & mask)) << 3;

			bank[0] = base | 0;
			bank[1] = base | 1;
			bank[2] = base | 2;
			bank[3] = base | 3;
			bank[4] = base | 4;
			bank[5] = base | 5;
			bank[6] = base | 6;
			bank[7] = base | 7;
			break;
	}
	chr_swap_HP2018A(0 ^ swap, bank[0]);
	chr_swap_HP2018A(1 ^ swap, bank[1]);
	chr_swap_HP2018A(2 ^ swap, bank[2]);
	chr_swap_HP2018A(3 ^ swap, bank[3]);
	chr_swap_HP2018A(4 ^ swap, bank[4]);
	chr_swap_HP2018A(5 ^ swap, bank[5]);
	chr_swap_HP2018A(6 ^ swap, bank[6]);
	chr_swap_HP2018A(7 ^ swap, bank[7]);
}
INLINE static void chr_swap_HP2018A(BYTE slot, WORD value) {
	control_bank(info.chr.rom.max.banks_1k)
	chr.bank_1k[slot] = chr_pnt(value << 10);
}
