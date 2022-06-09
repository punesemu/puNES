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
#include "mappers.h"
#include "info.h"
#include "mem_map.h"
#include "irqA12.h"
#include "save_slot.h"

INLINE static void prg_fix_394(BYTE value);
INLINE static void prg_swap_394(WORD address, WORD value);
INLINE static void chr_fix_394(BYTE value);
INLINE static void chr_swap_394(WORD address, WORD value);

_m394 m394;

void map_init_394(void) {
	map_init_JYASIC(MAP394);

	EXTCL_AFTER_MAPPER_INIT(394);
	EXTCL_CPU_WR_MEM(394);
	EXTCL_SAVE_MAPPER(394);
	EXTCL_CPU_EVERY_CYCLE(394);
	EXTCL_RD_PPU(394);
	EXTCL_RD_CHR(394);
	EXTCL_WR_NMT(394);
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
		memset(&mmc3, 0x00, sizeof(mmc3));
		memset(&irqA12, 0x00, sizeof(irqA12));
		memset(&m394, 0x00, sizeof(m394));

		m394.reg[1] = 0x0F;
		m394.reg[3] = 0x10;

		m394.mmc3[0] = 0;
		m394.mmc3[1] = 2;
		m394.mmc3[2] = 4;
		m394.mmc3[3] = 5;
		m394.mmc3[4] = 6;
		m394.mmc3[5] = 7;
		m394.mmc3[6] = 0;
		m394.mmc3[7] = 0;
	}

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_after_mapper_init_394(void) {
	if (m394.reg[1] & 0x10) {
		state_fix_JYASIC();
	} else {
		prg_fix_394(mmc3.bank_to_update);
		chr_fix_394(mmc3.bank_to_update);
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
			switch (address & 0xE001) {
				case 0x8000:
					if ((value & 0x40) != (mmc3.bank_to_update & 0x40)) {
						prg_fix_394(value);
					}
					if ((value & 0x80) != (mmc3.bank_to_update & 0x80)) {
						chr_fix_394(value);
					}
					mmc3.bank_to_update = value;
					return;
				case 0x8001: {
					WORD cbase = (mmc3.bank_to_update & 0x80) << 5;

					m394.mmc3[mmc3.bank_to_update & 0x07] = value;

					switch (mmc3.bank_to_update & 0x07) {
						case 0:
							chr_swap_394(cbase ^ 0x0000, value & (~1));
							chr_swap_394(cbase ^ 0x0400, value | 1);
							return;
						case 1:
							chr_swap_394(cbase ^ 0x0800, value & (~1));
							chr_swap_394(cbase ^ 0x0C00, value | 1);
							return;
						case 2:
							chr_swap_394(cbase ^ 0x1000, value);
							return;
						case 3:
							chr_swap_394(cbase ^ 0x1400, value);
							return;
						case 4:
							chr_swap_394(cbase ^ 0x1800, value);
							return;
						case 5:
							chr_swap_394(cbase ^ 0x1C00, value);
							return;
						case 6:
							if (mmc3.bank_to_update & 0x40) {
								prg_swap_394(0xC000, value);
							} else {
								prg_swap_394(0x8000, value);
							}
							return;
						case 7:
							prg_swap_394(0xA000, value);
							return;
					}
					return;
				}
			}
			extcl_cpu_wr_mem_MMC3(address, value);
		}
	}
}
BYTE extcl_save_mapper_394(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m394.reg);
	save_slot_ele(mode, slot, m394.mmc3);
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
void extcl_rd_ppu_394(WORD address) {
	if (m394.reg[1] & 0x10) {
		extcl_rd_ppu_JYASIC(address);
	}
}
BYTE extcl_rd_chr_394(WORD address) {
	if (m394.reg[1] & 0x10) {
		return (extcl_rd_chr_JYASIC(address));
	}
	return (chr.bank_1k[address >> 10][address & 0x3FF]);
}
void extcl_wr_nmt_394(WORD address, BYTE value) {
	if (m394.reg[1] & 0x10) {
		extcl_wr_nmt_JYASIC(address, value);
		return;
	}
	ntbl.bank_1k[(address & 0x0FFF) >> 10][address & 0x3FF] = value;
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

INLINE static void prg_fix_394(BYTE value) {
	if (value & 0x40) {
		prg_swap_394(0x8000, ~1);
		prg_swap_394(0xC000, m394.mmc3[6]);
	} else {
		prg_swap_394(0x8000, m394.mmc3[6]);
		prg_swap_394(0xC000, ~1);
	}
	prg_swap_394(0xA000, m394.mmc3[7]);
	prg_swap_394(0xE000, ~0);
}
INLINE static void prg_swap_394(WORD address, WORD value) {
	WORD base = ((m394.reg[3] & 0x08) << 1) | ((m394.reg[1] & 0x01) << 5);
	WORD mask = 0x1F >> !(m394.reg[3] & 0x10);

	if (!(m394.reg[1] & 0x08)) {
		base = (base | ((m394.reg[3] & 0x07) << 1)) & 0xFC;
		mask = 0x03;
		value = (address >> 13) & 0x03;
	}

	value = (base & ~mask)| (value & mask);
	control_bank(info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, (address >> 13) & 0x03, value);
	map_prg_rom_8k_update();
}
INLINE static void chr_fix_394(BYTE value) {
	WORD cbase = (value & 0x80) << 5;

	chr_swap_394(cbase ^ 0x0000, m394.mmc3[0] & (~1));
	chr_swap_394(cbase ^ 0x0400, m394.mmc3[0] |   1);
	chr_swap_394(cbase ^ 0x0800, m394.mmc3[1] & (~1));
	chr_swap_394(cbase ^ 0x0C00, m394.mmc3[1] |   1);
	chr_swap_394(cbase ^ 0x1000, m394.mmc3[2]);
	chr_swap_394(cbase ^ 0x1400, m394.mmc3[3]);
	chr_swap_394(cbase ^ 0x1800, m394.mmc3[4]);
	chr_swap_394(cbase ^ 0x1C00, m394.mmc3[5]);
}
INLINE static void chr_swap_394(WORD address, WORD value) {
	WORD base =((m394.reg[3] & 0x40) << 1) | ((m394.reg[1] & 0x01) << 8);
	WORD mask = 0xFF >> !(m394.reg[3] & 0x80);

	value = base | (value & mask);
	control_bank(info.chr.rom.max.banks_1k)
	chr.bank_1k[address >> 10] = chr_pnt(value << 10);
}
