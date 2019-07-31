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

#include <string.h>
#include "mappers.h"
#include "info.h"
#include "mem_map.h"
#include "cpu.h"
#include "save_slot.h"

BYTE *unifsmb2j_prg_6000;

void map_init_UNIFSMB2J(void) {
	EXTCL_CPU_WR_MEM(UNIFSMB2J);
	EXTCL_CPU_RD_MEM(UNIFSMB2J);
	EXTCL_SAVE_MAPPER(UNIFSMB2J);
	EXTCL_CPU_EVERY_CYCLE(UNIFSMB2J);
	mapper.internal_struct[0] = (BYTE *) &unifsmb2j;
	mapper.internal_struct_size[0] = sizeof(unifsmb2j);

	memset(&unifsmb2j, 0x00, sizeof(unifsmb2j));

	map_prg_rom_8k(4, 0, 0);
	unifsmb2j_prg_6000 = prg_chip_byte_pnt(1, unifsmb2j.reg << 13);

	info.mapper.extend_wr = TRUE;
}
void extcl_cpu_wr_mem_UNIFSMB2J(WORD address, BYTE value) {
	if (address == 0x4027) {
		unifsmb2j.reg = value & 0x01;
		_control_bank(unifsmb2j.reg, info.prg.rom[1].max.banks_8k)
		unifsmb2j_prg_6000 = prg_chip_byte_pnt(1, unifsmb2j.reg << 13);
	} else if (address == 0x4068) {
		unifsmb2j.irq.active = value & 0x01;
		unifsmb2j.irq.count = 0;
		irq.high &= ~EXT_IRQ;
	}
}
BYTE extcl_cpu_rd_mem_UNIFSMB2J(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	if ((address >= 0x4042) && (address <= 0x4055)) {
		return (0xFF);
	} else if ((address >= 0x6000) && (address <= 0x7FFF)) {
		return (unifsmb2j_prg_6000[address & 0x1FFF]);
	}
	return (openbus);
}
BYTE extcl_save_mapper_UNIFSMB2J(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, unifsmb2j.reg);
	save_slot_ele(mode, slot, unifsmb2j.irq.active);
	save_slot_ele(mode, slot, unifsmb2j.irq.count);

	if (mode == SAVE_SLOT_READ) {
		unifsmb2j_prg_6000 = prg_chip_byte_pnt(1, unifsmb2j.reg << 13);
	}

	return (EXIT_OK);
}
void extcl_cpu_every_cycle_UNIFSMB2J(void) {
	if (unifsmb2j.irq.active) {
		if (unifsmb2j.irq.count < 5750) {
			unifsmb2j.irq.count++;
		} else {
			irq.high |= EXT_IRQ;
			unifsmb2j.irq.active = 0;
		}
	}
}
