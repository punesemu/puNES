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

#ifndef PPU_INLINE_H_
#define PPU_INLINE_H_

#include "external_calls.h"
#include "memmap.h"

INLINE static BYTE ppu_rd_mem(BYTE cidx, WORD address);

INLINE static BYTE ppu_rd_mem(BYTE cidx, WORD address) {
	if (extcl_rd_ppu_mem) {
		extcl_rd_ppu_mem(cidx, address);
	}
	address &= 0x3FFF;
	if (address < 0x2000) {
		return (extcl_rd_chr ? extcl_rd_chr(cidx, address) : chr_rd(cidx, address));
	}
	if (address < 0x3F00) {
		return (extcl_rd_nmt ? extcl_rd_nmt(cidx, address) : nmt_rd(cidx, address));
	}
	return (nes[cidx].m.memmap_palette.color[address & 0x1F]);
}

#endif /* PPU_INLINE_H_ */
