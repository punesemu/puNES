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
#include "irqA12.h"
#include "save_slot.h"

INLINE static void prg_fix_353(void);
INLINE static void prg_swap_353(WORD address, WORD value);
INLINE static void chr_fix_353(void);
INLINE static void chr_swap_353(WORD address, WORD value);
INLINE static void mirroring_fix_353(void);
INLINE static void mirroring_swap_353(BYTE slot, BYTE value);

struct _m353 {
	BYTE reg;
	WORD mmc3[8];
	BYTE chr_writable;
} m353;

void map_init_353(void) {
	EXTCL_AFTER_MAPPER_INIT(353);
	EXTCL_CPU_WR_MEM(353);
	EXTCL_SAVE_MAPPER(353);
	EXTCL_WR_CHR(353);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *)&m353;
	mapper.internal_struct_size[0] = sizeof(m353);
	mapper.internal_struct[1] = (BYTE *)&mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);

	memset(&mmc3, 0x00, sizeof(mmc3));
	memset(&irqA12, 0x00, sizeof(irqA12));
	memset(&m353, 0x00, sizeof(m353));

	m353.mmc3[0] = 0;
	m353.mmc3[1] = 2;
	m353.mmc3[2] = 4;
	m353.mmc3[3] = 5;
	m353.mmc3[4] = 6;
	m353.mmc3[5] = 7;
	m353.mmc3[6] = 0;
	m353.mmc3[7] = 1;

	if (info.format == NES_2_0) {
		if (info.chr.ram.banks_8k_plus > 0) {
			map_chr_ram_extra_init(info.chr.ram.banks_8k_plus * 0x2000);
		}
	} else {
		map_chr_ram_extra_init(0x2000);
	}

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_after_mapper_init_353(void) {
	prg_fix_353();
	chr_fix_353();
	mirroring_fix_353();
}
void extcl_cpu_wr_mem_353(WORD address, BYTE value) {
	if ((address & 0x0FFF) == 0x0080) {
		m353.reg = (address >> 13) & 0x03;
		prg_fix_353();
		chr_fix_353();
		mirroring_fix_353();
		return;
	}
	switch (address & 0xE001) {
		case 0x8000:
			mmc3.bank_to_update = value;
			prg_fix_353();
			chr_fix_353();
			mirroring_fix_353();
			return;
		case 0x8001: {
			WORD cbase = (mmc3.bank_to_update & 0x80) << 5;

			m353.mmc3[mmc3.bank_to_update & 0x07] = value;

			switch (mmc3.bank_to_update & 0x07) {
				case 0:
					prg_fix_353();
					chr_fix_353();
					mirroring_fix_353();
					return;
				case 1:
					chr_swap_353(cbase ^ 0x0800, value & (~1));
					chr_swap_353(cbase ^ 0x0C00, value | 1);
					if ((m353.reg == 0) && !(mmc3.bank_to_update & 0x80)) {
						mirroring_swap_353(2, value);
						mirroring_swap_353(3, value);
					}
					return;
				case 2:
					chr_swap_353(cbase ^ 0x1000, value);
					if ((m353.reg == 0) && (mmc3.bank_to_update & 0x80)) {
						mirroring_swap_353(0, value);
					}
					return;
				case 3:
					chr_swap_353(cbase ^ 0x1400, value);
					if ((m353.reg == 0) && (mmc3.bank_to_update & 0x80)) {
						mirroring_swap_353(1, value);
					}
					return;
				case 4:
					chr_swap_353(cbase ^ 0x1800, value);
					if ((m353.reg == 0) && (mmc3.bank_to_update & 0x80)) {
						mirroring_swap_353(2, value);
					}
					return;
				case 5:
					chr_swap_353(cbase ^ 0x1C00, value);
					if ((m353.reg == 0) && (mmc3.bank_to_update & 0x80)) {
						mirroring_swap_353(3, value);
					}
					return;
				case 6:
					if (mmc3.bank_to_update & 0x40) {
						prg_swap_353(0xC000, value);
					} else {
						prg_swap_353(0x8000, value);
					}
					return;
				case 7:
					prg_swap_353(0xA000, value);
					return;
			}
			return;
		case 0xA000:
			if (m353.reg == 0) {
				return;
			}
			break;
		}
	}
	extcl_cpu_wr_mem_MMC3(address, value);
}
BYTE extcl_save_mapper_353(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m353.reg);
	save_slot_ele(mode, slot, m353.mmc3);
	save_slot_ele(mode, slot, m353.chr_writable);
	save_slot_mem(mode, slot, chr.extra.data, chr.extra.size, FALSE);
	extcl_save_mapper_MMC3(mode, slot, fp);

	if (mode == SAVE_SLOT_READ) {
		chr_fix_353();
		mirroring_fix_353();
	}

	return (EXIT_OK);
}
void extcl_wr_chr_353(WORD address, BYTE value) {
	if (m353.chr_writable) {
		chr.bank_1k[address >> 10][address & 0x3FF] = value;
	}
}

INLINE static void prg_fix_353(void) {
	if (mmc3.bank_to_update & 0x40) {
		prg_swap_353(0x8000, ~1);
		prg_swap_353(0xC000, m353.mmc3[6]);
	} else {
		prg_swap_353(0x8000, m353.mmc3[6]);
		prg_swap_353(0xC000, ~1);
	}
	prg_swap_353(0xA000, m353.mmc3[7]);
	prg_swap_353(0xE000, ~0);
}
INLINE static void prg_swap_353(WORD address, WORD value) {
	WORD base = m353.reg << 5;
	WORD mask = 0x1F;

	if (m353.reg == 2) {
		base |= (m353.mmc3[0] & 0x80 ? 0x10 : 0x00);
		mask >>= 1;
	} else if ((m353.reg == 3) && !(m353.mmc3[0] & 0x80) && (address >= 0xC000)) {
		base = 0x70;
		mask = 0x0F;
		value = m353.mmc3[address >> 13];
	}

	value = (base & ~mask) | (value & mask);
	control_bank(info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, (address >> 13) & 0x03, value);
	map_prg_rom_8k_update();
}
INLINE static void chr_fix_353(void) {
	WORD cbase = (mmc3.bank_to_update & 0x80) << 5;

	chr_swap_353(cbase ^ 0x0000, m353.mmc3[0] & (~1));
	chr_swap_353(cbase ^ 0x0400, m353.mmc3[0] |   1);
	chr_swap_353(cbase ^ 0x0800, m353.mmc3[1] & (~1));
	chr_swap_353(cbase ^ 0x0C00, m353.mmc3[1] |   1);
	chr_swap_353(cbase ^ 0x1000, m353.mmc3[2]);
	chr_swap_353(cbase ^ 0x1400, m353.mmc3[3]);
	chr_swap_353(cbase ^ 0x1800, m353.mmc3[4]);
	chr_swap_353(cbase ^ 0x1C00, m353.mmc3[5]);
}
INLINE static void chr_swap_353(WORD address, WORD value) {
	WORD base = m353.reg << 7;
	WORD mask = 0x7F;

	if ((m353.reg == 2) && (m353.mmc3[0] & 0x80)) {
		m353.chr_writable = TRUE;
		value = address >> 10;
		control_bank(info.chr.ram.max.banks_1k)
		chr.bank_1k[address >> 10] = &chr.extra.data[value << 10];
	} else {
		m353.chr_writable = FALSE;
		value = (base & ~mask) | (value & mask);
		control_bank(info.chr.rom.max.banks_1k)
		chr.bank_1k[address >> 10] = chr_pnt(value << 10);
	}
}
INLINE static void mirroring_fix_353(void) {
	if (m353.reg == 0) {
		 if (!(m353.mmc3[0] & 0x80)) {
			mirroring_swap_353(0, m353.mmc3[0]);
			mirroring_swap_353(1, m353.mmc3[0]);
			mirroring_swap_353(2, m353.mmc3[1]);
			mirroring_swap_353(3, m353.mmc3[1]);
		 } else {
			mirroring_swap_353(0, m353.mmc3[2]);
			mirroring_swap_353(1, m353.mmc3[3]);
			mirroring_swap_353(2, m353.mmc3[4]);
			mirroring_swap_353(3, m353.mmc3[5]);
		 }
	}
}
INLINE static void mirroring_swap_353(BYTE slot, BYTE value) {
	ntbl.bank_1k[slot] = &ntbl.data[((value >> 7) ^ 0x01) << 10];
}
