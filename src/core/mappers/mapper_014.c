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
#include "irqA12.h"
#include "save_slot.h"

INLINE static WORD chr_base(int slot);

void prg_swap_mmc3_014(WORD address, WORD value);
void chr_swap_mmc3_014(WORD address, WORD value);

void prg_swap_vrc2and4_014(WORD address, WORD value);
void chr_swap_vrc2and4_014(WORD address, WORD value);

struct _m014 {
	BYTE reg;
} m014;

void map_init_014(void) {
	EXTCL_AFTER_MAPPER_INIT(014);
	EXTCL_CPU_WR_MEM(014);
	EXTCL_CPU_RD_MEM(014);
	EXTCL_SAVE_MAPPER(014);
	EXTCL_CPU_EVERY_CYCLE(014);
	EXTCL_PPU_000_TO_34X(014);
	EXTCL_PPU_000_TO_255(014);
	EXTCL_PPU_256_TO_319(014);
	EXTCL_PPU_320_TO_34X(014);
	EXTCL_UPDATE_R2006(014);
	mapper.internal_struct[0] = (BYTE *)&m014;
	mapper.internal_struct_size[0] = sizeof(m014);
	mapper.internal_struct[1] = (BYTE *)&mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);
	mapper.internal_struct[2] = (BYTE *)&vrc2and4;
	mapper.internal_struct_size[2] = sizeof(vrc2and4);

	memset(&irqA12, 0x00, sizeof(irqA12));
	memset(&m014, 0x00, sizeof(m014));

	init_MMC3();
	MMC3_prg_swap = prg_swap_mmc3_014;
	MMC3_chr_swap = chr_swap_mmc3_014;

	init_VRC2and4(VRC24_VRC2, 0x01, 0x02, TRUE);
	VRC2and4_prg_swap = prg_swap_vrc2and4_014;
	VRC2and4_chr_swap = chr_swap_vrc2and4_014;

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_after_mapper_init_014(void) {
	if (m014.reg & 0x02) {
		extcl_after_mapper_init_MMC3();
	} else {
		extcl_after_mapper_init_VRC2and4();
	}
}
void extcl_cpu_wr_mem_014(WORD address, BYTE value) {
	if (address == 0xA131) {
		m014.reg = value;
		extcl_after_mapper_init_014();
		return;
	}
	if (m014.reg & 0x02) {
		extcl_cpu_wr_mem_MMC3(address, value);
	} else {
		extcl_cpu_wr_mem_VRC2and4(address, value);
	}
}
BYTE extcl_cpu_rd_mem_014(WORD address, BYTE openbus) {
	if (!(m014.reg & 0x02)) {
		return (extcl_cpu_rd_mem_VRC2and4(address, openbus));
	}
	return (openbus);
}
BYTE extcl_save_mapper_014(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m014.reg);
	if (extcl_save_mapper_MMC3(mode, slot, fp) == EXIT_ERROR) return (EXIT_ERROR);
	return (extcl_save_mapper_VRC2and4(mode, slot, fp));
}
void extcl_cpu_every_cycle_014(void) {
	if (m014.reg & 0x02) {
		extcl_cpu_every_cycle_MMC3();
	}
}
void extcl_ppu_000_to_34x_014(void) {
	if (m014.reg & 0x02) {
		extcl_ppu_000_to_34x_MMC3();
	}
}
void extcl_ppu_000_to_255_014(void) {
	if (m014.reg & 0x02) {
		extcl_ppu_000_to_255_MMC3();
	}
}
void extcl_ppu_256_to_319_014(void) {
	if (m014.reg & 0x02) {
		extcl_ppu_256_to_319_MMC3();
	}
}
void extcl_ppu_320_to_34x_014(void) {
	if (m014.reg & 0x02) {
		extcl_ppu_320_to_34x_MMC3();
	}
}
void extcl_update_r2006_014(WORD new_r2006, WORD old_r2006) {
	if (m014.reg & 0x02) {
		extcl_update_r2006_MMC3(new_r2006, old_r2006);
	}
}

INLINE static WORD chr_base(int slot) {
	switch (slot) {
		case 0:
		case 1:
		case 2:
		case 3:
			return((m014.reg & 0x08) << 5);
		case 4:
		case 5:
			return((m014.reg & 0x20) << 3);
		case 6:
		case 7:
			return((m014.reg & 0x80) << 1);
		default:
			break;
	}
	return (0);
}

void prg_swap_mmc3_014(WORD address, WORD value) {
	prg_swap_MMC3_base(address, (value & 0x3F));
}
void chr_swap_mmc3_014(WORD address, WORD value) {
	chr_swap_MMC3_base(address, (chr_base(address >> 10) | (value & 0xFF)));
}

void prg_swap_vrc2and4_014(WORD address, WORD value) {
	prg_swap_VRC2and4_base(address, (value & 0x1F));
}
void chr_swap_vrc2and4_014(WORD address, WORD value) {
	chr_swap_VRC2and4_base(address, (chr_base(address >> 10) | (value & 0xFF)));
}
