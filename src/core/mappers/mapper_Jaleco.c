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
#include "mem_map.h"
#include "info.h"
#include "mappers.h"
#include "cpu.h"
#include "ppu.h"
#include "save_slot.h"

void map_init_Jaleco(BYTE model) {
	switch (model) {
		case JF11:
			EXTCL_CPU_WR_MEM(Jaleco_JF11);
			info.mapper.extend_wr = TRUE;
			if (info.reset >= HARD) {
				map_prg_rom_8k(4, 0, 0);
			}
			break;
		case JF13:
			EXTCL_CPU_WR_MEM(Jaleco_JF13);
			info.mapper.extend_wr = TRUE;
			if (info.reset >= HARD) {
				map_prg_rom_8k(4, 0, 0);
			}
			break;
		case JF16:
			EXTCL_CPU_WR_MEM(Jaleco_JF16);
			break;
		case JF17:
			EXTCL_CPU_WR_MEM(Jaleco_JF17);
			break;
		case JF19:
			EXTCL_CPU_WR_MEM(Jaleco_JF19);
			break;
		default:
			break;
	}
}

void extcl_cpu_wr_mem_Jaleco_JF05(WORD address, BYTE value) {
	DBWORD bank;

	if ((address < 0x6000) || (address >= 0x8000)) {
		return;
	}

	value = (((value >> 1) & 0x1) | ((value << 1) & 0x2));
	control_bank_with_AND(0x03, info.chr.rom.max.banks_8k)
	bank = value << 13;
	chr.bank_1k[0] = chr_pnt(bank);
	chr.bank_1k[1] = chr_pnt(bank | 0x0400);
	chr.bank_1k[2] = chr_pnt(bank | 0x0800);
	chr.bank_1k[3] = chr_pnt(bank | 0x0C00);
	chr.bank_1k[4] = chr_pnt(bank | 0x1000);
	chr.bank_1k[5] = chr_pnt(bank | 0x1400);
	chr.bank_1k[6] = chr_pnt(bank | 0x1800);
	chr.bank_1k[7] = chr_pnt(bank | 0x1C00);
}

void extcl_cpu_wr_mem_Jaleco_JF11(WORD address, BYTE value) {
	BYTE save = value;
	DBWORD bank;

	if ((address < 0x6000) || (address >= 0x8000)) {
		return;
	}

	value >>= 4;
	control_bank_with_AND(0x03, info.prg.rom.max.banks_32k)
	map_prg_rom_8k(4, 0, value);
	map_prg_rom_8k_update();

	value = save;
	control_bank_with_AND(0x0F, info.chr.rom.max.banks_8k)
	bank = value << 13;
	chr.bank_1k[0] = chr_pnt(bank);
	chr.bank_1k[1] = chr_pnt(bank | 0x0400);
	chr.bank_1k[2] = chr_pnt(bank | 0x0800);
	chr.bank_1k[3] = chr_pnt(bank | 0x0C00);
	chr.bank_1k[4] = chr_pnt(bank | 0x1000);
	chr.bank_1k[5] = chr_pnt(bank | 0x1400);
	chr.bank_1k[6] = chr_pnt(bank | 0x1800);
	chr.bank_1k[7] = chr_pnt(bank | 0x1C00);
}

void extcl_cpu_wr_mem_Jaleco_JF13(WORD address, BYTE value) {
	BYTE save = value;
	DBWORD bank;

	/* FIXME: da 0x7000 a 0x7FFF c'e' la gestione dell'audio */
	if ((address < 0x6000) || (address >= 0x7000)) {
		return;
	}

	value >>= 4;
	control_bank_with_AND(0x03, info.prg.rom.max.banks_32k)
	map_prg_rom_8k(4, 0, value);
	map_prg_rom_8k_update();

	value = ((save & 0x40) >> 4) | (save & 0x03);
	control_bank(info.chr.rom.max.banks_8k)
	bank = value << 13;
	chr.bank_1k[0] = chr_pnt(bank);
	chr.bank_1k[1] = chr_pnt(bank | 0x0400);
	chr.bank_1k[2] = chr_pnt(bank | 0x0800);
	chr.bank_1k[3] = chr_pnt(bank | 0x0C00);
	chr.bank_1k[4] = chr_pnt(bank | 0x1000);
	chr.bank_1k[5] = chr_pnt(bank | 0x1400);
	chr.bank_1k[6] = chr_pnt(bank | 0x1800);
	chr.bank_1k[7] = chr_pnt(bank | 0x1C00);
}

void extcl_cpu_wr_mem_Jaleco_JF16(WORD address, BYTE value) {
	BYTE save = value &= prg_rom_rd(address);
	DBWORD bank;

	control_bank_with_AND(0x07, info.prg.rom.max.banks_16k)
	map_prg_rom_8k(2, 0, value);
	map_prg_rom_8k_update();

	value = save >> 4;
	control_bank_with_AND(0x0F, info.chr.rom.max.banks_8k)
	bank = value << 13;
	chr.bank_1k[0] = chr_pnt(bank);
	chr.bank_1k[1] = chr_pnt(bank | 0x0400);
	chr.bank_1k[2] = chr_pnt(bank | 0x0800);
	chr.bank_1k[3] = chr_pnt(bank | 0x0C00);
	chr.bank_1k[4] = chr_pnt(bank | 0x1000);
	chr.bank_1k[5] = chr_pnt(bank | 0x1400);
	chr.bank_1k[6] = chr_pnt(bank | 0x1800);
	chr.bank_1k[7] = chr_pnt(bank | 0x1C00);

	if (save & 0x08) {
		mirroring_SCR1();
		if (info.mapper.submapper == HOLYDIVER) {
			mirroring_V();
		}
	} else {
		mirroring_SCR0();
		if (info.mapper.submapper == HOLYDIVER) {
			mirroring_H();
		}
	}
}

void extcl_cpu_wr_mem_Jaleco_JF17(WORD address, BYTE value) {
	/* bus conflict */
	BYTE save = value &= prg_rom_rd(address);
	DBWORD bank;

	if (save & 0x80) {
		control_bank_with_AND(0x0F, info.prg.rom.max.banks_16k)
		map_prg_rom_8k(2, 0, value);
		map_prg_rom_8k_update();
	}

	if (save & 0x40) {
		value = save;
		control_bank_with_AND(0x0F, info.chr.rom.max.banks_8k)
		bank = value << 13;
		chr.bank_1k[0] = chr_pnt(bank);
		chr.bank_1k[1] = chr_pnt(bank | 0x0400);
		chr.bank_1k[2] = chr_pnt(bank | 0x0800);
		chr.bank_1k[3] = chr_pnt(bank | 0x0C00);
		chr.bank_1k[4] = chr_pnt(bank | 0x1000);
		chr.bank_1k[5] = chr_pnt(bank | 0x1400);
		chr.bank_1k[6] = chr_pnt(bank | 0x1800);
		chr.bank_1k[7] = chr_pnt(bank | 0x1C00);
	}

	/* FIXME : aggiungere l'emulazione del D7756C */
}

void extcl_cpu_wr_mem_Jaleco_JF19(WORD address, BYTE value) {
	/* bus conflict */
	BYTE save = value &= prg_rom_rd(address);
	DBWORD bank;

	if (save & 0x80) {
		control_bank_with_AND(0x0F, info.prg.rom.max.banks_16k)
		map_prg_rom_8k(2, 2, value);
		map_prg_rom_8k_update();
	}

	if (save & 0x40) {
		value = save;
		control_bank_with_AND(0x0F, info.chr.rom.max.banks_8k)
		bank = value << 13;
		chr.bank_1k[0] = chr_pnt(bank);
		chr.bank_1k[1] = chr_pnt(bank | 0x0400);
		chr.bank_1k[2] = chr_pnt(bank | 0x0800);
		chr.bank_1k[3] = chr_pnt(bank | 0x0C00);
		chr.bank_1k[4] = chr_pnt(bank | 0x1000);
		chr.bank_1k[5] = chr_pnt(bank | 0x1400);
		chr.bank_1k[6] = chr_pnt(bank | 0x1800);
		chr.bank_1k[7] = chr_pnt(bank | 0x1C00);
	}

	/* FIXME : aggiungere l'emulazione del D7756C */
}

