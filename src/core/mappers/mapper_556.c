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

#include <stdlib.h>
#include <string.h>
#include "mappers.h"
#include "info.h"
#include "mem_map.h"
#include "irqA12.h"
#include "save_slot.h"

INLINE static void prg_fix_556(void);
INLINE static void chr_fix_556(void);

INLINE static WORD prg_base(void);
INLINE static WORD prg_mask(void);
INLINE static WORD chr_base(void);
INLINE static WORD chr_mask(void);

void prg_swap_556_mmc3(WORD address, WORD value);
void chr_swap_556_mmc3(WORD address, WORD value);

INLINE static void cpu_wr_mem_vrc4(WORD address, BYTE value);
INLINE static void prg_fix_vrc4(void);
INLINE static void chr_fix_vrc4(void);

struct _m556 {
	WORD index;
	WORD reg[4];
	// VRC4
	struct _m556_vrc4 {
		BYTE prg[3];
		WORD chr[8];
		BYTE swap;
	} vrc4;
} m556;

void map_init_556(void) {
	map_init_VRC4(VRC4E);

	EXTCL_AFTER_MAPPER_INIT(556);
	EXTCL_CPU_WR_MEM(556);
	EXTCL_SAVE_MAPPER(556);
	EXTCL_CPU_EVERY_CYCLE(556);
	EXTCL_PPU_000_TO_34X(556);
	EXTCL_PPU_000_TO_255(556);
	EXTCL_PPU_256_TO_319(556);
	EXTCL_PPU_320_TO_34X(556);
	EXTCL_UPDATE_R2006(556);
	mapper.internal_struct[0] = (BYTE *)&m556;
	mapper.internal_struct_size[0] = sizeof(m556);
	mapper.internal_struct[1] = (BYTE *)&mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);
	mapper.internal_struct[2] = (BYTE *)&vrc4;
	mapper.internal_struct_size[2] = sizeof(vrc4);

	memset(&irqA12, 0x00, sizeof(irqA12));
	memset(&m556, 0x00, sizeof(m556));

	init_MMC3();
	MMC3_prg_swap = prg_swap_556_mmc3;
	MMC3_chr_swap = chr_swap_556_mmc3;

	m556.reg[2] = 0x0F;

	info.mapper.extend_wr = TRUE;

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_after_mapper_init_556(void) {
	prg_fix_556();
	chr_fix_556();
}
void extcl_cpu_wr_mem_556(WORD address, BYTE value) {
	if ((address >= 0x5000) && (address <= 0x5FFF)) {
		if (!(m556.reg[3] & 0x80)) {
			switch (m556.index) {
				case 0:
					m556.reg[0] = value;
					chr_fix_556();
					break;
				case 1:
					m556.reg[1] = value;
					prg_fix_556();
					break;
				case 2:
					m556.reg[2] = value;
					chr_fix_556();
					break;
				case 3:
					m556.reg[3] = value;
					prg_fix_556();
					chr_fix_556();
					break;
			}
			m556.index = (m556.index + 1) & 0x03;
		}
		return;
	}
	if (address >= 0x8000) {
		if (m556.reg[2] & 0x80) {
			cpu_wr_mem_vrc4(address, value);
		} else {
			extcl_cpu_wr_mem_MMC3(address, value);
		}
	}
}
BYTE extcl_save_mapper_556(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m556.index);
	save_slot_ele(mode, slot, m556.reg);
	save_slot_ele(mode, slot, m556.vrc4.prg);
	save_slot_ele(mode, slot, m556.vrc4.chr);
	save_slot_ele(mode, slot, m556.vrc4.swap);
	extcl_save_mapper_MMC3(mode, slot, fp);
	extcl_save_mapper_VRC4(mode, slot, fp);

	return (EXIT_OK);
}
void extcl_cpu_every_cycle_556(void) {
	if (m556.reg[2] & 0x80) {
		extcl_cpu_every_cycle_VRC4();
	} else {
		extcl_cpu_every_cycle_MMC3();
	}
}
void extcl_ppu_000_to_34x_556(void) {
	if (!(m556.reg[2] & 0x80)) {
		extcl_ppu_000_to_34x_MMC3();
	}
}
void extcl_ppu_000_to_255_556(void) {
	if (!(m556.reg[2] & 0x80)) {
		extcl_ppu_000_to_255_MMC3();
	}
}
void extcl_ppu_256_to_319_556(void) {
	if (!(m556.reg[2] & 0x80)) {
		extcl_ppu_256_to_319_MMC3();
	}
}
void extcl_ppu_320_to_34x_556(void) {
	if (!(m556.reg[2] & 0x80)) {
		extcl_ppu_320_to_34x_MMC3();
	}
}
void extcl_update_r2006_556(WORD new_r2006, WORD old_r2006) {
	if (!(m556.reg[2] & 0x80)) {
		extcl_update_r2006_MMC3(new_r2006, old_r2006);
	}
}

INLINE static void prg_fix_556(void) {
	if (m556.reg[2] & 0x80) {
		prg_fix_vrc4();
	} else {
		MMC3_prg_fix();
	}
}
INLINE static void chr_fix_556(void) {
	if (m556.reg[2] & 0x80) {
		chr_fix_vrc4();
	} else {
		MMC3_chr_fix();
	}
}

INLINE static WORD prg_base(void) {
	return (((m556.reg[3] & 0x40) << 2) | m556.reg[1]);
}
INLINE static WORD prg_mask(void) {
	return (~m556.reg[3] & 0x3F);
}
INLINE static WORD chr_base(void) {
	return (((m556.reg[3] & 0x40) << 6) | ((m556.reg[2] & 0xF0) << 4) | m556.reg[0]);
}
INLINE static WORD chr_mask(void) {
	return (0xFF >> (~m556.reg[2] & 0x0F));
}

void prg_swap_556_mmc3(WORD address, WORD value) {
	WORD base = prg_base();
	WORD mask = prg_mask();

	value = (base & ~mask) | (value & mask);
	control_bank(info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, (address >> 13) & 0x03, value);
	map_prg_rom_8k_update();
}
void chr_swap_556_mmc3(WORD address, WORD value) {
	WORD base = chr_base();
	WORD mask = chr_mask();

	value = (base & ~mask) | (value & mask);
	control_bank(info.chr.rom.max.banks_1k)
	chr.bank_1k[address >> 10] = chr_pnt(value << 10);
}

INLINE static void cpu_wr_mem_vrc4(WORD address, BYTE value) {
	WORD vrc4_address = 0;

	if (address & 0x0800) {
		address = (address & 0xFFF3) | ((address & 0x0004) << 1) | ((address & 0x0008) >> 1);
	}

	vrc4_address = address_VRC4(address);

	switch (vrc4_address) {
		case 0x8000:
			m556.vrc4.prg[0] = value;
			prg_fix_556();
			break;
		case 0xA000:
			m556.vrc4.prg[1] = value;
			prg_fix_556();
			break;
		case 0x9002:
		case 0x9003:
			m556.vrc4.swap = value & 0x02;
			prg_fix_556();
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

			m556.vrc4.chr[reg] = vrc4_address & 0x0001 ?
				(m556.vrc4.chr[reg] & 0x000F) | ((value & 0x0F) << 4) :
				(m556.vrc4.chr[reg] & 0x00F0) | (value & 0x0F);
			chr_fix_556();
			break;
		}
		default:
			extcl_cpu_wr_mem_VRC4(address, value);
			break;
	}
}
INLINE static void prg_fix_vrc4(void) {
	WORD base = prg_base();
	WORD mask = prg_mask();
	WORD bank = 0;

	bank = (base & ~mask) | (m556.vrc4.prg[0] & mask);
	_control_bank(bank, info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, 0 ^ m556.vrc4.swap, bank);

	bank = (base & ~mask) | (m556.vrc4.prg[1] & mask);
	_control_bank(bank, info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, 1, bank);

	bank = (base & ~mask) | (0xFE & mask);
	_control_bank(bank, info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, 2 ^ m556.vrc4.swap, bank);

	bank = (base & ~mask) | (0xFF & mask);
	_control_bank(bank, info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, 3, bank);

	map_prg_rom_8k_update();
}
INLINE static void chr_fix_vrc4(void) {
	WORD base = chr_base();
	WORD mask = chr_mask();
	DBWORD bank = 0;

	bank = (base & ~mask) | (m556.vrc4.chr[0] & mask);
	_control_bank(bank, info.chr.rom.max.banks_1k)
	chr.bank_1k[0] = chr_pnt(bank << 10);

	bank = (base & ~mask) | (m556.vrc4.chr[1] & mask);
	_control_bank(bank, info.chr.rom.max.banks_1k)
	chr.bank_1k[1] = chr_pnt(bank << 10);

	bank = (base & ~mask) | (m556.vrc4.chr[2] & mask);
	_control_bank(bank, info.chr.rom.max.banks_1k)
	chr.bank_1k[2] = chr_pnt(bank << 10);

	bank = (base & ~mask) | (m556.vrc4.chr[3] & mask);
	_control_bank(bank, info.chr.rom.max.banks_1k)
	chr.bank_1k[3] = chr_pnt(bank << 10);

	bank = (base & ~mask) | (m556.vrc4.chr[4] & mask);
	_control_bank(bank, info.chr.rom.max.banks_1k)
	chr.bank_1k[4] = chr_pnt(bank << 10);

	bank = (base & ~mask) | (m556.vrc4.chr[5] & mask);
	_control_bank(bank, info.chr.rom.max.banks_1k)
	chr.bank_1k[5] = chr_pnt(bank << 10);

	bank = (base & ~mask) | (m556.vrc4.chr[6] & mask);
	_control_bank(bank, info.chr.rom.max.banks_1k)
	chr.bank_1k[6] = chr_pnt(bank << 10);

	bank = (base & ~mask) | (m556.vrc4.chr[7] & mask);
	_control_bank(bank, info.chr.rom.max.banks_1k)
	chr.bank_1k[7] = chr_pnt(bank << 10);
}
