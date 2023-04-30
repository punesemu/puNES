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
#include "cpu.h"
#include "save_slot.h"

INLINE static void sync_083(void);

struct _m083 {
	BYTE is2kbank;
	BYTE isnot2kbank;
	BYTE mode;
	BYTE bank;
	BYTE dip;
	BYTE low[4];
	BYTE reg[11];

	struct _m083_irq {
		BYTE active;
		WORD count;
	} irq;
} m083;

void map_init_083(void) {
	EXTCL_CPU_WR_MEM(083);
	EXTCL_CPU_RD_MEM(083);
	EXTCL_SAVE_MAPPER(083);
	EXTCL_CPU_EVERY_CYCLE(083);

	mapper.internal_struct[0] = (BYTE *)&m083;
	mapper.internal_struct_size[0] = sizeof(m083);

	if (info.reset >= HARD) {
		memset(&m083, 0x00, sizeof(m083));
	}

	sync_083();

	switch (info.id) {
		case MAP83_REG0:
			m083.dip = 0;
			break;
		case MAP83_DGP:
			m083.dip = 0xFF;
			info.prg.ram.banks_8k_plus = 1;
			break;
		default:
			m083.dip = 0xFF;
			break;
	}

	info.mapper.extend_wr = TRUE;
}
void extcl_cpu_wr_mem_083(WORD address, BYTE value) {
	switch (address) {
		case 0x5100:
		case 0x5101:
		case 0x5102:
		case 0x5103:
			m083.low[address & 0x0003] = value;
			break;
		case 0x8000:
			m083.is2kbank = TRUE;
			m083.bank = value;
			m083.mode |= 0x40;
			sync_083();
			break;
		case 0xB000:
		case 0xB0FF:
		case 0xB1FF:
			m083.bank = value;
			m083.mode |= 0x40;
			sync_083();
			break;
		case 0x8100:
			m083.mode = value | (m083.mode & 0x40);
			sync_083();
			break;
		case 0x8200:
			m083.irq.count &= 0xFF00;
			m083.irq.count |= value;
			irq.high &= ~EXT_IRQ;
			break;
		case 0x8201:
			m083.irq.active = m083.mode & 0x80;
			m083.irq.count &= 0x00FF;
			m083.irq.count |= (value << 8);
			break;
		case 0x8300:
		case 0x8301:
		case 0x8302:
			m083.reg[(address & 0x03) | 0x08] = value;
			m083.mode &= 0xBF;
			sync_083();
			break;
		case 0x8312:
		case 0x8313:
		case 0x8314:
		case 0x8315:
			m083.reg[address & 0x0007] = value;
			m083.isnot2kbank = TRUE;
			sync_083();
			break;
		case 0x8310:
		case 0x8311:
		case 0x8316:
		case 0x8317:
			m083.reg[address & 0x0007] = value;
			sync_083();
			break;
	}
}
BYTE extcl_cpu_rd_mem_083(WORD address, BYTE openbus, BYTE before) {
	if (address == 0x5000) {
		return (m083.dip);
	}
	if ((address >= 0x5100) && (address <= 0x5103)) {
		return (m083.low[address & 0x0003]);
	}
	if (address <= 0x5FFF) {
		return (before);
	}
	/* nestopia */
	if ((address <= 0x7FFF) && !info.prg.ram.banks_8k_plus) {
		return ((m083.mode & 0x20) ? openbus : (address >> 8));
	}
	/*          */
	return (openbus);
}
BYTE extcl_save_mapper_083(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m083.is2kbank);
	save_slot_ele(mode, slot, m083.isnot2kbank);
	save_slot_ele(mode, slot, m083.mode);
	save_slot_ele(mode, slot, m083.bank);
	save_slot_ele(mode, slot, m083.dip);
	save_slot_ele(mode, slot, m083.low);
	save_slot_ele(mode, slot, m083.reg);
	save_slot_ele(mode, slot, m083.irq.active);
	save_slot_ele(mode, slot, m083.irq.count);

	return (EXIT_OK);
}
void extcl_cpu_every_cycle_083(void) {
	if (m083.irq.active) {
		if (--m083.irq.count == 0) {
			irq.high |= EXT_IRQ;
			m083.irq.active = 0;
			m083.irq.count = 0xFFFF;
		}
	}
}

INLINE static void sync_083(void) {
	WORD value;

	switch (m083.mode & 0x03) {
		case 0:
			mirroring_V();
			break;
		case 1:
			mirroring_H();
			break;
		case 2:
			mirroring_SCR0();
			break;
		case 3:
			mirroring_SCR1();
			break;
	}

	if (m083.is2kbank && !m083.isnot2kbank) {
		SDBWORD bank;

		value = m083.reg[0];
		control_bank(info.chr.rom.max.banks_2k)
		bank = value << 11;
		chr.bank_1k[0] = chr_pnt(bank);
		chr.bank_1k[1] = chr_pnt(bank | 0x0400);
		value = m083.reg[1];
		control_bank(info.chr.rom.max.banks_2k)
		bank = value << 11;
		chr.bank_1k[2] = chr_pnt(bank);
		chr.bank_1k[3] = chr_pnt(bank | 0x0400);
		value = m083.reg[6];
		control_bank(info.chr.rom.max.banks_2k)
		bank = value << 11;
		chr.bank_1k[4] = chr_pnt(bank);
		chr.bank_1k[5] = chr_pnt(bank | 0x0400);
		value = m083.reg[7];
		control_bank(info.chr.rom.max.banks_2k)
		bank = value << 11;
		chr.bank_1k[6] = chr_pnt(bank);
		chr.bank_1k[7] = chr_pnt(bank | 0x0400);
	} else {
		BYTE i;

		for (i = 0; i < 8; i++) {
			value = ((m083.bank << 4) & 0x0300) | m083.reg[i];
			control_bank(info.chr.rom.max.banks_1k)
			chr.bank_1k[i] = chr_pnt(value << 10);
		}
	}
	if (m083.mode & 0x40) {
		value = (m083.bank & 0x3F);
		control_bank(info.prg.rom.max.banks_16k)
		map_prg_rom_8k(2, 0, value);
		value = (m083.bank & 0x30) | 0x0F;
		control_bank(info.prg.rom.max.banks_16k)
		map_prg_rom_8k(2, 2, value);
	} else {
		value = m083.reg[8];
		control_bank(info.prg.rom.max.banks_8k)
		map_prg_rom_8k(1, 0, value);
		value = m083.reg[9];
		control_bank(info.prg.rom.max.banks_8k)
		map_prg_rom_8k(1, 1, value);
		value = m083.reg[10];
		control_bank(info.prg.rom.max.banks_8k)
		map_prg_rom_8k(1, 2, value);
		map_prg_rom_8k(1, 3, info.prg.rom.max.banks_8k);
	}
	map_prg_rom_8k_update();
}
