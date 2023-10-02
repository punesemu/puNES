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
#include "cpu.h"
#include "ppu.h"
#include "save_slot.h"

INLINE static void prg_fix_547(void);
INLINE static void prg_swap_547(WORD address, WORD value);
INLINE static void chr_fix_547(void);
INLINE static void wram_fix_547(void);
INLINE static void wram_swap_547(WORD address, WORD value);
INLINE static void mirroring_fix_547(void);

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
	EXTCL_RD_CHR(547);
	mapper.internal_struct[0] = (BYTE *)&m547;
	mapper.internal_struct_size[0] = sizeof(m547);

	if (info.reset >= HARD) {
		memset(&m547, 0x00, sizeof(m547));
	}

	info.mapper.extend_rd = TRUE;
}
void extcl_after_mapper_init_547(void) {
	prg_fix_547();
	chr_fix_547();
	wram_fix_547();
	mirroring_fix_547();
}
void extcl_cpu_wr_mem_547(BYTE nidx, WORD address, BYTE value) {
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
				nes[nidx].c.irq.high &= ~EXT_IRQ;
				return;
			case 0xD900:
				m547.irq.after = value & 0x01;
				m547.irq.enable = value & 0x02;
				if (m547.irq.enable) {
					m547.irq.counter = m547.irq.latch;
				}
				nes[nidx].c.irq.high &= ~EXT_IRQ;
				return;
			case 0xDA00:
				mirroring_fix_547();
				return;
			default:
				return;
		}
	}
}
BYTE extcl_cpu_rd_mem_547(BYTE nidx, WORD address, UNUSED(BYTE openbus)) {
	switch (address & 0xF000) {
		case 0xD000:
			if ((address == 0xDC00) || (address == 0xDD00)) {
				BYTE row = m547.reg[13] - 0x20;
				BYTE col = m547.reg[12] - 0x20;

				// "row" and "col" are the first and second 7-bit JIS X 0208 code byte, respectively, each minus the $21 offset.
				if ((row < 0x60) && (col < 0x60)) {
					static const BYTE page_table[0x24] = {
						// JIS X 0208 rows $20-$4F. $20 is not a valid row number.
						0x0, 0x0, 0x2, 0x2, 0x1, 0x1, 0x4, 0x5, 0x6,
						0x7, 0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF,
						// JIS X 0208 rows $50-$7F. $7F is not a valid row number.
						0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8,
						0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF, 0xD, 0xD
					};
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
			return (prgrom_rd(nidx, address));
		default:
			return (address >= 0x8000 ? prgrom_rd(nidx, address) : wram_rd(nidx, address));
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
	return (EXIT_OK);
}
void extcl_cpu_every_cycle_547(BYTE nidx) {
	if (m547.irq.enable) {
		m547.irq.counter++;
		if (!m547.irq.counter) {
			m547.irq.counter = m547.irq.latch;
			nes[nidx].c.irq.high |= EXT_IRQ;
		}
	}
}
BYTE extcl_rd_chr_547(BYTE nidx, WORD address) {
	// controllo di trattare il background
	if (((address & 0xFFF7) == nes[nidx].p.ppu.bck_adr)) {
		if (m547.qt.byte & 0x40) {
			if (address & 0x0008) {
				return (m547.qt.byte & 0x80 ? 0xFF : 0x00);
			} else {
				size_t addr256K = ((m547.qt.byte & 0x3F) << 12) | (address & 0x0FFF);

				if (chrrom_size() == S128K) {
					addr256K = ((addr256K & 0x07) << 1) | ((addr256K & 0x10) >> 4) | ((addr256K & 0x3FFE0) >> 1);
				}
				return (chrrom_byte(addr256K));
			}
		}
		return (vram_byte(nidx, ((m547.qt.byte & 0x01) << 12) | (address & 0x0FFF)));
	}
	return (chr_rd(nidx, address));
}
void extcl_wr_nmt_547(BYTE nidx, WORD address, BYTE value) {
	if (m547.reg[10] & 0x01) {
		BYTE bank = (m547.reg[10] & 0x02)
			? (address & 0x800) ? 1 : 0
			: (address & 0x400) ? 1 : 0;

		m547.qt.ram[(bank << 10) | (address & 0x3FF)] = value;
		return;
	}
	nmt_wr(nidx, address, value);
}
BYTE extcl_rd_nmt_547(BYTE nidx, WORD address) {
	if ((address & 0x03FF) < 0x3C0) {
		BYTE bank = (m547.reg[10] & 0x02)
			? (address & 0x800) ? 1 : 0
			: (address & 0x400) ? 1 : 0;

		m547.qt.byte = m547.qt.ram[(bank << 10) | (address & 0x3FF)];
	}
	return (nmt_rd(nidx, address));
}

INLINE static void prg_fix_547(void) {
	prg_swap_547(0x8000, m547.reg[2]);
	prg_swap_547(0xA000, m547.reg[3]);
	prg_swap_547(0xC000, m547.reg[4]);
	memmap_auto_8k(0, MMCPU(0xE000), 0x4F);
}
INLINE static void prg_swap_547(WORD address, WORD value) {
	WORD bank = value & 0x40 ? (value & 0x3F) + 0x10 : value & 0x0F;

	memmap_auto_8k(0, MMCPU(address),bank);
}
INLINE static void chr_fix_547(void) {
	memmap_vram_4k(0, MMPPU(0x0000), (m547.reg[5] & 0x01));
	memmap_vram_4k(0, MMPPU(0x1000), 1);
}
INLINE static void wram_fix_547(void) {
	wram_swap_547(0x6000, m547.reg[0]);
	wram_swap_547(0x7000, m547.reg[1]);
}
INLINE static void wram_swap_547(WORD address, WORD value) {
	memmap_auto_4k(0, MMCPU(address), (value & 0x01) | ((value & 0x80) >> 2));
}
INLINE static void mirroring_fix_547(void) {
	if (m547.reg[10] & 0x02) {
		mirroring_H(0);
	} else {
		mirroring_V(0);
	}
}
