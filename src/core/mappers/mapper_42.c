/*
 *  Copyright (C) 2010-2016 Fabio Cavallo (aka FHorse)
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
#include "save_slot.h"
#include "cpu.h"

void map_init_42(void) {
	EXTCL_CPU_WR_MEM(42);
	EXTCL_CPU_RD_MEM(42);
	EXTCL_SAVE_MAPPER(42);
	EXTCL_CPU_EVERY_CYCLE(42);
	mapper.internal_struct[0] = (BYTE *) &m42;
	mapper.internal_struct_size[0] = sizeof(m42);

	map_prg_rom_8k(4, 0, (info.prg.rom.banks_16k >> 1) - 1);
	m42.prg_8k_6000 = prg_chip_byte_pnt(0, 0 << 13);
}
void extcl_cpu_wr_mem_42(WORD address, BYTE value) {
	switch (address & 0xE003) {
		case 0x8000: {
			DBWORD bank;

			control_bank(info.chr.rom.max.banks_8k)
			bank = value << 13;
			chr.bank_1k[0] = chr_chip_byte_pnt(0, bank);
			chr.bank_1k[1] = chr_chip_byte_pnt(0, bank | 0x0400);
			chr.bank_1k[2] = chr_chip_byte_pnt(0, bank | 0x0800);
			chr.bank_1k[3] = chr_chip_byte_pnt(0, bank | 0x0C00);
			chr.bank_1k[4] = chr_chip_byte_pnt(0, bank | 0x1000);
			chr.bank_1k[5] = chr_chip_byte_pnt(0, bank | 0x1400);
			chr.bank_1k[6] = chr_chip_byte_pnt(0, bank | 0x1800);
			chr.bank_1k[7] = chr_chip_byte_pnt(0, bank | 0x1C00);
			return;
		}
		case 0xE000:
			control_bank(info.prg.rom.max.banks_8k)
			m42.rom_map_to = value;
			m42.prg_8k_6000 = prg_chip_byte_pnt(0, value << 13);
			return;
		case 0xE001:
			if (value == 0x00) {
				mirroring_H();
				return;
			}
			if (value == 0x08) {
				mirroring_V();
				return;
			}
			return;
		case 0xE002:
			m42.irq.active = value & 0x02;
			if (!m42.irq.active) {
				m42.irq.count = 0;
			}
			irq.high &= ~EXT_IRQ;
			return;
	}
}
BYTE extcl_cpu_rd_mem_42(WORD address, BYTE openbus, BYTE before) {
	if ((address > 0x5FFF) && (address < 0x8000)) {
		return (m42.prg_8k_6000[address & 0x1FFF]);
	}
	return (openbus);
}
BYTE extcl_save_mapper_42(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m42.rom_map_to);
	save_slot_ele(mode, slot, m42.prg_8k_6000);
	save_slot_ele(mode, slot, m42.irq.active);
	save_slot_ele(mode, slot, m42.irq.count);

	if (mode == SAVE_SLOT_READ) {
		m42.prg_8k_6000 = prg_chip_byte_pnt(0, m42.rom_map_to << 13);
	}

	return (EXIT_OK);
}
void extcl_cpu_every_cycle_42(void) {
	if (m42.irq.active) {
		m42.irq.count++;
		if (m42.irq.count >= 32768) {
			m42.irq.count -= 32768;
		}
		if (m42.irq.count >= 24576) {
			irq.high |= EXT_IRQ;
		} else {
			irq.high &= ~EXT_IRQ;
		}
	}
}
