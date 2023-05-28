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
#include "irqA12.h"
#include "save_slot.h"

INLINE static void prg_fix_359(void);
INLINE static void chr_fix_359(void);
INLINE static void wram_fix_359(void);
INLINE static void mirroring_fix_359(void);

struct _m359 {
	BYTE prg[6];
	BYTE chr[10];
	BYTE mirroring;
	struct _m359_irq {
		BYTE mode;
		BYTE auto_enable;
		BYTE enable;
		BYTE reload;
		WORD counter;
	} irq;
} m359;

void map_init_359(void) {
	EXTCL_CPU_WR_MEM(359);
	EXTCL_AFTER_MAPPER_INIT(359);
	EXTCL_CPU_WR_MEM(359);
	EXTCL_SAVE_MAPPER(359);
	EXTCL_CPU_EVERY_CYCLE(359);
	EXTCL_PPU_000_TO_34X(359);
	EXTCL_PPU_000_TO_255(359);
	EXTCL_PPU_256_TO_319(359);
	EXTCL_PPU_320_TO_34X(359);
	EXTCL_UPDATE_R2006(359);
	EXTCL_IRQ_A12_CLOCK(359);
	mapper.internal_struct[0] = (BYTE *)&m359;
	mapper.internal_struct_size[0] = sizeof(m359);

	memset(&irqA12, 0x00, sizeof(irqA12));
	memset(&m359, 0x00, sizeof(m359));

	m359.prg[0] = 0xFC;
	m359.prg[1] = 0xFD;
	m359.prg[2] = 0xFE;
	m359.prg[2] = 0xFF;
	m359.prg[4] = 0x00;
	m359.prg[5] = 0x3F;

	m359.chr[1] = 0x01;
	m359.chr[2] = 0x02;
	m359.chr[3] = 0x03;
	m359.chr[4] = 0x04;
	m359.chr[5] = 0x05;
	m359.chr[6] = 0x06;
	m359.chr[7] = 0x07;
	m359.chr[8] = 0x00;
	m359.chr[9] = 0xFF;

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_after_mapper_init_359(void) {
	prg_fix_359();
	chr_fix_359();
	wram_fix_359();
	mirroring_fix_359();
}
void extcl_cpu_wr_mem_359(WORD address, BYTE value) {
	switch (address & 0xF000) {
		case 0x8000:
			m359.prg[address & 0x03] = value;
			prg_fix_359();
			wram_fix_359();
			break;
		case 0x9000:
			switch (address & 0x03) {
				case 0:
					m359.prg[4] = value;
					prg_fix_359();
					wram_fix_359();
					break;
				case 1:
					switch (value & 0x03) {
						case 0:
							m359.prg[5] = 0x3F;
							break;
						case 1:
							m359.prg[5] = 0x1F;
							break;
						case 2:
							m359.prg[5] = 0x2F;
							break;
						case 3:
							m359.prg[5] = 0x0F;
							break;
					}
					m359.chr[9] = value & 0x40 ? 0xFF : 0x7F;
					prg_fix_359();
					wram_fix_359();
					chr_fix_359();
					break;
				case 2:
					m359.mirroring = value;
					mirroring_fix_359();
					break;
				case 3:
					m359.chr[8] = value;
					chr_fix_359();
					break;
			}
			break;
		case 0xA000:
		case 0xB000:
			m359.chr[((address & 0x1000) >> 10) | (address & 0x03)] = value;
			chr_fix_359();
			break;
		case 0xC000:
			switch (address & 0x03) {
				case 0:
					if (m359.irq.auto_enable) {
						m359.irq.enable = FALSE;
					}
					m359.irq.counter = (m359.irq.counter & 0xFF00) | value;
					break;
				case 1:
					if (m359.irq.auto_enable) {
						m359.irq.enable = TRUE;
					}
					m359.irq.counter = (m359.irq.counter & 0x00FF) | (value << 8);
					m359.irq.reload = TRUE;
					break;
				case 2:
					m359.irq.enable = value & 0x01;
					m359.irq.mode = value & 0x02;
					m359.irq.auto_enable = value & 0x04;
					break;
				case 3:
					m359.irq.enable = value & 0x01;
					break;
			}
			irq.high &= ~EXT_IRQ;
			break;
	}
}
BYTE extcl_save_mapper_359(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m359.prg);
	save_slot_ele(mode, slot, m359.chr);
	save_slot_ele(mode, slot, m359.mirroring);
	save_slot_ele(mode, slot, m359.irq.mode);
	save_slot_ele(mode, slot, m359.irq.auto_enable);
	save_slot_ele(mode, slot, m359.irq.enable);
	save_slot_ele(mode, slot, m359.irq.reload);
	save_slot_ele(mode, slot, m359.irq.counter);

	return (EXIT_OK);
}
void extcl_cpu_every_cycle_359(void) {
	if (m359.irq.enable && !m359.irq.mode) {
		m359.irq.counter--;
		if (!m359.irq.counter) {
			irq.high |= EXT_IRQ;
		}
	}
}
void extcl_ppu_000_to_34x_359(void) {
	irqA12_RS();
}
void extcl_ppu_000_to_255_359(void) {
	if (r2001.visible) {
		irqA12_SB();
	}
}
void extcl_ppu_256_to_319_359(void) {
	irqA12_BS();
}
void extcl_ppu_320_to_34x_359(void) {
	irqA12_SB();
}
void extcl_update_r2006_359(WORD new_r2006, WORD old_r2006) {
	irqA12_IO(new_r2006, old_r2006);
}
void extcl_irq_A12_clock_359(void) {
	BYTE counter;

	if (!m359.irq.mode) {
		return;
	}

	counter = m359.irq.counter & 0x00FF;

	if (!counter) {
		counter = (m359.irq.counter & 0xFF00) >> 8;
		irqA12.reload = FALSE;
	} else {
		counter--;
	}
	if (!counter && m359.irq.enable) {
		irq.high |= EXT_IRQ;
	}
	m359.irq.counter = (m359.irq.counter & 0xFF00) | counter;
}

INLINE static void prg_fix_359(void) {
	WORD base = (m359.prg[4] & 0x38) << 1;
	WORD mask = m359.prg[5];

	memmap_auto_8k(MMCPU(0x8000), (base | (m359.prg[0] & mask)));
	memmap_auto_8k(MMCPU(0xA000), (base | (m359.prg[1] & mask)));
	memmap_auto_8k(MMCPU(0xC000), (base | (m359.prg[2] & mask)));
	memmap_auto_8k(MMCPU(0xE000), (base | (0xFF & mask)));
}
INLINE static void chr_fix_359(void) {
	DBWORD bank;

	if (mapper.write_vram) {
		return;
	}

	if (info.mapper.id == 540) {
		bank = m359.chr[0];
		_control_bank(bank, info.chr.rom.max.banks_2k)
		bank = bank << 11;
		chr.bank_1k[0] = chr_pnt(bank);
		chr.bank_1k[1] = chr_pnt(bank | 0x0400);

		bank = m359.chr[1];
		_control_bank(bank, info.chr.rom.max.banks_2k)
		bank = bank << 11;
		chr.bank_1k[2] = chr_pnt(bank);
		chr.bank_1k[3] = chr_pnt(bank | 0x0400);

		bank = m359.chr[6];
		_control_bank(bank, info.chr.rom.max.banks_2k)
		bank = bank << 11;
		chr.bank_1k[4] = chr_pnt(bank);
		chr.bank_1k[5] = chr_pnt(bank | 0x0400);

		bank = m359.chr[7];
		_control_bank(bank, info.chr.rom.max.banks_2k)
		bank = bank << 11;
		chr.bank_1k[6] = chr_pnt(bank);
		chr.bank_1k[7] = chr_pnt(bank | 0x0400);
	} else {
		WORD outer = m359.chr[8] << 7;
		WORD mask = m359.chr[9];

		bank = outer | (m359.chr[0] & mask);
		_control_bank(bank, info.chr.rom.max.banks_1k)
		bank = bank << 10;
		chr.bank_1k[0] = chr_pnt(bank);

		bank = outer | (m359.chr[1] & mask);
		_control_bank(bank, info.chr.rom.max.banks_1k)
		bank = bank << 10;
		chr.bank_1k[1] = chr_pnt(bank);

		bank = outer | (m359.chr[2] & mask);
		_control_bank(bank, info.chr.rom.max.banks_1k)
		bank = bank << 10;
		chr.bank_1k[2] = chr_pnt(bank);

		bank = outer | (m359.chr[3] & mask);
		_control_bank(bank, info.chr.rom.max.banks_1k)
		bank = bank << 10;
		chr.bank_1k[3] = chr_pnt(bank);

		bank = outer | (m359.chr[4] & mask);
		_control_bank(bank, info.chr.rom.max.banks_1k)
		bank = bank << 10;
		chr.bank_1k[4] = chr_pnt(bank);

		bank = outer | (m359.chr[5] & mask);
		_control_bank(bank, info.chr.rom.max.banks_1k)
		bank = bank << 10;
		chr.bank_1k[5] = chr_pnt(bank);

		bank = outer | (m359.chr[6] & mask);
		_control_bank(bank, info.chr.rom.max.banks_1k)
		bank = bank << 10;
		chr.bank_1k[6] = chr_pnt(bank);

		bank = outer | (m359.chr[7] & mask);
		_control_bank(bank, info.chr.rom.max.banks_1k)
		bank = bank << 10;
		chr.bank_1k[7] = chr_pnt(bank);
	}
}
INLINE static void wram_fix_359(void) {
	WORD base = (m359.prg[4] & 0x38) << 1;
	WORD mask = m359.prg[5];

	memmap_prgrom_8k(MMCPU(0x6000), (base | (m359.prg[3] & mask)));
}
INLINE static void mirroring_fix_359(void) {
	switch (m359.mirroring & 0x03) {
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
}
