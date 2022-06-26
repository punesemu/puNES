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
#include "info.h"
#include "mem_map.h"
#include "cpu.h"
#include "ppu.h"
#include "save_slot.h"

INLINE static void prg_fix_91(void);
INLINE static void chr_fix_91(void);
INLINE static void mirroring_fix_91(void);

struct _m91 {
	BYTE prg[2];
	BYTE chr[4];
	BYTE mir;
	BYTE reg;
	struct _m91_irq {
		BYTE enable;
		struct _m91_irq_ppu {
			BYTE counter;
		} ppu;
		struct _m91_irq_cpu {
			BYTE prescaler;
			SWORD counter;
		} cpu;
	} irq;
} m91;

void map_init_91(void) {
	EXTCL_AFTER_MAPPER_INIT(91);
	EXTCL_CPU_WR_MEM(91);
	EXTCL_SAVE_MAPPER(91);
	if (info.mapper.submapper == 1) {
		EXTCL_CPU_EVERY_CYCLE(91);
	} else {
		EXTCL_PPU_256_TO_319(91);
	}
	mapper.internal_struct[0] = (BYTE *)&m91;
	mapper.internal_struct_size[0] = sizeof(m91);

	memset(&m91, 0x00, sizeof(m91));

	info.mapper.extend_wr = TRUE;
}
void extcl_after_mapper_init_91(void) {
	prg_fix_91();
	chr_fix_91();
	if (info.mapper.submapper == 1) {
		mirroring_fix_91();
	}
}
void extcl_cpu_wr_mem_91(WORD address, BYTE value) {
	switch (address & 0xF000) {
		case 0x6000:
			if (info.mapper.submapper == 1) {
				switch (address & 0x07) {
					case 0:
					case 1:
					case 2:
					case 3:
						m91.chr[address & 0x03] = value;
						chr_fix_91();
						break;
					case 4:
						m91.mir = TRUE;
						mirroring_fix_91();
						break;
					case 5:
						m91.mir = FALSE;
						mirroring_fix_91();
						break;
					case 6:
						m91.irq.cpu.counter = (m91.irq.cpu.counter & 0xFF00) | value;
						break;
					case 7:
						m91.irq.cpu.counter = (m91.irq.cpu.counter & 0x00FF) | (value << 8);
						break;
				}
			} else {
				m91.chr[address & 0x03] = value;
				chr_fix_91();
			}
			return;;
		case 0x7000:
			switch (address & 0x0003) {
				case 0:
				case 1:
					m91.prg[address & 0x01] = value;
					prg_fix_91();
					return;
				case 2:
					m91.irq.enable = FALSE;
					m91.irq.ppu.counter = 0;
					irq.high &= ~EXT_IRQ;
					return;
				case 3:
					m91.irq.enable = TRUE;
					m91.irq.cpu.prescaler = 3;
					irq.high &= ~EXT_IRQ;
					return;
			}
			break;
		case 0x8000:
		case 0x9000:
			m91.reg = address & 0xFF;
			prg_fix_91();
			break;

	}
}
BYTE extcl_save_mapper_91(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m91.prg);
	save_slot_ele(mode, slot, m91.chr);
	save_slot_ele(mode, slot, m91.mir);
	save_slot_ele(mode, slot, m91.reg);
	save_slot_ele(mode, slot, m91.irq.enable);
	save_slot_ele(mode, slot, m91.irq.ppu.counter);
	save_slot_ele(mode, slot, m91.irq.cpu.prescaler);
	save_slot_ele(mode, slot, m91.irq.cpu.counter);

	return (EXIT_OK);
}
void extcl_ppu_256_to_319_91(void) {
	if (ppu.frame_x != 319) {
		return;
	}

	if (m91.irq.enable && (m91.irq.ppu.counter < 8)) {
		m91.irq.ppu.counter++;
		if (m91.irq.ppu.counter >= 8) {
			irq.high |= EXT_IRQ;
		}
	}
}
void extcl_cpu_every_cycle_91(void) {
	m91.irq.cpu.prescaler = (m91.irq.cpu.prescaler + 1) & 0x03;
	if (!m91.irq.cpu.prescaler) {
		m91.irq.cpu.counter -= 5;
		if ((m91.irq.cpu.counter <= 0) && m91.irq.enable) {
			irq.high |= EXT_IRQ;
		}
	}
}

INLINE static void prg_fix_91(void) {
	WORD base = m91.reg << 3;
	WORD mask = 0x0F;
	WORD bank;

	base &= ~mask;

	bank = base | (m91.prg[0] & mask);
	_control_bank(bank, info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, 0, bank);

	bank = base | (m91.prg[1] & mask);
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
INLINE static void chr_fix_91(void) {
	DBWORD bank;
	WORD base = (m91.reg & 0x01) << 8;

	bank = base | m91.chr[0];
	_control_bank(bank, info.chr.rom.max.banks_2k)
	bank <<= 11;
	chr.bank_1k[0] = chr_pnt(bank);
	chr.bank_1k[1] = chr_pnt(bank | 0x400);

	bank = base | m91.chr[1];
	_control_bank(bank, info.chr.rom.max.banks_2k)
	bank <<= 11;
	chr.bank_1k[2] = chr_pnt(bank);
	chr.bank_1k[3] = chr_pnt(bank | 0x400);

	bank = base | m91.chr[2];
	_control_bank(bank, info.chr.rom.max.banks_2k)
	bank <<= 11;
	chr.bank_1k[4] = chr_pnt(bank);
	chr.bank_1k[5] = chr_pnt(bank | 0x400);

	bank = base | m91.chr[3];
	_control_bank(bank, info.chr.rom.max.banks_2k)
	bank <<= 11;
	chr.bank_1k[6] = chr_pnt(bank);
	chr.bank_1k[7] = chr_pnt(bank | 0x400);
}
INLINE static void mirroring_fix_91(void) {
	if (m91.mir) {
		mirroring_H();
	} else {
		mirroring_V();
	}
}
