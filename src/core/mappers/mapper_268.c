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

// Thx to NewRisingSun (NintendulatorNRS) and guys of Nintendulator
// NintendulatorNRS/src-mappers/src/iNES/mapper268.cpp

#include <string.h>
#include "mappers.h"
#include "save_slot.h"

void prg_swap_mmc3_268(WORD address, WORD value);
void chr_swap_mmc3_268(WORD address, WORD value);
void wram_fix_mmc3_268(void);

struct _m268 {
	BYTE reg[8];
} m268;
struct _m268tmp {
	WORD rstart;
	WORD rstop;
} m268tmp;

void map_init_268(void) {
	EXTCL_AFTER_MAPPER_INIT(MMC3);
	EXTCL_CPU_WR_MEM(268);
	EXTCL_SAVE_MAPPER(268);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *)&m268;
	mapper.internal_struct_size[0] = sizeof(m268);
	mapper.internal_struct[1] = (BYTE *)&mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);

	memset(&nes[0].irqA12, 0x00, sizeof(nes[0].irqA12));
	memset(&m268, 0x00, sizeof(m268));

	init_MMC3(HARD);
	MMC3_prg_swap = prg_swap_mmc3_268;
	MMC3_chr_swap = chr_swap_mmc3_268;
	MMC3_wram_fix = wram_fix_mmc3_268;

	if ((info.mapper.id == 224) || (info.mapper.submapper & 0x01)) {
		m268tmp.rstart = 0x5000;
		m268tmp.rstop = 0x5FFF;
	} else {
		m268tmp.rstart = 0x6000;
		m268tmp.rstop = 0x7FFF;
	}

	info.mapper.extend_wr = TRUE;

	nes[0].irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_268(BYTE nidx, WORD address, BYTE value) {
	if ((address >= m268tmp.rstart) && (address <= m268tmp.rstop)) {
		if (!(m268.reg[3] & 0x80) || ((address & 0x07) == 2)) {
			if  ((address & 0x07) == 2) {
				if (m268.reg[2] & 0x80) {
					value = (value & 0x0F) | (m268.reg[2] & 0xF0);
				}
				value &= ((~m268.reg[2] & 0x70) >> 3) | 0xF1;
			}
			m268.reg[address & 0x07] = value;
			MMC3_chr_fix();
			MMC3_prg_fix();
		}
		return;
	}
	if (address >= 0x8000) {
		extcl_cpu_wr_mem_MMC3(nidx, address, value);
	}
}
BYTE extcl_save_mapper_268(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m268.reg);
	return (extcl_save_mapper_MMC3(mode, slot, fp));
}

void prg_swap_mmc3_268(WORD address, WORD value) {
	int mask_mmc3 = (m268.reg[3] & 0x10 ? 0x00 : 0x0F) | (m268.reg[0] & 0x40 ? 0x00 : 0x10) |
		(m268.reg[1] & 0x80 ? 0x00 : 0x20) | (m268.reg[1] & 0x40 ? 0x40 : 0x00) | (m268.reg[1] & 0x20 ? 0x80 : 0x00);
	int mask_gnrom = 0;
	int offset = 0;
	int masked_offset = 0;
	int slot = (address >> 13) & 0x03;
	BYTE chip = FALSE;
	DBWORD bank = 0;

	switch (info.mapper.submapper & ~1) {
		default:
			mask_gnrom = (m268.reg[3] & 0x10 ? (m268.reg[1] & 0x02 ? 0x03 : 0x01) : 0x00);
			offset = (m268.reg[3] & 0x0E) | ((m268.reg[0] & 0x07) << 4) | (m268.reg[1] & 0x10 ? 0x80 : 0x00) |
				((m268.reg[1] & 0x0C) << 6) | ((m268.reg[0] & 0x30) << 6);
			break;
		case 2:
			mask_gnrom = (m268.reg[3] & 0x10 ? (m268.reg[1] & 0x10 ? 0x01 : 0x03) : 0x00);
			offset = (m268.reg[3] & 0x0E) | ((m268.reg[0] & 0x07) << 4) | (m268.reg[1] & 0x08 ? 0x80 : 0x00) |
				(m268.reg[1] & 0x04 ? 0x100 : 0x00) | (m268.reg[1] & 0x02 ? 0x200 : 0x00) |((m268.reg[0] & 0x30) << 6);
			break;
		case 4:
			mask_gnrom = (m268.reg[3] & 0x10? (m268.reg[1] & 0x02 ? 0x03 : 0x01) : 0x00);
			offset = (m268.reg[3] & 0x0E) | ((m268.reg[0] & 0x07) << 4) | ((m268.reg[0] & 0x30) << 3);
			break;
		case 6:
			mask_gnrom = (m268.reg[3] & 0x10? (m268.reg[1] & 0x02 ? 0x03 : 0x01) : 0x00);
			offset = (m268.reg[3] & 0x0E) | ((m268.reg[0] & 0x07) << 4) | (m268.reg[1] & 0x10 ? 0x80 : 0x00) |
				((m268.reg[1] & 0x0C) << 6) | ((m268.reg[0] & 0x30) << 6);
			offset &= (int)(prgrom_size() / S8K / 2) - 1;
			if (m268.reg[0] & 0x80) {
				chip = m268.reg[0] & 0x08;
			} else {
				chip = ((mmc3.bank_to_update & 0x80) ? mmc3.reg[2] : mmc3.reg[0]) & 0x80;
			}
			if (chip) {
				offset |= (int)(prgrom_size() / S8K / 2);
			}
			break;
	}

	masked_offset = offset & ~(mask_mmc3 | mask_gnrom);

	if (m268.reg[3] & 0x40) {
		if (!(mmc3.bank_to_update & 0x40)) {
			switch (address) {
				case 0xC000:
					bank = masked_offset | (2 & mask_gnrom);
					break;
				case 0xE000:
					bank = masked_offset | (3 & mask_gnrom);
					break;
			}
		}
	} else {
		bank = (value & mask_mmc3) | masked_offset | (slot & mask_gnrom);
	}

	prg_swap_MMC3_base(address, bank);
}
void chr_swap_mmc3_268(WORD address, WORD value) {
	int mask_mmc3 = m268.reg[3] & 0x10 ? 0x00 : m268.reg[0] & 0x80 ? 0x7F : 0xFF;
	int offset = ((m268.reg[2] & 0x0F) << 3) | ((m268.reg[0] & 0x08) << 4);
	int mask_gnrom = m268.reg[3] & 0x10 ? 0x07 : 0x00;
	int masked_offset = 0;
	BYTE enabled = !(((info.mapper.submapper & ~1) == 8) && (m268.reg[0] & 0x10));
	BYTE is_vram = FALSE;
	DBWORD bank = 0;
	int slot = 0;

	switch (info.mapper.submapper & ~1) {
		default:
			offset |= ((m268.reg[0] & 0x30) << 4);
			break;
		case 4:
			offset |= ((m268.reg[0] & 0x06) << 7) | ((m268.reg[0] & 0x30) << 6);
			break;
	}

	masked_offset = offset & ~(mask_mmc3 | mask_gnrom);

	if (m268.reg[3] & 0x40) {
		switch (address ^ ((mmc3.bank_to_update & 0x80) << 5)) {
			case 0x0000:
				value = mmc3.reg[0];
				break;
			case 0x0800:
				value = mmc3.reg[1];
				break;
			case 0x0400:
			case 0x0C00:
				value = 0;
				break;
		}
		slot = (address >> 10) & 0x03;
		is_vram = vram_size(0) != 0;
	} else {
		slot = (address >> 10) & 0x07;
		is_vram = (m268.reg[4] & 0x01) && ((value & 0xFE) == (m268.reg[4] & 0xFE)) && vram_size(0);
	}

	bank = (value & mask_mmc3) | masked_offset | (slot & mask_gnrom);

	if (is_vram) {
		memmap_vram_wp_1k(0, MMPPU(address), bank, TRUE, enabled);
	} else {
		memmap_auto_wp_1k(0, MMPPU(address), bank, TRUE, enabled);
	}
}
void wram_fix_mmc3_268(void) {
	if (mmc3.wram_protect & 0x20) {
		// Hack for FS005 games on Mindkids board that only work with emulation.
		mmc3.wram_protect &= ~0x40;
	}
	wram_fix_MMC3_base();
}
