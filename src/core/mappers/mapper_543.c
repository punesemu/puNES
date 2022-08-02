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
#include "mem_map.h"
#include "cpu.h"
#include "save_slot.h"

INLINE static void prg_fix_543(void);
INLINE static void prg_ram_fix_543(void);
INLINE static void chr_fix_543(void);
INLINE static void mirroring_fix_543(void);

struct _m543 {
	BYTE pos;
	BYTE reg;
} m543;

void map_init_543(void) {
	EXTCL_AFTER_MAPPER_INIT(543);
	EXTCL_CPU_WR_MEM(543);
	EXTCL_SAVE_MAPPER(543);
	mapper.internal_struct[0] = (BYTE *)&m543;
	mapper.internal_struct_size[0] = sizeof(m543);
	mapper.internal_struct[1] = (BYTE *)&mmc1;
	mapper.internal_struct_size[1] = sizeof(mmc1);

	memset(&m543, 0x00, sizeof(m543));
	memset(&mmc1, 0x00, sizeof(mmc1));

	mmc1.ctrl = 0x0C;
	mmc1.prg_mode = 3;
	mmc1.prg_mask = 0x0F;
	mmc1.prg_upper = 0;
	mmc1.chr_upper = 0;

	mmc1.prg0 = 0x0E;
	mmc1.chr1 = 1;

	if (info.prg.ram.banks_8k_plus != 8) {
		info.prg.ram.banks_8k_plus = 8;
	}

	info.mapper.extend_wr = TRUE;
}
void extcl_after_mapper_init_543(void) {
	prg_fix_543();
	prg_ram_fix_543();
	chr_fix_543();
	mirroring_fix_543();
}
void extcl_cpu_wr_mem_543(WORD address, BYTE value) {
	switch (address & 0xF000) {
		case 0x5000:
			m543.reg |= (((value & 0x08) >> 3) << m543.pos);
			if (m543.pos++ == 3) {
				mmc1.prg_upper = (m543.reg & 0x0F) << 4;
				m543.reg = m543.pos = 0;
				prg_fix_543();
				prg_ram_fix_543();
				chr_fix_543();
			}
			break;
		case 0x8000:
		case 0x9000:
		case 0xA000:
		case 0xB000:
		case 0xC000:
		case 0xD000:
		case 0xE000:
		case 0xF000:
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
						mirroring_fix_543();
						prg_fix_543();
						chr_fix_543();
						break;
					case 1:
						mmc1.chr0 = mmc1.reg;
						prg_ram_fix_543();
						chr_fix_543();
						break;
					case 2:
						mmc1.chr1 = mmc1.reg;
						chr_fix_543();
						break;
					case 3:
						mmc1.prg0 = mmc1.reg;
						cpu.prg_ram_rd_active = (mmc1.prg0 & 0x10 ? FALSE : TRUE);
						cpu.prg_ram_wr_active = cpu.prg_ram_rd_active;
						prg_fix_543();
						break;
				}
				mmc1.pos = mmc1.reg = 0;
			}
			break;
	}
}
BYTE extcl_save_mapper_543(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m543.pos);
	save_slot_ele(mode, slot, m543.reg);
	extcl_save_mapper_MMC1(mode, slot, fp);

	return (EXIT_OK);
}

INLINE static void prg_fix_543(void) {
	WORD bank;

	switch (mmc1.prg_mode) {
		case 0:
		case 1:
			bank = (mmc1.prg_upper | (mmc1.prg0 & mmc1.prg_mask)) >> 1;
			_control_bank(bank, info.prg.rom.max.banks_32k)
			map_prg_rom_8k(4, 0, bank);
			break;
		case 2:
			bank = mmc1.prg_upper | (mmc1.prg0 & mmc1.prg_mask);
			_control_bank(bank, info.prg.rom.max.banks_16k)
			map_prg_rom_8k(2, 2, bank);

			bank = mmc1.prg_upper;
			_control_bank(bank, info.prg.rom.max.banks_16k)
			map_prg_rom_8k(2, 0, bank);
			break;
		case 3:
			bank = mmc1.prg_upper | (mmc1.prg0 & mmc1.prg_mask);
			_control_bank(bank, info.prg.rom.max.banks_16k)
			map_prg_rom_8k(2, 0, bank);

			bank = mmc1.prg_upper | mmc1.prg_mask;
			_control_bank(bank, info.prg.rom.max.banks_16k)
			map_prg_rom_8k(2, 2, bank);
			break;
	}
	map_prg_rom_8k_update();
}
INLINE static void prg_ram_fix_543(void) {
	WORD bank;

	if (mmc1.prg_upper & 0x20) {
		bank = 0x04 | ((mmc1.prg_upper & 0x40) >> 5) | ((mmc1.prg_upper & 0x10) >> 4);
	} else {
		bank = ((mmc1.prg_upper & 0x10) >> 3) | ((mmc1.chr0 & 0x08) >> 3);
	}
	prg.ram_plus_8k = &prg.ram_plus[bank << 13];
}
INLINE static void chr_fix_543(void) {
	DBWORD bank;

	if (mmc1.chr_mode) {
		bank = mmc1.chr0 & 0x07;
		_control_bank(bank, info.chr.rom.max.banks_4k)
		bank <<= 12;
		chr.bank_1k[0] = chr_pnt(bank);
		chr.bank_1k[1] = chr_pnt(bank | 0x0400);
		chr.bank_1k[2] = chr_pnt(bank | 0x0800);
		chr.bank_1k[3] = chr_pnt(bank | 0x0C00);

		bank = mmc1.chr1 & 0x07;
		_control_bank(bank, info.chr.rom.max.banks_4k)
		bank <<= 12;
		chr.bank_1k[4] = chr_pnt(bank);
		chr.bank_1k[5] = chr_pnt(bank | 0x0400);
		chr.bank_1k[6] = chr_pnt(bank | 0x0800);
		chr.bank_1k[7] = chr_pnt(bank | 0x0C00);
		return;
	}

	bank = (mmc1.chr0 & 0x07) >> 1;
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
INLINE static void mirroring_fix_543(void) {
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
