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
#include "info.h"
#include "mem_map.h"
#include "irqA12.h"
#include "save_slot.h"

void prg_swap_370(WORD address, WORD value);
void chr_swap_370(WORD address, WORD value);
void mirroring_fix_370(void);

INLINE static BYTE mirroring_TLSROM(BYTE value);

struct _m370 {
	BYTE reg;
} m370;
struct _m370tmp {
	BYTE dipswitch;
} m370tmp;

void map_init_370(void) {
	EXTCL_AFTER_MAPPER_INIT(MMC3);
	EXTCL_CPU_WR_MEM(370);
	EXTCL_CPU_RD_MEM(370);
	EXTCL_SAVE_MAPPER(370);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *)&m370;
	mapper.internal_struct_size[0] = sizeof(m370);
	mapper.internal_struct[1] = (BYTE *)&mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);

	memset(&irqA12, 0x00, sizeof(irqA12));
	memset(&m370, 0x00, sizeof(m370));

	init_MMC3();
	MMC3_prg_swap = prg_swap_370;
	MMC3_chr_swap = chr_swap_370;
	MMC3_mirroring_fix = mirroring_fix_370;

	if (info.reset == RESET) {
		m370tmp.dipswitch ^= 0x80;
	} else if (((info.reset == CHANGE_ROM) || (info.reset == POWER_UP))) {
		m370tmp.dipswitch = 0x80;
	}

	info.mapper.extend_wr = TRUE;

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_370(WORD address, BYTE value) {
	if ((address >= 0x5000) && (address <= 0x5FFF)) {
		if (cpu.prg_ram_wr_active) {
			m370.reg = address & 0xFF;
			MMC3_prg_fix();
			MMC3_chr_fix();
		}
		return;
	}
	if (address >= 0x8000) {
		extcl_cpu_wr_mem_MMC3(address, value);
	}
}
BYTE extcl_cpu_rd_mem_370(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	if ((address >= 0x5000) && (address <= 0x5FFF)) {
		return ((m370tmp.dipswitch & 0x80) | (openbus & 0x7F));
	}
	return (openbus);
}
BYTE extcl_save_mapper_370(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m370.reg);
	extcl_save_mapper_MMC3(mode, slot, fp);

	return (EXIT_OK);
}

void prg_swap_370(WORD address, WORD value) {
	WORD base = (m370.reg & 0x38) << 1;
	WORD mask = 0x1F >> ((m370.reg & 0x20) >> 5);

	prg_swap_MMC3(address, (base | (value & mask)));
}
void chr_swap_370(WORD address, WORD value) {
	WORD base = (m370.reg & 0x07) << 7;
	WORD mask = 0xFF >> ((~m370.reg & 0x04) >> 2);

	chr_swap_MMC3(address, (base | (value & mask)));
}
void mirroring_fix_370(void) {
	if ((m370.reg & 0x07) == 0x01) {
		if (mmc3.bank_to_update & 0x80) {
			ntbl.bank_1k[0] = &ntbl.data[mirroring_TLSROM(mmc3.reg[2]) << 10];
			ntbl.bank_1k[1] = &ntbl.data[mirroring_TLSROM(mmc3.reg[3]) << 10];
			ntbl.bank_1k[2] = &ntbl.data[mirroring_TLSROM(mmc3.reg[4]) << 10];
			ntbl.bank_1k[3] = &ntbl.data[mirroring_TLSROM(mmc3.reg[5]) << 10];
		} else {
			ntbl.bank_1k[0] = &ntbl.data[mirroring_TLSROM(mmc3.reg[0]) << 10];
			ntbl.bank_1k[1] = ntbl.bank_1k[0];
			ntbl.bank_1k[2] = &ntbl.data[mirroring_TLSROM(mmc3.reg[1]) << 10];
			ntbl.bank_1k[3] = ntbl.bank_1k[2];
		}
		return;
	}
	mirroring_fix_MMC3();
}

INLINE static BYTE mirroring_TLSROM(BYTE value) {
	return ((value >> 7) ^ 0x01);
}
