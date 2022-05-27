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

#include <string.h>
#include <stdlib.h>
#include "mappers.h"
#include "info.h"
#include "mem_map.h"
#include "ines.h"
#include "save_slot.h"

struct _m218tmp {
	BYTE va10;
} m218tmp;

void map_init_218(void) {
	EXTCL_WR_NMT(218);
	EXTCL_RD_NMT(218);
	EXTCL_WR_CHR(218);
	EXTCL_RD_CHR(218);

	switch (ines.flags[FL6] & 0x09) {
		case 0:
			m218tmp.va10 = 10;
			break;
		case 1:
			m218tmp.va10 = 11;
			break;
		case 8:
			m218tmp.va10 = 12;
			break;
		case 9:
			m218tmp.va10 = 13;
			break;
	}
}
void extcl_wr_nmt_218(WORD address, BYTE value) {
	extcl_wr_chr_218((address & 0x0FFF) + 0x2000, value);
}
BYTE extcl_rd_nmt_218(WORD address) {
	return (extcl_rd_chr_218((address & 0x0FFF) + 0x2000));
}

void extcl_wr_chr_218(WORD address, BYTE value) {
	address = (address & 0x03FF) | ((address >> m218tmp.va10) << 10);
	ntbl.data[address] = value;
}
BYTE extcl_rd_chr_218(WORD address) {
	address = (address & 0x03FF) | ((address >> m218tmp.va10) << 10);
	return (ntbl.data[address]);
}
