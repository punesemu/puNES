/*
 *  Copyright (C) 2010-2022 Fabio Cavallo (aka FHorse)
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

#include "mappers.h"
#include "info.h"
#include "mem_map.h"

static BYTE protection[8][8] = {
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x03, 0x04, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x01, 0x00, 0x04, 0x05, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x01, 0x02, 0x04, 0x0F, 0x00}
};

struct _unif158b {
	// da non salvare
	BYTE prot;
} unif158b;

void map_init_UNIF158B(void) {
	map_init_UNIF8237(U8237);

	EXTCL_CPU_WR_MEM(UNIF158B);
	EXTCL_CPU_RD_MEM(UNIF158B);

	unif158b.prot = 0x07;

	info.mapper.extend_rd = TRUE;
}
void extcl_cpu_wr_mem_UNIF158B(WORD address, BYTE value) {
	if ((address >= 0x5000) && (address <= 0x5FFF)) {
		if (address == 0x5002) {
			unif158b.prot = value & 0x07;
		}
	}
	extcl_cpu_wr_mem_UNIF8237(address, value);
}

BYTE extcl_cpu_rd_mem_UNIF158B(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	if ((address >= 0x5000) && (address <= 0x5FFF)) {
		return ((openbus & 0xF0) | protection[unif158b.prot][address & 0x07]);
	}
	return (openbus);
}
