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

#include "mappers.h"

void prg_swap_jyasic_386(WORD address, DBWORD value);
void chr_swap_jyasic_386(WORD address, DBWORD value);
void wram_swap_jyasic_386(WORD address, DBWORD value);
void mirroring_swap_jyasic_386(WORD address, DBWORD value);

INLINE static DBWORD prg_bank(DBWORD value);
INLINE static DBWORD chr_bank(DBWORD value);

void map_init_386(void) {
	EXTCL_AFTER_MAPPER_INIT(JYASIC);
	EXTCL_CPU_WR_MEM(JYASIC);
	EXTCL_CPU_RD_MEM(JYASIC);
	EXTCL_SAVE_MAPPER(JYASIC);
	EXTCL_CPU_EVERY_CYCLE(JYASIC);
	EXTCL_RD_PPU_MEM(JYASIC);
	EXTCL_RD_CHR(JYASIC);
	EXTCL_PPU_000_TO_255(JYASIC);
	EXTCL_PPU_256_TO_319(JYASIC);
	EXTCL_PPU_320_TO_34X(JYASIC);
	EXTCL_UPDATE_R2006(JYASIC);
	mapper.internal_struct[0] = (BYTE *)&jyasic;
	mapper.internal_struct_size[0] = sizeof(jyasic);

	init_JYASIC(TRUE);
	JYASIC_prg_swap = prg_swap_jyasic_386;
	JYASIC_chr_swap = chr_swap_jyasic_386;
	JYASIC_wram_swap = wram_swap_jyasic_386;
	JYASIC_mirroring_swap = mirroring_swap_jyasic_386;
}

void prg_swap_jyasic_386(WORD address, DBWORD value) {
	prg_swap_JYASIC_base(address, prg_bank(value));
}
void chr_swap_jyasic_386(WORD address, DBWORD value) {
	chr_swap_JYASIC_base(address, chr_bank(value));
}
void wram_swap_jyasic_386(WORD address, DBWORD value) {
	wram_swap_JYASIC_base(address, prg_bank(value));
}
void mirroring_swap_jyasic_386(WORD address, DBWORD value) {
	mirroring_swap_JYASIC_base(address, chr_bank(value));
}

INLINE static DBWORD prg_bank(DBWORD value) {
	return ((((((jyasic.mode[3] & 0x02) << 4) | (jyasic.mode[3] & 0x08) << 3)) | (value & 0x1F)));
}
INLINE static DBWORD chr_bank(DBWORD value) {
	WORD base = ((jyasic.mode[3] & 0x01) << 8) | ((jyasic.mode[3] & 0x0C) << 7);
	WORD mask = 0xFF;

	if (jyasic.mode[3] & 0x20) {
		base &= 0x0600;
		mask = 0x1FF;
	}
	return (base | (value & mask));
}
