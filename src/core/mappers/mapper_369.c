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

void prg_fix_mmc3_369(void);
void prg_swap_mmc3_369(WORD address, WORD value);
void chr_fix_mmc3_369(void);
void chr_swap_mmc3_369(WORD address, WORD value);
void wram_fix_mmc3_369(void);

struct _m369 {
	BYTE reg;
	BYTE smb2j;
	struct _m369_irq {
		BYTE enable;
		WORD counter;
	} irq;
} m369;

void map_init_369(void) {
	EXTCL_AFTER_MAPPER_INIT(MMC3);
	EXTCL_CPU_WR_MEM(369);
	EXTCL_SAVE_MAPPER(369);
	EXTCL_CPU_EVERY_CYCLE(369);
	EXTCL_PPU_000_TO_34X(369);
	EXTCL_PPU_000_TO_255(369);
	EXTCL_PPU_256_TO_319(369);
	EXTCL_PPU_320_TO_34X(369);
	EXTCL_UPDATE_R2006(369);
	mapper.internal_struct[0] = (BYTE *)&m369;
	mapper.internal_struct_size[0] = sizeof(m369);
	mapper.internal_struct[1] = (BYTE *)&mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);

	if (info.reset >= HARD) {
		memset(&nes[0].irqA12, 0x00, sizeof(nes[0].irqA12));
		memset(&m369, 0x00, sizeof(m369));
	}

	init_MMC3(info.reset);
	MMC3_prg_fix = prg_fix_mmc3_369;
	MMC3_prg_swap = prg_swap_mmc3_369;
	MMC3_chr_fix = chr_fix_mmc3_369;
	MMC3_chr_swap = chr_swap_mmc3_369;
	MMC3_wram_fix = wram_fix_mmc3_369;

	info.mapper.extend_wr = TRUE;

	nes[0].irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_369(BYTE nidx, WORD address, BYTE value) {
	switch (address & 0xF000) {
		case 0x4000:
			if (address & 0x0100) {
				m369.reg = value;
				MMC3_prg_fix();
				MMC3_chr_fix();
				MMC3_wram_fix();
			}
			return;
		case 0x8000:
		case 0x9000:
			if (m369.reg == 0x13) {
				m369.irq.enable = FALSE;
				nes[nidx].c.irq.high &= ~EXT_IRQ;
			}
			if (!(address & 0x0001)) {
				extcl_cpu_wr_mem_MMC3(nidx, address, value);
			} else {
				mmc3.reg[mmc3.bank_to_update & 0x07] = value;
				switch (mmc3.bank_to_update & 0x07) {
					case 0:
					case 1:
					case 2:
					case 3:
					case 4:
					case 5:
						MMC3_chr_fix();
						return;
					case 6:
					case 7:
						MMC3_prg_fix();
						return;
				}
			}
			return;
		case 0xA000:
		case 0xB000:
			if (m369.reg == 0x13) {
				m369.irq.enable = value & 0x02;
			}
			extcl_cpu_wr_mem_MMC3(nidx, address, value);
			return;
		case 0xC000:
		case 0xD000:
			extcl_cpu_wr_mem_MMC3(nidx, address, value);
			return;
		case 0xE000:
		case 0xF000:
			if (m369.reg == 0x13) {
				m369.smb2j = value;
				MMC3_prg_fix();
			}
			extcl_cpu_wr_mem_MMC3(nidx, address, value);
			return;
		default:
			return;
	}
}
BYTE extcl_save_mapper_369(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m369.reg);
	save_slot_ele(mode, slot, m369.smb2j);
	save_slot_ele(mode, slot, m369.irq.enable);
	save_slot_ele(mode, slot, m369.irq.counter);
	return (extcl_save_mapper_MMC3(mode, slot, fp));
}
void extcl_cpu_every_cycle_369(BYTE nidx) {
	if (m369.reg == 0x13) {
		if (m369.irq.enable) {
			m369.irq.counter = (m369.irq.counter + 1) & 0x0FFF;
			if (!m369.irq.counter) {
				nes[nidx].c.irq.high |= EXT_IRQ;
			}
		}
	} else {
		extcl_cpu_every_cycle_MMC3(nidx);
	}
}
void extcl_ppu_000_to_34x_369(BYTE nidx) {
	if (!(m369.reg == 0x13)) {
		extcl_ppu_000_to_34x_MMC3(nidx);
	}
}
void extcl_ppu_000_to_255_369(BYTE nidx) {
	if (!(m369.reg == 0x13)) {
		extcl_ppu_000_to_255_MMC3(nidx);
	}
}
void extcl_ppu_256_to_319_369(BYTE nidx) {
	if (!(m369.reg == 0x13)) {
		extcl_ppu_256_to_319_MMC3(nidx);
	}
}
void extcl_ppu_320_to_34x_369(BYTE nidx) {
	if (!(m369.reg == 0x13)) {
		extcl_ppu_320_to_34x_MMC3(nidx);
	}
}
void extcl_update_r2006_369(BYTE nidx, WORD new_r2006, WORD old_r2006) {
	if (!(m369.reg == 0x13)) {
		extcl_update_r2006_MMC3(nidx, new_r2006, old_r2006);
	}
}

void prg_fix_mmc3_369(void) {
	switch (m369.reg) {
		case 0x00:
		case 0x01:
			memmap_auto_32k(0, MMCPU(0x8000), m369.reg);
			return;
		case 0x13:
			memmap_auto_8k(0, MMCPU(0x8000), 0x0C);
			memmap_auto_8k(0, MMCPU(0xA000), 0x0D);
			memmap_auto_8k(0, MMCPU(0xC000), (0x08 | (m369.smb2j & 0x03)));
			memmap_auto_8k(0, MMCPU(0xE000), 0x0F);
			return;
		case 0x37:
		case 0xFF:
			prg_fix_MMC3_base();
			return;
	}
}
void prg_swap_mmc3_369(WORD address, WORD value) {
	WORD base = (m369.reg == 0x37) ? 0x10 : 0x20;
	WORD mask = (m369.reg == 0x37) ? 0x0F : 0x1F;

	prg_swap_MMC3_base(address, ((base & ~mask) | (value & mask)));
}
void chr_fix_mmc3_369(void) {
	switch (m369.reg) {
		case 0x00:
		case 0x01:
		case 0x13:
			memmap_auto_8k(0, MMPPU(0x0000), (m369.reg & 0x03));
			return;
		case 0x37:
		case 0xFF:
			chr_fix_MMC3_base();
			return;
	}
}
void chr_swap_mmc3_369(WORD address, WORD value) {
	WORD base = (m369.reg == 0x37) ? 0x0080 : 0x0100;
	WORD mask = 0x7F;

	chr_swap_MMC3_base(address, ((base & ~mask) | (value & mask)));
}
void wram_fix_mmc3_369(void) {
	if (m369.reg == 0x13) {
		memmap_prgrom_8k(0, MMCPU(0x6000), 0x0E);
	} else {
		wram_fix_MMC3_base();
	}
}
