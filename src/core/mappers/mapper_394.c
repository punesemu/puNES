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

void prg_swap_mmc3_394(WORD address, WORD value);
void chr_swap_mmc3_394(WORD address, WORD value);

_m394 m394;

void map_init_394(void) {
	map_init_JYASIC(MAP394);

	EXTCL_AFTER_MAPPER_INIT(394);
	EXTCL_CPU_WR_MEM(394);
	EXTCL_CPU_RD_MEM(394);
	EXTCL_SAVE_MAPPER(394);
	EXTCL_CPU_EVERY_CYCLE(394);
	EXTCL_RD_PPU_MEM(394);
	EXTCL_RD_CHR(394);
	EXTCL_PPU_000_TO_34X(394);
	EXTCL_PPU_000_TO_255(394);
	EXTCL_PPU_256_TO_319(394);
	EXTCL_PPU_320_TO_34X(394);
	EXTCL_UPDATE_R2006(394);
	mapper.internal_struct[0] = (BYTE *)&m394;
	mapper.internal_struct_size[0] = sizeof(m394);
	mapper.internal_struct[1] = (BYTE *)&jyasic;
	mapper.internal_struct_size[1] = sizeof(jyasic);
	mapper.internal_struct[2] = (BYTE *)&mmc3;
	mapper.internal_struct_size[2] = sizeof(mmc3);

	if (info.reset >= HARD) {
		memset(&irqA12, 0x00, sizeof(irqA12));
		memset(&m394, 0x00, sizeof(m394));

		m394.reg[1] = 0x0F;
		m394.reg[3] = 0x10;
	}

	init_MMC3();
	MMC3_prg_swap = prg_swap_mmc3_394;
	MMC3_chr_swap = chr_swap_mmc3_394;

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_after_mapper_init_394(void) {
	if (m394.reg[1] & 0x10) {
		extcl_after_mapper_init_JYASIC();
	} else {
		extcl_after_mapper_init_MMC3();
	}
}
void extcl_cpu_wr_mem_394(WORD address, BYTE value) {
	if ((address >= 0x5000) && (address <= 0x5FFF)) {
		if (cpu.prg_ram_wr_active) {
			m394.reg[address & 0x03] = value;
			extcl_after_mapper_init_394();
		}
		return;
	}
	if (address >= 0x8000) {
		if (m394.reg[1] & 0x10) {
			extcl_cpu_wr_mem_JYASIC(address, value);
		} else {
			extcl_cpu_wr_mem_MMC3(address, value);
		}
	}
}
BYTE extcl_cpu_rd_mem_394(WORD address, BYTE openbus, BYTE before) {
	if (m394.reg[1] & 0x10) {
		return (extcl_cpu_rd_mem_JYASIC(address, openbus, before));
	}
	return (openbus);
}
BYTE extcl_save_mapper_394(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m394.reg);
	extcl_save_mapper_MMC3(mode, slot, fp);
	extcl_save_mapper_JYASIC(mode, slot, fp);

	if (mode == SAVE_SLOT_READ) {
		extcl_after_mapper_init_394();
	}

	return (EXIT_OK);
}
void extcl_cpu_every_cycle_394(void) {
	if (m394.reg[1] & 0x10) {
		extcl_cpu_every_cycle_JYASIC();
	} else {
		extcl_cpu_every_cycle_MMC3();
	}
}
void extcl_rd_ppu_mem_394(WORD address) {
	if (m394.reg[1] & 0x10) {
		extcl_rd_ppu_mem_JYASIC(address);
	}
}
BYTE extcl_rd_chr_394(WORD address) {
	if (m394.reg[1] & 0x10) {
		return (extcl_rd_chr_JYASIC(address));
	}
	return (chr.bank_1k[address >> 10][address & 0x3FF]);
}
void extcl_ppu_000_to_34x_394(void) {
	if (!(m394.reg[1] & 0x10)) {
		extcl_ppu_000_to_34x_MMC3();
	}
}
void extcl_ppu_000_to_255_394(void) {
	if (m394.reg[1] & 0x10) {
		extcl_ppu_000_to_255_JYASIC();
	} else {
		extcl_ppu_000_to_255_MMC3();
	}
}
void extcl_ppu_256_to_319_394(void) {
	if (m394.reg[1] & 0x10) {
		extcl_ppu_256_to_319_JYASIC();
	} else {
		extcl_ppu_256_to_319_MMC3();
	}
}
void extcl_ppu_320_to_34x_394(void) {
	if (m394.reg[1] & 0x10) {
		extcl_ppu_320_to_34x_JYASIC();
	} else {
		extcl_ppu_320_to_34x_MMC3();
	}
}
void extcl_update_r2006_394(WORD new_r2006, WORD old_r2006) {
	if (m394.reg[1] & 0x10) {
		extcl_update_r2006_JYASIC(new_r2006, old_r2006);
	} else {
		extcl_update_r2006_MMC3(new_r2006, old_r2006);
	}
}

void prg_swap_mmc3_394(WORD address, WORD value) {
	WORD base = ((m394.reg[3] & 0x08) << 1) | ((m394.reg[1] & 0x01) << 5);
	WORD mask = 0x1F >> !(m394.reg[3] & 0x10);

	if (!(m394.reg[1] & 0x08)) {
		base = (base | ((m394.reg[3] & 0x07) << 1)) & 0xFC;
		mask = 0x03;
		value = (address >> 13) & 0x03;
	}
	prg_swap_MMC3_base(address, ((base & ~mask)| (value & mask)));
}
void chr_swap_mmc3_394(WORD address, WORD value) {
	WORD base =((m394.reg[3] & 0x40) << 1) | ((m394.reg[1] & 0x01) << 8);
	WORD mask = 0xFF >> !(m394.reg[3] & 0x80);

	chr_swap_MMC3_base(address, (base | (value & mask)));
}
