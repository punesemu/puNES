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

INLINE static void prg_fix_319(void);
INLINE static void chr_fix_319(void);
INLINE static void mirroring_fix_319(void);

struct _m319 {
	BYTE reg[3];
} m319;

void map_init_319(void) {
	EXTCL_AFTER_MAPPER_INIT(319);
	EXTCL_CPU_WR_MEM(319);
	EXTCL_CPU_RD_MEM(319);
	EXTCL_SAVE_MAPPER(319);
	map_internal_struct_init((BYTE *)&m319, sizeof(m319));

	if (info.reset >= HARD) {
		memset(&m319, 0x00, sizeof(m319));
	}

	m319.reg[0] = 0;
	m319.reg[1] = 0;

	info.mapper.extend_wr = TRUE;
}
void extcl_after_mapper_init_319(void) {
	prg_fix_319();
	chr_fix_319();
	mirroring_fix_319();
}
void extcl_cpu_wr_mem_319(UNUSED(BYTE nidx), WORD address, BYTE value) {
	if ((address >= 0x6000) && (address <= 0x7FFF)) {
		switch (address & 0x04) {
			case 0x00:
				m319.reg[0] = value;
				chr_fix_319();
				return;
			case 0x04:
				m319.reg[1] = value;
				prg_fix_319();
				mirroring_fix_319();
				return;
		}
	} else if (address >= 0x8000) {
		m319.reg[2] = value;
		chr_fix_319();
		return;
	}
}
BYTE extcl_cpu_rd_mem_319(BYTE nidx, WORD address, UNUSED(BYTE openbus)) {
	if ((address >= 0x5000) && (address <= 0x5FFF)) {
		return (dipswitch.value);
	}
	return (wram_rd(nidx, address));
}
BYTE extcl_save_mapper_319(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m319.reg);
	return (EXIT_OK);
}

INLINE static void prg_fix_319(void) {
	// The publicly-available UNIF ROM file of Prima Soft 9999999-in-1 has the order of the 16 KiB PRG-ROM
	// banks slightly mixed up, so that the PRG A14 mode bit operates on A16 instead of A14. To obtain the
	// correct bank order, use UNIF 16 KiB PRG banks 0, 4, 1, 5, 2, 6, 3, 7.
	WORD bank = info.crc32.prg == 0xC25FD362
		? (m319.reg[1] & 0x38) >> 3
		: ((m319.reg[1] & 0x18) >> 2) | ((m319.reg[1] & 0x20) >> 5);
	WORD mask = info.crc32.prg == 0xC25FD362
		? (m319.reg[1] & 0x40) >> 4
		: (m319.reg[1] & 0x40) >> 6;

	memmap_auto_16k(0, MMCPU(0x8000), (bank & ~mask));
	memmap_auto_16k(0, MMCPU(0xC000), (bank | mask));
}
INLINE static void chr_fix_319(void) {
	WORD mask = (m319.reg[0] & 0x01) << 2;

	memmap_auto_8k(0, MMPPU(0x0000), (((m319.reg[0] >> 4) & ~mask) | ((m319.reg[2] << 2) & mask)));
}
INLINE static void mirroring_fix_319(void) {
	if (m319.reg[1] & 0x80) {
		mirroring_V(0);
	} else {
		mirroring_H(0);
	}
}
