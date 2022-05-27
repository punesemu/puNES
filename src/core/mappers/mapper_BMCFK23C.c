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

INLINE static void state_fix_BMCFK23C(void);
INLINE static void prg_fix_BMCFK23C(void);
INLINE static void prg_ram_fix_BMCFK23C(void);
INLINE static void chr_fix_BMCFK23C(void);
INLINE static void chr_swap_BMCFK23C(BYTE slot, WORD value);
INLINE static BYTE chr_ram_BMCFK23C(WORD value);
INLINE static void mirroring_fix_BMCFK23C(void);

struct _bmcfk23c {
	BYTE cpu5xxx[4];
	BYTE cpu8xxx[4];
	BYTE cnrom_chr_reg;
	BYTE ram_register_enable;
	BYTE ram_outer_register_enable;
	BYTE mmc3[12];
	WORD prg_base;

	// da non salvare
	BYTE *prg_4000;
	BYTE *prg_6000;
} bmcfk23c;

static const BYTE prg_mask[8] = { 0x3F, 0x1F, 0x0F, 0x07, 0x03, 0x01, 0x00, 0x00 };

void map_init_BMCFK23C(void) {
	EXTCL_AFTER_MAPPER_INIT(BMCFK23C);
	EXTCL_CPU_WR_MEM(BMCFK23C);
	EXTCL_CPU_RD_MEM(BMCFK23C);
	EXTCL_SAVE_MAPPER(BMCFK23C);
	EXTCL_WR_CHR(BMCFK23C);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *)&bmcfk23c;
	mapper.internal_struct_size[0] = sizeof(bmcfk23c);

	memset(&bmcfk23c, 0x00, sizeof(bmcfk23c));
	memset(&irqA12, 0x00, sizeof(irqA12));

	bmcfk23c.mmc3[0] = 0;
	bmcfk23c.mmc3[1] = 2;
	bmcfk23c.mmc3[2] = 4;
	bmcfk23c.mmc3[3] = 5;
	bmcfk23c.mmc3[4] = 6;
	bmcfk23c.mmc3[5] = 7;
	bmcfk23c.mmc3[6] = 0;
	bmcfk23c.mmc3[7] = 1;
	bmcfk23c.mmc3[8] = 0xFE;
	bmcfk23c.mmc3[9] = 0xFF;
	bmcfk23c.mmc3[10] = 0xFF;
	bmcfk23c.mmc3[11] = 0xFF;

	if (info.format == NES_2_0) {
		if (info.chr.ram.banks_8k_plus > 0) {
			map_chr_ram_extra_init(info.chr.ram.banks_8k_plus * 0x2000);
		}
	} else {
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
				map_chr_ram_extra_init(0x2000);
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
		bmcfk23c.prg_base = (info.crc32.prg == 0x6C574B50) ? 0x40 : 0x20;
		info.mapper.submapper = BMCFK23C;
	}
	if (prg_size() >= (8192 * 1024)) {
		info.mapper.submapper = FS005;
	}
	if ((info.crc32.prg == 0x3655B7BC) || // New 4-in-1 Supergame (YH4239) [p1][U][!].unf
		(info.crc32.prg == 0x60C6D8CD) || // 9-in-1 (KY-9005) [p1][U][!].unf
		(info.crc32.prg == 0x63A87C95)) { // 8-in-1 Supergame (KY8002) [p1][U].unf
		info.mapper.submapper = LP8002KB;
	}

	info.prg.ram.banks_8k_plus = 4;

	info.mapper.extend_wr = info.mapper.extend_rd = TRUE;

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_after_mapper_init_BMCFK23C(void) {
	state_fix_BMCFK23C();
}
void extcl_cpu_wr_mem_BMCFK23C(WORD address, BYTE value) {
	if (address < 0x8000) {
		if (((address >= 0x5000) && (address <= 0x5FFF)) &&
			(!bmcfk23c.ram_register_enable || bmcfk23c.ram_outer_register_enable)) {
			WORD mask = 0x5010;

			if ((address & mask) != mask) {
				return;
			}
			switch (address & 0x03) {
				case 0:
					bmcfk23c.cpu5xxx[0] = value;
					if (info.mapper.submapper == FS005) {
						bmcfk23c.prg_base = (bmcfk23c.prg_base & ~0x0180) | ((value & 0x80) << 1) | ((value & 0x08) << 4);
					}
					break;
				case 1:
					bmcfk23c.cpu5xxx[1] = value;
					bmcfk23c.prg_base = (bmcfk23c.prg_base & ~0x7F) | (value & 0x7F);
					break;
				case 2:
					bmcfk23c.cpu5xxx[2] = value;
					bmcfk23c.cnrom_chr_reg = 0;
					if (info.mapper.submapper == FS005) {
						bmcfk23c.prg_base = (bmcfk23c.prg_base & ~0x0E00) | ((value & 0xC0) << 3) | ((value & 0x20) << 6);
					}
					break;
				case 3:
					bmcfk23c.cpu5xxx[3] = value;
					break;
			}
			state_fix_BMCFK23C();
		} else {
			if ((address >= 0x4000) && (address <= 0x5FFF)) {
				if (bmcfk23c.ram_register_enable && cpu.prg_ram_wr_active) {
					bmcfk23c.prg_4000[address & 0x1FFF] = value;
				}
			} else if ((address >= 0x6000) && (address <= 0x7FFF)) {
				if (bmcfk23c.ram_register_enable && cpu.prg_ram_wr_active) {
					bmcfk23c.prg_6000[address & 0x1FFF] = value;
				}
			}
			return;
		}
	} else {
		if (bmcfk23c.cpu5xxx[0] & 0x40) {
			bmcfk23c.cnrom_chr_reg = value & 0x03;
			chr_fix_BMCFK23C();
		}
		switch (address & 0xE003) {
			case 0x8000:
			case 0x8002:
				// Subtype 2, 16384 KiB PRG-ROM, no CHR-ROM: Like Subtype 0, but MMC3 registers $46 and $47 swapped.
				if ((info.mapper.submapper == FS005) && ((value == 0x46) || (value == 0x47))) {
					value ^= 1;
				}
				bmcfk23c.cpu8xxx[0] = value;
				state_fix_BMCFK23C();
				break;
			// Some multicart games depend on writes to $9FFF not doing anything as
			// a result of the MMC3 address mask being $E003 rather than the standard $E001.
			//case 0x8003:
			case 0x8001: {
				BYTE reg = (bmcfk23c.cpu5xxx[3] & 0x02) && (bmcfk23c.cpu8xxx[0] & 0x08) ?
					(0x08 | (bmcfk23c.cpu8xxx[0] & 0x03)) : bmcfk23c.cpu8xxx[0] & 0x07;

				bmcfk23c.cpu8xxx[1] = value;
				bmcfk23c.mmc3[reg] = value;
				state_fix_BMCFK23C();
				break;
			}
			case 0xA000:
			case 0xA002:
				bmcfk23c.cpu8xxx[2] = value;
				state_fix_BMCFK23C();
				break;
			case 0xA001:
			case 0xA003:
				if (!(value & 0x20)) {
					value &= 0xC0;
				}
				bmcfk23c.cpu8xxx[3] = value;
				bmcfk23c.ram_register_enable = value & 0x20;
				bmcfk23c.ram_outer_register_enable = value & 0x40;

				switch (value & 0x80) {
					case 0x00:
						cpu.prg_ram_rd_active = cpu.prg_ram_wr_active = FALSE;
						break;
					case 0x80:
						cpu.prg_ram_rd_active = cpu.prg_ram_wr_active = TRUE;
						break;
				}
				state_fix_BMCFK23C();
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
BYTE extcl_cpu_rd_mem_BMCFK23C(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	if (address < 0x8000) {
		if (!bmcfk23c.ram_register_enable ||
			(bmcfk23c.ram_outer_register_enable && ((address >= 0x5000) && (address <= 0x5FFF))))  {
			return (openbus);
		}
		if ((address >= 0x4000) && (address <= 0x5FFF)) {
			return (bmcfk23c.ram_register_enable && cpu.prg_ram_rd_active ? bmcfk23c.prg_4000[address & 0x1FFF] : openbus);
		} else if ((address >= 0x6000) && (address <= 0x7FFF)) {
			return (bmcfk23c.ram_register_enable && cpu.prg_ram_rd_active ? bmcfk23c.prg_6000[address & 0x1FFF] : openbus);
		}
	}
	return (openbus);
}
BYTE extcl_save_mapper_BMCFK23C(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, bmcfk23c.cpu5xxx);
	save_slot_ele(mode, slot, bmcfk23c.cpu8xxx);
	save_slot_ele(mode, slot, bmcfk23c.cnrom_chr_reg);
	save_slot_ele(mode, slot, bmcfk23c.ram_register_enable);
	save_slot_ele(mode, slot, bmcfk23c.ram_outer_register_enable);
	save_slot_ele(mode, slot, bmcfk23c.mmc3);
	save_slot_ele(mode, slot, bmcfk23c.prg_base);
	save_slot_mem(mode, slot, chr.extra.data, chr.extra.size, FALSE);

	if (mode == SAVE_SLOT_READ) {
		state_fix_BMCFK23C();
	}

	return (EXIT_OK);
}
void extcl_wr_chr_BMCFK23C(WORD address, BYTE value) {
	BYTE slot = address >> 10;

	if (info.chr.rom.is_ram ||
		((chr.bank_1k[slot] >= chr.extra.data) && (chr.bank_1k[slot] < (chr.extra.data + chr.extra.size)))) {
		chr.bank_1k[slot][address & 0x3FF] = value;
	}
}

INLINE static void state_fix_BMCFK23C(void) {
	prg_fix_BMCFK23C();
	chr_fix_BMCFK23C();
	prg_ram_fix_BMCFK23C();
	mirroring_fix_BMCFK23C();
}
INLINE static void prg_fix_BMCFK23C(void) {
	WORD bank[4];

	switch (bmcfk23c.cpu5xxx[0] & 0x07) {
		case 0:
		case 1:
		case 2: {
			BYTE swap = (bmcfk23c.cpu8xxx[0] & 0x40) >> 5;
			WORD outer = (bmcfk23c.prg_base << 1);

			if (bmcfk23c.cpu5xxx[3] & 0x02) {
				bank[0] = outer | bmcfk23c.mmc3[6];
				bank[1] = outer | bmcfk23c.mmc3[7];
				bank[2] = outer | bmcfk23c.mmc3[8];
				bank[3] = outer | bmcfk23c.mmc3[9];
			} else {
				BYTE mask = prg_mask[bmcfk23c.cpu5xxx[0] & 0x07];

				outer &= ~mask;

				bank[0] = outer | (bmcfk23c.mmc3[6] & mask);
				bank[1] = outer | (bmcfk23c.mmc3[7] & mask);
				bank[2] = outer | (0xFE & mask);
				bank[3] = outer | (0xFF & mask);
			}

			_control_bank(bank[0], info.prg.rom.max.banks_8k)
			map_prg_rom_8k(1, 0 ^ swap, bank[0]);

			_control_bank(bank[1], info.prg.rom.max.banks_8k)
			map_prg_rom_8k(1, 1, bank[1]);

			_control_bank(bank[2], info.prg.rom.max.banks_8k)
			map_prg_rom_8k(1, 2 ^ swap, bank[2]);

			_control_bank(bank[3], info.prg.rom.max.banks_8k)
			map_prg_rom_8k(1, 3, bank[3]);

			map_prg_rom_8k_update();
			break;
		}
		case 3:
			bank[0] = bmcfk23c.prg_base;
			_control_bank(bank[0], info.prg.rom.max.banks_16k)
			map_prg_rom_8k(2, 0, bank[0]);
			map_prg_rom_8k(2, 2, bank[0]);
			map_prg_rom_8k_update();
			break;
		case 4:
			bank[0] = (bmcfk23c.prg_base & 0x0FFF) >> 1;
			_control_bank(bank[0], info.prg.rom.max.banks_32k)
			map_prg_rom_8k(4, 0, bank[0]);
			map_prg_rom_8k_update();
			break;
		default:
			break;
	}
}
INLINE static void prg_ram_fix_BMCFK23C(void) {
	WORD bank = (bmcfk23c.cpu8xxx[3] & 0x03);

	bmcfk23c.prg_4000 = bmcfk23c.ram_register_enable ? prg.ram_plus_8k + (((bank + 1) & 0x03) << 13) : NULL;
	bmcfk23c.prg_6000 = prg.ram_plus_8k + (bmcfk23c.ram_register_enable ? (bank << 13) : 0);
}
INLINE static void chr_fix_BMCFK23C(void) {
	WORD outer = 0, bank[8];
	BYTE swap = 0;

	if (bmcfk23c.cpu5xxx[0] & 0x40) {
		WORD mask = (info.mapper.submapper != LP8002KB) && !(bmcfk23c.cpu5xxx[0] & 0x20) ?
			(bmcfk23c.cpu5xxx[0] & 0x10 ? 1 : 3) : 0;

		outer = ((bmcfk23c.cnrom_chr_reg & mask) | bmcfk23c.cpu5xxx[2]) << 3;

		bank[0] = outer | 0;
		bank[1] = outer | 1;
		bank[2] = outer | 2;
		bank[3] = outer | 3;
		bank[4] = outer | 4;
		bank[5] = outer | 5;
		bank[6] = outer | 6;
		bank[7] = outer | 7;
	} else {
		outer = bmcfk23c.cpu5xxx[2] << 3;
		swap = (bmcfk23c.cpu8xxx[0] & 0x80) >> 5;

		if (bmcfk23c.cpu5xxx[3] & 0x02) {
			bank[0] = outer | bmcfk23c.mmc3[0];
			bank[1] = outer | bmcfk23c.mmc3[10];
			bank[2] = outer | bmcfk23c.mmc3[1];
			bank[3] = outer | bmcfk23c.mmc3[11];
			bank[4] = outer | bmcfk23c.mmc3[2];
			bank[5] = outer | bmcfk23c.mmc3[3];
			bank[6] = outer | bmcfk23c.mmc3[4];
			bank[7] = outer | bmcfk23c.mmc3[5];
		} else {
			BYTE mask = (bmcfk23c.cpu5xxx[0] & 0x10 ? 0x7F : 0xFF);

			outer &= ~mask;

			bank[0] = outer | ((bmcfk23c.mmc3[0] & 0xFE) & mask);
			bank[1] = outer | ((bmcfk23c.mmc3[0] | 0x01) & mask);
			bank[2] = outer | ((bmcfk23c.mmc3[1] & 0xFE) & mask);
			bank[3] = outer | ((bmcfk23c.mmc3[1] | 0x01) & mask);
			bank[4] = outer | (bmcfk23c.mmc3[2] & mask);
			bank[5] = outer | (bmcfk23c.mmc3[3] & mask);
			bank[6] = outer | (bmcfk23c.mmc3[4] & mask);
			bank[7] = outer | (bmcfk23c.mmc3[5] & mask);
		}
	}
	chr_swap_BMCFK23C(0 ^ swap, bank[0]);
	chr_swap_BMCFK23C(1 ^ swap, bank[1]);
	chr_swap_BMCFK23C(2 ^ swap, bank[2]);
	chr_swap_BMCFK23C(3 ^ swap, bank[3]);
	chr_swap_BMCFK23C(4 ^ swap, bank[4]);
	chr_swap_BMCFK23C(5 ^ swap, bank[5]);
	chr_swap_BMCFK23C(6 ^ swap, bank[6]);
	chr_swap_BMCFK23C(7 ^ swap, bank[7]);
}
INLINE static void chr_swap_BMCFK23C(BYTE slot, WORD value) {
	if (chr_ram_BMCFK23C(value)) {
		control_bank(info.chr.ram.max.banks_1k)
		chr.bank_1k[slot] = &chr.extra.data[value << 10];
	} else {
		control_bank(info.chr.rom.max.banks_1k)
		chr.bank_1k[slot] = chr_pnt(value << 10);
	}
}
INLINE static BYTE chr_ram_BMCFK23C(WORD value) {
	return (chr.extra.data && ((bmcfk23c.cpu5xxx[0] & 0x20) ||
		(bmcfk23c.ram_register_enable && (bmcfk23c.cpu8xxx[3] & 0x04) && (value <= 7))));
}
INLINE static void mirroring_fix_BMCFK23C(void) {
	// Single-screen mirroring is only available when the RAM Configuration Register is enabled ($A001.5).
	switch ((bmcfk23c.cpu8xxx[2] & 0x03) & ((bmcfk23c.cpu8xxx[3] & 0x20) ? 0x03 : 0x01)) {
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
