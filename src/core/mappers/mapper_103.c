/*
 *  Copyright (C) 2010-2024 Fabio Cavallo (aka FHorse)
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

INLINE static void prg_fix_103(void);
INLINE static void wram_fix_103(void);
INLINE static void mirroring_fix_103(void);

struct _m103 {
	WORD reg[3];
} m103;

void map_init_103(void) {
	EXTCL_AFTER_MAPPER_INIT(103);
	EXTCL_CPU_WR_MEM(103);
	EXTCL_CPU_RD_MEM(103);
	EXTCL_SAVE_MAPPER(103);
	map_internal_struct_init((BYTE *)&m103, sizeof(m103));

	if (info.reset >= HARD) {
		memset(&m103, 0x00, sizeof(m103));
	}

	info.mapper.extend_wr = TRUE;
	info.mapper.extend_rd = TRUE;
}
void extcl_after_mapper_init_103(void) {
	prg_fix_103();
	wram_fix_103();
	mirroring_fix_103();
}
void extcl_cpu_wr_mem_103(UNUSED(BYTE nidx), WORD address, BYTE value) {
	switch (address & 0xF000) {
		case 0x6000:
		case 0x7000:
			if (m103.reg[2] & 0x10) {
				// Writes to regions where RAM can be mapped will always write to RAM,
				// even if RAM isn't enabled for reading.(PRGROM)
				wram_direct_wr(address & 0x1FFF, value);
			}
			return;
		case 0x8000:
			m103.reg[0] = value;
			wram_fix_103();
			return;
		case 0xB000:
		case 0xC000:
		case 0xD000:
			if ((address >= 0xB800) && (address <= 0xD7FF)) {
				// Writes to regions where RAM can be mapped will always write to RAM,
				// even if RAM isn't enabled for reading (PRGROM).
				wram_direct_wr(0x2000 + (address - 0xB800), value);
			}
			return;
		case 0xE000:
			m103.reg[1] = value;
			mirroring_fix_103();
			return;
		case 0xF000:
			m103.reg[2] = value;
			wram_fix_103();
			return;
		default:
			return;
	}
}
BYTE extcl_cpu_rd_mem_103(BYTE nidx, WORD address, BYTE openbus) {
	if (address >= 0x8000) {
		openbus = prgrom_rd(nidx, address);
		if ((address >= 0xB800) && (address <= 0xD7FF) && !(m103.reg[2] & 0x10)) {
			return (wram_direct_rd(0x2000 + (address - 0xB800), openbus));
		}
		return (openbus);
	}
	return (wram_rd(nidx, address));
}
BYTE extcl_save_mapper_103(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m103.reg);
	return (EXIT_OK);
}

INLINE static void prg_fix_103(void) {
	memmap_auto_32k(0, MMCPU(0x8000), 3);
}
INLINE static void wram_fix_103(void) {
	if (m103.reg[2] & 0x10) {
		memmap_prgrom_8k(0, MMCPU(0x6000), m103.reg[0]);
	} else {
		memmap_auto_8k(0, MMCPU(0x6000), 0);
	}
}
INLINE static void mirroring_fix_103(void) {
	if (m103.reg[1] & 0x08) {
		mirroring_H(0);
	} else {
		mirroring_V(0);
	}
}
