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

INLINE static void prg_fix_082(void);
INLINE static void chr_fix_082(void);
INLINE static void wram_fix_082(void);
INLINE static void mirroring_fix_082(void);

INLINE static BYTE prg_reg_control_082(BYTE value);

struct _m082 {
	BYTE prg[3];
	BYTE chr[6];
	BYTE control;
	BYTE wram_enable[3];
	struct m082_irq {
		BYTE latch;
		BYTE control;
		WORD counter;
	} irq;
} m082;

void map_init_082(void) {
	EXTCL_AFTER_MAPPER_INIT(082);
	EXTCL_CPU_WR_MEM(082);
	EXTCL_SAVE_MAPPER(082);
	EXTCL_CPU_EVERY_CYCLE(082);
	mapper.internal_struct[0] = (BYTE *)&m082;
	mapper.internal_struct_size[0] = sizeof(m082);

	if ((info.reset == CHANGE_ROM) || (info.reset == POWER_UP)) {
		memmap_wram_region_init(0, S1K);
		if (info.mapper.battery) {
			wram_set_ram_size(0);
			wram_set_nvram_size(S4K + S1K);
		}
	}

	if (info.reset >= HARD) {
		memset(&m082, 0x00, sizeof(m082));

		m082.prg[1] = 0x01;
		m082.prg[2] = 0xFE;
	}

	info.mapper.extend_wr = TRUE;
}
void extcl_after_mapper_init_082(void) {
	prg_fix_082();
	chr_fix_082();
	wram_fix_082();
	mirroring_fix_082();
}
void extcl_cpu_wr_mem_082(BYTE nidx, WORD address, BYTE value) {
	if ((address >= 0x7EF0) && (address <= 0x7EFF)) {
		switch (address) {
			case 0x7EF0:
			case 0x7EF1:
			case 0x7EF2:
			case 0x7EF3:
			case 0x7EF4:
			case 0x7EF5:
				m082.chr[address & 0x07] = value;
				chr_fix_082();
				return;
			case 0x7EF6:
				m082.control = value;
				chr_fix_082();
				mirroring_fix_082();
				return;
			case 0x7EF7:
			case 0x7EF8:
			case 0x7EF9:
				m082.wram_enable[address - 0x7EF7] = value;
				wram_fix_082();
				return;
			case 0x7EFA:
			case 0x7EFB:
			case 0x7EFC:
				m082.prg[address - 0x7EFA] = value;
				prg_fix_082();
				return;
			case 0x7EFD:
				m082.irq.latch = value;
				return;
			case 0x7EFE:
				m082.irq.control = value;
				return;
			case 0x7EFF:
				m082.irq.counter = m082.irq.latch ? (m082.irq.latch + 1) * 16 : 1;
				nes[nidx].c.irq.high &= ~EXT_IRQ;
				return;
		}
	}
}

BYTE extcl_save_mapper_082(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m082.prg);
	save_slot_ele(mode, slot, m082.chr);
	save_slot_ele(mode, slot, m082.control);
	save_slot_ele(mode, slot, m082.wram_enable);
	save_slot_ele(mode, slot, m082.irq.latch);
	save_slot_ele(mode, slot, m082.irq.control);
	save_slot_ele(mode, slot, m082.irq.counter);
	return (EXIT_OK);
}
void extcl_cpu_every_cycle_082(BYTE nidx) {
	if ((m082.irq.control & 0x01) && !(m082.irq.control & 0x04)) {
		if (m082.irq.counter) {
			m082.irq.counter--;
		}
	} else {
		m082.irq.counter = m082.irq.latch ? (m082.irq.latch + 2) * 16 : 17;
	}
	if ((m082.irq.control & 0x02) && !m082.irq.counter) {
		nes[nidx].c.irq.high |= EXT_IRQ;
	} else {
		nes[nidx].c.irq.high &= ~EXT_IRQ;
	}
}

INLINE static void prg_fix_082(void) {
	memmap_auto_8k(0, MMCPU(0x8000), prg_reg_control_082(m082.prg[0]));
	memmap_auto_8k(0, MMCPU(0xA000), prg_reg_control_082(m082.prg[1]));
	memmap_auto_8k(0, MMCPU(0xC000), prg_reg_control_082(m082.prg[2]));
	memmap_auto_8k(0, MMCPU(0xE000), 0xFF);
}
INLINE static void chr_fix_082(void) {
	WORD swap = (m082.control & 0x02) << 11;

	memmap_auto_2k(0, MMPPU(0x0000 ^ swap), (m082.chr[0] >> 1));
	memmap_auto_2k(0, MMPPU(0x0800 ^ swap), (m082.chr[1] >> 1));
	memmap_auto_1k(0, MMPPU(0x1000 ^ swap), m082.chr[2]);
	memmap_auto_1k(0, MMPPU(0x1400 ^ swap), m082.chr[3]);
	memmap_auto_1k(0, MMPPU(0x1800 ^ swap), m082.chr[4]);
	memmap_auto_1k(0, MMPPU(0x1C00 ^ swap), m082.chr[5]);
}
INLINE static void wram_fix_082(void) {
	BYTE enable = 0;

	enable = m082.wram_enable[0] == 0xCA;
	memmap_auto_wp_2k(0, MMCPU(0x6000), 0, enable, enable);
	enable = m082.wram_enable[1] == 0x69;
	memmap_auto_wp_2k(0, MMCPU(0x6800), 1, enable, enable);
	enable = m082.wram_enable[2] == 0x84;
	memmap_auto_wp_1k(0, MMCPU(0x7000), 4, enable, enable);
	memmap_disable_1k(0, MMCPU(0x7400));
	memmap_disable_1k(0, MMCPU(0x7800));
	memmap_disable_1k(0, MMCPU(0x7C00));
}
INLINE static void mirroring_fix_082(void) {
	if (m082.control & 0x01) {
		mirroring_V(0);
	} else {
		mirroring_H(0);
	}
}

INLINE static BYTE prg_reg_control_082(BYTE value) {
	if (info.mapper.id == 552) {
		return(((value & 0x20) >> 5) | ((value & 0x10) >> 3) | ((value & 0x08) >> 1) | ((value & 0x04) << 1) |
			((value & 0x02) << 3) | ((value & 0x01) << 5));
	}
	return (value >> 2);
}
