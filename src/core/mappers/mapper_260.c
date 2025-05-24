/*
 *  Copyright (C) 2010-2024 Fabio Cavallo (aka FHorse)
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
#include "save_slot.h"

INLINE static void prg_fix_260(void);
INLINE static void chr_fix_260(void);
INLINE static void mirroring_fix_260(void);

struct _m260 {
	BYTE cpu5xxx[4];
	BYTE cpu8xxx[2];
	BYTE cnrom_chr_reg;
	BYTE mmc3[8];
	WORD prg_base;
} m260;

void map_init_260(void) {
	EXTCL_AFTER_MAPPER_INIT(260);
	EXTCL_CPU_WR_MEM(260);
	EXTCL_CPU_RD_MEM(260);
	EXTCL_SAVE_MAPPER(260);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	map_internal_struct_init((BYTE *)&m260, sizeof(m260));

	memset(&m260, 0x00, sizeof(m260));
	if (info.reset >= HARD) {
		memset(&nes[0].irqA12, 0x00, sizeof(nes[0].irqA12));

		m260.mmc3[0] = 0;
		m260.mmc3[1] = 2;
		m260.mmc3[2] = 4;
		m260.mmc3[3] = 5;
		m260.mmc3[4] = 6;
		m260.mmc3[5] = 7;
		m260.mmc3[6] = 0;
		m260.mmc3[7] = 0;
	}

	init_MMC3(info.reset);

	info.mapper.extend_wr = TRUE;

	nes[0].irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_after_mapper_init_260(void) {
	prg_fix_260();
	chr_fix_260();
	mirroring_fix_260();
}
void extcl_cpu_wr_mem_260(BYTE nidx, WORD address, BYTE value) {
	if (address < 0x8000) {
		if ((address >= 0x5000) && (address <= 0x5FFF)) {
			if (m260.cpu5xxx[0] & 0x80) {
				return;
			}
			switch (address & 0x03) {
				case 0:
					m260.cpu5xxx[0] = value;
					prg_fix_260();
					chr_fix_260();
					break;
				case 1:
					m260.cpu5xxx[1] = value;
					m260.prg_base = (m260.prg_base & ~0x3F) | (value & 0x3F);
					prg_fix_260();
					break;
				case 2:
					m260.cpu5xxx[2] = value;
					chr_fix_260();
					break;
				case 3:
					m260.cpu5xxx[3] = value;
					break;
			}
		}
	} else {
		if ((m260.cpu5xxx[0] & 0x07) >= 6) {
			m260.cnrom_chr_reg = value & 0x03;
			chr_fix_260();
			mirroring_fix_260();
		}
		switch (address & 0xE001) {
			case 0x8000:
				m260.cpu8xxx[0] = value;
				prg_fix_260();
				chr_fix_260();
				break;
			case 0x8001:
				m260.cpu8xxx[1] = value;
				m260.mmc3[m260.cpu8xxx[0] & 0x07] = value;
				prg_fix_260();
				chr_fix_260();
				break;
			case 0xA000:
			case 0xA001:
			case 0xC000:
			case 0xC001:
			case 0xE000:
			case 0xE001:
				extcl_cpu_wr_mem_MMC3(nidx, address, value);
				break;
		}
	}
}
BYTE extcl_cpu_rd_mem_260(BYTE nidx, WORD address, BYTE openbus) {
	if ((address >= 0x5000) && (address <= 0x5FFF)) {
		return ((openbus & ~0x03) | (dipswitch.value & 0x03));
	}
	return (wram_rd(nidx, address));
}
BYTE extcl_save_mapper_260(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m260.cpu5xxx);
	save_slot_ele(mode, slot, m260.cpu8xxx);
	save_slot_ele(mode, slot, m260.cnrom_chr_reg);
	save_slot_ele(mode, slot, m260.mmc3);
	save_slot_ele(mode, slot, m260.prg_base);
	return (EXIT_OK);
}

INLINE static void prg_fix_260(void) {
	switch (m260.cpu5xxx[0] & 0x07) {
		case 0:
		case 1:
		case 2:
		case 3: {
			WORD swap = (m260.cpu8xxx[0] & 0x40) << 8;
			WORD base = m260.prg_base << 1;
			WORD mask = m260.cpu5xxx[0] & 0x02 ? 0x0F : 0x1F;

			base &= ~mask;
			memmap_auto_8k(0, MMCPU(0x8000 ^ swap), (base | (m260.mmc3[6] & mask)));
			memmap_auto_8k(0, MMCPU(0xA000), (base | (m260.mmc3[7] & mask)));
			memmap_auto_8k(0, MMCPU(0xC000 ^ swap), (base | (0xFE & mask)));
			memmap_auto_8k(0, MMCPU(0xE000), (base | (0xFF & mask)));
			break;
		}
		case 4:
			memmap_auto_16k(0, MMCPU(0x8000), m260.prg_base);
			memmap_auto_16k(0, MMCPU(0xC000), m260.prg_base);
			break;
		case 5:
		case 6:
		case 7:
			memmap_auto_32k(0, MMCPU(0x8000), (m260.prg_base >> 1));
			break;
	}
}
INLINE static void chr_fix_260(void) {
	WORD mask = 0, bank[8], base = m260.cpu5xxx[2] << 3;
	WORD swap = 0;

	switch (m260.cpu5xxx[0] & 0x07) {
		default:
		case 0:
		case 1:
		case 2:
		case 3:
			swap = (m260.cpu8xxx[0] & 0x80) << 5;
			mask = m260.cpu5xxx[0] & 0x01 ? 0x7F : 0xFF;
			base &= ~mask;

			bank[0] = base | ((m260.mmc3[0] & 0xFE) & mask);
			bank[1] = base | ((m260.mmc3[0] | 0x01) & mask);
			bank[2] = base | ((m260.mmc3[1] & 0xFE) & mask);
			bank[3] = base | ((m260.mmc3[1] | 0x01) & mask);
			bank[4] = base | (m260.mmc3[2] & mask);
			bank[5] = base | (m260.mmc3[3] & mask);
			bank[6] = base | (m260.mmc3[4] & mask);
			bank[7] = base | (m260.mmc3[5] & mask);
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
			mask = m260.cpu5xxx[0] & 0x01 ? 0x03 : 0x01;
			base = ((m260.cpu5xxx[2] & ~mask) | (m260.cnrom_chr_reg & mask)) << 3;

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
	memmap_auto_1k(0, MMPPU(0x0000 ^ swap), bank[0]);
	memmap_auto_1k(0, MMPPU(0x0400 ^ swap), bank[1]);
	memmap_auto_1k(0, MMPPU(0x0800 ^ swap), bank[2]);
	memmap_auto_1k(0, MMPPU(0x0C00 ^ swap), bank[3]);
	memmap_auto_1k(0, MMPPU(0x1000 ^ swap), bank[4]);
	memmap_auto_1k(0, MMPPU(0x1400 ^ swap), bank[5]);
	memmap_auto_1k(0, MMPPU(0x1800 ^ swap), bank[6]);
	memmap_auto_1k(0, MMPPU(0x1C00 ^ swap), bank[7]);
}
INLINE static void mirroring_fix_260(void) {
	if ((m260.cpu5xxx[0] & 0x07) >= 6) {
		if (m260.cnrom_chr_reg & 0x04) {
			mirroring_H(0);
		} else {
			mirroring_V(0);
		}
	}
}
