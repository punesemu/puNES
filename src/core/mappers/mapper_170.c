/*
 *  Copyright (C) 2010-2026 Fabio Cavallo (aka FHorse)
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

struct _m170 {
	BYTE reg;
} m170;

void map_init_170(void) {
	EXTCL_CPU_WR_MEM(170);
	EXTCL_CPU_RD_MEM(170);
	EXTCL_SAVE_MAPPER(170);
	map_internal_struct_init((BYTE *)&m170, sizeof(m170));

	if (info.reset >= HARD) {
		memset(&m170, 0x00, sizeof(m170));
	}

	info.mapper.extend_wr = TRUE;
}
void extcl_cpu_wr_mem_170(UNUSED(BYTE nidx), WORD address, BYTE value) {
	if ((address == 0x6502) || (address == 0x7000)) {
		m170.reg = value;
	}
}
BYTE extcl_cpu_rd_mem_170(BYTE nidx, WORD address, UNUSED(BYTE openbus)) {
	if ((address == 0x7001) || (address == 0x7777)) {
		return (((m170.reg & 0x40) << 1) | (wram_rd(nidx, address) & 0x7F));
	}
	return (wram_rd(nidx, address));
}
BYTE extcl_save_mapper_170(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m170.reg);
	return (EXIT_OK);
}
