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
#include "irqA12.h"
#include "save_slot.h"

INLINE static void prg_fix_176(void);
INLINE static void chr_fix_176(void);
INLINE static void chr_swap_176(WORD address, WORD value);
INLINE static void wram_fix_176(void);
INLINE static void mirroring_fix_176(void);

INLINE static BYTE mmc3ext(void);

struct _m176 {
	BYTE cpu4800;
	BYTE cpu5xxx[8];
	BYTE cpu8xxx[3];
	BYTE ram_cfg_reg;
	BYTE ram_out_reg;
	BYTE ram_enabled;
	BYTE mmc3_bank_to_update;
	BYTE mmc3[16];
} m176;

void map_init_176(void) {
	EXTCL_AFTER_MAPPER_INIT(176);
	EXTCL_CPU_WR_MEM(176);
	EXTCL_SAVE_MAPPER(176);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *)&m176;
	mapper.internal_struct_size[0] = sizeof(m176);

	if (info.format != NES_2_0) {
		if (info.mapper.battery) {
			info.mapper.submapper = 2;
			wram_set_ram_size(S32K);
		} else if ((prgrom_size() == S1M) && (chrrom_size() == S1M)) {
			info.mapper.submapper = 1;
		}  else if ((prgrom_size() == S256K) && (chrrom_size() == S128K)) {
			info.mapper.submapper = 1;
		}  else if ((prgrom_size() >= S8M) && !chrrom_size()) {
			info.mapper.submapper = 2;
		}  else if ((prgrom_size() >= S4M) && !chrrom_size()) {
			info.mapper.submapper = 3;
		}
		if (chrrom_size()) {
			if ((prgrom_size() == S2M) && (chrrom_size() == S512K)) {
				vram_set_ram_size(0, S8K);
			} else {
				vram_set_ram_size(0, 0);
			}
		} else {
			switch (prgrom_size()) {
				case S2M:
				case S4M:
					vram_set_ram_size(0, S128K);
					break;
				default:
					vram_set_ram_size(0, S8K);
					break;
			}
		}
	}

	memset(&nes[0].irqA12, 0x00, sizeof(nes[0].irqA12));
	memset(&m176, 0x00, sizeof(m176));

	m176.mmc3[0] = 0;
	m176.mmc3[1] = 2;
	m176.mmc3[2] = 4;
	m176.mmc3[3] = 5;
	m176.mmc3[4] = 6;
	m176.mmc3[5] = 7;
	m176.mmc3[6] = 0;
	m176.mmc3[7] = 1;
	m176.mmc3[8] = 0xFE;
	m176.mmc3[9] = 0xFF;
	m176.mmc3[10] = 0x01;
	m176.mmc3[11] = 0x03;

	if ((info.mapper.submapper == 1) || (info.mapper.submapper == 3)) {
		m176.cpu5xxx[0] = 0x07;
	}
	if (info.mapper.id == 523) {
		m176.cpu8xxx[1] = (info.mapper.mirroring & 0x01) ^ 0x01;
	}

	// (JY-224) 1998 97格鬥天王 激鬥篇 7-in-1.nes
	if (info.crc32.total == 0x3C894AD1) {
		m176.cpu5xxx[0] = 0x00;
	}

	// Super Rockman 6-in-1 (861234C Hack).unif
	// Super Rockman 4 ha bisogno di almeno 8k di wram per funzionare correttamente
//	wram_set_ram_size(S32K);

	if (!dipswitch.used) {
		dipswitch.value = 0x10;
	}

	info.mapper.extend_wr = TRUE;

	nes[0].irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_after_mapper_init_176(void) {
	prg_fix_176();
	chr_fix_176();
	wram_fix_176();
	mirroring_fix_176();
}
void extcl_cpu_wr_mem_176(BYTE nidx, WORD address, BYTE value) {
	switch (address & 0xF000) {
		case 0x4000:
			if ((address & 0x0800) && (info.mapper.submapper == 5)) {
				m176.cpu4800 = value;
				prg_fix_176();
			}
			return;
		case 0x5000:
			if (!m176.ram_cfg_reg || m176.ram_out_reg) {
				WORD mask = 0x5000 | dipswitch.value;

				if ((address & mask) != mask) {
					return;
				}
				m176.cpu5xxx[address & (info.mapper.submapper == 3 ? 0x07 : 0x03)] = value;
				extcl_after_mapper_init_176();
			}
			return;
		case 0x8000:
		case 0x9000:
			switch (address & 0x0003) {
				case 0:
					m176.mmc3_bank_to_update = ((info.mapper.submapper == 2) && (value & 0x40) && ((value & 0x07) >= 6))
						? value ^ 1
						: value;
					break;
				case 1:
					m176.mmc3[m176.mmc3_bank_to_update & (mmc3ext() ? 0x0F : 0x07)] = value;
					break;
				case 2:
				case 3:
					break;
			}
			m176.cpu8xxx[0] = value;
			extcl_after_mapper_init_176();
			return;
		case 0xA000:
		case 0xB000:
			if (address & 0x0001) {
				m176.cpu8xxx[2] = value;
				m176.ram_cfg_reg = (value & 0x20) && (info.mapper.submapper == 2);
				m176.ram_out_reg = value & 0x40;
				m176.ram_enabled = value & 0x80;
			} else {
				m176.cpu8xxx[1] = value;
			}
			m176.cpu8xxx[0] = value;
			extcl_after_mapper_init_176();
			return;
		case 0xC000:
		case 0xD000:
		case 0xE000:
		case 0xF000:
			extcl_cpu_wr_mem_MMC3(nidx, address, value);
			m176.cpu8xxx[0] = value;
			extcl_after_mapper_init_176();
			return;
	}
}
BYTE extcl_save_mapper_176(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m176.cpu4800);
	save_slot_ele(mode, slot, m176.cpu5xxx);
	save_slot_ele(mode, slot, m176.cpu8xxx);
	save_slot_ele(mode, slot, m176.ram_cfg_reg);
	save_slot_ele(mode, slot, m176.ram_out_reg);
	save_slot_ele(mode, slot, m176.ram_enabled);
	save_slot_ele(mode, slot, m176.mmc3);
	return (EXIT_OK);
}

INLINE static void prg_fix_176(void) {
	static const WORD prg_mask[8] = {
		0x3F, 0x1F, 0x0F, 0x00,
		0x00, 0x00, 0x7F, 0xFF
	};
	BYTE ext = mmc3ext();
	BYTE mode = m176.cpu5xxx[0] & 0x07;
	WORD mask = ext ? 0xFF : prg_mask[mode];
	WORD base = m176.cpu5xxx[1] & 0x7F;

	switch (info.mapper.submapper) {
		case 1:
			if ((mode == 7) || ext) {
				mask = 0xFF;
			}
			break;
		case 2:
			base |= ((m176.cpu5xxx[0] & 0x08) << 4) | ((m176.cpu5xxx[0] & 0x80) << 1) |
				((m176.cpu5xxx[2] & 0xC0) << 3) | ((m176.cpu5xxx[2] & 0x20) << 6);
			break;
		case 3:
			base |= (m176.cpu5xxx[5] << 7);
			if (mode == 7) {
				mask = 0xFF;
			}
			break;
		case 4:
			base |= m176.cpu5xxx[2] & 0x80;
			break;
		case 5:
			base = (base & 0x1F) | (m176.cpu4800 << 5);
			break;
	}

	switch (mode) {
		case 3:
			memmap_auto_16k(0, MMCPU(0x8000), base);
			memmap_auto_16k(0, MMCPU(0xC000), base);
			return;
		case 4:
			base >>= 1;
			memmap_auto_32k(0, MMCPU(0x8000), base);
			return;
		case 5:
			memmap_auto_16k(0, MMCPU(0x8000), ((base & 0xFFF8) | (m176.cpu8xxx[0] & 0x07) | 0x00));
			memmap_auto_16k(0, MMCPU(0xC000), ((base & 0xFFF8) | (m176.cpu8xxx[0] & 0x07) | 0x07));
			return;
		default: {
			WORD swap = (m176.mmc3_bank_to_update & 0x40) << 8;
			WORD bank[4];

			base <<= 1;

			bank[0] = (base & ~mask) | (m176.mmc3[6] & mask);
			bank[1] = (base & ~mask) | (m176.mmc3[7] & mask);
			bank[2] = (base & ~mask) | ((!ext ? 0xFE : m176.mmc3[8]) & mask);
			bank[3] = (base & ~mask) | ((!ext ? 0xFF : m176.mmc3[9]) & mask);

			memmap_auto_8k(0, MMCPU(0x8000 ^ swap), bank[0]);
			memmap_auto_8k(0, MMCPU(0xA000), bank[1]);
			memmap_auto_8k(0, MMCPU(0xC000 ^ swap), bank[2]);
			memmap_auto_8k(0, MMCPU(0xE000), bank[3]);
			return;
		}
	}
}
INLINE static void chr_fix_176(void) {
	DBWORD base = m176.cpu5xxx[2] << 3;
	DBWORD bank[8];

	if (info.mapper.submapper == 3) {
		base |= m176.cpu5xxx[6] << 11;
	}

	if ((m176.cpu5xxx[0] & 0x60) || (!chrrom_size() && (vram_size(0) == S8K))) {
		DBWORD mask = !(m176.cpu5xxx[0] & 0x20) && ((info.mapper.submapper == 1) || (info.mapper.submapper == 5))
			? (m176.cpu5xxx[0] & 0x010 ? 0x08 : 0x18)
			: 0x00;

		base = (info.mapper.submapper == 5 ? base : (base & ~mask)) | ((m176.cpu8xxx[0] << 3) & mask);
		bank[0] = base | 0;
		bank[1] = base | 1;
		bank[2] = base | 2;
		bank[3] = base | 3;
		bank[4] = base | 4;
		bank[5] = base | 5;
		bank[6] = base | 6;
		bank[7] = base | 7;
	} else {
		DBWORD mask = (m176.cpu5xxx[0] & 0x10) ? 0x7F : 0xFF;

		if (mmc3ext()) {
			bank[0] = (base & ~mask) | (m176.mmc3[0]  & mask);
			bank[1] = (base & ~mask) | (m176.mmc3[10] & mask);
			bank[2] = (base & ~mask) | (m176.mmc3[1]  & mask);
			bank[3] = (base & ~mask) | (m176.mmc3[11] & mask);
			bank[4] = (base & ~mask) | (m176.mmc3[2]  & mask);
			bank[5] = (base & ~mask) | (m176.mmc3[3]  & mask);
			bank[6] = (base & ~mask) | (m176.mmc3[4]  & mask);
			bank[7] = (base & ~mask) | (m176.mmc3[5]  & mask);
		} else {
			bank[0] = (base & ~mask) | ((m176.mmc3[0] & mask) & ~1) | 0;
			bank[1] = (base & ~mask) | ((m176.mmc3[0] & mask) & ~1) | 1;
			bank[2] = (base & ~mask) | ((m176.mmc3[1] & mask) & ~1) | 0;
			bank[3] = (base & ~mask) | ((m176.mmc3[1] & mask) & ~1) | 1;
			bank[4] = (base & ~mask) | (m176.mmc3[2] & mask);
			bank[5] = (base & ~mask) | (m176.mmc3[3] & mask);
			bank[6] = (base & ~mask) | (m176.mmc3[4] & mask);
			bank[7] = (base & ~mask) | (m176.mmc3[5] & mask);
		}
	}

	if (info.mapper.id == 523) {
		memmap_auto_2k(0, MMPPU(0x0000), bank[0]);
		memmap_auto_2k(0, MMPPU(0x0800), bank[2]);
		memmap_auto_2k(0, MMPPU(0x1000), bank[4]);
		memmap_auto_2k(0, MMPPU(0x1800), bank[6]);
	} else {
		WORD swap = (m176.mmc3_bank_to_update & 0x80) << 5;

		chr_swap_176(0x0000 ^ swap, bank[0]);
		chr_swap_176(0x0400 ^ swap, bank[1]);
		chr_swap_176(0x0800 ^ swap, bank[2]);
		chr_swap_176(0x0C00 ^ swap, bank[3]);
		chr_swap_176(0x1000 ^ swap, bank[4]);
		chr_swap_176(0x1400 ^ swap, bank[5]);
		chr_swap_176(0x1800 ^ swap, bank[6]);
		chr_swap_176(0x1C00 ^ swap, bank[7]);
	}
}
INLINE static void chr_swap_176(WORD address, WORD value) {
	if (((vram_size(0) && (m176.cpu5xxx[0] & 0x20) && !(m176.cpu5xxx[0] & 0x40)) || m176.ram_cfg_reg) &&
		(m176.cpu8xxx[2] & 0x04) && (value < 8)) {
		memmap_vram_1k(0, MMPPU(address), value);
	} else {
		memmap_auto_1k(0, MMPPU(address), value);
	}
}
INLINE static void wram_fix_176(void) {
	const WORD bank = (m176.cpu8xxx[2] & 0x03);

	// 7654 3210
	// ---- ----
	// RFE. SCWW
	// |||  ||||
	// |||  ||++- Select 8 KiB PRG-RAM bank at $6000-$7FFF. Ignored if Bit 5 is clear.
	// |||  |+--- Select the memory type in the first 8 KiB of CHR space. Ignored if Bit 5 is clear.
	// |||  |      0: First 8 KiB are CHR-ROM
	// |||  |      1: First 8 KiB are CHR-RAM
	// |||  +---- Unknown
	// ||+------- RAM Configuration Register Enable
	// ||          0: RAM Configuration Register disabled, $A001 functions as on MMC3, 8 KiB of WRAM
	// ||          1: RAM Configuration Register enabled, 32 KiB of WRAM
	// |+-------- Outer Bank Registers Enable. Ignored if Bit 5 is clear.
	// |           0: Outer Bank Registers disabled, $5000-$5FFF maps to the second 4 KiB of the 8 KiB WRAM bank 2
	// |           1: Outer Bank Registers enabled in the $5000-$5FFF range
	// +--------- PRG RAM enable (0: disable, 1: enable)
	if (m176.ram_enabled || m176.ram_cfg_reg) {
		if (m176.ram_cfg_reg) {
			memmap_auto_8k(0, MMCPU(0x4000), bank + 1);
			memmap_auto_8k(0, MMCPU(0x6000), bank);
		} else {
			// MMC3 mode
			memmap_disable_4k(0, MMCPU(0x5000));
			memmap_auto_wp_8k(0, MMCPU(0x6000), 0, m176.ram_enabled >> 7, !m176.ram_out_reg);
		}
	} else {
		memmap_disable_16k(0, MMCPU(0x4000));
	}
}
INLINE static void mirroring_fix_176(void) {
	// Single-screen mirroring is only available when the RAM Configuration Register is enabled ($A001.5).
	switch ((m176.cpu8xxx[1] & 0x03) & (m176.ram_cfg_reg ? 0x03 : 0x01)) {
		case 0:
			mirroring_V(0);
			break;
		case 1:
			mirroring_H(0);
			break;
		case 2:
			mirroring_SCR0(0);
			break;
		case 3:
			mirroring_SCR1(0);
			break;
	}
}

INLINE static BYTE mmc3ext(void) {
	return ((m176.cpu5xxx[3] & 0x02) && ((info.mapper.submapper == 1) || (info.mapper.submapper == 2) ||
		(info.mapper.id == 523)));
}
