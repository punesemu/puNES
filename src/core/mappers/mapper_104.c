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

INLINE static void prg_fix_104(void);

struct _m104 {
	BYTE reg[2];
	int cycles;
} m104;

void map_init_104(void) {
	EXTCL_AFTER_MAPPER_INIT(104);
	EXTCL_CPU_WR_MEM(104);
	EXTCL_SAVE_MAPPER(104);
	EXTCL_CPU_EVERY_CYCLE(104);
	mapper.internal_struct[0] = (BYTE *)&m104;
	mapper.internal_struct_size[0] = sizeof(m104);

	if (info.reset >= HARD) {
		memset(&m104, 0x00, sizeof(m104));
	}
}
void extcl_after_mapper_init_104(void) {
	prg_fix_104();
}
void extcl_cpu_wr_mem_104(UNUSED(BYTE nidx), WORD address, BYTE value) {
	switch (address & 0xF000) {
		case 0x8000:
		case 0x9000:
		case 0xA000:
		case 0xB000:
			// At CPU cycle 101077 (0.06sec after powerup) game writes $0C (0b1100) to $8927, which would
			// normally result in switching to Micro Machines bank and protect outer register from further
			// writes. However, 100n capacitor in RC circuit is charged by very high 39k resistance
			// and at this time it hasn't charged yet so the register is still hold in reset and ignores
			// writes. Then game compares $0C with $08 and because comparison fails, it jumps to reset ($FFFC).
			if (!(m104.reg[0] & 0x08) && (m104.cycles >= 110000)) {
				m104.reg[0] = value;
				prg_fix_104();
			}
			return;
		case 0xC000:
		case 0xD000:
		case 0xE000:
		case 0xF000:
			m104.reg[1] = value;
			prg_fix_104();
			return;
	}
}
BYTE extcl_save_mapper_104(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m104.reg);
	save_slot_ele(mode, slot, m104.cycles);
	return (EXIT_OK);
}
void extcl_cpu_every_cycle_104(UNUSED(BYTE nidx)) {
	if (m104.cycles < 110000) {
		m104.cycles++;
	}
}

INLINE static void prg_fix_104(void) {
	DBWORD base = (m104.reg[0] << 4);

	memmap_auto_16k(0, MMCPU(0x8000), (base | (m104.reg[1] & 0x0F)));
	memmap_auto_16k(0, MMCPU(0xC000), (base | 0x0F));
}
