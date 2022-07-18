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
#include "cpu.h"
#include "save_slot.h"

INLINE static void prg_fix_550(void);
INLINE static void chr_fix_550(void);
INLINE static void mirroring_fix_550(void);

INLINE static void cpu_wr_mem_mmc1(WORD address, BYTE value);

struct _m550 {
	BYTE reg[2];
} m550;

void map_init_550(void) {
	EXTCL_AFTER_MAPPER_INIT(550);
	EXTCL_CPU_WR_MEM(550);
	EXTCL_SAVE_MAPPER(550);
	mapper.internal_struct[0] = (BYTE *)&m550;
	mapper.internal_struct_size[0] = sizeof(m550);
	mapper.internal_struct[1] = (BYTE *)&mmc1;
	mapper.internal_struct_size[1] = sizeof(mmc1);

	memset(&mmc1, 0x00, sizeof(mmc1));
	memset(&m550, 0x00, sizeof(m550));

	mmc1.ctrl = 0x0C;
	mmc1.prg_mode = 3;

	info.mapper.extend_wr = TRUE;
}
void extcl_after_mapper_init_550(void) {
	prg_fix_550();
	chr_fix_550();
	mirroring_fix_550();
}
void extcl_cpu_wr_mem_550(WORD address, BYTE value) {
	if ((address >= 0x7000) && (address <= 0x7FFF)) {
		if (!(m550.reg[0] & 0x08)) {
			m550.reg[0] = address & 0x0F;
			prg_fix_550();
			chr_fix_550();
			mirroring_fix_550();
		}
		return;
	}
	if (address >= 0x8000) {
		m550.reg[1] = value;
		if ((m550.reg[0] & 0x06) == 0x06) {
			cpu_wr_mem_mmc1(address, value);
		}
		prg_fix_550();
		chr_fix_550();
		mirroring_fix_550();
	}
}
BYTE extcl_save_mapper_550(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m550.reg);
	extcl_save_mapper_MMC1(mode, slot, fp);

	return (EXIT_OK);
}

INLINE static void prg_fix_550(void) {
	WORD base = m550.reg[0] & 0x07;
	WORD bank;

	if ((m550.reg[0] & 0x06) == 0x06) {
		WORD mask = 0x07;

		base <<= 2;

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
	} else {
		bank = (base << 1) | ((m550.reg[1] & 0x10) >> 4);
		_control_bank(bank, info.prg.rom.max.banks_32k)
		map_prg_rom_8k(4, 0, bank);
	}
	map_prg_rom_8k_update();
}
INLINE static void chr_fix_550(void) {
	WORD base = m550.reg[0] & 0x06;
	DBWORD bank;

	if ((m550.reg[0] & 0x06) == 0x06) {
		WORD mask = 0x07;

		base <<= 2;

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
	} else {
		bank = (base << 1) | (m550.reg[1] & 0x03);
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
}
INLINE static void mirroring_fix_550(void) {
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

INLINE static void cpu_wr_mem_mmc1(WORD address, BYTE value) {
	if (((m550.reg[0] & 0x06) == 0x06)) {
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
					break;
				case 1:
					mmc1.chr0 = mmc1.reg;
					break;
				case 2:
					mmc1.chr1 = mmc1.reg;
					break;
				case 3:
					mmc1.prg0 = mmc1.reg;
					cpu.prg_ram_rd_active = (mmc1.prg0 & 0x10 ? FALSE : TRUE);
					cpu.prg_ram_wr_active = cpu.prg_ram_rd_active;
					break;
			}
			mmc1.pos = mmc1.reg = 0;
		}
	}
}
