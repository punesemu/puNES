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

enum _m116_mappers { M116_MMC3 = 1, M116_VRC2, M116_MMC1 };

INLINE static void prg_fix_116(void);
INLINE static void chr_fix_116(void);
INLINE static void mirroring_fix_116(void);

INLINE static void switch_mode(void);
INLINE static WORD prg_base(void);
INLINE static WORD prg_mask(void);
INLINE static WORD chr_base(void);
INLINE static WORD chr_mask(void);

void prg_swap_116_mmc3(WORD address, WORD value);
void chr_swap_116_mmc3(WORD address, WORD value);

INLINE static void cpu_wr_mem_vrc2(WORD address, BYTE value);
INLINE static void prg_fix_vrc2(void);
INLINE static void chr_fix_vrc2(void);

void prg_swap_116_mmc1(WORD address, WORD value);
void chr_swap_116_mmc1(WORD address, WORD value);

struct _m116 {
	BYTE mapper;
	WORD reg;
	// VRC2
	struct _m116_vrc2 {
		BYTE prg[3];
		WORD chr[8];
	} vrc2;
} m116;
struct _m116tmp {
	BYTE dipswitch;
} m116tmp;

void map_init_116(void) {
	map_init_VRC2(VRC2B, 0x00);

	EXTCL_AFTER_MAPPER_INIT(116);
	EXTCL_CPU_WR_MEM(116);
	EXTCL_SAVE_MAPPER(116);
	EXTCL_CPU_EVERY_CYCLE(116);
	EXTCL_PPU_000_TO_34X(116);
	EXTCL_PPU_000_TO_255(116);
	EXTCL_PPU_256_TO_319(116);
	EXTCL_PPU_320_TO_34X(116);
	EXTCL_UPDATE_R2006(116);
	mapper.internal_struct[0] = (BYTE *)&m116;
	mapper.internal_struct_size[0] = sizeof(m116);
	mapper.internal_struct[1] = (BYTE *)&mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);
	mapper.internal_struct[2] = (BYTE *)&vrc2;
	mapper.internal_struct_size[2] = sizeof(vrc2);
	mapper.internal_struct[1] = (BYTE *)&mmc1;
	mapper.internal_struct_size[1] = sizeof(mmc1);

	memset(&irqA12, 0x00, sizeof(irqA12));
	memset(&m116, 0x00, sizeof(m116));

	m116.reg = 0x01;

	init_MMC3();
	MMC3_prg_swap = prg_swap_116_mmc3;
	MMC3_chr_swap = chr_swap_116_mmc3;

	init_MMC1(MMC1A);
	MMC1_prg_swap = prg_swap_116_mmc1;
	MMC1_chr_swap = chr_swap_116_mmc1;

	m116.vrc2.chr[0] = 0xFF;
	m116.vrc2.chr[1] = 0xFF;
	m116.vrc2.chr[2] = 0xFF;
	m116.vrc2.chr[3] = 0xFF;

	// AV Kyuukyoku Mahjong 2 (Asia) (Ja) (Ge De) (Unl).nes
	if ((prg_size() == (size_t)(1024 * 128)) && (prg_size() == chr_size())) {
		info.mapper.submapper = 2;
	};

	if (info.reset == RESET) {
		if (info.mapper.submapper == 3) {
			m116tmp.dipswitch = (++m116tmp.dipswitch > 4) ? 0 : m116tmp.dipswitch;
		}
	} else if (((info.reset == CHANGE_ROM) || (info.reset == POWER_UP))) {
		m116tmp.dipswitch = 0;
	}

	info.mapper.extend_wr = TRUE;

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_after_mapper_init_116(void) {
	switch_mode();
	prg_fix_116();
	chr_fix_116();
	mirroring_fix_116();
}
void extcl_cpu_wr_mem_116(WORD address, BYTE value) {
	if ((address >= 0x4000) && (address <= 0x5FFF)) {
		if (address & 0x0100) {
			m116.reg = value;
			switch_mode();
			prg_fix_116();
			chr_fix_116();
		}
		return;
	}
	if (address >= 0x8000) {
		if (m116.mapper == M116_MMC3) {
			extcl_cpu_wr_mem_MMC3(address, value);
		} else if (m116.mapper == M116_VRC2) {
			cpu_wr_mem_vrc2(address, value);
		} else if (m116.mapper == M116_MMC1) {
			extcl_cpu_wr_mem_MMC1(address, value);
		}
	}
}
BYTE extcl_save_mapper_116(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m116.mapper);
	save_slot_ele(mode, slot, m116.reg);
	save_slot_ele(mode, slot, m116.vrc2.prg);
	save_slot_ele(mode, slot, m116.vrc2.chr);
	extcl_save_mapper_MMC3(mode, slot, fp);
	extcl_save_mapper_MMC1(mode, slot, fp);

	return (EXIT_OK);
}
void extcl_cpu_every_cycle_116(void) {
	if (m116.mapper == M116_MMC3) {
		extcl_cpu_every_cycle_MMC3();
	}
}
void extcl_ppu_000_to_34x_116(void) {
	if (m116.mapper == M116_MMC3) {
		extcl_ppu_000_to_34x_MMC3();
	}
}
void extcl_ppu_000_to_255_116(void) {
	if (m116.mapper == M116_MMC3) {
		extcl_ppu_000_to_255_MMC3();
	}
}
void extcl_ppu_256_to_319_116(void) {
	if (m116.mapper == M116_MMC3) {
		extcl_ppu_256_to_319_MMC3();
	}
}
void extcl_ppu_320_to_34x_116(void) {
	if (m116.mapper == M116_MMC3) {
		extcl_ppu_320_to_34x_MMC3();
	}
}
void extcl_update_r2006_116(WORD new_r2006, WORD old_r2006) {
	if (m116.mapper == M116_MMC3) {
		extcl_update_r2006_MMC3(new_r2006, old_r2006);
	}
}

INLINE static void prg_fix_116(void) {
	if (m116.mapper == M116_MMC3) {
		MMC3_prg_fix();
	} else if (m116.mapper == M116_VRC2) {
		prg_fix_vrc2();
	} else if (m116.mapper == M116_MMC1) {
		MMC1_prg_fix();
	}
}
INLINE static void chr_fix_116(void) {
	if (m116.mapper == M116_MMC3) {
		MMC3_chr_fix();
	} else if (m116.mapper == M116_VRC2) {
		chr_fix_vrc2();
	} else if (m116.mapper == M116_MMC1) {
		MMC1_chr_fix();
	}
}
INLINE static void mirroring_fix_116(void) {
	if (m116.mapper == M116_MMC3) {
		MMC3_mirroring_fix();
	}
}

INLINE static void switch_mode(void) {
	switch (m116.reg & 0x03) {
		case 1:
			m116.mapper = M116_MMC3;
			break;
		case 2:
		case 3:
			m116.mapper = M116_MMC1;
			irq.high &= ~EXT_IRQ;
			if (info.mapper.submapper != 1) {
				extcl_cpu_wr_mem_MMC1(0x8000, 0x80);
			}
			break;
		case 0:
			m116.mapper = M116_VRC2;
			irq.high &= ~EXT_IRQ;
			break;
	}
}
INLINE static WORD prg_base(void) {
	return (m116tmp.dipswitch ? (m116tmp.dipswitch + 1) * 0x10 : 0x00);
}
INLINE static WORD prg_mask(void) {
	return (info.mapper.submapper != 3 ? 0x3F : m116tmp.dipswitch ? 0x0F : 0x1F);
}
INLINE static WORD chr_base(void) {
	return (m116tmp.dipswitch ? (m116tmp.dipswitch + 1) * 0x80 : 0x00);
}
INLINE static WORD chr_mask(void) {
	return (m116tmp.dipswitch ? 0x7F : 0xFF);
}

void prg_swap_116_mmc3(WORD address, WORD value) {
	WORD base = prg_base();
	WORD mask = prg_mask();

	value = (base & ~mask) | (value & mask);
	control_bank(info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, (address >> 13) & 0x03, value);
	map_prg_rom_8k_update();
}
void chr_swap_116_mmc3(WORD address, WORD value) {
	WORD base = ((m116.reg & 0x04) << 6) | chr_base();
	WORD mask = chr_mask();

	value = (base & ~mask) | (value & mask);
	control_bank(info.chr.rom.max.banks_1k)
	chr.bank_1k[address >> 10] = chr_pnt(value << 10);
}

INLINE static void cpu_wr_mem_vrc2(WORD address, BYTE value) {
	WORD vrc2_address = address_VRC2(address);

	switch (vrc2_address) {
		case 0x8000:
			m116.vrc2.prg[0] = value;
			prg_fix_116();
			break;
		case 0xA000:
			m116.vrc2.prg[1] = value;
			prg_fix_116();
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
			BYTE reg = ((vrc2_address - 0xB000) >> 11) | ((vrc2_address & 0x0003) >> 1);

			m116.vrc2.chr[reg] = vrc2_address & 0x0001 ?
				(m116.vrc2.chr[reg] & 0x000F) | (value << 4) :
				(m116.vrc2.chr[reg] & 0x0FF0) | (value & 0x0F);
			chr_fix_116();
			break;
		}
		default:
			extcl_cpu_wr_mem_VRC2(address, value);
			break;
	}
}
INLINE static void prg_fix_vrc2(void) {
	WORD base = prg_base();
	WORD mask = prg_mask();
	WORD bank = 0;

	bank = (base & ~mask) | (m116.vrc2.prg[0] & mask);
	_control_bank(bank, info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, 0, bank);

	bank = (base & ~mask) | (m116.vrc2.prg[1] & mask);
	_control_bank(bank, info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, 1, bank);

	bank = (base & ~mask) | (0xFE & mask);
	_control_bank(bank, info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, 2, bank);

	bank = (base & ~mask) | (0xFF & mask);
	_control_bank(bank, info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, 3, bank);

	map_prg_rom_8k_update();
}
INLINE static void chr_fix_vrc2(void) {
	WORD base = ((m116.reg & 0x04) << 6) | chr_base();
	WORD mask = chr_mask();
	DBWORD bank = 0;

	bank = (base & ~mask) | (m116.vrc2.chr[0] & mask);
	_control_bank(bank, info.chr.rom.max.banks_1k)
	chr.bank_1k[0] = chr_pnt(bank << 10);

	bank = (base & ~mask) | (m116.vrc2.chr[1] & mask);
	_control_bank(bank, info.chr.rom.max.banks_1k)
	chr.bank_1k[1] = chr_pnt(bank << 10);

	bank = (base & ~mask) | (m116.vrc2.chr[2] & mask);
	_control_bank(bank, info.chr.rom.max.banks_1k)
	chr.bank_1k[2] = chr_pnt(bank << 10);

	bank = (base & ~mask) | (m116.vrc2.chr[3] & mask);
	_control_bank(bank, info.chr.rom.max.banks_1k)
	chr.bank_1k[3] = chr_pnt(bank << 10);

	bank = (base & ~mask) | (m116.vrc2.chr[4] & mask);
	_control_bank(bank, info.chr.rom.max.banks_1k)
	chr.bank_1k[4] = chr_pnt(bank << 10);

	bank = (base & ~mask) | (m116.vrc2.chr[5] & mask);
	_control_bank(bank, info.chr.rom.max.banks_1k)
	chr.bank_1k[5] = chr_pnt(bank << 10);

	bank = (base & ~mask) | (m116.vrc2.chr[6] & mask);
	_control_bank(bank, info.chr.rom.max.banks_1k)
	chr.bank_1k[6] = chr_pnt(bank << 10);

	bank = (base & ~mask) | (m116.vrc2.chr[7] & mask);
	_control_bank(bank, info.chr.rom.max.banks_1k)
	chr.bank_1k[7] = chr_pnt(bank << 10);
}

void prg_swap_116_mmc1(WORD address, WORD value) {
	if (info.mapper.submapper == 2) {
		value >>= 1;
	} else {
		WORD base = prg_base() >> 1;
		WORD mask = prg_mask() >> 1;

		value = (base & ~mask) | (value & mask);
	}
	prg_swap_MMC1(address, value);
}
void chr_swap_116_mmc1(WORD address, WORD value) {
	WORD base = chr_base() >> 2;
	WORD mask = chr_mask() >> 2;

	chr_swap_MMC1(address, (base & ~mask) | (value & mask));
}
