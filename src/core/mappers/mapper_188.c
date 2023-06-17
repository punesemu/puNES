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

// TODO aggiungere l'emulzione del microfono

INLINE static void prg_fix_188(void);

struct _m188 {
	BYTE reg;
} m188;

void map_init_188(void) {
	EXTCL_AFTER_MAPPER_INIT(188);
	EXTCL_CPU_WR_MEM(188);
	EXTCL_CPU_RD_MEM(188);
	EXTCL_SAVE_MAPPER(188);
	mapper.internal_struct[0] = (BYTE *)&m188;
	mapper.internal_struct_size[0] = sizeof(m188);

	if (info.reset >= HARD) {
		memset(&m188, 0x00, sizeof(m188));
	}
}
void extcl_after_mapper_init_188(void) {
	prg_fix_188();
}
void extcl_cpu_wr_mem_188(UNUSED(WORD address), BYTE value) {
	m188.reg = value;
	prg_fix_188();
}
BYTE extcl_cpu_rd_mem_188(WORD address, UNUSED(BYTE openbus)) {
	if ((address >= 0x6000) && (address <= 0x7FFF)) {
		return (0x03);
	}
	return (wram_rd(address));
}
BYTE extcl_save_mapper_188(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m188.reg);

	return (EXIT_OK);
}

INLINE static void prg_fix_188(void) {
	memmap_auto_16k(MMCPU(0x8000), (m188.reg & 0x10 ? m188.reg & 0x07 : m188.reg | 0x08));
	memmap_auto_16k(MMCPU(0xC000), 0x07);
}
