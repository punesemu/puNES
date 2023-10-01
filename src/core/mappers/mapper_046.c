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

INLINE static void prg_fix_046(void);
INLINE static void chr_fix_046(void);

struct _m046 {
	BYTE reg[2];
} m046;

void map_init_046(void) {
	EXTCL_AFTER_MAPPER_INIT(046);
	EXTCL_CPU_WR_MEM(046);
	EXTCL_SAVE_MAPPER(046);
	mapper.internal_struct[0] = (BYTE *)&m046;
	mapper.internal_struct_size[0] = sizeof(m046);

	memset(&m046, 0x00, sizeof(m046));

	info.mapper.extend_wr = TRUE;
}
void extcl_after_mapper_init_046(void) {
	prg_fix_046();
	chr_fix_046();
}
void extcl_cpu_wr_mem_046(BYTE nidx, WORD address, BYTE value) {
	if ((address >= 0x6000) && (address <= 0x7FFF)) {
		m046.reg[0] = value;
		prg_fix_046();
		chr_fix_046();
		return;
	} else if (address >= 0x8000) {
		// bus conflict
		m046.reg[1] = value & prgrom_rd(nidx, address);
		prg_fix_046();
		chr_fix_046();
		return;
	}
}
BYTE extcl_save_mapper_046(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m046.reg);
	return (EXIT_OK);
}

INLINE static void prg_fix_046(void) {
	memmap_auto_32k(0, MMCPU(0x8000), (((m046.reg[0] & 0x0F) << 1) | (m046.reg[1] & 0x01)));
}
INLINE static void chr_fix_046(void) {
	memmap_auto_8k(0, MMPPU(0x0000), (((m046.reg[0] & 0xF0) >> 1) | ((m046.reg[1] & 0x70) >> 4)));
}
