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

#include "mappers.h"
#include "info.h"
#include "mem_map.h"
#include "save_slot.h"
#include "cpu.h"

struct _m042 {
	WORD reg;
	struct _m042_irq {
		BYTE enabled;
		uint32_t count;
	} irq;
} m042;
struct _m042tmp {
	BYTE *prg_6000;
} m042tmp;

void map_init_042(void) {
	if ((info.mapper.submapper < 1) || (info.mapper.submapper > 3)) {
		if (info.chr.rom.banks_8k && !mapper.write_vram) {
			// Ai Senshi Nicole
			info.mapper.submapper = 1;
		} else if (info.prg.rom.banks_16k > (128 /16)) {
			// Green Beret
			info.mapper.submapper = 2;
		} else {
			// Mario Baby
			info.mapper.submapper = 3;
		}
	}
	switch(info.mapper.submapper) {
		default:
		case 1:
			EXTCL_CPU_WR_MEM(42_submapper1);
			EXTCL_CPU_RD_MEM(042);
			EXTCL_SAVE_MAPPER(042);
			break;
		case 2:
			map_init_AC08();
			return;
		case 3:
			EXTCL_CPU_WR_MEM(42_submapper3);
			EXTCL_CPU_RD_MEM(042);
			EXTCL_SAVE_MAPPER(042);
			EXTCL_CPU_EVERY_CYCLE(042);
			break;
	}
	mapper.internal_struct[0] = (BYTE *)&m042;
	mapper.internal_struct_size[0] = sizeof(m042);

	map_prg_rom_8k(4, 0, (info.prg.rom.banks_16k >> 1) - 1);
	m042tmp.prg_6000 = prg_pnt(0 << 13);
}
void extcl_cpu_wr_mem_42_submapper1(WORD address, BYTE value) {
	switch (address & 0xE000) {
		case 0x8000: {
			DBWORD bank;

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
			return;
		}
		case 0xE000:
			control_bank(info.prg.rom.max.banks_8k)
			m042.reg = value;
			m042tmp.prg_6000 = prg_pnt(value << 13);
			return;
	}
}
void extcl_cpu_wr_mem_42_submapper3(WORD address, BYTE value) {
	switch (address & 0xE003) {
		case 0xE000:
			control_bank(info.prg.rom.max.banks_8k)
			m042.reg = value;
			m042tmp.prg_6000 = prg_pnt(value << 13);
			return;
		case 0xE001:
			if (value == 0x08) {
				mirroring_H();
			} else  {
				mirroring_V();
			}
			return;
		case 0xE002:
			m042.irq.enabled = value & 0x02;
			if (!m042.irq.enabled) {
				m042.irq.count = 0;
			}
			irq.high &= ~EXT_IRQ;
			return;
	}
}
BYTE extcl_cpu_rd_mem_042(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	if ((address >= 0x6000) && (address <= 0x7FFF)) {
		return (m042tmp.prg_6000[address & 0x1FFF]);
	}
	return (openbus);
}
BYTE extcl_save_mapper_042(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m042.reg);
	save_slot_ele(mode, slot, m042.irq.enabled);
	save_slot_ele(mode, slot, m042.irq.count);

	if (mode == SAVE_SLOT_READ) {
		m042tmp.prg_6000 = prg_pnt(m042.reg << 13);
	}

	return (EXIT_OK);
}
void extcl_cpu_every_cycle_042(void) {
	if (m042.irq.enabled) {
		m042.irq.count++;
		if (m042.irq.count >= 32768) {
			m042.irq.count -= 32768;
		}
		if (m042.irq.count >= 24576) {
			irq.high |= EXT_IRQ;
		} else {
			irq.high &= ~EXT_IRQ;
		}
	}
}
