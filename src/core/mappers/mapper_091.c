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
#include "ppu.h"
#include "save_slot.h"

INLINE static void prg_fix_091(void);
INLINE static void chr_fix_091(void);
INLINE static void mirroring_fix_091(void);

struct _m091 {
	BYTE prg[2];
	BYTE chr[4];
	BYTE mir;
	BYTE reg;
	struct _m091_irq {
		BYTE enable;
		struct _m091_irq_ppu {
			BYTE counter;
		} ppu;
		struct _m091_irq_cpu {
			BYTE prescaler;
			SWORD counter;
		} cpu;
	} irq;
} m091;

void map_init_091(void) {
	EXTCL_AFTER_MAPPER_INIT(091);
	EXTCL_CPU_WR_MEM(091);
	EXTCL_SAVE_MAPPER(091);
	if (info.mapper.submapper == 1) {
		EXTCL_CPU_EVERY_CYCLE(091);
	} else {
		EXTCL_PPU_256_TO_319(091);
	}
	mapper.internal_struct[0] = (BYTE *)&m091;
	mapper.internal_struct_size[0] = sizeof(m091);

	memset(&m091, 0x00, sizeof(m091));

	info.mapper.extend_wr = TRUE;
}
void extcl_after_mapper_init_091(void) {
	prg_fix_091();
	chr_fix_091();
	if (info.mapper.submapper == 1) {
		mirroring_fix_091();
	}
}
void extcl_cpu_wr_mem_091(WORD address, BYTE value) {
	switch (address & 0xF000) {
		case 0x6000:
			if (info.mapper.submapper == 1) {
				switch (address & 0x07) {
					case 0:
					case 1:
					case 2:
					case 3:
						m091.chr[address & 0x03] = value;
						chr_fix_091();
						break;
					case 4:
						m091.mir = TRUE;
						mirroring_fix_091();
						break;
					case 5:
						m091.mir = FALSE;
						mirroring_fix_091();
						break;
					case 6:
						m091.irq.cpu.counter = (m091.irq.cpu.counter & 0xFF00) | value;
						break;
					case 7:
						m091.irq.cpu.counter = (m091.irq.cpu.counter & 0x00FF) | (value << 8);
						break;
				}
			} else {
				m091.chr[address & 0x03] = value;
				chr_fix_091();
			}
			return;;
		case 0x7000:
			switch (address & 0x0003) {
				case 0:
				case 1:
					m091.prg[address & 0x01] = value;
					prg_fix_091();
					return;
				case 2:
					m091.irq.enable = FALSE;
					m091.irq.ppu.counter = 0;
					irq.high &= ~EXT_IRQ;
					return;
				case 3:
					m091.irq.enable = TRUE;
					m091.irq.cpu.prescaler = 3;
					irq.high &= ~EXT_IRQ;
					return;
			}
			break;
		case 0x8000:
		case 0x9000:
			m091.reg = address & 0xFF;
			prg_fix_091();
			break;

	}
}
BYTE extcl_save_mapper_091(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m091.prg);
	save_slot_ele(mode, slot, m091.chr);
	save_slot_ele(mode, slot, m091.mir);
	save_slot_ele(mode, slot, m091.reg);
	save_slot_ele(mode, slot, m091.irq.enable);
	save_slot_ele(mode, slot, m091.irq.ppu.counter);
	save_slot_ele(mode, slot, m091.irq.cpu.prescaler);
	save_slot_ele(mode, slot, m091.irq.cpu.counter);

	return (EXIT_OK);
}
void extcl_ppu_256_to_319_091(void) {
	if (ppu.frame_x != 319) {
		return;
	}

	if (m091.irq.enable && (m091.irq.ppu.counter < 8)) {
		m091.irq.ppu.counter++;
		if (m091.irq.ppu.counter >= 8) {
			irq.high |= EXT_IRQ;
		}
	}
}
void extcl_cpu_every_cycle_091(void) {
	m091.irq.cpu.prescaler = (m091.irq.cpu.prescaler + 1) & 0x03;
	if (!m091.irq.cpu.prescaler) {
		m091.irq.cpu.counter -= 5;
		if ((m091.irq.cpu.counter <= 0) && m091.irq.enable) {
			irq.high |= EXT_IRQ;
		}
	}
}

INLINE static void prg_fix_091(void) {
	WORD base = m091.reg << 3;
	WORD mask = 0x0F;
	WORD bank;

	base &= ~mask;

	bank = base | (m091.prg[0] & mask);
	_control_bank(bank, info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, 0, bank);

	bank = base | (m091.prg[1] & mask);
	_control_bank(bank, info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, 1, bank);

	bank = base | (0xFE & mask);
	_control_bank(bank, info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, 2, bank);

	bank = base | (0xFF & mask);
	_control_bank(bank, info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, 3, bank);

	map_prg_rom_8k_update();
}
INLINE static void chr_fix_091(void) {
	DBWORD bank;
	WORD base = (m091.reg & 0x01) << 8;

	bank = base | m091.chr[0];
	_control_bank(bank, info.chr.rom.max.banks_2k)
	bank <<= 11;
	chr.bank_1k[0] = chr_pnt(bank);
	chr.bank_1k[1] = chr_pnt(bank | 0x400);

	bank = base | m091.chr[1];
	_control_bank(bank, info.chr.rom.max.banks_2k)
	bank <<= 11;
	chr.bank_1k[2] = chr_pnt(bank);
	chr.bank_1k[3] = chr_pnt(bank | 0x400);

	bank = base | m091.chr[2];
	_control_bank(bank, info.chr.rom.max.banks_2k)
	bank <<= 11;
	chr.bank_1k[4] = chr_pnt(bank);
	chr.bank_1k[5] = chr_pnt(bank | 0x400);

	bank = base | m091.chr[3];
	_control_bank(bank, info.chr.rom.max.banks_2k)
	bank <<= 11;
	chr.bank_1k[6] = chr_pnt(bank);
	chr.bank_1k[7] = chr_pnt(bank | 0x400);
}
INLINE static void mirroring_fix_091(void) {
	if (m091.mir) {
		mirroring_H();
	} else {
		mirroring_V();
	}
}
