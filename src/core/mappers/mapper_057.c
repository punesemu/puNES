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
#include "save_slot.h"

struct _m057 {
	BYTE reg[2];
} m057;
struct _m057tmp {
	BYTE dipswitch;
	BYTE mask;
	BYTE start;
	SBYTE op;
} m057tmp;

void map_init_057(void) {
	EXTCL_CPU_WR_MEM(057);
	EXTCL_CPU_RD_MEM(057);
	EXTCL_SAVE_MAPPER(057);
	mapper.internal_struct[0] = (BYTE *)&m057;
	mapper.internal_struct_size[0] = sizeof(m057);

	if (info.reset >= HARD) {
		memset(&m057, 0x00, sizeof(m057));
	}

	if (
		(info.crc32.prg == 0xF77A2663) || // 4-in-1 (ES-Q803B_20210617) (Unl) [p1].nes
		(info.crc32.prg == 0xDB6228A0) || // 4-in-1_YH-4132.nes
		(info.crc32.prg == 0x71B7EC3A) || // (YH-4131) Exciting Sport Turbo 4-in-1.nes
		(info.crc32.prg == 0xEE722DE3) || // (YH-4135) Exciting Sport Turbo 4-in-1.nes
		(info.crc32.prg == 0xD35D3D8F)) { // (YH-4136) Exciting Sport Turbo 4-in-1.nes
		m057tmp.mask = 0x01;
		m057tmp.start = 0x01;
		m057tmp.op = 1;
	} else if (info.crc32.prg == 0xC74F9C72) { // 1998 Series No. 10.nes
		m057tmp.mask = 0x03;
		m057tmp.start = 0x02;
		m057tmp.op = -1;
	} else if (info.crc32.prg == 0xA8930B3B) { // 32-in-1 (42, 52, 62-in-1) (ABCARD-02) (Unl) [p1].nes
		m057tmp.mask = 0x03;
		m057tmp.start = 0x03;
		m057tmp.op = -1;
	} else {
		m057tmp.mask = 0x03;
		m057tmp.start = 0x00;
		m057tmp.op = 1;
	}

	if (info.reset == RESET) {
		m057tmp.dipswitch = (m057tmp.dipswitch + m057tmp.op) & m057tmp.mask;
	} else if (((info.reset == CHANGE_ROM) || (info.reset == POWER_UP))) {
		m057tmp.dipswitch = m057tmp.start;
	}

	extcl_cpu_wr_mem_057(0x8800, 0x00);
}
void extcl_cpu_wr_mem_057(WORD address, BYTE value) {
	DBWORD bank;

	if (address & 0x0800) {
		m057.reg[0] = value;

		if (m057.reg[0] & 0x08) {
			mirroring_H();
		} else  {
			mirroring_V();
		}

		if (m057.reg[0] & 0x10) {
			value = (m057.reg[0] & 0xC0) >> 6;
			control_bank(info.prg.rom.max.banks_32k)
			map_prg_rom_8k(4, 0, value);
		} else {
			value = (m057.reg[0] & 0xE0) >> 5;
			control_bank(info.prg.rom.max.banks_16k)
			map_prg_rom_8k(2, 0, value);
			map_prg_rom_8k(2, 2, value);
		}

		map_prg_rom_8k_update();
	} else {
		m057.reg[1] = value;
	}

	value = (m057.reg[1] & 0x07) | (m057.reg[0] & 0x07) | ((m057.reg[1] & 0x40) >> 3);
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
BYTE extcl_cpu_rd_mem_057(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	if ((address >= 0x6000) && (address <= 0x6FFF)) {
		return (m057tmp.dipswitch);
	}
	return (openbus);
}
BYTE extcl_save_mapper_057(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m057.reg);

	return (EXIT_OK);
}
