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

INLINE static void prg_fix_369(void);
INLINE static void prg_swap_369(WORD address, WORD value);
INLINE static void chr_fix_369(void);
INLINE static void chr_swap_369(WORD address, WORD value);

struct _m369 {
	BYTE reg;
	WORD mmc3[8];
	BYTE smb2j;
	struct _m369_irq {
		BYTE enable;
		WORD counter;
	} irq;
} m369;
struct _m369tmp {
	BYTE *prg_6000;
} m369tmp;

void map_init_369(void) {
	EXTCL_AFTER_MAPPER_INIT(369);
	EXTCL_CPU_WR_MEM(369);
	EXTCL_CPU_RD_MEM(369);
	EXTCL_SAVE_MAPPER(369);
	EXTCL_CPU_EVERY_CYCLE(369);
	EXTCL_PPU_000_TO_34X(369);
	EXTCL_PPU_000_TO_255(369);
	EXTCL_PPU_256_TO_319(369);
	EXTCL_PPU_320_TO_34X(369);
	EXTCL_UPDATE_R2006(369);
	mapper.internal_struct[0] = (BYTE *)&m369;
	mapper.internal_struct_size[0] = sizeof(m369);
	mapper.internal_struct[1] = (BYTE *)&mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);

	memset(&mmc3, 0x00, sizeof(mmc3));
	memset(&irqA12, 0x00, sizeof(irqA12));
	memset(&m369, 0x00, sizeof(m369));

	m369.mmc3[0] = 0;
	m369.mmc3[1] = 2;
	m369.mmc3[2] = 4;
	m369.mmc3[3] = 5;
	m369.mmc3[4] = 6;
	m369.mmc3[5] = 7;
	m369.mmc3[6] = 0;
	m369.mmc3[7] = 0;

	if (!info.chr.ram.banks_8k_plus) {
		info.chr.ram.banks_8k_plus = 1;
	}

	info.mapper.extend_wr = TRUE;
	info.mapper.ram_plus_op_controlled_by_mapper = TRUE;

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_after_mapper_init_369(void) {
	prg_fix_369();
	chr_fix_369();
}
void extcl_cpu_wr_mem_369(WORD address, BYTE value) {
	switch (address & 0xF000) {
		case 0x4000:
			if (cpu.prg_ram_wr_active && (address & 0x0100)) {
				m369.reg = value;
				prg_fix_369();
				chr_fix_369();
			}
			return;
		case 0x6000:
		case 0x7000:
			if (!m369tmp.prg_6000) {
				prg.ram_plus_8k[address & 0x1FFF] = value;
			}
			return;
		case 0x8000:
		case 0x9000:
			if (m369.reg == 0x13) {
				m369.irq.enable = FALSE;
				irq.high &= ~EXT_IRQ;
			}
			if (!(address & 0x0001)) {
				BYTE bank_to_update = mmc3.bank_to_update;

				mmc3.bank_to_update = value;
				if ((mmc3.bank_to_update & 0x40) != (bank_to_update & 0x40)) {
					prg_fix_369();
				}
				if ((mmc3.bank_to_update & 0x80) != (bank_to_update & 0x80)) {
					chr_fix_369();
				}
			} else {
				m369.mmc3[mmc3.bank_to_update & 0x07] = value;
				switch (mmc3.bank_to_update & 0x07) {
					case 0:
					case 1:
					case 2:
					case 3:
					case 4:
					case 5:
						chr_fix_369();
						return;
					case 6:
					case 7:
						prg_fix_369();
						return;
				}
			}
			return;
		case 0xA000:
		case 0xB000:
			if (m369.reg == 0x13) {
				m369.irq.enable = value & 0x02;
			}
			extcl_cpu_wr_mem_MMC3(address, value);
			return;
		case 0xC000:
		case 0xD000:
			extcl_cpu_wr_mem_MMC3(address, value);
			return;
		case 0xE000:
		case 0xF000:
			if (m369.reg == 0x13) {
				m369.smb2j = value;
				prg_fix_369();
			}
			extcl_cpu_wr_mem_MMC3(address, value);
			return;
	}
}
BYTE extcl_cpu_rd_mem_369(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	switch (address & 0xF000) {
		case 0x6000:
		case 0x7000:
			return (m369tmp.prg_6000 ? m369tmp.prg_6000[address & 0x1FFF] : prg.ram_plus_8k[address & 0x1FFF]);
	}
	return (openbus);
}
BYTE extcl_save_mapper_369(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m369.reg);
	save_slot_ele(mode, slot, m369.mmc3);
	save_slot_ele(mode, slot, m369.smb2j);
	save_slot_ele(mode, slot, m369.irq.enable);
	save_slot_ele(mode, slot, m369.irq.counter);
	extcl_save_mapper_MMC3(mode, slot, fp);

	if (mode == SAVE_SLOT_READ) {
		prg_fix_369();
	}

	return (EXIT_OK);
}
void extcl_cpu_every_cycle_369(void) {
	if (m369.reg == 0x13) {
		if (m369.irq.enable) {
			m369.irq.counter = (m369.irq.counter + 1) & 0x0FFF;
			if (!m369.irq.counter) {
				irq.high |= EXT_IRQ;
			}
		}
	} else {
		extcl_cpu_every_cycle_MMC3();
	}
}
void extcl_ppu_000_to_34x_369(void) {
	if (!(m369.reg == 0x13)) {
		irqA12_RS();
	}
}
void extcl_ppu_000_to_255_369(void) {
	if (!(m369.reg == 0x13)) {
		if (r2001.visible) {
			irqA12_SB();
		}
	}
}
void extcl_ppu_256_to_319_369(void) {
	if (!(m369.reg == 0x13)) {
		irqA12_BS();
	}
}
void extcl_ppu_320_to_34x_369(void) {
	if (!(m369.reg == 0x13)) {
		irqA12_SB();
	}
}
void extcl_update_r2006_369(WORD new_r2006, WORD old_r2006) {
	if (!(m369.reg == 0x13)) {
		irqA12_IO(new_r2006, old_r2006);
	}
}

INLINE static void prg_fix_369(void) {
	WORD bank;

	m369tmp.prg_6000 = NULL;

	switch (m369.reg) {
		case 0x00:
			bank = 0x00;
			_control_bank(bank, info.prg.rom.max.banks_32k)
			map_prg_rom_8k(4, 0, bank);
			map_prg_rom_8k_update();
			return;
		case 0x01:
			bank = 0x01;
			_control_bank(bank, info.prg.rom.max.banks_32k)
			map_prg_rom_8k(4, 0, bank);
			map_prg_rom_8k_update();
			return;
		case 0x13:
			bank = 0x0E;
			_control_bank(bank, info.prg.rom.max.banks_8k)
			m369tmp.prg_6000 = prg_pnt(bank << 13);

			bank = 0x0C;
			_control_bank(bank, info.prg.rom.max.banks_8k)
			map_prg_rom_8k(1, 0, bank);

			bank = 0x0D;
			_control_bank(bank, info.prg.rom.max.banks_8k)
			map_prg_rom_8k(1, 1, bank);

			bank = 0x08 | (m369.smb2j & 0x03);
			_control_bank(bank, info.prg.rom.max.banks_8k)
			map_prg_rom_8k(1, 2, bank);

			bank = 0x0F;
			_control_bank(bank, info.prg.rom.max.banks_8k)
			map_prg_rom_8k(1, 3, bank);

			map_prg_rom_8k_update();
			return;
		case 0x37:
		case 0xFF:
			if (mmc3.bank_to_update & 0x40) {
				prg_swap_369(0x8000, ~1);
				prg_swap_369(0xC000, m369.mmc3[6]);
			} else {
				prg_swap_369(0x8000, m369.mmc3[6]);
				prg_swap_369(0xC000, ~1);
			}
			prg_swap_369(0xA000, m369.mmc3[7]);
			prg_swap_369(0xE000, ~0);
			return;
	}
}
INLINE static void prg_swap_369(WORD address, WORD value) {
	WORD base = (m369.reg == 0x37) ? 0x10 : 0x20;
	WORD mask = (m369.reg == 0x37) ? 0x0F : 0x1F;

	value = (base & ~mask) | (value & mask);
	control_bank(info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, (address >> 13) & 0x03, value);
	map_prg_rom_8k_update();
}
INLINE static void chr_fix_369(void) {
	DBWORD bank;

	switch (m369.reg) {
		case 0x00:
			bank = 0x00;
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
			return;
		case 0x01:
			bank = 0x01;
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
			return;
		case 0x13:
			bank = 0x03;
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
			return;
		case 0x37:
		case 0xFF: {
			WORD cbase = (mmc3.bank_to_update & 0x80) << 5;

			chr_swap_369(cbase ^ 0x0000, m369.mmc3[0] & (~1));
			chr_swap_369(cbase ^ 0x0400, m369.mmc3[0] |   1);
			chr_swap_369(cbase ^ 0x0800, m369.mmc3[1] & (~1));
			chr_swap_369(cbase ^ 0x0C00, m369.mmc3[1] |   1);
			chr_swap_369(cbase ^ 0x1000, m369.mmc3[2]);
			chr_swap_369(cbase ^ 0x1400, m369.mmc3[3]);
			chr_swap_369(cbase ^ 0x1800, m369.mmc3[4]);
			chr_swap_369(cbase ^ 0x1C00, m369.mmc3[5]);
			return;
		}
	}
}
INLINE static void chr_swap_369(WORD address, WORD value) {
	WORD base = (m369.reg == 0x37) ? 0x0080 : 0x0100;
	WORD mask = 0x7F;

	value = (base & ~mask) | (value & mask);
	control_bank(info.chr.rom.max.banks_1k)
	chr.bank_1k[address >> 10] = chr_pnt(value << 10);
}
