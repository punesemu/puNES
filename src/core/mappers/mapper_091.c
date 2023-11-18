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
#include "save_slot.h"

INLINE static void prg_fix_091(void);
INLINE static void chr_fix_091(void);
INLINE static void mirroring_fix_091(void);

struct _m091 {
	BYTE prg[2];
	BYTE chr[4];
	BYTE mirroring;
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
void extcl_cpu_wr_mem_091(BYTE nidx, WORD address, BYTE value) {
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
						m091.mirroring = TRUE;
						mirroring_fix_091();
						break;
					case 5:
						m091.mirroring = FALSE;
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
					nes[nidx].c.irq.high &= ~EXT_IRQ;
					return;
				case 3:
					m091.irq.enable = TRUE;
					m091.irq.cpu.prescaler = 3;
					nes[nidx].c.irq.high &= ~EXT_IRQ;
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
	save_slot_ele(mode, slot, m091.mirroring);
	save_slot_ele(mode, slot, m091.reg);
	save_slot_ele(mode, slot, m091.irq.enable);
	save_slot_ele(mode, slot, m091.irq.ppu.counter);
	save_slot_ele(mode, slot, m091.irq.cpu.prescaler);
	save_slot_ele(mode, slot, m091.irq.cpu.counter);
	return (EXIT_OK);
}
void extcl_ppu_256_to_319_091(BYTE nidx) {
	if (nes[nidx].p.ppu.frame_x != 319) {
		return;
	}
	if (m091.irq.enable && (m091.irq.ppu.counter < 8)) {
		m091.irq.ppu.counter++;
		if (m091.irq.ppu.counter >= 8) {
			nes[nidx].c.irq.high |= EXT_IRQ;
		}
	}
}
void extcl_cpu_every_cycle_091(BYTE nidx) {
	m091.irq.cpu.prescaler = (m091.irq.cpu.prescaler + 1) & 0x03;
	if (!m091.irq.cpu.prescaler) {
		m091.irq.cpu.counter -= 5;
		if ((m091.irq.cpu.counter <= 0) && m091.irq.enable) {
			nes[nidx].c.irq.high |= EXT_IRQ;
		}
	}
}

INLINE static void prg_fix_091(void) {
	WORD mask = 0x0F;
	WORD base = (m091.reg << 3) & ~mask;

	memmap_auto_8k(0, MMCPU(0x8000), (base | (m091.prg[0] & mask)));
	memmap_auto_8k(0, MMCPU(0xA000), (base | (m091.prg[1] & mask)));
	memmap_auto_8k(0, MMCPU(0xC000), (base | (0xFE & mask)));
	memmap_auto_8k(0, MMCPU(0xE000), (base | (0xFF & mask)));
}
INLINE static void chr_fix_091(void) {
	WORD base = (m091.reg & 0x01) << 8;

	memmap_auto_2k(0, MMPPU(0x0000), (base | m091.chr[0]));
	memmap_auto_2k(0, MMPPU(0x0800), (base | m091.chr[1]));
	memmap_auto_2k(0, MMPPU(0x1000), (base | m091.chr[2]));
	memmap_auto_2k(0, MMPPU(0x1800), (base | m091.chr[3]));
}
INLINE static void mirroring_fix_091(void) {
	if (m091.mirroring) {
		mirroring_H(0);
	} else {
		mirroring_V(0);
	}
}
