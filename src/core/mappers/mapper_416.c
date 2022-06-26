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

INLINE static void prg_fix_416(void);
INLINE static void chr_fix_416(void);
INLINE static void mirroring_fix_416(void);

struct _m416 {
	BYTE reg[2];
	struct _m416_irq {
		BYTE enable;
		WORD counter;
	} irq;
} m416;
struct _m416tmp {
	BYTE *prg_6000;
} m416tmp;

void map_init_416(void) {
	EXTCL_AFTER_MAPPER_INIT(416);
	EXTCL_CPU_WR_MEM(416);
	EXTCL_CPU_RD_MEM(416);
	EXTCL_SAVE_MAPPER(416);
	EXTCL_CPU_EVERY_CYCLE(416);
	mapper.internal_struct[0] = (BYTE *)&m416;
	mapper.internal_struct_size[0] = sizeof(m416);

	if (info.reset >= HARD) {
		memset(&m416, 0x00, sizeof(m416));
	} else {
		m416.reg[0] = 0;
		m416.irq.enable = 0;
		m416.irq.counter = 0;
	}

	info.mapper.extend_wr = info.mapper.extend_rd = TRUE;
}
void extcl_after_mapper_init_416(void) {
	prg_fix_416();
	chr_fix_416();
	mirroring_fix_416();
}
void extcl_cpu_wr_mem_416(WORD address, BYTE value) {
	switch (address & 0xF000) {
		case 0x4000:
		case 0x5000:
			if (address & 0x0020) {
				if (address & 0x100) {
					m416.irq.enable = value & 0x01;
				} else {
					m416.reg[0] = value;
					prg_fix_416();
				}
			}
			break;
		case 0x8000:
		case 0x9000:
			m416.reg[1] = value;
			prg_fix_416();
			chr_fix_416();
			mirroring_fix_416();
			break;
	}
}
BYTE extcl_cpu_rd_mem_416(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	if ((address >= 0x6000) && (address <= 0x7FFF)) {
		return (m416tmp.prg_6000[address & 0x1FFF]);
	}
	return (openbus);
}
BYTE extcl_save_mapper_416(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m416.reg);
	save_slot_ele(mode, slot, m416.irq.enable);
	save_slot_ele(mode, slot, m416.irq.counter);

	if (mode == SAVE_SLOT_READ) {
		prg_fix_416();
	}

	return (EXIT_OK);
}
void extcl_cpu_every_cycle_416(void) {
	if (m416.irq.enable && (++m416.irq.counter == 0x1000)) {
		irq.high |= EXT_IRQ;
		return;
	}
	m416.irq.counter = 0;
	irq.high &= ~EXT_IRQ;
}

INLINE static void prg_fix_416(void) {
	WORD bank;

	bank = 0x07;
	_control_bank(bank, info.prg.rom.max.banks_8k)
	m416tmp.prg_6000 = prg_pnt(bank << 13);

	if (m416.reg[1] & 0x08) {
		bank = ((m416.reg[1] & 0x08) >> 1) | ((m416.reg[1] & 0x80) >> 6) | ((m416.reg[1] & 0x20) >> 5);

		if (m416.reg[1] & 0x80) {
			bank >>= 1;
			_control_bank(bank, info.prg.rom.max.banks_32k)
			map_prg_rom_8k(4, 0, bank);
		} else if (m416.reg[1] & 0x40) {
			_control_bank(bank, info.prg.rom.max.banks_16k)
			map_prg_rom_8k(2, 0, bank);
			map_prg_rom_8k(2, 2, bank);
		} else {
			bank <<= 1;
			_control_bank(bank, info.prg.rom.max.banks_8k)
			map_prg_rom_8k(1, 0, bank);
			map_prg_rom_8k(1, 1, bank);
			map_prg_rom_8k(1, 2, bank);
			map_prg_rom_8k(1, 3, bank);
		}
	} else {
		bank = 0;
		_control_bank(bank, info.prg.rom.max.banks_8k)
		map_prg_rom_8k(1, 0, bank);

		bank = 1;
		_control_bank(bank, info.prg.rom.max.banks_8k)
		map_prg_rom_8k(1, 1, bank);

		bank = (m416.reg[0] & 0x08) | ((m416.reg[0] & 0x01) << 2) | ((m416.reg[0] & 0x06) >> 1);
		_control_bank(bank, info.prg.rom.max.banks_8k)
		map_prg_rom_8k(1, 2, bank);

		bank = 3;
		_control_bank(bank, info.prg.rom.max.banks_8k)
		map_prg_rom_8k(1, 3, bank);
	}

	map_prg_rom_8k_update();
}
INLINE static void chr_fix_416(void) {
	DBWORD bank;

	bank = (m416.reg[1] & 0x06) >> 1;
	_control_bank(bank, info.chr.rom.max.banks_8k)
	bank <<= 13;
	chr.bank_1k[0] = chr_pnt(bank);
	chr.bank_1k[1] = chr_pnt(bank | 0x0400);
	chr.bank_1k[2] = chr_pnt(bank | 0x0800);
	chr.bank_1k[3] = chr_pnt(bank | 0x0C00);
	chr.bank_1k[4] = chr_pnt(bank | 0x1000);
	chr.bank_1k[5] = chr_pnt(bank | 0x1400);
	chr.bank_1k[6] = chr_pnt(bank | 0x1800);
	chr.bank_1k[7] = chr_pnt(bank | 0x1C00);
}
INLINE static void mirroring_fix_416(void) {
	if (m416.reg[1] & 0x04) {
		mirroring_H();
	} else {
		mirroring_V();
	}
}
