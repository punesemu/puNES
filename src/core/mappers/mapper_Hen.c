/*
 *  Copyright (C) 2010-2020 Fabio Cavallo (aka FHorse)
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

#include "mappers.h"
#include "info.h"
#include "mem_map.h"

BYTE type;

void map_init_Hen(BYTE model) {
	switch (model) {
		case HEN_177:
		case HEN_FANKONG:
			EXTCL_CPU_WR_MEM(Hen_177);
			break;
		case HEN_XJZB:
			EXTCL_CPU_WR_MEM(Hen_xjzb);
			info.mapper.extend_wr = TRUE;
			break;
	}

	if (info.reset >= HARD) {
		map_prg_rom_8k(4, 0, 0);
	}

	type = model;
}

void extcl_cpu_wr_mem_Hen_177(UNUSED(WORD address), BYTE value) {
	if (type != HEN_FANKONG) {
		if (value & 0x20) {
			mirroring_H();
		} else {
			mirroring_V();
		}
	}

	control_bank(info.prg.rom[0].max.banks_32k)
	map_prg_rom_8k(4, 0, value);
	map_prg_rom_8k_update();
}

void extcl_cpu_wr_mem_Hen_xjzb(WORD address, BYTE value) {
	if ((address < 0x5000) || (address > 0x5FFF)) {
		return;
	}

	value >>= 1;
	control_bank(info.prg.rom[0].max.banks_32k)
	map_prg_rom_8k(4, 0, value);
	map_prg_rom_8k_update();
}
