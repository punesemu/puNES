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

INLINE static void prg_fix_547(void);
INLINE static void prg_swap_547(WORD address, WORD value);
INLINE static void chr_fix_547(void);
INLINE static void wram_fix_547(void);
INLINE static void wram_swap_547(WORD address, WORD value);
INLINE static void mirroring_fix_547(void);

static const BYTE page_table[0x24] ={
	// JIS X 0208 rows $20-$4F. $20 is not a valid row number.
	0x0, 0x0, 0x2, 0x2, 0x1, 0x1, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF,
	// JIS X 0208 rows $50-$7F. $7F is not a valid row number.
	0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF, 0xD, 0xD
};
struct _m547 {
	BYTE reg[16];
	struct _m547_qt {
		BYTE ram[0x800];
		BYTE byte;
	} qt;
	struct __m547_irq {
		BYTE after;
		BYTE enable;
		WORD counter;
		WORD latch;
	} irq;
} m547;

void map_init_547(void) {
	EXTCL_AFTER_MAPPER_INIT(547);
	EXTCL_CPU_WR_MEM(547);
	EXTCL_CPU_RD_MEM(547);
	EXTCL_SAVE_MAPPER(547);
	EXTCL_CPU_EVERY_CYCLE(547);
	EXTCL_WR_NMT(547);
	EXTCL_RD_NMT(547);
	EXTCL_WR_CHR(547);
	EXTCL_RD_CHR(547);
	mapper.internal_struct[0] = (BYTE *)&m547;
	mapper.internal_struct_size[0] = sizeof(m547);

	memset(&m547, 0x00, sizeof(m547));

	if (info.format != NES_2_0) {
		info.chr.ram.banks_8k_plus = 1;
	}

	info.mapper.extend_rd = TRUE;
}
void extcl_after_mapper_init_547(void) {
	prg_fix_547();
	chr_fix_547();
	wram_fix_547();
	mirroring_fix_547();
}
void extcl_cpu_wr_mem_547(WORD address, BYTE value) {
	if ((address & 0xF000) == 0xD000) {
		BYTE reg = (address & 0x0F00) >> 8;

		m547.reg[reg] = value;

		switch (address & 0xFF00) {
			case 0xD000:
			case 0xD100:
				wram_fix_547();
				return;
			case 0xD200:
			case 0xD300:
			case 0xD400:
				prg_fix_547();
				return;
			case 0xD500:
				chr_fix_547();
				return;
			case 0xD600:
				m547.irq.latch = (m547.irq.latch & 0xFF00) | value;
				return;
			case 0xD700:
				m547.irq.latch = (m547.irq.latch & 0x00FF) | (value << 8);
				return;
			case 0xD800:
				m547.irq.enable = m547.irq.after;
				irq.high &= ~EXT_IRQ;
				return;
			case 0xD900:
				m547.irq.after = value & 0x01;
				m547.irq.enable = value & 0x02;
				if (m547.irq.enable) {
					m547.irq.counter = m547.irq.latch;
				}
				irq.high &= ~EXT_IRQ;
				return;
			case 0xDA00:
				mirroring_fix_547();
				return;
			default:
				return;
		}
	}
}
BYTE extcl_cpu_rd_mem_547(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	switch (address & 0xF000) {
		case 0xD000:
			if ((address == 0xDC00) || (address == 0xDD00)) {
				BYTE row = m547.reg[13] - 0x20;
				BYTE col = m547.reg[12] - 0x20;

				// "row" and "col" are the first and second 7-bit JIS X 0208 code byte, respectively, each minus the $21 offset.
				if ((row < 0x60) && (col < 0x60)) {
					WORD code =
						(col % 32) +              // First, go through 32 columns of a column-third.
						(row % 16) * 32 +         // Then, through 16 rows of a row-third.
						(col / 32) * 32 * 16 +    // Then, through three column-thirds.
						(row / 16) * 32 * 16 * 3; // Finally, through three row-thirds.
					WORD glyph = (code & 0xFF) | (page_table[code >> 8] << 8);
					DBWORD tile = glyph * 4;      // four tiles per glyph

					if (address == 0xDC00) {
						// tile number
						return ((tile & 0xFF) | (m547.reg[11] & 0x03));
					} else {
						// bank byte
						return ((tile >> 8) | (m547.reg[11] & 0x04 ? 0x80 : 0x00) | 0x40);
					}
				} else {
					return (0);
				}
			}
			return (openbus);
		default:
			return (openbus);
	}
}
BYTE extcl_save_mapper_547(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m547.reg);
	save_slot_ele(mode, slot, m547.qt.ram);
	save_slot_ele(mode, slot, m547.qt.byte);
	save_slot_ele(mode, slot, m547.irq.after);
	save_slot_ele(mode, slot, m547.irq.enable);
	save_slot_ele(mode, slot, m547.irq.counter);
	save_slot_ele(mode, slot, m547.irq.latch);

	if (mode == SAVE_SLOT_READ) {
		wram_fix_547();
		chr_fix_547();
	}

	return (EXIT_OK);
}
void extcl_cpu_every_cycle_547(void) {
	if (m547.irq.enable) {
		m547.irq.counter++;
		if (!m547.irq.counter) {
			m547.irq.counter = m547.irq.latch;
			irq.high |= EXT_IRQ;
		}
	}
}
BYTE extcl_wr_nmt_547(WORD address, BYTE value) {
	if (m547.reg[10] & 0x01) {
		BYTE bank = mapper.mirroring == MIRRORING_HORIZONTAL ? (address & 0x800) ? 1 : 0 : (address & 0x400) ? 1 : 0;

		m547.qt.ram[(bank << 10) | (address & 0x3FF)] = value;
		return (TRUE);
	}
	return (FALSE);
}
BYTE extcl_rd_nmt_547(WORD address) {
	address &= 0x0FFF;
	if ((address & 0x03FF) < 0x3C0) {
		BYTE bank = mapper.mirroring == MIRRORING_HORIZONTAL ? (address & 0x800) ? 1 : 0 : (address & 0x400) ? 1 : 0;

		m547.qt.byte = m547.qt.ram[(bank << 10) | (address & 0x3FF)];
	}
	return (ntbl.bank_1k[address >> 10][address & 0x3FF]);
}
void extcl_wr_chr_547(WORD address, BYTE value) {
	chr.bank_1k[address >> 10][address & 0x3FF] = value;
}
BYTE extcl_rd_chr_547(WORD address) {
	// controllo di trattare il background
	if (((address & 0xFFF7) == ppu.bck_adr)) {
		if (m547.qt.byte & 0x40) {
			if (address & 0x0008) {
				return (m547.qt.byte & 0x80 ? 0xFF : 0x00);
			} else {
				DBWORD addr256K = ((m547.qt.byte & 0x3F) << 12) | (address & 0x0FFF);

				if (chr_size() == (size_t)(128 * 1024)) {
					addr256K = ((addr256K & 0x00007) << 1) | ((addr256K & 0x00010) >> 4) | ((addr256K & 0x3FFE0) >> 1);
				}
				return (chr_rom()[addr256K]);
			}
		}
		return (chr.extra.data[((m547.qt.byte & 0x01) << 12) | (address & 0x0FFF)]);
	}
	return (chr.bank_1k[address >> 10][address & 0x3FF]);
}

INLINE static void prg_fix_547(void) {
	WORD value = 0;

	prg_swap_547(0x8000, m547.reg[2]);
	prg_swap_547(0xA000, m547.reg[3]);
	prg_swap_547(0xC000, m547.reg[4]);

	value = 0x4F;
	control_bank(info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, 3, value);

	map_prg_rom_8k_update();
}
INLINE static void prg_swap_547(WORD address, WORD value) {
	value = value & 0x40 ? (value & 0x3F) + 0x10 : value & 0x0F;
	control_bank(info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, (address >> 13) & 0x03, value);
}
INLINE static void chr_fix_547(void) {
	DBWORD value;

	value = m547.reg[5] & 0x01;
	control_bank(info.chr.ram.max.banks_4k)
	value <<= 12;
	chr.bank_1k[0] = &chr.extra.data[value];
	chr.bank_1k[1] = &chr.extra.data[value | 0x400];
	chr.bank_1k[2] = &chr.extra.data[value | 0x800];
	chr.bank_1k[3] = &chr.extra.data[value | 0xC00];

	value = 1;
	control_bank(info.chr.ram.max.banks_4k)
	value <<= 12;
	chr.bank_1k[4] = &chr.extra.data[value];
	chr.bank_1k[5] = &chr.extra.data[value | 0x400];
	chr.bank_1k[6] = &chr.extra.data[value | 0x800];
	chr.bank_1k[7] = &chr.extra.data[value | 0xC00];
}
INLINE static void wram_fix_547(void) {
	wram_swap_547(0x6000, m547.reg[0]);
	wram_swap_547(0x7000, m547.reg[1]);
}
INLINE static void wram_swap_547(WORD address, WORD value) {
	wram_map_auto_4k(address, (value & 0x01) | ((value & 0x80) >> 2));
}
INLINE static void mirroring_fix_547(void) {
	if (m547.reg[10] & 0x02) {
		mirroring_H();
	} else {
		mirroring_V();
	}
}
