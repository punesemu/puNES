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

#include <stdlib.h>
#include <string.h>
#include "mappers.h"
#include "info.h"
#include "mem_map.h"
#include "irqA12.h"
#include "save_slot.h"

enum _m351_mappers { M351_MMC3 = 1, M351_VRC4, M351_MMC1 };

INLINE static void prg_fix_351(void);
INLINE static void chr_fix_351(void);

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

INLINE static void cpu_wr_mem_vrc4(WORD address, BYTE value);
INLINE static void prg_fix_vrc4(void);
INLINE static void chr_fix_vrc4(void);

INLINE static void cpu_wr_mem_mmc1(WORD address, BYTE value);
INLINE static void prg_fix_mmc1(void);
INLINE static void chr_fix_mmc1(void);
INLINE static void mirroring_fix_mmc1(void);

struct _m351 {
	BYTE mapper;
	WORD reg[4];
	// MMC3
	WORD mmc3[8];
	// VRC4
	struct _m351_vrc4 {
		BYTE prg[3];
		WORD chr[8];
		BYTE swap;
	} vrc4;
} m351;

void map_init_351(void) {
	map_init_VRC4(VRC4E);

	EXTCL_AFTER_MAPPER_INIT(351);
	EXTCL_CPU_WR_MEM(351);
	EXTCL_SAVE_MAPPER(351);
	EXTCL_WR_CHR(351);
	EXTCL_CPU_EVERY_CYCLE(351);
	EXTCL_PPU_000_TO_34X(351);
	EXTCL_PPU_000_TO_255(351);
	EXTCL_PPU_256_TO_319(351);
	EXTCL_PPU_320_TO_34X(351);
	EXTCL_UPDATE_R2006(351);
	mapper.internal_struct[0] = (BYTE *)&m351;
	mapper.internal_struct_size[0] = sizeof(m351);
	mapper.internal_struct[1] = (BYTE *)&mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);
	mapper.internal_struct[2] = (BYTE *)&vrc4;
	mapper.internal_struct_size[2] = sizeof(vrc4);
	mapper.internal_struct[1] = (BYTE *)&mmc1;
	mapper.internal_struct_size[1] = sizeof(mmc1);

	memset(&mmc1, 0x00, sizeof(mmc1));
	memset(&mmc3, 0x00, sizeof(mmc3));
	memset(&irqA12, 0x00, sizeof(irqA12));
	memset(&m351, 0x00, sizeof(m351));

	mmc1.ctrl = 0x0C;
	mmc1.prg_mode = 3;

	m351.mmc3[0] = 0;
	m351.mmc3[1] = 2;
	m351.mmc3[2] = 4;
	m351.mmc3[3] = 5;
	m351.mmc3[4] = 6;
	m351.mmc3[5] = 7;
	m351.mmc3[6] = 0;
	m351.mmc3[7] = 1;

	map_chr_ram_extra_init(0x2000);

	if ((info.reset == CHANGE_ROM) || (info.reset == POWER_UP)) {
		// Unusually, in CHR-RAM mode (R=1), CHR-ROM becomes the second half of an enlarged PRG address space that
		// becomes addressable via register $5001. At least one multicart containing both TLROM and UNROM games makes
		// use of this feature and puts the UNROM game's PRG data into CHR-ROM. This seems to be possible as the mapper ASIC,
		// PRG and CHR-ROM are under a single glob.
		if (!mapper.write_vram) {
			prg_rom() = realloc(prg_rom(), prg_size() + chr_size());
			memcpy(prg_rom() + prg_size(), chr_rom(), chr_size());
			prg_size() = prg_size() + chr_size();
			info.prg.rom.banks_16k = prg_size() / 0x4000;
			info.prg.rom.banks_8k = info.prg.rom.banks_16k * 2;
			map_set_banks_max_prg();
		}
	}

	info.mapper.extend_wr = TRUE;

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_after_mapper_init_351(void) {
	switch_mode();
	prg_fix_351();
	chr_fix_351();
}
void extcl_cpu_wr_mem_351(WORD address, BYTE value) {
	if ((address >= 0x5000) && (address <= 0x5FFF)) {
		BYTE reg = address & 0x03;

		switch (reg) {
			case 0:
				m351.reg[0] = value;
				switch_mode();
				break;
			case 1:
			case 2:
				m351.reg[reg] = value;
				break;
		}
		prg_fix_351();
		chr_fix_351();
		return;
	}
	if (address >= 0x8000) {
		if (m351.mapper == M351_MMC3) {
			cpu_wr_mmc3(address, value);
		} else if (m351.mapper == M351_VRC4) {
			cpu_wr_mem_vrc4(address, value);
		} else if (m351.mapper == M351_MMC1) {
			cpu_wr_mem_mmc1(address, value);
		}
	}
}
BYTE extcl_save_mapper_351(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m351.mapper);
	save_slot_ele(mode, slot, m351.reg);
	save_slot_ele(mode, slot, m351.mmc3);
	save_slot_ele(mode, slot, m351.vrc4.prg);
	save_slot_ele(mode, slot, m351.vrc4.chr);
	save_slot_ele(mode, slot, m351.vrc4.swap);
	extcl_save_mapper_MMC3(mode, slot, fp);
	extcl_save_mapper_VRC4(mode, slot, fp);
	extcl_save_mapper_MMC1(mode, slot, fp);

	return (EXIT_OK);
}
void extcl_wr_chr_351(WORD address, BYTE value) {
	BYTE slot = address >> 10;

	if ((chr.bank_1k[slot] >= chr.extra.data) && (chr.bank_1k[slot] < (chr.extra.data + chr.extra.size))) {
		chr.bank_1k[slot][address & 0x3FF] = value;
	}
}
void extcl_cpu_every_cycle_351(void) {
	if (m351.mapper == M351_MMC3) {
		extcl_cpu_every_cycle_MMC3();
	} else if (m351.mapper == M351_VRC4) {
		extcl_cpu_every_cycle_VRC4();
	}
}
void extcl_ppu_000_to_34x_351(void) {
	if (m351.mapper == M351_MMC3) {
		extcl_ppu_000_to_34x_MMC3();
	}
}
void extcl_ppu_000_to_255_351(void) {
	if (m351.mapper == M351_MMC3) {
		extcl_ppu_000_to_255_MMC3();
	}
}
void extcl_ppu_256_to_319_351(void) {
	if (m351.mapper == M351_MMC3) {
		extcl_ppu_256_to_319_MMC3();
	}
}
void extcl_ppu_320_to_34x_351(void) {
	if (m351.mapper == M351_MMC3) {
		extcl_ppu_320_to_34x_MMC3();
	}
}
void extcl_update_r2006_351(WORD new_r2006, WORD old_r2006) {
	if (m351.mapper == M351_MMC3) {
		extcl_update_r2006_MMC3(new_r2006, old_r2006);
	}
}

INLINE static void prg_fix_351(void) {
	WORD bank;

	if (m351.reg[2] & 0x10) {
		if (m351.reg[2] & 0x04) {
			bank = m351.reg[1] >> 2;
			_control_bank(bank, info.prg.rom.max.banks_16k)
			map_prg_rom_8k(2, 0, bank);
			map_prg_rom_8k(2, 2, bank);
		} else {
			bank = m351.reg[1] >> 3;
			_control_bank(bank, info.prg.rom.max.banks_32k)
			map_prg_rom_8k(4, 0, bank);
		}
		map_prg_rom_8k_update();
	} else {
		if (m351.mapper == M351_MMC3) {
			prg_fix_mmc3();
		} else if (m351.mapper == M351_VRC4) {
			prg_fix_vrc4();
		} else if (m351.mapper == M351_MMC1) {
			prg_fix_mmc1();
		}
	}
}
INLINE static void chr_fix_351(void) {
	DBWORD bank;

	if (m351.reg[2] & 0x01) {
		chr.bank_1k[0] = &chr.extra.data[0 | 0x0000];
		chr.bank_1k[1] = &chr.extra.data[0 | 0x0400];
		chr.bank_1k[2] = &chr.extra.data[0 | 0x0800];
		chr.bank_1k[3] = &chr.extra.data[0 | 0x0C00];
		chr.bank_1k[4] = &chr.extra.data[0 | 0x1000];
		chr.bank_1k[5] = &chr.extra.data[0 | 0x1400];
		chr.bank_1k[6] = &chr.extra.data[0 | 0x1800];
		chr.bank_1k[7] = &chr.extra.data[0 | 0x1C00];
	} else if (m351.reg[2] & 0x40) {
		bank = m351.reg[0] >> 2;
		_control_bank(bank, info.chr.rom.max.banks_8k)
		bank <<= 13;
		chr.bank_1k[0] = chr_pnt(bank);
		chr.bank_1k[1] = chr_pnt(bank | 0x0400);
		chr.bank_1k[2] = chr_pnt(bank | 0x0800);
		chr.bank_1k[3] = chr_pnt(bank | 0x0C00);
		chr.bank_1k[4] = chr_pnt(bank | 0x1000);
		chr.bank_1k[5] = chr_pnt(bank | 0x1400);
		chr.bank_1k[6] = chr_pnt(bank | 0x1800);
		chr.bank_1k[7] = chr_pnt(bank | 0x1C00);
	} else {
		if (m351.mapper == M351_MMC3) {
			chr_fix_mmc3();
		} else if (m351.mapper == M351_VRC4) {
			chr_fix_vrc4();
		} else if (m351.mapper == M351_MMC1) {
			chr_fix_mmc1();
		}
	}
}

INLINE static void switch_mode(void) {
	switch (m351.reg[0] & 0x03) {
		case 0:
		case 1:
			m351.mapper = M351_MMC3;
			break;
		case 2:
			m351.mapper = M351_MMC1;
			irq.high &= ~EXT_IRQ;
			break;
		case 3:
			m351.mapper = M351_VRC4;
			irq.high &= ~EXT_IRQ;
			break;
	}
}
INLINE static WORD prg_base(void) {
	return (m351.reg[1] >> 1);
}
INLINE static WORD prg_mask(void) {
	return (0x1F >> ((m351.reg[2] & 0x04) >> 2));
}
INLINE static WORD chr_base(void) {
	return (m351.reg[0] << 1);
}
INLINE static WORD chr_mask(void) {
	return ((m351.reg[2] & 0x10) ? 0x1F : (m351.reg[2] & 0x20 ? 0x7F : 0xFF));
}

INLINE static void cpu_wr_mmc3(WORD address, BYTE value) {
	switch (address & 0xE001) {
		case 0x8000: {
			BYTE old = mmc3.bank_to_update;

			mmc3.bank_to_update = value;

			if ((value & 0x40) != (old & 0x40)) {
				prg_fix_351();
			}
			if ((value & 0x80) != (old & 0x80)) {
				chr_fix_351();
			}
			return;
		}
		case 0x8001: {
			m351.mmc3[mmc3.bank_to_update & 0x07] = value;
			switch (mmc3.bank_to_update & 0x07) {
				case 0:
				case 1:
				case 2:
				case 3:
				case 4:
				case 5:
					chr_fix_351();
					return;
				case 6:
				case 7:
					prg_fix_351();
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
		prg_swap_mmc3(0xC000, m351.mmc3[6]);
	} else {
		prg_swap_mmc3(0x8000, m351.mmc3[6]);
		prg_swap_mmc3(0xC000, ~1);
	}
	prg_swap_mmc3(0xA000, m351.mmc3[7]);
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

	chr_swap_mmc3(cbase ^ 0x0000, m351.mmc3[0] & (~1));
	chr_swap_mmc3(cbase ^ 0x0400, m351.mmc3[0] | 1);
	chr_swap_mmc3(cbase ^ 0x0800, m351.mmc3[1] & (~1));
	chr_swap_mmc3(cbase ^ 0x0C00, m351.mmc3[1] | 1);
	chr_swap_mmc3(cbase ^ 0x1000, m351.mmc3[2]);
	chr_swap_mmc3(cbase ^ 0x1400, m351.mmc3[3]);
	chr_swap_mmc3(cbase ^ 0x1800, m351.mmc3[4]);
	chr_swap_mmc3(cbase ^ 0x1C00, m351.mmc3[5]);
}
INLINE static void chr_swap_mmc3(WORD address, WORD value) {
	WORD base = chr_base();
	WORD mask = chr_mask();

	value = (base & ~mask) | (value & mask);
	control_bank(info.chr.rom.max.banks_1k)
	chr.bank_1k[address >> 10] = chr_pnt(value << 10);
}

INLINE static void cpu_wr_mem_vrc4(WORD address, BYTE value) {
	WORD vrc4_address;

	if (address & 0x0800) {
		address = (address & 0xFFF3) | ((address & 0x0004) << 1) | ((address & 0x0008) >> 1);
	}

	vrc4_address = address_VRC4(address);

	switch (vrc4_address) {
		case 0x8000:
			m351.vrc4.prg[0] = value;
			prg_fix_351();
			break;
		case 0xA000:
			m351.vrc4.prg[1] = value;
			prg_fix_351();
			break;
		case 0x9002:
		case 0x9003:
			m351.vrc4.swap = value & 0x02;
			prg_fix_351();
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

			m351.vrc4.chr[reg] = vrc4_address & 0x0001 ?
				(m351.vrc4.chr[reg] & 0x000F) | (value << 4) :
				(m351.vrc4.chr[reg] & 0x0FF0) | (value & 0x0F);
			chr_fix_351();
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
	WORD bank;

	bank = (base & ~mask) | (m351.vrc4.prg[0] & mask);
	_control_bank(bank, info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, 0 ^ m351.vrc4.swap, bank);

	bank = (base & ~mask) | (m351.vrc4.prg[1] & mask);
	_control_bank(bank, info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, 1, bank);

	bank = (base & ~mask) | (0xFE & mask);
	_control_bank(bank, info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, 2 ^ m351.vrc4.swap, bank);

	bank = (base & ~mask) | (0xFF & mask);
	_control_bank(bank, info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, 3, bank);

	map_prg_rom_8k_update();
}
INLINE static void chr_fix_vrc4(void) {
	WORD base = chr_base();
	WORD mask = chr_mask();
	DBWORD bank;

	bank = (base & ~mask) | (m351.vrc4.chr[0] & mask);
	_control_bank(bank, info.chr.rom.max.banks_1k)
	chr.bank_1k[0] = chr_pnt(bank << 10);

	bank = (base & ~mask) | (m351.vrc4.chr[1] & mask);
	_control_bank(bank, info.chr.rom.max.banks_1k)
	chr.bank_1k[1] = chr_pnt(bank << 10);

	bank = (base & ~mask) | (m351.vrc4.chr[2] & mask);
	_control_bank(bank, info.chr.rom.max.banks_1k)
	chr.bank_1k[2] = chr_pnt(bank << 10);

	bank = (base & ~mask) | (m351.vrc4.chr[3] & mask);
	_control_bank(bank, info.chr.rom.max.banks_1k)
	chr.bank_1k[3] = chr_pnt(bank << 10);

	bank = (base & ~mask) | (m351.vrc4.chr[4] & mask);
	_control_bank(bank, info.chr.rom.max.banks_1k)
	chr.bank_1k[4] = chr_pnt(bank << 10);

	bank = (base & ~mask) | (m351.vrc4.chr[5] & mask);
	_control_bank(bank, info.chr.rom.max.banks_1k)
	chr.bank_1k[5] = chr_pnt(bank << 10);

	bank = (base & ~mask) | (m351.vrc4.chr[6] & mask);
	_control_bank(bank, info.chr.rom.max.banks_1k)
	chr.bank_1k[6] = chr_pnt(bank << 10);

	bank = (base & ~mask) | (m351.vrc4.chr[7] & mask);
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
				prg_fix_351();
				chr_fix_351();
				break;
			case 1:
				mmc1.chr0 = mmc1.reg;
				chr_fix_351();
				break;
			case 2:
				mmc1.chr1 = mmc1.reg;
				chr_fix_351();
				break;
			case 3:
				mmc1.prg0 = mmc1.reg;
				cpu.prg_ram_rd_active = (mmc1.prg0 & 0x10 ? FALSE : TRUE);
				cpu.prg_ram_wr_active = cpu.prg_ram_rd_active;
				prg_fix_351();
				break;
		}
		mmc1.pos = mmc1.reg = 0;
	}
}
INLINE static void prg_fix_mmc1(void) {
	WORD base = prg_base() >> 1;
	WORD mask = prg_mask() >> 1;
	WORD bank;

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
