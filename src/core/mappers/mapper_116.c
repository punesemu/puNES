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

INLINE static void switch_mode(void);
INLINE static void cpu_wr_mmc3(WORD address, BYTE value);
INLINE static WORD prg_base(void);
INLINE static WORD prg_mask(void);
INLINE static WORD chr_base(void);
INLINE static WORD chr_mask(void);

INLINE static void prg_fix_mmc3(void);
INLINE static void prg_swap_mmc3(WORD address, WORD value);
INLINE static void chr_fix_mmc3(void);
INLINE static void chr_swap_mmc3(WORD address, WORD value);

INLINE static void cpu_wr_mem_vrc2(WORD address, BYTE value);
INLINE static void prg_fix_vrc2(void);
INLINE static void chr_fix_vrc2(void);

INLINE static void cpu_wr_mem_mmc1(WORD address, BYTE value);
INLINE static void prg_fix_mmc1(void);
INLINE static void chr_fix_mmc1(void);
INLINE static void mirroring_fix_mmc1(void);

struct _m116 {
	BYTE mapper;
	WORD reg;
	// MMC3
	WORD mmc3[8];
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

	memset(&mmc1, 0x00, sizeof(mmc1));
	memset(&mmc3, 0x00, sizeof(mmc3));
	memset(&irqA12, 0x00, sizeof(irqA12));
	memset(&m116, 0x00, sizeof(m116));

	m116.reg = 0x01;

	m116.mmc3[0] = 0;
	m116.mmc3[1] = 2;
	m116.mmc3[2] = 4;
	m116.mmc3[3] = 5;
	m116.mmc3[4] = 6;
	m116.mmc3[5] = 7;
	m116.mmc3[6] = 0;
	m116.mmc3[7] = 1;

	m116.vrc2.chr[0] = 0xFF;
	m116.vrc2.chr[1] = 0xFF;
	m116.vrc2.chr[2] = 0xFF;
	m116.vrc2.chr[3] = 0xFF;

	mmc1.ctrl = 0x0C;
	mmc1.prg_mode = 3;

	// AV Kyuukyoku Mahjong 2 (Asia) (Ja) (Ge De) (Unl).nes
	if ((prg_size() == (1024 * 128)) && (prg_size() == chr_size())) {
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
			cpu_wr_mmc3(address, value);
		} else if (m116.mapper == M116_VRC2) {
			cpu_wr_mem_vrc2(address, value);
		} else if (m116.mapper == M116_MMC1) {
			cpu_wr_mem_mmc1(address, value);
		}
	}
}
BYTE extcl_save_mapper_116(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m116.mapper);
	save_slot_ele(mode, slot, m116.reg);
	save_slot_ele(mode, slot, m116.mmc3);
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
		prg_fix_mmc3();
	} else if (m116.mapper == M116_VRC2) {
		prg_fix_vrc2();
	} else if (m116.mapper == M116_MMC1) {
		prg_fix_mmc1();
	}
}
INLINE static void chr_fix_116(void) {
	if (m116.mapper == M116_MMC3) {
		chr_fix_mmc3();
	} else if (m116.mapper == M116_VRC2) {
		chr_fix_vrc2();
	} else if (m116.mapper == M116_MMC1) {
		chr_fix_mmc1();
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
				cpu_wr_mem_mmc1(0x8000, 0x80);
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

INLINE static void cpu_wr_mmc3(WORD address, BYTE value) {
	switch (address & 0xE001) {
		case 0x8000: {
			BYTE old = mmc3.bank_to_update;

			mmc3.bank_to_update = value;

			if ((value & 0x40) != (old & 0x40)) {
				prg_fix_116();
			}
			if ((value & 0x80) != (old & 0x80)) {
				chr_fix_116();
			}
			return;
		}
		case 0x8001: {
			m116.mmc3[mmc3.bank_to_update & 0x07] = value;
			switch (mmc3.bank_to_update & 0x07) {
				case 0:
				case 1:
				case 2:
				case 3:
				case 4:
				case 5:
					chr_fix_116();
					return;
				case 6:
				case 7:
					prg_fix_116();
					return;
			}
			return;
		}
	}
	extcl_cpu_wr_mem_MMC3(address, value);
}
INLINE static void prg_fix_mmc3(void) {
	if (mmc3.bank_to_update & 0x40) {
		prg_swap_mmc3(0x8000, ~1);
		prg_swap_mmc3(0xC000, m116.mmc3[6]);
	} else {
		prg_swap_mmc3(0x8000, m116.mmc3[6]);
		prg_swap_mmc3(0xC000, ~1);
	}
	prg_swap_mmc3(0xA000, m116.mmc3[7]);
	prg_swap_mmc3(0xE000, ~0);
}
INLINE static void prg_swap_mmc3(WORD address, WORD value) {
	WORD base = prg_base();
	WORD mask = prg_mask();

	value = (base & ~mask) | (value & mask);
	control_bank(info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, (address >> 13) & 0x03, value);
	map_prg_rom_8k_update();
}
INLINE static void chr_fix_mmc3(void) {
	WORD cbase = (mmc3.bank_to_update & 0x80) << 5;

	chr_swap_mmc3(cbase ^ 0x0000, m116.mmc3[0] & (~1));
	chr_swap_mmc3(cbase ^ 0x0400, m116.mmc3[0] | 1);
	chr_swap_mmc3(cbase ^ 0x0800, m116.mmc3[1] & (~1));
	chr_swap_mmc3(cbase ^ 0x0C00, m116.mmc3[1] | 1);
	chr_swap_mmc3(cbase ^ 0x1000, m116.mmc3[2]);
	chr_swap_mmc3(cbase ^ 0x1400, m116.mmc3[3]);
	chr_swap_mmc3(cbase ^ 0x1800, m116.mmc3[4]);
	chr_swap_mmc3(cbase ^ 0x1C00, m116.mmc3[5]);
}
INLINE static void chr_swap_mmc3(WORD address, WORD value) {
	WORD base = ((m116.reg & 0x04) << 6) | chr_base();
	WORD mask = chr_mask();

	value = (base & ~mask) | (value & mask);
	control_bank(info.chr.rom.max.banks_1k)
	chr.bank_1k[address >> 10] = chr_pnt(value << 10);
}

INLINE static void cpu_wr_mem_vrc2(WORD address, BYTE value) {
	WORD vrc2_address;

	vrc2_address = address_VRC2(address);

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
	WORD bank;

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
	DBWORD bank;

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

INLINE static void cpu_wr_mem_mmc1(WORD address, BYTE value) {
	if (mmc1.reset) {
		mmc1.reset = FALSE;
		if (cpu.double_wr) {
			return;
		}
	}
	if (value & 0x80) {
		mmc1.reset = TRUE;
		mmc1.pos = mmc1.reg = 0;
		mmc1.ctrl |= 0x0C;
		return;
	}

	mmc1.reg |= ((value & 0x01) << mmc1.pos);

	if (mmc1.pos++ == 4) {
		BYTE reg = (address >> 13) & 0x03;

		switch (reg) {
			case 0:
				mmc1.ctrl = mmc1.reg;
				mmc1.prg_mode = (mmc1.ctrl & 0x0C) >> 2;
				mmc1.chr_mode = (mmc1.ctrl & 0x10) >> 4;
				mirroring_fix_mmc1();
				prg_fix_116();
				chr_fix_116();
				break;
			case 1:
				mmc1.chr0 = mmc1.reg;
				chr_fix_116();
				break;
			case 2:
				mmc1.chr1 = mmc1.reg;
				chr_fix_116();
				break;
			case 3:
				mmc1.prg0 = mmc1.reg;
				cpu.prg_ram_rd_active = (mmc1.prg0 & 0x10 ? FALSE : TRUE);
				cpu.prg_ram_wr_active = cpu.prg_ram_rd_active;
				prg_fix_116();
				break;
		}
		mmc1.pos = mmc1.reg = 0;
	}
}
INLINE static void prg_fix_mmc1(void) {
	WORD base = prg_base() >> 1;
	WORD mask = prg_mask() >> 1;
	WORD bank;

	if (info.mapper.submapper == 2) {
		bank = ((base & ~mask) | (mmc1.prg0  & mask)) >> 1;
		_control_bank(bank, info.prg.rom.max.banks_16k)
		map_prg_rom_8k(2, 0, bank);

		bank = ((base & ~mask) | mask) >> 1;
		_control_bank(bank, info.prg.rom.max.banks_16k)
		map_prg_rom_8k(2, 2, bank);
	} else {
		switch (mmc1.prg_mode) {
			case 0:
			case 1:
				bank = ((base & ~mask) | (mmc1.prg0 & mask)) >> 1;
				_control_bank(bank, info.prg.rom.max.banks_32k)
				map_prg_rom_8k(4, 0, bank);
				break;
			case 2:
				bank = (base & ~mask) | (mmc1.prg0 & mask);
				_control_bank(bank, info.prg.rom.max.banks_16k)
				map_prg_rom_8k(2, 2, bank);

				bank = base & ~mask;
				_control_bank(bank, info.prg.rom.max.banks_16k)
				map_prg_rom_8k(2, 0, bank);
				break;
			case 3:
				bank = (base & ~mask) | (mmc1.prg0 & mask);
				_control_bank(bank, info.prg.rom.max.banks_16k)
				map_prg_rom_8k(2, 0, bank);

				bank = (base & ~mask) | mask;
				_control_bank(bank, info.prg.rom.max.banks_16k)
				map_prg_rom_8k(2, 2, bank);
				break;
		}
	}
	map_prg_rom_8k_update();
}
INLINE static void chr_fix_mmc1(void) {
	WORD base = chr_base() >> 2;
	WORD mask = chr_mask() >> 2;
	DBWORD bank;

	bank = (base & ~mask) | (mmc1.chr0 & mask);
	_control_bank(bank, info.chr.rom.max.banks_4k)
	bank <<= 12;
	chr.bank_1k[0] = chr_pnt(bank);
	chr.bank_1k[1] = chr_pnt(bank | 0x0400);
	chr.bank_1k[2] = chr_pnt(bank | 0x0800);
	chr.bank_1k[3] = chr_pnt(bank | 0x0C00);

	bank = (base & ~mask) | (mmc1.chr1 & mask);
	_control_bank(bank, info.chr.rom.max.banks_4k)
	bank <<= 12;
	chr.bank_1k[4] = chr_pnt(bank);
	chr.bank_1k[5] = chr_pnt(bank | 0x0400);
	chr.bank_1k[6] = chr_pnt(bank | 0x0800);
	chr.bank_1k[7] = chr_pnt(bank | 0x0C00);
}
INLINE static void mirroring_fix_mmc1(void) {
	switch (mmc1.ctrl & 0x03) {
		case 0x00:
			mirroring_SCR0();
			break;
		case 0x01:
			mirroring_SCR1();
			break;
		case 0x02:
			mirroring_V();
			break;
		case 0x03:
			mirroring_H();
			break;
	}
}
