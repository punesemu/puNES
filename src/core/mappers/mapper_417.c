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
#include "cpu.h"
#include "save_slot.h"

INLINE static void prg_fix_417(void);
INLINE static void chr_fix_417(void);
INLINE static void mirroring_fix_417(void);

struct _m417 {
	BYTE prg[3];
	BYTE chr[8];
	BYTE mir[4];
	struct _m417_irq {
		BYTE enable;
		WORD counter;
	} irq;
} m417;

void map_init_417(void) {
	EXTCL_AFTER_MAPPER_INIT(417);
	EXTCL_CPU_WR_MEM(417);
	EXTCL_SAVE_MAPPER(417);
	EXTCL_CPU_EVERY_CYCLE(417);
	mapper.internal_struct[0] = (BYTE *)&m417;
	mapper.internal_struct_size[0] = sizeof(m417);

	memset(&m417, 0x00, sizeof(m417));
}
void extcl_after_mapper_init_417(void) {
	prg_fix_417();
	chr_fix_417();
	mirroring_fix_417();
}
void extcl_cpu_wr_mem_417(WORD address, BYTE value) {
	switch (address & 0x8073) {
		case 0x8000:
		case 0x8001:
		case 0x8002:
			m417.prg[address & 0x03] = value;
			prg_fix_417();
			break;
		case 0x8010:
		case 0x8011:
		case 0x8012:
		case 0x8013:
			m417.chr[address & 0x03] = value;
			chr_fix_417();
			break;
		case 0x8020:
		case 0x8021:
		case 0x8022:
		case 0x8023:
			m417.chr[0x04 | (address & 0x03)] = value;
			chr_fix_417();
			break;
		case 0x8030:
			m417.irq.enable = TRUE;
			m417.irq.counter = 0;
			break;
		case 0x8040:
			m417.irq.enable = FALSE;
			irq.high &= ~EXT_IRQ;
			break;
		case 0x8050:
		case 0x8051:
		case 0x8052:
		case 0x8053:
			m417.mir[address & 0x03] = value;
			mirroring_fix_417();
			break;
	}
}
BYTE extcl_save_mapper_417(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m417.prg);
	save_slot_ele(mode, slot, m417.chr);
	save_slot_ele(mode, slot, m417.mir);
	save_slot_ele(mode, slot, m417.irq.enable);
	save_slot_ele(mode, slot, m417.irq.counter);

	if (mode == SAVE_SLOT_READ) {
		mirroring_fix_417();
	}

	return (EXIT_OK);
}
void extcl_cpu_every_cycle_417(void) {
	if ((++m417.irq.counter & 0x400) && m417.irq.enable) {
		m417.irq.counter = 0;
		irq.high |= EXT_IRQ;
	}
}

INLINE static void prg_fix_417(void) {
	WORD bank;

	bank = m417.prg[0];
	_control_bank(bank, info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, 0, bank);

	bank = m417.prg[1];
	_control_bank(bank, info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, 1, bank);

	bank = m417.prg[2];
	_control_bank(bank, info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, 2, bank);

	bank = 0xFF;
	_control_bank(bank, info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, 3, bank);

	map_prg_rom_8k_update();
}
INLINE static void chr_fix_417(void) {
	DBWORD bank;

	bank = m417.chr[0];
	_control_bank(bank, info.chr.rom.max.banks_1k)
	bank <<= 10;
	chr.bank_1k[0] = chr_pnt(bank);

	bank = m417.chr[1];
	_control_bank(bank, info.chr.rom.max.banks_1k)
	bank <<= 10;
	chr.bank_1k[1] = chr_pnt(bank);

	bank = m417.chr[2];
	_control_bank(bank, info.chr.rom.max.banks_1k)
	bank <<= 10;
	chr.bank_1k[2] = chr_pnt(bank);

	bank = m417.chr[3];
	_control_bank(bank, info.chr.rom.max.banks_1k)
	bank <<= 10;
	chr.bank_1k[3] = chr_pnt(bank);

	bank = m417.chr[4];
	_control_bank(bank, info.chr.rom.max.banks_1k)
	bank <<= 10;
	chr.bank_1k[4] = chr_pnt(bank);

	bank = m417.chr[5];
	_control_bank(bank, info.chr.rom.max.banks_1k)
	bank <<= 10;
	chr.bank_1k[5] = chr_pnt(bank);

	bank = m417.chr[6];
	_control_bank(bank, info.chr.rom.max.banks_1k)
	bank <<= 10;
	chr.bank_1k[6] = chr_pnt(bank);

	bank = m417.chr[7];
	_control_bank(bank, info.chr.rom.max.banks_1k)
	bank <<= 10;
	chr.bank_1k[7] = chr_pnt(bank);
}
INLINE static void mirroring_fix_417(void) {
	WORD bank;

	bank = m417.mir[0] & 0x01;
	ntbl.bank_1k[0] = &ntbl.data[bank << 10];

	bank = m417.mir[1] & 0x01;
	ntbl.bank_1k[1] = &ntbl.data[bank << 10];

	bank = m417.mir[2] & 0x01;
	ntbl.bank_1k[2] = &ntbl.data[bank << 10];

	bank = m417.mir[3] & 0x01;
	ntbl.bank_1k[3] = &ntbl.data[bank << 10];
}
