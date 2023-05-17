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

INLINE static void prg_fix_176(void);
INLINE static void chr_fix_176(void);
INLINE static void chr_swap_176(BYTE slot, WORD value);
INLINE static BYTE chr_ram_176(WORD value);
INLINE static void wram_fix_176(void);
INLINE static void mirroring_fix_176(void);
INLINE static void tmp_fix_176(BYTE max, BYTE index, const WORD *ds);

struct _m176 {
	BYTE cpu4800;
	BYTE cpu5xxx[8];
	BYTE cpu8xxx[4];
	BYTE cnrom_chr_reg;
	BYTE ram_cfg_reg;
	BYTE ram_out_reg;
	BYTE ram_enabled;
	BYTE mmc3[12];
	WORD prg;
} m176;
struct _m176tmp {
	BYTE ds_used;
	BYTE max;
	BYTE index;
	const WORD *dipswitch;
} m176tmp;

void map_init_176(void) {
	EXTCL_AFTER_MAPPER_INIT(176);
	EXTCL_CPU_WR_MEM(176);
	EXTCL_SAVE_MAPPER(176);
	EXTCL_WR_CHR(176);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *)&m176;
	mapper.internal_struct_size[0] = sizeof(m176);

	memset(&irqA12, 0x00, sizeof(irqA12));
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
	m176.mmc3[10] = 0xFF;
	m176.mmc3[11] = 0xFF;

	if (info.format != NES_2_0) {
		if ((info.crc32.prg == 0x678DE5AA) || // 120-in-1 (Unl)[U].unf
			(info.crc32.prg == 0xE6D869ED) || // 6-in-1 Rockman (Unl) [U][!].unf
			(info.crc32.prg == 0xE79F157E) || // 23 Plus 222-in-1 (Unl) [U][!].unf
			(info.crc32.prg == 0x7B766BC1) || // Super 24-in-1 [U][p1][!].unf
			(info.crc32.prg == 0x6C979BAC) || // 10-in-1 Omake Game (FC Mobile)[!].unf
			(info.crc32.prg == 0xC8238ADE) || // Super Rockman 6-in-1 (861234C Hack).unif
			(info.crc32.prg == 0xD14617D7)) { // He Jin Zhuang Bei 150-in-1 Real Game (Unl) [U][!].unf
			if (mapper.write_vram) {
				info.chr.rom.banks_8k = 32;
			} else {
				info.chr.ram.banks_8k_plus = 1;
			}
		}
	}

	if (info.mapper.submapper == DEFAULT) {
		info.mapper.submapper = LP8002KB;
	}
	if ((prg_size() == (1024 * 128)) && (chr_size() == (1024 * 64))) {
		info.mapper.submapper = BMCFK23C;
	}
	if ((prg_size() == (1024 * 256)) && (chr_size() == (1024 * 128))) {
		info.mapper.submapper = BMCFK23C;
	}
	if ((prg_size() == (1024 * 1024)) && (prg_size() == chr_size())) {
		// (JY-224) 1998 97格鬥天王 激鬥篇 7-in-1.nes
		m176.prg = (info.crc32.prg == 0x6C574B50) ? 0x40 : 0x20;
		info.mapper.submapper = BMCFK23C;
	}
	if (prg_size() >= (8192 * 1024) && mapper.write_vram) {
		info.mapper.submapper = FS005;
	}
	if (prg_size() == (4096 * 1024) && mapper.write_vram) {
		info.mapper.submapper = JX9003B;
	}
	if ((info.crc32.prg == 0x3655B7BC) || // New 4-in-1 Supergame (YH4239) [p1][U][!].unf
		(info.crc32.prg == 0x60C6D8CD) || // 9-in-1 (KY-9005) [p1][U][!].unf
		(info.crc32.prg == 0x63A87C95)) { // 8-in-1 Supergame (KY8002) [p1][U].unf
		info.mapper.submapper = LP8002KB;
	}
	if (info.crc32.prg == 0xB50D8FBC) { // Game 500-in-1.nes
		info.mapper.submapper = HST162;
	}

	if (info.mapper.submapper == JX9003B) {
		m176.cpu5xxx[0] = 0x07;
	}

	if (info.reset == RESET) {
		if (m176tmp.ds_used) {
			m176tmp.index = (m176tmp.index + 1) % m176tmp.max;
		}
	} else if (((info.reset == CHANGE_ROM) || (info.reset == POWER_UP))) {
		if (
			(info.crc32.prg == 0x81907A3B) || // (KY-9006) 9-in-1 Super Game.nes
			(info.crc32.prg == 0x26ABC25E) || // 9-in-1 - Pokemon Yellow (FK23C board)[p4][!].nes
			(info.crc32.prg == 0xCD028ED2) || // 6-in-1 (Multi)[Unknown][YH602].nes
			(info.crc32.prg == 0xD6F095DC) || // 8-in-1 (Multi)[Unknown][YH801].nes
			(info.crc32.prg == 0xCFAE9BF7)) { // 6-in-1 - Spider Man 2 (FK23C board)[p4][!].nes
			static WORD ds[] = { 0x10, 0x20 };

			tmp_fix_176(LENGTH(ds), 0, &ds[0]);
		} else if (
			(info.crc32.prg == 0xA3265AEE) || // Super Game 4-in-1 (6-in-1) (KT-3445AB_20210418) (Unl) [p1].nes
			(info.crc32.prg == 0x324E55C4) || // Super Games 4-in-1(KT-3445AB_20201201) (Unl) [p1].nes
			(info.crc32.prg == 0x0007E045)) { // Super Game 4-in-1 (6-in-1) (BS-8174) (Unl) [p1].nes
			static WORD ds[] = { 0x10, 0x20 };

			tmp_fix_176(LENGTH(ds), 1, &ds[0]);
		} else if (
			(info.crc32.prg == 0xC08E77C1) || // 5-in-1 (19, 66, 90, 93, 113, 133, 200-in-1) (KD-1512_20210408) (Unl) [p1].nes
			(info.crc32.prg == 0x613BBEE9) || // Super Game 15-in-1 (7, 80, 82, 102, 122, 160-in-1) (KD-6012) (Unl) [p1].nes
			(info.crc32.prg == 0x03CF81B4)) { // 16 in 1 (KD-1512).nes
			static WORD ds[] = { 0x400, 0x010, 0x020, 0x080, 0x100, 0x200, 0x040 };

			tmp_fix_176(LENGTH(ds), 0, &ds[0]);
		} else if (info.crc32.prg == 0x30FF6159) { // 15-in-1 (20, 25, 80, 99, 160, 210, 260-in-1) (BS-6008) (Unl) [p1].nes
			static WORD ds[] = { 0x010, 0x080, 0x400, 0x100, 0x020, 0x040, 0x200, 0x800 };

			tmp_fix_176(LENGTH(ds), 0, &ds[0]);
		} else if (info.crc32.prg == 0x2F3DB40D) { // 17-in-1 (76000, 1000000, 9999999-in-1) (KD-6038) (Unl) [p1].nes
			static WORD ds[] = { 0x010, 0x020, 0x080, 0x040 };

			tmp_fix_176(LENGTH(ds), 0, &ds[0]);
		} else if (
			(info.crc32.prg == 0xA9C3824E) || // Super Game 4-in-1 (8, 64, 128-in-1) (KT-8008) (Unl) [p1].nes
			(info.crc32.prg == 0x55BEBFCB) || // 44-in-1 (Super Game KT-A) [p1][!].nes
			(info.crc32.prg == 0x4D5C057A)) { // 19-in-1 (7600, 9999999, 999999999-in-1) (BS-8307) (Unl) [p1].nes
			static WORD ds[] = { 0x010, 0x020, 0x040, 0x080 };

			tmp_fix_176(LENGTH(ds), 0, &ds[0]);
		} else if (
			(info.crc32.prg == 0x8052AA1E) || // (FK-014) 128-in-1.nes
			(info.crc32.prg == 0xDF4E55F3)) { // 160-in-1.nes
			static WORD ds[] = { 0x080, 0x010, 0x400, 0x100, 0x020, 0x040, 0x200, 0x800 };

			tmp_fix_176(LENGTH(ds), 1, &ds[0]);
		} else if (
			(info.crc32.prg == 0x8ACCE289) || // (FK-022) 178-in-1.nes
			(info.crc32.prg == 0x7BF49FEE) || // (FK-021) 180-in-1.nes
			(info.crc32.prg == 0xD2A7F82B)) { // (BS-6028) 180-in-1.nes
			static WORD ds[] = { 0x080, 0x010, 0x400, 0x100, 0x020, 0x040, 0x200, 0x800 };

			tmp_fix_176(LENGTH(ds), 6, &ds[0]);
		} else if (info.crc32.prg == 0x5507A5A8) { // (FK-033) 52-in-1.nes
			static WORD ds[] = { 0x080, 0x010, 0x400, 0x100, 0x020, 0x040, 0x200, 0x800 };

			tmp_fix_176(LENGTH(ds), 3, &ds[0]);
		} else if (
			(info.crc32.prg == 0x67F8DFE4) || // Super Game 4-in-1 (7, 30, 65, 111, 9999999-in-1) (KB-0306N-2_20201107) (Unl) [p1].nes
			(info.crc32.prg == 0x48A64738) || // Super 4-in-1 (7, 60, 99, 111, 9999999-in-1) (KB-4016) (Unl) [p1].nes
			(info.crc32.prg == 0xFDF94F9E)) { // Super 4-in-1 (7, 25, 28, 111, 9999999-in-1) (KB-4009) (Unl) [p1].nes
			static WORD ds[] = { 0x010, 0x080, 0x400, 0x100, 0x020, 0x800, 0x040, 0x200 };

			tmp_fix_176(LENGTH(ds), 2, &ds[0]);
		} else if (info.crc32.prg == 0x41252709) { // Super Game 16-in-1 (19, 31, 51, 56, 112, 121, 126-in-1) (BS-6002) (Unl) [p1].nes
			static WORD ds[] = { 0x010, 0x080, 0x400, 0x100, 0x020, 0x800, 0x200, 0x040 };

			tmp_fix_176(LENGTH(ds), 0, &ds[0]);
		} else if (info.crc32.prg == 0xB2BC6FF8) { // Super Game 16-in-1 (500, 9999999, 999999999-in-1) (FK037) (Unl) [p1].nes
			static WORD ds[] = { 0x010, 0x100, 0x020, 0x200, 0x040, 0x400, 0x080, 0x800 };

			tmp_fix_176(LENGTH(ds), 0, &ds[0]);
		} else if (info.crc32.prg == 0x8F1D2425) { // Super Game 20-in-1 (6, 16, 36, 56, 99, 210-in-1) (KD-6026) (Unl) [p1].nes
			static WORD ds[] = { 0x400, 0x080, 0x010, 0x100, 0x200, 0x020, 0x040 };

			tmp_fix_176(LENGTH(ds), 0, &ds[0]);
		} else if (info.crc32.prg == 0xDDA1E214) { // Super Game 28-in-1 (15, 30, 36, 52, 160, 180, 255-in-1) (BS-6017) (Unl) [p1].nes
			static WORD ds[] = { 0x080, 0x010, 0x400, 0x020, 0x100, 0x040, 0x200, 0x800 };

			tmp_fix_176(LENGTH(ds), 2, &ds[0]);
		}
	}

	// Super Rockman 6-in-1 (861234C Hack).unif
	// Super Rockman 4 ha bisogno di almeno 8k di wram per funzionare correttamente
	//wram_set_ram_size(32 * 1024);

	info.mapper.extend_wr = TRUE;

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_after_mapper_init_176(void) {
	prg_fix_176();
	chr_fix_176();
	wram_fix_176();
	mirroring_fix_176();
}
void extcl_cpu_wr_mem_176(WORD address, BYTE value) {
	if (address < 0x8000) {
		const WORD bank = address & 0xF000;

		switch (bank) {
			case 0x4000:
			case 0x5000:
				if ((!m176.ram_cfg_reg || m176.ram_out_reg) && (bank == 0x5000)) {
					WORD mask = 0x5000 | (m176tmp.ds_used ? m176tmp.dipswitch[m176tmp.index] : 0);

					if ((address & mask) != mask) {
						return;
					}

					switch (address & (info.mapper.submapper == JX9003B ? 0x07 : 0x03)) {
						case 0:
							m176.cpu5xxx[0] = value;
							if (info.mapper.submapper == FS005) {
								m176.prg = (m176.prg & ~0x0180) | ((value & 0x80) << 1) | ((value & 0x08) << 4);
							}
							break;
						case 1:
							m176.cpu5xxx[1] = value;
							if (info.mapper.submapper == HST162) {
								m176.prg = (m176.prg & ~0x1F) | (value & 0x1F);
							} else {
								m176.prg = (m176.prg & ~0x7F) | (value & 0x7F);
							}
							break;
						case 2:
							m176.cpu5xxx[2] = value;
							m176.cnrom_chr_reg = 0;
							if (info.mapper.submapper == FS005) {
								m176.prg = (m176.prg & ~0x0E00) | ((value & 0xC0) << 3) | ((value & 0x20) << 6);
							}
							break;
						case 3:
							m176.cpu5xxx[3] = value;
							break;
						case 5:
							m176.cpu5xxx[5] = value;
							if (info.mapper.submapper == JX9003B) {
								m176.prg = (m176.prg & 0x007F) | ((value & 0x0F) << 7);
							}
							break;
						case 6:
							m176.cpu5xxx[6] = value;
							break;
					}
					extcl_after_mapper_init_176();
					return;
				} else if ((address == 0x4800) && (info.mapper.submapper == HST162)) {
					m176.cpu4800 = value;
					m176.prg = (m176.prg & 0x001F) | ((value & 0x3F) << 5);
					prg_fix_176();
				}
				return;
			default:
				return;
		}
	} else {
		if (m176.cpu5xxx[0] & 0x40) {
			m176.cnrom_chr_reg = value & 0x03;
			chr_fix_176();
		}
		switch (address & 0xE003) {
			case 0x8000:
			case 0x8002:
				// Subtype 2, 16384 KiB PRG-ROM, no CHR-ROM: Like Subtype 0, but MMC3 registers $46 and $47 swapped.
				if ((info.mapper.submapper == FS005) && ((value == 0x46) || (value == 0x47))) {
					value ^= 1;
				}
				m176.cpu8xxx[0] = value;
				extcl_after_mapper_init_176();
				break;
				// Some multicart games depend on writes to $9FFF not doing anything as
				// a result of the MMC3 address mask being $E003 rather than the standard $E001.
				//case 0x8003:
			case 0x8001: {
				BYTE reg = (m176.cpu5xxx[3] & 0x02) && (m176.cpu8xxx[0] & 0x08) ?
					(0x08 | (m176.cpu8xxx[0] & 0x03)) : m176.cpu8xxx[0] & 0x07;

				m176.cpu8xxx[1] = value;
				m176.mmc3[reg] = value;
				extcl_after_mapper_init_176();
				break;
			}
			case 0xA000:
			case 0xA002:
				m176.cpu8xxx[2] = value;
				extcl_after_mapper_init_176();
				break;
			case 0xA001:
			case 0xA003:
				if (!(value & 0x20)) {
					value &= 0xC0;
				}
				m176.cpu8xxx[3] = value;
				m176.ram_cfg_reg = (value & 0x20) && (info.mapper.submapper == FS005);
				m176.ram_out_reg = value & 0x40;
				m176.ram_enabled = value & 0x80;
				extcl_after_mapper_init_176();
				break;
			case 0xC000:
			case 0xC001:
			case 0xC002:
			case 0xC003:
			case 0xE000:
			case 0xE001:
			case 0xE002:
			case 0xE003:
				extcl_cpu_wr_mem_MMC3(address, value);
				break;
		}
	}
}
BYTE extcl_save_mapper_176(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m176.cpu4800);
	save_slot_ele(mode, slot, m176.cpu5xxx);
	save_slot_ele(mode, slot, m176.cpu8xxx);
	save_slot_ele(mode, slot, m176.cnrom_chr_reg);
	save_slot_ele(mode, slot, m176.ram_cfg_reg);
	save_slot_ele(mode, slot, m176.ram_out_reg);
	save_slot_ele(mode, slot, m176.ram_enabled);
	save_slot_ele(mode, slot, m176.mmc3);
	save_slot_ele(mode, slot, m176.prg);

	if (mode == SAVE_SLOT_READ) {
		extcl_after_mapper_init_176();
	}

	return (EXIT_OK);
}
void extcl_wr_chr_176(WORD address, BYTE value) {
	const BYTE slot = address >> 10;

	if (map_chr_ram_slot_in_range(slot)) {
		chr.bank_1k[slot][address & 0x3FF] = value;
	}
}

INLINE static void prg_fix_176(void) {
	switch (m176.cpu5xxx[0] & 0x07) {
		case 0:
		case 1:
		case 2:
		case 6:
		case 7: {
			//WORD swap = (m176.cpu8xxx[0] & 0x40) >> 5;
			WORD swap = (m176.cpu8xxx[0] & 0x40) << 8;
			WORD outer = (m176.prg << 1);
			WORD bank[4];

			if (m176.cpu5xxx[3] & 0x02) {
				bank[0] = outer | m176.mmc3[6];
				bank[1] = outer | m176.mmc3[7];
				bank[2] = outer | m176.mmc3[8];
				bank[3] = outer | m176.mmc3[9];
			} else {
				const WORD prg_mask[8] = {
					0x3F, 0x1F, 0x0F, 0x07,
					0x03, 0x01, 0x7F, 0xFF
				};
				BYTE mask = prg_mask[m176.cpu5xxx[0] & 0x07];

				outer &= ~mask;

				bank[0] = outer | (m176.mmc3[6] & mask);
				bank[1] = outer | (m176.mmc3[7] & mask);
				bank[2] = outer | (0xFE & mask);
				bank[3] = outer | (0xFF & mask);
			}

			memmap_auto_8k(0x8000 ^ swap, bank[0]);
			memmap_auto_8k(0xA000, bank[1]);
			memmap_auto_8k(0xC000 ^ swap, bank[2]);
			memmap_auto_8k(0xE000, bank[3]);
			return;
		}
		case 3:
			memmap_auto_16k(0x8000, m176.prg);
			memmap_auto_16k(0xC000, m176.prg);
			return;
		case 4:
			memmap_auto_32k(0x8000, (m176.prg & 0x0FFF) >> 1);
			return;
		default:
			return;
	}
}
INLINE static void chr_fix_176(void) {
	WORD outer = 0, bank[8];
	BYTE swap = 0;

	if (m176.cpu5xxx[0] & 0x40) {
		WORD mask = (info.mapper.submapper != LP8002KB) && !(m176.cpu5xxx[0] & 0x20) ?
			(m176.cpu5xxx[0] & 0x10 ? 1 : 3) : 0;

		outer = ((m176.cnrom_chr_reg & mask) | m176.cpu5xxx[2]) << 3;

		if (info.mapper.submapper == JX9003B) {
			outer |= ((m176.cpu5xxx[6] & 0x0F) << 11);
		}

		bank[0] = outer | 0;
		bank[1] = outer | 1;
		bank[2] = outer | 2;
		bank[3] = outer | 3;
		bank[4] = outer | 4;
		bank[5] = outer | 5;
		bank[6] = outer | 6;
		bank[7] = outer | 7;
	} else {
		outer = m176.cpu5xxx[2] << 3;
		swap = (m176.cpu8xxx[0] & 0x80) >> 5;

		if (info.mapper.submapper == JX9003B) {
			outer |= ((m176.cpu5xxx[6] & 0x0F) << 11);
		}

		if (m176.cpu5xxx[3] & 0x02) {
			bank[0] = outer | m176.mmc3[0];
			bank[1] = outer | m176.mmc3[10];
			bank[2] = outer | m176.mmc3[1];
			bank[3] = outer | m176.mmc3[11];
			bank[4] = outer | m176.mmc3[2];
			bank[5] = outer | m176.mmc3[3];
			bank[6] = outer | m176.mmc3[4];
			bank[7] = outer | m176.mmc3[5];
		} else {
			BYTE mask = (m176.cpu5xxx[0] & 0x10 ? 0x7F : 0xFF);

			outer &= ~mask;

			bank[0] = outer | ((m176.mmc3[0] & 0xFE) & mask);
			bank[1] = outer | ((m176.mmc3[0] | 0x01) & mask);
			bank[2] = outer | ((m176.mmc3[1] & 0xFE) & mask);
			bank[3] = outer | ((m176.mmc3[1] | 0x01) & mask);
			bank[4] = outer | (m176.mmc3[2] & mask);
			bank[5] = outer | (m176.mmc3[3] & mask);
			bank[6] = outer | (m176.mmc3[4] & mask);
			bank[7] = outer | (m176.mmc3[5] & mask);
		}
	}
	chr_swap_176(0 ^ swap, bank[0]);
	chr_swap_176(1 ^ swap, bank[1]);
	chr_swap_176(2 ^ swap, bank[2]);
	chr_swap_176(3 ^ swap, bank[3]);
	chr_swap_176(4 ^ swap, bank[4]);
	chr_swap_176(5 ^ swap, bank[5]);
	chr_swap_176(6 ^ swap, bank[6]);
	chr_swap_176(7 ^ swap, bank[7]);
}
INLINE static void chr_swap_176(BYTE slot, WORD value) {
	if (chr_ram_176(value)) {
		control_bank(info.chr.ram.max.banks_1k)
		chr.bank_1k[slot] = &chr.extra.data[value << 10];
	} else {
		control_bank(info.chr.rom.max.banks_1k)
		chr.bank_1k[slot] = chr_pnt(value << 10);
	}
}
INLINE static BYTE chr_ram_176(WORD value) {
	return (chr.extra.data && ((m176.cpu5xxx[0] & 0x20) ||
		(m176.ram_cfg_reg && (m176.cpu8xxx[3] & 0x04) && (value <= 7))));
}
INLINE static void wram_fix_176(void) {
	const WORD bank = (m176.cpu8xxx[3] & 0x03);

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
			memmap_auto_8k(0x4000, bank + 1);
			memmap_auto_8k(0x6000, bank);
		} else {
			// MMC3 mode
			memmap_disable_4k(0x5000);
			memmap_auto_wp_8k(0x6000, 0, m176.ram_enabled >> 7, !m176.ram_out_reg);
		}
	} else {
		memmap_disable_16k(0x4000);
	}
}
INLINE static void mirroring_fix_176(void) {
	// Single-screen mirroring is only available when the RAM Configuration Register is enabled ($A001.5).
	switch ((m176.cpu8xxx[2] & 0x03) & (m176.ram_cfg_reg ? 0x03 : 0x01)) {
		case 0:
			mirroring_V();
			break;
		case 1:
			mirroring_H();
			break;
		case 2:
			mirroring_SCR0();
			break;
		case 3:
			mirroring_SCR1();
			break;
	}
}
INLINE static void tmp_fix_176(BYTE max, BYTE index, const WORD *ds) {
	m176tmp.ds_used = TRUE;
	m176tmp.max = max;
	m176tmp.index = index;
	m176tmp.dipswitch = ds;
}

