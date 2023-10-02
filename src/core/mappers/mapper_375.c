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

INLINE static void prg_fix_375(void);
INLINE static void chr_fix_375(void);
INLINE static void mirroring_fix_375(void);

struct _m375 {
	WORD reg[2];
} m375;

void map_init_375(void) {
	EXTCL_AFTER_MAPPER_INIT(375);
	EXTCL_CPU_WR_MEM(375);
	EXTCL_SAVE_MAPPER(375);
	mapper.internal_struct[0] = (BYTE *)&m375;
	mapper.internal_struct_size[0] = sizeof(m375);

	memset(&m375, 0x00, sizeof(m375));
}
void extcl_after_mapper_init_375(void) {
	prg_fix_375();
	chr_fix_375();
	mirroring_fix_375();
}
void extcl_cpu_wr_mem_375(UNUSED(BYTE nidx), WORD address, BYTE value) {
	if (!(m375.reg[0] & 0x0800)) {
		m375.reg[0] = address;
		chr_fix_375();
		mirroring_fix_375();
	}
	m375.reg[1] = value;
	prg_fix_375();
}
BYTE extcl_save_mapper_375(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m375.reg);
	return (EXIT_OK);
}

INLINE static void prg_fix_375(void) {
	WORD bank = ((m375.reg[0] & 0x400) >> 4) | ((m375.reg[0] & 0x100) >> 3) | ((m375.reg[0] & 0xF80) >> 2);
	WORD bit0 = m375.reg[0] & 0x0001;
	WORD bit7 = (m375.reg[0] & 0x0080) >> 7;
	WORD bit9 = (m375.reg[0] & 0x0200) >> 9;
	WORD bit11 = (m375.reg[0] & 0x0800) >> 11;

	memmap_auto_16k(0, MMCPU(0x8000), (((bank & ~bit0) & ~(bit11 * 7)) | (bit11 * m375.reg[1])));
	memmap_auto_16k(0, MMCPU(0xC000), (((bank |  bit0) & ~(!bit7 * !bit9 * 7)) | (bit7 * bit9 * 7)));
}
INLINE static void chr_fix_375(void) {
	memmap_vram_wp_8k(0, MMPPU(0x0000), 0, TRUE, !(m375.reg[0] & 0x80));
}
INLINE static void mirroring_fix_375(void) {
	if (m375.reg[0] & 0x02) {
		mirroring_H(0);
	} else {
		mirroring_V(0);
	}
}
