/*
 *  Copyright (C) 2010-2024 Fabio Cavallo (aka FHorse)
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

void prg_swap_jyasic_394(WORD address, DBWORD value);
void chr_swap_jyasic_394(WORD address, DBWORD value);
void wram_swap_jyasic_394(WORD address, DBWORD value);
void mirroring_swap_jyasic_394(WORD address, DBWORD value);

void prg_swap_mmc3_394(WORD address, WORD value);
void chr_swap_mmc3_394(WORD address, WORD value);

INLINE static WORD prg_base(void);
INLINE static WORD chr_base(void);

_m394 m394;

void map_init_394(void) {
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
	map_internal_struct_init((BYTE *)&m394, sizeof(m394));
	map_internal_struct_init((BYTE *)&jyasic, sizeof(jyasic));
	map_internal_struct_init((BYTE *)&mmc3, sizeof(mmc3));

	if (info.reset >= HARD) {
		memset(&nes[0].irqA12, 0x00, sizeof(nes[0].irqA12));
		memset(&m394, 0x00, sizeof(m394));

		m394.reg[1] = 0x0F;
		m394.reg[3] = 0x10;
	}

	init_JYASIC(TRUE, info.reset);
	JYASIC_prg_swap = prg_swap_jyasic_394;
	JYASIC_chr_swap = chr_swap_jyasic_394;
	JYASIC_wram_swap = wram_swap_jyasic_394;
	JYASIC_mirroring_swap = mirroring_swap_jyasic_394;

	init_MMC3(info.reset);
	MMC3_prg_swap = prg_swap_mmc3_394;
	MMC3_chr_swap = chr_swap_mmc3_394;

	nes[0].irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_after_mapper_init_394(void) {
	if (m394.reg[1] & 0x10) {
		extcl_after_mapper_init_JYASIC();
	} else {
		extcl_after_mapper_init_MMC3();
	}
}
void extcl_cpu_wr_mem_394(BYTE nidx, WORD address, BYTE value) {
	if ((address >= 0x5000) && (address <= 0x5FFF)) {
		m394.reg[address & 0x03] = value;
		extcl_after_mapper_init_394();
	}
	if (m394.reg[1] & 0x10) {
		extcl_cpu_wr_mem_JYASIC(nidx, address, value);
	} else {
		extcl_cpu_wr_mem_MMC3(nidx, address, value);
	}
}
BYTE extcl_cpu_rd_mem_394(BYTE nidx, WORD address, BYTE openbus) {
	if (m394.reg[1] & 0x10) {
		return (extcl_cpu_rd_mem_JYASIC(nidx, address, openbus));
	}
	return (wram_rd(nidx, address));
}
BYTE extcl_save_mapper_394(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m394.reg);
	if (extcl_save_mapper_MMC3(mode, slot, fp) == EXIT_ERROR) return (EXIT_ERROR);
	return (extcl_save_mapper_JYASIC(mode, slot, fp));
}
void extcl_cpu_every_cycle_394(BYTE nidx) {
	if (m394.reg[1] & 0x10) {
		extcl_cpu_every_cycle_JYASIC(nidx);
	} else {
		extcl_cpu_every_cycle_MMC3(nidx);
	}
}
void extcl_rd_ppu_mem_394(BYTE nidx, WORD address) {
	if (m394.reg[1] & 0x10) {
		extcl_rd_ppu_mem_JYASIC(nidx, address);
	}
}
BYTE extcl_rd_chr_394(BYTE nidx, WORD address) {
	if (m394.reg[1] & 0x10) {
		return (extcl_rd_chr_JYASIC(nidx, address));
	}
	return (chr_rd(nidx, address));
}
void extcl_ppu_000_to_34x_394(BYTE nidx) {
	if (!(m394.reg[1] & 0x10)) {
		extcl_ppu_000_to_34x_MMC3(nidx);
	}
}
void extcl_ppu_000_to_255_394(BYTE nidx) {
	if (m394.reg[1] & 0x10) {
		extcl_ppu_000_to_255_JYASIC(nidx);
	} else {
		extcl_ppu_000_to_255_MMC3(nidx);
	}
}
void extcl_ppu_256_to_319_394(BYTE nidx) {
	if (m394.reg[1] & 0x10) {
		extcl_ppu_256_to_319_JYASIC(nidx);
	} else {
		extcl_ppu_256_to_319_MMC3(nidx);
	}
}
void extcl_ppu_320_to_34x_394(BYTE nidx) {
	if (m394.reg[1] & 0x10) {
		extcl_ppu_320_to_34x_JYASIC(nidx);
	} else {
		extcl_ppu_320_to_34x_MMC3(nidx);
	}
}
void extcl_update_r2006_394(BYTE nidx, WORD new_r2006, WORD old_r2006) {
	if (m394.reg[1] & 0x10) {
		extcl_update_r2006_JYASIC(nidx, new_r2006, old_r2006);
	} else {
		extcl_update_r2006_MMC3(nidx, new_r2006, old_r2006);
	}
}

void prg_swap_jyasic_394(WORD address, DBWORD value) {
	prg_swap_JYASIC_base(address, (prg_base() | (value & 0x1F)));
}
void chr_swap_jyasic_394(WORD address, DBWORD value) {
	chr_swap_JYASIC_base(address, (chr_base() | (value & 0xFF)));
}
void wram_swap_jyasic_394(WORD address, DBWORD value) {
	wram_swap_JYASIC_base(address, (prg_base() | (value & 0x1F)));
}
void mirroring_swap_jyasic_394(WORD address, DBWORD value) {
	mirroring_swap_JYASIC_base(address, (chr_base() | (value & 0xFF)));
}

void prg_swap_mmc3_394(WORD address, WORD value) {
	WORD base = prg_base();
	WORD mask = 0x1F >> !(m394.reg[3] & 0x10);

	if (!(m394.reg[1] & 0x08)) {
		base = (base | ((m394.reg[3] & 0x07) << 1)) & 0xFC;
		mask = 0x03;
		value = (address >> 13) & 0x03;
	}
	prg_swap_MMC3_base(address, ((base & ~mask)| (value & mask)));
}
void chr_swap_mmc3_394(WORD address, WORD value) {
	WORD base = chr_base();
	WORD mask = 0xFF >> !(m394.reg[3] & 0x80);

	chr_swap_MMC3_base(address, (base | (value & mask)));
}

INLINE static WORD prg_base(void) {
	return (((m394.reg[3] & 0x08) << 1) | ((m394.reg[1] & 0x01) << 5));
}
INLINE static WORD chr_base(void) {
	return ((m394.reg[3] & 0x40) << 1) | ((m394.reg[1] & 0x01) << 8);
}
