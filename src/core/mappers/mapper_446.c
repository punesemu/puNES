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
#include "mem_map.h"
#include "cpu.h"
#include "save_slot.h"

INLINE static void prg_fix_446(void);
INLINE static void chr_fix_446(void);
INLINE static void mirroring_fix_446(void);

struct _m446 {
	BYTE reg[8];
} m446;

void map_init_446(void) {
	EXTCL_AFTER_MAPPER_INIT(446);
	EXTCL_CPU_WR_MEM(446);
	EXTCL_SAVE_MAPPER(446);
	EXTCL_WR_CHR(446);
	mapper.internal_struct[0] = (BYTE *)&m446;
	mapper.internal_struct_size[0] = sizeof(m446);
	mapper.internal_struct[1] = (BYTE *)&mmc1;
	mapper.internal_struct_size[1] = sizeof(mmc1);

	if (info.reset >= HARD) {
		memset(&m446, 0x00, sizeof(m446));
	}

	memset(&mmc1, 0x00, sizeof(mmc1));

	mmc1.ctrl = 0x0C;
	mmc1.prg_mode = 3;
	mmc1.prg_mask = 0x0F;
	mmc1.chr1 = 1;

	info.mapper.extend_wr = TRUE;
}
void extcl_after_mapper_init_446(void) {
	prg_fix_446();
	chr_fix_446();
	mirroring_fix_446();
}
void extcl_cpu_wr_mem_446(WORD address, BYTE value) {
	switch (address & 0xF000) {
		case 0x5000:
			if (!(m446.reg[0] & 0x80)) {
				m446.reg[address & 0x07] = value;
				prg_fix_446();
				chr_fix_446();
				mirroring_fix_446();
			}
			break;
		case 0x6000:
		case 0x7000:
			break;
		case 0x8000:
		case 0x9000:
		case 0xA000:
		case 0xB000:
		case 0xC000:
		case 0xD000:
		case 0xE000:
		case 0xF000:
			if ((m446.reg[0] & 0x80) && (m446.reg[4] == 0x04)) {
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
							mirroring_fix_446();
							prg_fix_446();
							chr_fix_446();
							break;
						case 1:
							mmc1.chr0 = mmc1.reg;
							chr_fix_446();
							break;
						case 2:
							mmc1.chr1 = mmc1.reg;
							chr_fix_446();
							break;
						case 3:
							mmc1.prg0 = mmc1.reg;
							cpu.prg_ram_rd_active = (mmc1.prg0 & 0x10 ? FALSE : TRUE);
							cpu.prg_ram_wr_active = cpu.prg_ram_rd_active;
							prg_fix_446();
							break;
					}
					mmc1.pos = mmc1.reg = 0;
				}
			}
			break;
		default:
			break;
	}
}
BYTE extcl_save_mapper_446(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m446.reg);
	extcl_save_mapper_MMC1(mode, slot, fp);

	return (EXIT_OK);
}
void extcl_wr_chr_446(WORD address, BYTE value) {
	if (!(m446.reg[5] & 0x04)) {
		chr.bank_1k[address >> 10][address & 0x3FF] = value;
	}
}

INLINE static void prg_fix_446(void) {
	WORD base = (m446.reg[2] << 8) | m446.reg[1];
	WORD bank;

	if (m446.reg[0] & 0x80) {
		WORD mask = ~m446.reg[3];

		if (m446.reg[4] == 0x00) {
			bank = base | (0x00 & mask);
			_control_bank(bank, info.prg.rom.max.banks_8k)
			map_prg_rom_8k(1, 0, bank);

			bank = base | (0x01 & mask);
			_control_bank(bank, info.prg.rom.max.banks_8k)
			map_prg_rom_8k(1, 1, bank);

			bank = base | (0x02 & mask);
			_control_bank(bank, info.prg.rom.max.banks_8k)
			map_prg_rom_8k(1, 2, bank);

			bank = base | (0x03 & mask);
			_control_bank(bank, info.prg.rom.max.banks_8k)
			map_prg_rom_8k(1, 3, bank);
		} else if (m446.reg[4] == 0x04) {
			base >>= 1;
			mask >>= 1;

			switch (mmc1.prg_mode) {
				case 0:
				case 1:
					bank = (base | (mmc1.prg0 & mask)) >> 1;
					_control_bank(bank, info.prg.rom.max.banks_32k)
					map_prg_rom_8k(4, 0, bank);
					break;
				case 2:
					bank = base | (mmc1.prg0 & mask);
					_control_bank(bank, info.prg.rom.max.banks_16k)
					map_prg_rom_8k(2, 2, bank);

					bank = base & ~mask;
					_control_bank(bank, info.prg.rom.max.banks_16k)
					map_prg_rom_8k(2, 0, bank);
					break;
				case 3:
					bank = base | (mmc1.prg0 & mask);
					_control_bank(bank, info.prg.rom.max.banks_16k)
					map_prg_rom_8k(2, 0, bank);

					bank = base | mask;
					_control_bank(bank, info.prg.rom.max.banks_16k)
					map_prg_rom_8k(2, 2, bank);
					break;
				default:
					break;
			}
		}
	} else {
		bank = base;
		_control_bank(bank, info.prg.rom.max.banks_8k)
		map_prg_rom_8k(1, 0, bank);

		bank = 0x3D;
		_control_bank(bank, info.prg.rom.max.banks_8k)
		map_prg_rom_8k(1, 1, bank);

		bank = 0x3E;
		_control_bank(bank, info.prg.rom.max.banks_8k)
		map_prg_rom_8k(1, 2, bank);

		bank = 0x3F;
		_control_bank(bank, info.prg.rom.max.banks_8k)
		map_prg_rom_8k(1, 3, bank);
	}
	map_prg_rom_8k_update();
}
INLINE static void chr_fix_446(void) {
	DBWORD bank = 0;

	if ((m446.reg[0] & 0x80) && (m446.reg[4] == 0x04)) {
		WORD base = m446.reg[6] >> 2;
		WORD mask = 0x1F;

		bank = base | (mmc1.chr0 & mask);
		_control_bank(bank, info.chr.rom.max.banks_4k)
		bank <<= 12;
		chr.bank_1k[0] = chr_pnt(bank);
		chr.bank_1k[1] = chr_pnt(bank | 0x0400);
		chr.bank_1k[2] = chr_pnt(bank | 0x0800);
		chr.bank_1k[3] = chr_pnt(bank | 0x0C00);

		bank = base | (mmc1.chr1 & mask);
		_control_bank(bank, info.chr.rom.max.banks_4k)
		bank <<= 12;
		chr.bank_1k[4] = chr_pnt(bank);
		chr.bank_1k[5] = chr_pnt(bank | 0x0400);
		chr.bank_1k[6] = chr_pnt(bank | 0x0800);
		chr.bank_1k[7] = chr_pnt(bank | 0x0C00);

		return;
	}

	bank = m446.reg[6];
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
}
INLINE static void mirroring_fix_446(void) {
	if ((m446.reg[0] & 0x80) && (m446.reg[4] == 0x04)) {
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
		return;
	}
	if (m446.reg[0] & 0x01) {
		mirroring_H();
	} else {
		mirroring_V();
	}
}
