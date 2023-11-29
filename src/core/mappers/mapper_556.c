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

INLINE static void fix_all(void);
INLINE static WORD prg_base(void);
INLINE static WORD prg_mask(void);
INLINE static WORD chr_base(void);
INLINE static WORD chr_mask(void);

void prg_swap_mmc3_556(WORD address, WORD value);
void chr_swap_mmc3_556(WORD address, WORD value);

void prg_swap_vrc2and4_556(WORD address, WORD value);
void chr_swap_vrc2and4_556(WORD address, WORD value);

struct _m556 {
	WORD index;
	WORD reg[4];
} m556;

void map_init_556(void) {
	EXTCL_AFTER_MAPPER_INIT(556);
	EXTCL_CPU_WR_MEM(556);
	EXTCL_SAVE_MAPPER(556);
	EXTCL_CPU_EVERY_CYCLE(556);
	EXTCL_PPU_000_TO_34X(556);
	EXTCL_PPU_000_TO_255(556);
	EXTCL_PPU_256_TO_319(556);
	EXTCL_PPU_320_TO_34X(556);
	EXTCL_UPDATE_R2006(556);
	map_internal_struct_init((BYTE *)&m556, sizeof(m556));
	map_internal_struct_init((BYTE *)&mmc3, sizeof(mmc3));
	map_internal_struct_init((BYTE *)&vrc2and4, sizeof(vrc2and4));

	if (info.reset >= HARD) {
		memset(&nes[0].irqA12, 0x00, sizeof(nes[0].irqA12));
	}

	memset(&m556, 0x00, sizeof(m556));
	m556.reg[2] = 0x0F;

	init_MMC3(info.reset);
	MMC3_prg_swap = prg_swap_mmc3_556;
	MMC3_chr_swap = chr_swap_mmc3_556;

	init_VRC2and4(VRC24_VRC4, 0x05, 0x0A, TRUE, info.reset);
	VRC2and4_prg_swap = prg_swap_vrc2and4_556;
	VRC2and4_chr_swap = chr_swap_vrc2and4_556;

	info.mapper.extend_wr = TRUE;

	nes[0].irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_after_mapper_init_556(void) {
	fix_all();
}
void extcl_cpu_wr_mem_556(BYTE nidx, WORD address, BYTE value) {
	if ((address >= 0x5000) && (address <= 0x5FFF)) {
		if (!(m556.reg[3] & 0x80)) {
			m556.reg[m556.index] = value;
			m556.index = (m556.index + 1) & 0x03;
			fix_all();
		}
		return;
	}
	if (m556.reg[2] & 0x80) {
		if (address & 0x0800) {
			address = (address & 0xFFF3) | ((address & 0x0004) << 1) | ((address & 0x0008) >> 1);
		}
		extcl_cpu_wr_mem_VRC2and4(nidx, address, value);
	} else {
		extcl_cpu_wr_mem_MMC3(nidx, address, value);
	}
}
BYTE extcl_save_mapper_556(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m556.index);
	save_slot_ele(mode, slot, m556.reg);
	if (extcl_save_mapper_MMC3(mode, slot, fp) == EXIT_ERROR) return (EXIT_ERROR);
	return (extcl_save_mapper_VRC2and4(mode, slot, fp));
}
void extcl_cpu_every_cycle_556(BYTE nidx) {
	if (m556.reg[2] & 0x80) {
		extcl_cpu_every_cycle_VRC2and4(nidx);
	} else {
		extcl_cpu_every_cycle_MMC3(nidx);
	}
}
void extcl_ppu_000_to_34x_556(BYTE nidx) {
	if (!(m556.reg[2] & 0x80)) {
		extcl_ppu_000_to_34x_MMC3(nidx);
	}
}
void extcl_ppu_000_to_255_556(BYTE nidx) {
	if (!(m556.reg[2] & 0x80)) {
		extcl_ppu_000_to_255_MMC3(nidx);
	}
}
void extcl_ppu_256_to_319_556(BYTE nidx) {
	if (!(m556.reg[2] & 0x80)) {
		extcl_ppu_256_to_319_MMC3(nidx);
	}
}
void extcl_ppu_320_to_34x_556(BYTE nidx) {
	if (!(m556.reg[2] & 0x80)) {
		extcl_ppu_320_to_34x_MMC3(nidx);
	}
}
void extcl_update_r2006_556(BYTE nidx, WORD new_r2006, WORD old_r2006) {
	if (!(m556.reg[2] & 0x80)) {
		extcl_update_r2006_MMC3(nidx, new_r2006, old_r2006);
	}
}

INLINE static void fix_all(void) {
	if (m556.reg[2] & 0x80) {
		extcl_after_mapper_init_VRC2and4();
	} else {
		extcl_after_mapper_init_MMC3();
	}
}
INLINE static WORD prg_base(void) {
	return (((m556.reg[3] & 0x40) << 2) | m556.reg[1]);
}
INLINE static WORD prg_mask(void) {
	return (~m556.reg[3] & 0x3F);
}
INLINE static WORD chr_base(void) {
	return (((m556.reg[3] & 0x40) << 6) | ((m556.reg[2] & 0xF0) << 4) | m556.reg[0]);
}
INLINE static WORD chr_mask(void) {
	return (0xFF >> (~m556.reg[2] & 0x0F));
}

void prg_swap_mmc3_556(WORD address, WORD value) {
	WORD base = prg_base();
	WORD mask = prg_mask();

	prg_swap_MMC3_base(address, ((base & ~mask) | (value & mask)));
}
void chr_swap_mmc3_556(WORD address, WORD value) {
	WORD base = chr_base();
	WORD mask = chr_mask();

	chr_swap_MMC3_base(address, ((base & ~mask) | (value & mask)));
}

void prg_swap_vrc2and4_556(WORD address, WORD value) {
	WORD base = prg_base();
	WORD mask = prg_mask();

	prg_swap_VRC2and4_base(address, ((base & ~mask) | (value & mask)));
}
void chr_swap_vrc2and4_556(WORD address, WORD value) {
	WORD base = chr_base();
	WORD mask = chr_mask();

	chr_swap_VRC2and4_base(address, ((base & ~mask) | (value & mask)));
}
