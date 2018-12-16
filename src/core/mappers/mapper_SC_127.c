/*
 *  Copyright (C) 2010-2019 Fabio Cavallo (aka FHorse)
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
#include "cpu.h"
#include "ppu.h"
#include "save_slot.h"

void map_init_SC_127(void) {
	EXTCL_CPU_WR_MEM(SC_127);
	EXTCL_CPU_RD_MEM(SC_127);
	EXTCL_SAVE_MAPPER(SC_127);
	EXTCL_PPU_256_TO_319(SC_127);

	mapper.internal_struct[0] = (BYTE *) &sc127;
	mapper.internal_struct_size[0] = sizeof(sc127);

	memset(&sc127, 0x00, sizeof(sc127));

	info.prg.ram.banks_8k_plus = 1;
}
void extcl_cpu_wr_mem_SC_127(WORD address, BYTE value) {
	switch (address) {
		case 0x8000:
		case 0x8001:
		case 0x8002:
			control_bank(info.prg.rom[0].max.banks_8k)
			map_prg_rom_8k(1, address & 0x03, value);
			map_prg_rom_8k_update();
			return;
		case 0x9000:
		case 0x9001:
		case 0x9002:
		case 0x9003:
		case 0x9004:
		case 0x9005:
		case 0x9006:
		case 0x9007:
			control_bank(info.chr.rom[0].max.banks_1k)
			chr.bank_1k[address & 0x07] = chr_chip_byte_pnt(0, value << 10);
			return;
		case 0xC002:
			sc127.irq.active = 0;
			irq.high &= ~EXT_IRQ;
			return;
		case 0xC003:
			sc127.irq.active = 1;
			return;
		case 0xC005:
			sc127.irq.count = value;
			return;
		case 0xD001:
			if (value & 0x01) {
				mirroring_H();
			} else {
				mirroring_V();
			}
			return;
	}
}
BYTE extcl_cpu_rd_mem_SC_127(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	if (address == 0x5800) {
		return (0x20);
	}
	return (openbus);
}
BYTE extcl_save_mapper_SC_127(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, sc127.irq.active);
	save_slot_ele(mode, slot, sc127.irq.count);

	return (EXIT_OK);
}
void extcl_ppu_256_to_319_SC_127(void) {
	if ((ppu.frame_x != 288) || (!sc127.irq.active)) {
		return;
	}

	if (sc127.irq.count > 0) {
		sc127.irq.count--;
	}

	if (sc127.irq.count == 0) {
		sc127.irq.active = 0;
		irq.delay = TRUE;
		irq.high |= EXT_IRQ;
	}
}
