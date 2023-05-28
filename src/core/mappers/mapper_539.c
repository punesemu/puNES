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
#include "mem_map.h"
#include "save_slot.h"

INLINE static void prg_fix_539(void);
INLINE static void wram_fix_539(void);
INLINE static void mirroring_fix_539(void);

INLINE static WORD wram_address(WORD address);

struct _m539 {
	BYTE reg[2];
} m539;

void map_init_539(void) {
	EXTCL_AFTER_MAPPER_INIT(539);
	EXTCL_CPU_WR_MEM(539);
	EXTCL_CPU_RD_MEM(539);
	EXTCL_SAVE_MAPPER(539);
	mapper.internal_struct[0] = (BYTE *)&m539;
	mapper.internal_struct_size[0] = sizeof(m539);

	memset(&m539, 0x00, sizeof(m539));

	m539.reg[1] = 0x08;

//	if (!info.chr.ram.banks_8k_plus) {
//		info.chr.ram.banks_8k_plus = 1;
//	}

	info.mapper.extend_rd = TRUE;
}
void extcl_after_mapper_init_539(void) {
	prg_fix_539();
	wram_fix_539();
	mirroring_fix_539();
}
void extcl_cpu_wr_mem_539(WORD address, BYTE value) {
	switch (address & 0xFF00) {
		case 0x8200: case 0xC000: case 0xC100: case 0xC200:
		case 0xC300: case 0xC400: case 0xC500: case 0xC600: case 0xC700: case 0xC800: case 0xC900: case 0xCA00:
		case 0xCB00: case 0xCC00: case 0xCD00: case 0xCE00: case 0xCF00: case 0xD000: case 0xD100: case 0xDF00:
			wram_byte(wram_address(address)) = value;
			return;
		case 0xA000: case 0xA100: case 0xA200: case 0xA300: case 0xA400: case 0xA500: case 0xA600: case 0xA700:
		case 0xA800: case 0xA900: case 0xAA00: case 0xAB00: case 0xAC00: case 0xAD00: case 0xAE00: case 0xAF00:
			m539.reg[0] = value;
			prg_fix_539();
			return;
		case 0xF000: case 0xF100: case 0xF200: case 0xF300: case 0xF400: case 0xF500: case 0xF600: case 0xF700:
		case 0xF800: case 0xF900: case 0xFA00: case 0xFB00: case 0xFC00: case 0xFD00: case 0xFE00: case 0xFF00:
			m539.reg[1] = value;
			mirroring_fix_539();
			return;
		default:
			return;
	}
}
BYTE extcl_cpu_rd_mem_539(WORD address, BYTE openbus) {
	switch (address & 0xFF00) {
		case 0x8200: case 0xC000: case 0xC100: case 0xC200:
		case 0xC300: case 0xC400: case 0xC500: case 0xC600: case 0xC700: case 0xC800: case 0xC900: case 0xCA00:
		case 0xCB00: case 0xCC00: case 0xCD00: case 0xCE00: case 0xCF00: case 0xD000: case 0xD100: case 0xDF00:
			return (wram_byte(wram_address(address)));
		default:
			return (openbus);
	}
}
BYTE extcl_save_mapper_539(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m539.reg);

	if (mode == SAVE_SLOT_READ) {
		wram_fix_539();
	}

	return (EXIT_OK);
}

INLINE static void prg_fix_539(void) {
	WORD bank = 0;

	bank = 0x0C;
	_control_bank(bank, info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, 0, bank);

	bank = m539.reg[0] & 0x0F;
	_control_bank(bank, info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, 1, bank);

	bank = 0x0E;
	_control_bank(bank, info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, 2, bank);

	bank = 0x0F;
	_control_bank(bank, info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, 3, bank);

	map_prg_rom_8k_update();
}
INLINE static void wram_fix_539(void) {
	// CPU $6000-$7FFF: 8 KiB fixed PRG-ROM bank $D
	//
	// Certain ranges in the CPU address space are overlaid with portions of 8 KiB of PRG-RAM as follows:
	// CPU $6000-$60FF
	// CPU $6200-$62FF
	// CPU $6400-$65FF
	// CPU $8200-$82FF
	// CPU $C000-$D1FF
	// CPU $DF00-$DFFF
	memmap_prgrom_8k(MMCPU(0x6000), 0x0D);
	memmap_auto_256b(MMCPU(0x6000), 0x18);
	memmap_auto_256b(MMCPU(0x6200), 0x1A);
	memmap_auto_256b(MMCPU(0x6400), 0x1C);
	memmap_auto_256b(MMCPU(0x6500), 0x1D);
}
INLINE static void mirroring_fix_539(void) {
	if (m539.reg[1] & 0x08) {
		mirroring_H();
	} else {
		mirroring_V();
	}
}
INLINE static WORD wram_address(WORD address) {
	return ((address < 0xC000 ? 0x1000 : 0x0000) | (address < 0x8000 ? 0x0800 : 0x0000) | (address & 0x1FFF));
}
