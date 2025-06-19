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

struct _m185 {
	BYTE reg;
	BYTE r2007_read_count;
} m185;

void map_init_185() {
	EXTCL_CPU_WR_MEM(185);
	EXTCL_SAVE_MAPPER(185);
	EXTCL_RD_CHR(185);
	map_internal_struct_init((BYTE *)&m185, sizeof(m185));

	if ((info.mapper.submapper & 0x0C) != 0x04) {
		EXTCL_RD_R2007(185);
	}

	if (info.reset >= HARD) {
		memset(&m185, 0x00, sizeof(m185));
	}
}
void extcl_cpu_wr_mem_185(UNUSED(BYTE nidx), UNUSED(WORD address), BYTE value) {
	value &= prgrom_rd(nidx, address);
	m185.reg = value;
}
BYTE extcl_save_mapper_185(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m185.reg);
	save_slot_ele(mode, slot, m185.r2007_read_count);
	return (EXIT_OK);
}
void extcl_rd_r2007_185(UNUSED(BYTE nidx)) {
	if (m185.r2007_read_count < 2) {
		m185.r2007_read_count++;
	}
}
BYTE extcl_rd_chr_185(BYTE nidx, WORD address) {
	if ((info.mapper.submapper & 0x0C) == 0x04) {
		return ((m185.reg & 0x03) == (info.mapper.submapper & 0x03) ? chr_rd(nidx, address) : 0xFF);
	}
	return (m185.r2007_read_count >= 2 ? chr_rd(nidx, address) : 0xFF);
}
