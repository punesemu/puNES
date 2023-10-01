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
#include "save_slot.h"

INLINE static void prg_fix_210(void);
INLINE static void chr_fix_210(void);
INLINE static void wram_fix_210(void);
INLINE static void mirroring_fix_210(void);

struct _m210 {
	WORD prg[4];
	WORD chr[8];
	BYTE wram_protect;
} m210;

void map_init_210(void) {
	EXTCL_AFTER_MAPPER_INIT(210);
	EXTCL_CPU_WR_MEM(210);
	EXTCL_SAVE_MAPPER(210);
	mapper.internal_struct[0] = (BYTE *)&m210;
	mapper.internal_struct_size[0] = sizeof(m210);

	if ((info.reset == CHANGE_ROM) || (info.reset == POWER_UP)) {
		memmap_wram_region_init(0, S2K);
	}

	if (info.reset >= HARD) {
		memset(&m210, 0x00, sizeof(m210));

		m210.prg[0] = 0xFC;
		m210.prg[1] = 0xFD;
		m210.prg[2] = 0xFE;
		m210.prg[3] = 0xFF;

		m210.chr[0] = 0;
		m210.chr[1] = 1;
		m210.chr[2] = 2;
		m210.chr[3] = 3;
		m210.chr[4] = 4;
		m210.chr[5] = 5;
		m210.chr[6] = 6;
		m210.chr[7] = 7;
	}
}
void extcl_after_mapper_init_210(void) {
	prg_fix_210();
	chr_fix_210();
	wram_fix_210();
	mirroring_fix_210();
}
void extcl_cpu_wr_mem_210(UNUSED(BYTE nidx), WORD address, BYTE value) {
	switch (address & 0xF000) {
		case 0x8000:
		case 0x9000:
		case 0xA000:
		case 0xB000:
			m210.chr[(address >> 11) & 0x07] = value;
			chr_fix_210();
			return;
		case 0xC000:
			if (!(address & 0x0800)) {
				m210.wram_protect = value & 0x01;
				wram_fix_210();
			}
			return;
		case 0xE000:
		case 0xF000: {
			int index = (address >> 11) & 0x03;

			if (index < 3) {
				m210.prg[index] = value;
				prg_fix_210();
				mirroring_fix_210();
			}
			return;
		}
	}
}
BYTE extcl_save_mapper_210(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m210.prg);
	save_slot_ele(mode, slot, m210.chr);
	save_slot_ele(mode, slot, m210.wram_protect);
	return (EXIT_OK);
}

INLINE static void prg_fix_210(void) {
	memmap_auto_8k(0, MMCPU(0x8000), (m210.prg[0] & 0x3F));
	memmap_auto_8k(0, MMCPU(0xA000), (m210.prg[1] & 0x3F));
	memmap_auto_8k(0, MMCPU(0xC000), (m210.prg[2] & 0x3F));
	memmap_auto_8k(0, MMCPU(0xE000), (m210.prg[3] & 0x3F));
}
INLINE static void chr_fix_210(void) {
	memmap_auto_1k(0, MMPPU(0x0000), m210.chr[0]);
	memmap_auto_1k(0, MMPPU(0x0400), m210.chr[1]);
	memmap_auto_1k(0, MMPPU(0x0800), m210.chr[2]);
	memmap_auto_1k(0, MMPPU(0x0C00), m210.chr[3]);
	memmap_auto_1k(0, MMPPU(0x1000), m210.chr[4]);
	memmap_auto_1k(0, MMPPU(0x1400), m210.chr[5]);
	memmap_auto_1k(0, MMPPU(0x1800), m210.chr[6]);
	memmap_auto_1k(0, MMPPU(0x1C00), m210.chr[7]);
}
INLINE static void wram_fix_210(void) {
	memmap_auto_wp_2k(0, MMCPU(0x6000), 0, TRUE, m210.wram_protect);
	memmap_auto_wp_2k(0, MMCPU(0x6800), 0, TRUE, m210.wram_protect);
	memmap_auto_wp_2k(0, MMCPU(0x7000), 0, TRUE, m210.wram_protect);
	memmap_auto_wp_2k(0, MMCPU(0x7800), 0, TRUE, m210.wram_protect);
}
INLINE static void mirroring_fix_210(void) {
	if (info.mapper.submapper != 1) {
		switch (m210.prg[0] >> 6) {
			case 0:
				mirroring_SCR0(0);
				break;
			case 1:
				mirroring_V(0);
				break;
			case 2:
				mirroring_H(0);
				break;
			case 3:
				mirroring_SCR1(0);
				break;
		}
	}
}
