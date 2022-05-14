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
#include "info.h"
#include "cpu.h"
#include "save_slot.h"

struct _bj56 {
	WORD irq_counter;
} bj56;

void map_init_BJ56(void) {
	EXTCL_CPU_WR_MEM(BJ56);
	EXTCL_SAVE_MAPPER(BJ56);
	EXTCL_CPU_EVERY_CYCLE(BJ56);
	mapper.internal_struct[0] = (BYTE *)&bj56;
	mapper.internal_struct_size[0] = sizeof(bj56);

	memset(&bj56, 0x00, sizeof(bj56));

	if (info.reset >= HARD) {
		BYTE value;

		value = ~0;
		control_bank(info.prg.rom.max.banks_32k)
		map_prg_rom_8k(4, 0, value);
	}

	mirroring_V();
}
void extcl_cpu_wr_mem_BJ56(WORD address, BYTE value) {
	switch (address & 0xE00F) {
		case 0x8000:
		case 0x8001:
		case 0x8002:
		case 0x8003:
		case 0x8004:
		case 0x8005:
		case 0x8006:
		case 0x8007:
			control_bank(info.chr.rom.max.banks_1k)
			chr.bank_1k[address & 0x07] = chr_pnt(value << 10);
			break;
		case 0x8008:
		case 0x8009:
		case 0x800A:
		case 0x800B:
			control_bank(info.prg.rom.max.banks_8k)
			map_prg_rom_8k(1, address & 0x03, value);
			map_prg_rom_8k_update();
			break;
		case 0x800D:
			bj56.irq_counter = 0;
			break;
		case 0x800F:
			irq.high &= ~EXT_IRQ;
			break;
	}
}
BYTE extcl_save_mapper_BJ56(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, bj56.irq_counter);

	return (EXIT_OK);
}
void extcl_cpu_every_cycle_BJ56(void) {
	if (++bj56.irq_counter & 0x1000) {
		irq.high |= EXT_IRQ;
	}
}
