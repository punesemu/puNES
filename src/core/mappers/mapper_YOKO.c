/*
 *  Copyright (C) 2010-2021 Fabio Cavallo (aka FHorse)
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

INLINE static void yoko_update(void);

struct _yoko {
	BYTE mode;
	BYTE bank;
	BYTE dip;
	BYTE low[4];
	BYTE reg[7];

	struct _yoko_irq {
		BYTE active;
		WORD count;
	} irq;
} yoko;

void map_init_YOKO(void) {
	EXTCL_CPU_WR_MEM(YOKO);
	EXTCL_CPU_RD_MEM(YOKO);
	EXTCL_SAVE_MAPPER(YOKO);
	EXTCL_CPU_EVERY_CYCLE(YOKO);
	mapper.internal_struct[0] = (BYTE *)&yoko;
	mapper.internal_struct_size[0] = sizeof(yoko);

	if (info.reset >= HARD) {
		memset(&yoko, 0x00, sizeof(yoko));
		yoko.dip = 0x03;
	} else if (info.reset == RESET) {
		yoko.mode = yoko.bank = 0;
		yoko.dip++;
		yoko.dip = yoko.dip & 0x03;
	}

	yoko_update();

	info.mapper.extend_wr = TRUE;
}
void extcl_cpu_wr_mem_YOKO(WORD address, BYTE value) {
	if ((address >= 0x5400) && (address <= 0x5FFF)) {
		yoko.low[address & 0x0003] = value;
		return;
	}

	switch (address & 0x8C17) {
		case 0x8000:
			yoko.bank = value;
			yoko_update();
			return;
		case 0x8400:
			yoko.mode = value;
			yoko_update();
			return;
		case 0x8800:
			yoko.irq.count &= 0xFF00;
			yoko.irq.count |= value;
			irq.high &= ~EXT_IRQ;
			return;
		case 0x8801:
			yoko.irq.active = yoko.mode & 0x80;
			yoko.irq.count &= 0x00FF;
			yoko.irq.count |= (value << 8);
			return;
		case 0x8C00:
		case 0x8C01:
		case 0x8C02:
			yoko.reg[address & 0x03] = value;
			yoko_update();
			return;
		case 0x8C10:
			yoko.reg[3] = value;
			yoko_update();
			return;
		case 0x8C11:
			yoko.reg[4] = value;
			yoko_update();
			return;
		case 0x8C16:
			yoko.reg[5] = value;
			yoko_update();
			return;
		case 0x8C17:
			yoko.reg[6] = value;
			yoko_update();
			return;
	}
}
BYTE extcl_cpu_rd_mem_YOKO(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	if ((address >= 0x5000) && (address <= 0x53FF)) {
		return ((openbus & 0xFC) | yoko.dip);
	}
	if ((address >= 0x5400) && (address <= 0x5FFF)) {
		return (yoko.low[address & 0x03]);
	}
	return (openbus);
}
BYTE extcl_save_mapper_YOKO(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, yoko.mode);
	save_slot_ele(mode, slot, yoko.bank);
	save_slot_ele(mode, slot, yoko.dip);
	save_slot_ele(mode, slot, yoko.reg);
	save_slot_ele(mode, slot, yoko.irq.active);
	save_slot_ele(mode, slot, yoko.irq.count);

	return (EXIT_OK);
}
void extcl_cpu_every_cycle_YOKO(void) {
	if (yoko.irq.active) {
		if (--yoko.irq.count == 0) {
			irq.high |= EXT_IRQ;
			yoko.irq.active = 0;
			yoko.irq.count = 0xFFFF;
		}
	}
}

INLINE static void yoko_update(void) {
	WORD value;
	SDBWORD bank;

	if (yoko.mode & 0x01) {
		mirroring_H();
	} else {
		mirroring_V();
	}

	value = yoko.reg[3];
	control_bank(info.chr.rom[0].max.banks_2k)
	bank = value << 11;
	chr.bank_1k[0] = chr_chip_byte_pnt(0, bank);
	chr.bank_1k[1] = chr_chip_byte_pnt(0, bank | 0x0400);
	value = yoko.reg[4];
	control_bank(info.chr.rom[0].max.banks_2k)
	bank = value << 11;
	chr.bank_1k[2] = chr_chip_byte_pnt(0, bank);
	chr.bank_1k[3] = chr_chip_byte_pnt(0, bank | 0x0400);
	value = yoko.reg[5];
	control_bank(info.chr.rom[0].max.banks_2k)
	bank = value << 11;
	chr.bank_1k[4] = chr_chip_byte_pnt(0, bank);
	chr.bank_1k[5] = chr_chip_byte_pnt(0, bank | 0x0400);
	value = yoko.reg[6];
	control_bank(info.chr.rom[0].max.banks_2k)
	bank = value << 11;
	chr.bank_1k[6] = chr_chip_byte_pnt(0, bank);
	chr.bank_1k[7] = chr_chip_byte_pnt(0, bank | 0x0400);

	if (yoko.mode & 0x10) {
		BYTE base = (yoko.bank & 0x08) << 1;

		value = (yoko.reg[0] & 0x0F) | base;
		control_bank(info.prg.rom[0].max.banks_8k)
		map_prg_rom_8k(1, 0, value);
		value = (yoko.reg[1] & 0x0F) | base;
		control_bank(info.prg.rom[0].max.banks_8k)
		map_prg_rom_8k(1, 1, value);
		value = (yoko.reg[2] & 0x0F) | base;
		control_bank(info.prg.rom[0].max.banks_8k)
		map_prg_rom_8k(1, 2, value);
		value = 0x0F | base;
		control_bank(info.prg.rom[0].max.banks_8k)
		map_prg_rom_8k(1, 3, value);
	} else {
		if (yoko.mode & 0x08) {
			value = yoko.bank >> 1;
			control_bank(info.prg.rom[0].max.banks_32k)
			map_prg_rom_8k(4, 0, value);
		} else {
			value = yoko.bank;
			control_bank(info.prg.rom[0].max.banks_16k)
			map_prg_rom_8k(2, 0, value);
			value = 0xFF;
			control_bank(info.prg.rom[0].max.banks_16k)
			map_prg_rom_8k(2, 2, value);
		}
	}
	map_prg_rom_8k_update();
}
