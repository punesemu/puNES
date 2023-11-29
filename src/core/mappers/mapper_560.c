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

INLINE static void prg_fix_560(void);

struct _m560 {
	BYTE reg;
	BYTE pa13;
	WORD nmt_address;
	BYTE ext_ram[0x400];
} m560;

void map_init_560(void) {
	EXTCL_AFTER_MAPPER_INIT(560);
	EXTCL_CPU_WR_MEM(560);
	EXTCL_SAVE_MAPPER(560);
	EXTCL_RD_CHR(560);
	EXTCL_WR_NMT(560);
	EXTCL_RD_NMT(560);
	map_internal_struct_init((BYTE *)&m560, sizeof(m560));

	memset(&m560, 0x00, sizeof(m560));

	m560.reg = 1;
}
void extcl_after_mapper_init_560(void) {
	prg_fix_560();
}
void extcl_cpu_wr_mem_560(UNUSED(BYTE nidx), UNUSED(WORD address), UNUSED(BYTE value)) {
	m560.reg = !m560.reg;
	prg_fix_560();
}
BYTE extcl_save_mapper_560(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m560.reg);
	save_slot_ele(mode, slot, m560.pa13);
	save_slot_ele(mode, slot, m560.nmt_address);
	save_slot_ele(mode, slot, m560.ext_ram);
	return (EXIT_OK);
}
BYTE extcl_rd_chr_560(UNUSED(BYTE nidx), WORD address) {
	WORD base = ((address >> 10) << 9) | ((address & 0x03F0) >> 1) | (address & 0x0007);

	m560.pa13 = FALSE;
	return (m560.reg
		? chrrom_byte(((m560.ext_ram[m560.nmt_address & 0x3FF] << 11) & 0x1F800) | base)
		: chrrom_byte(((address & 0x03FF) << 13) & 0x10000) | base);
}
void extcl_wr_nmt_560(BYTE nidx, WORD address, BYTE value) {
	if (((address & 0x0F00) >> 10) & 0x01) {
		m560.ext_ram[nes[nidx].p.r2006.value & 0x3FF] = value;
	}
	nmt_wr(nidx, address, value);
}
BYTE extcl_rd_nmt_560(BYTE nidx, WORD address) {
	if (!m560.pa13) {
		m560.nmt_address = nes[nidx].p.r2006.value;
	}
	m560.pa13 = TRUE;
	return (nmt_rd(nidx, address));
}

INLINE static void prg_fix_560(void) {
	memmap_auto_32k(0, MMCPU(0x8000), m560.reg);
}
