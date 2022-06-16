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
#include "save_slot.h"

INLINE static void prg_fix_WS(void);
INLINE static void chr_fix_WS(void);

static const WORD dipswitch_ws[] = { 0x00, 0x40, 0x80 };
struct _ws {
	BYTE reg[3];
} ws;
struct _wstmp {
	BYTE index;
	BYTE dipswitch;
} wstmp;

void map_init_WS(void) {
	EXTCL_AFTER_MAPPER_INIT(WS);
	EXTCL_CPU_WR_MEM(WS);
	EXTCL_CPU_RD_MEM(WS);
	EXTCL_SAVE_MAPPER(WS);
	mapper.internal_struct[0] = (BYTE *)&ws;
	mapper.internal_struct_size[0] = sizeof(ws);

	memset(&ws, 0x00, sizeof(ws));

	if (info.reset == RESET) {
		wstmp.index = (wstmp.index + 1) >= 3 ? 0 : wstmp.index + 1;
	} else if (((info.reset == CHANGE_ROM) || (info.reset == POWER_UP))) {
		wstmp.index = 0;
	}

	wstmp.dipswitch = dipswitch_ws[wstmp.index];

	info.mapper.extend_wr = info.mapper.extend_rd = TRUE;
}
void extcl_after_mapper_init_WS(void) {
	prg_fix_WS();
	chr_fix_WS();
}
void extcl_cpu_wr_mem_WS(WORD address, UNUSED(BYTE value)) {
	switch (address & 0xE001) {
		case 0x6000:
			ws.reg[0] = value;
			prg_fix_WS();
			chr_fix_WS();
			break;
		case 0x6001:
			ws.reg[1] = value;
			chr_fix_WS();
			break;
		case 0x8000 :
		case 0x8001 :
		case 0xA000 :
		case 0xA001 :
		case 0xC000 :
		case 0xC001 :
		case 0xE000 :
		case 0xE001 :
			ws.reg[2] = value;
			chr_fix_WS();
			break;
	}
	return;
}
BYTE extcl_cpu_rd_mem_WS(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	if (address >= 0x8000) {
		if ((ws.reg[1] & 0xC0) & wstmp.dipswitch) {
			return (0xFF);
		}
	}
	return (openbus);
}
BYTE extcl_save_mapper_WS(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, ws.reg);
	save_slot_ele(mode, slot, wstmp.index);
	save_slot_ele(mode, slot, wstmp.dipswitch);

	return (EXIT_OK);
}

INLINE static void prg_fix_WS(void) {
	BYTE value, base = ((ws.reg[0] & 0x40) >> 3) | (ws.reg[0] & 0x06);

	if (ws.reg[0] & 0x08) {
		value = base | (ws.reg[0] & 0x01);
		control_bank(info.prg.rom.max.banks_16k)
		map_prg_rom_8k(2, 0, value);
		map_prg_rom_8k(2, 2, value);
	} else {
		value = base >> 1;
		control_bank(info.prg.rom.max.banks_32k)
		map_prg_rom_8k(4, 0, value);
	}

	map_prg_rom_8k_update();

	if (ws.reg[0] & 0x10) {
		mirroring_H();
	} else {
		mirroring_V();
	}
}
INLINE static void chr_fix_WS(void) {
	DBWORD value = ((ws.reg[0] & 0x40) >> 3) | (ws.reg[1] & 0x04);

	switch (ws.reg[1] & 0x30) {
		case 0x00:
			value |= ws.reg[2] & 0x03;
			break;
		case 0x20:
			value |= (ws.reg[1] & 0x02) | (ws.reg[2] & 0x01);
			break;
		case 0x30:
			value |= ws.reg[1] & 0x03;
			break;
	}

	control_bank(info.chr.rom.max.banks_8k)
	value <<= 13;

	chr.bank_1k[0] = chr_pnt(value | 0x0000);
	chr.bank_1k[1] = chr_pnt(value | 0x0400);
	chr.bank_1k[2] = chr_pnt(value | 0x0800);
	chr.bank_1k[3] = chr_pnt(value | 0x0C00);
	chr.bank_1k[4] = chr_pnt(value | 0x1000);
	chr.bank_1k[5] = chr_pnt(value | 0x1400);
	chr.bank_1k[6] = chr_pnt(value | 0x1800);
	chr.bank_1k[7] = chr_pnt(value | 0x1C00);
}
