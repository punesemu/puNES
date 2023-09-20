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

#include <stdlib.h>
#include <string.h>
#include "mappers.h"
#include "irqA12.h"
#include "save_slot.h"

enum _m351_mappers { M351_MMC3 = 1, M351_VRC4, M351_MMC1 };

INLINE static void switch_mode(void);
INLINE static void fix_all(void);
INLINE static void prg_fix(void);
INLINE static void chr_fix(void);
INLINE static void wram_mirroring_fix(void);
INLINE static WORD prg_base(void);
INLINE static WORD prg_mask(void);
INLINE static WORD chr_base(void);
INLINE static WORD chr_mask(void);

void prg_swap_mmc3_351(WORD address, WORD value);
void chr_swap_mmc3_351(WORD address, WORD value);

void prg_swap_mmc1_351(WORD address, WORD value);
void chr_swap_mmc1_351(WORD address, WORD value);

void prg_swap_vrc2and4_351(WORD address, WORD value);
void chr_swap_vrc2and4_351(WORD address, WORD value);

struct _m351 {
	BYTE mapper;
	WORD reg[4];
} m351;

void map_init_351(void) {
	EXTCL_AFTER_MAPPER_INIT(351);
	EXTCL_CPU_WR_MEM(351);
	EXTCL_SAVE_MAPPER(351);
	EXTCL_CPU_EVERY_CYCLE(351);
	EXTCL_PPU_000_TO_34X(351);
	EXTCL_PPU_000_TO_255(351);
	EXTCL_PPU_256_TO_319(351);
	EXTCL_PPU_320_TO_34X(351);
	EXTCL_UPDATE_R2006(351);
	mapper.internal_struct[0] = (BYTE *)&m351;
	mapper.internal_struct_size[0] = sizeof(m351);
	mapper.internal_struct[1] = (BYTE *)&mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);
	mapper.internal_struct[2] = (BYTE *)&vrc2and4;
	mapper.internal_struct_size[2] = sizeof(vrc2and4);
	mapper.internal_struct[1] = (BYTE *)&mmc1;
	mapper.internal_struct_size[1] = sizeof(mmc1);

	memset(&irqA12, 0x00, sizeof(irqA12));
	memset(&m351, 0x00, sizeof(m351));

	init_MMC3(HARD);
	MMC3_prg_swap = prg_swap_mmc3_351;
	MMC3_chr_swap = chr_swap_mmc3_351;

	init_VRC2and4(VRC24_VRC4, 0x04, 0x08, TRUE, HARD);
	VRC2and4_prg_swap = prg_swap_vrc2and4_351;
	VRC2and4_chr_swap = chr_swap_vrc2and4_351;

	init_MMC1(MMC1A, HARD);
	MMC1_prg_swap = prg_swap_mmc1_351;
	MMC1_chr_swap = chr_swap_mmc1_351;

	if ((info.reset == CHANGE_ROM) || (info.reset == POWER_UP)) {
		// Unusually, in CHR-RAM mode (R=1), CHR-ROM becomes the second half of an enlarged PRG address space that
		// becomes addressable via register $5001. At least one multicart containing both TLROM and UNROM games makes
		// use of this feature and puts the UNROM game's PRG data into CHR-ROM. This seems to be possible as the mapper ASIC,
		// PRG and CHR-ROM are under a single glob.
		if (chrrom_size()) {
			size_t old_prgrom_size = prgrom_size();

			prgrom_set_size(prgrom_size() + chrrom_size());
			prgrom_pnt() = realloc(prgrom_pnt(), prgrom.real_size);
			memcpy(prgrom_pnt() + old_prgrom_size, chrrom_pnt(), chrrom_size());
		}
	}

	info.mapper.extend_wr = TRUE;

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_after_mapper_init_351(void) {
	switch_mode();
	fix_all();
}
void extcl_cpu_wr_mem_351(WORD address, BYTE value) {
	if ((address >= 0x5000) && (address <= 0x5FFF)) {
		BYTE reg = address & 0x03;

		switch (reg) {
			case 0:
				m351.reg[0] = value;
				switch_mode();
				break;
			case 1:
			case 2:
				m351.reg[reg] = value;
				break;
			default:
				break;
		}
		fix_all();
		return;
	}
	if (m351.mapper == M351_MMC3) {
		extcl_cpu_wr_mem_MMC3(address, value);
	} else if (m351.mapper == M351_VRC4) {
		if (address & 0x0800) {
			address = (address & 0xFFF3) | ((address & 0x0004) << 1) | ((address & 0x0008) >> 1);
		}
		extcl_cpu_wr_mem_VRC2and4(address, value);
	} else if (m351.mapper == M351_MMC1) {
		extcl_cpu_wr_mem_MMC1(address, value);
	}
}
BYTE extcl_cpu_rd_mem_351(WORD address, BYTE openbus) {
	if ((address >= 0x5000) && (address <= 0x5FFF)) {
		return ((openbus & 0xF8) | (dipswitch.value & 0x07));
	}
	return (wram_rd(address));
}
BYTE extcl_save_mapper_351(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m351.mapper);
	save_slot_ele(mode, slot, m351.reg);
	if (extcl_save_mapper_MMC3(mode, slot, fp) == EXIT_ERROR) return (EXIT_ERROR);
	if (extcl_save_mapper_VRC2and4(mode, slot, fp) == EXIT_ERROR) return (EXIT_ERROR);
	return (extcl_save_mapper_MMC1(mode, slot, fp));
}
void extcl_cpu_every_cycle_351(void) {
	if (m351.mapper == M351_MMC3) {
		extcl_cpu_every_cycle_MMC3();
	} else if (m351.mapper == M351_VRC4) {
		extcl_cpu_every_cycle_VRC2and4();
	}
}
void extcl_ppu_000_to_34x_351(void) {
	if (m351.mapper == M351_MMC3) {
		extcl_ppu_000_to_34x_MMC3();
	}
}
void extcl_ppu_000_to_255_351(void) {
	if (m351.mapper == M351_MMC3) {
		extcl_ppu_000_to_255_MMC3();
	}
}
void extcl_ppu_256_to_319_351(void) {
	if (m351.mapper == M351_MMC3) {
		extcl_ppu_256_to_319_MMC3();
	}
}
void extcl_ppu_320_to_34x_351(void) {
	if (m351.mapper == M351_MMC3) {
		extcl_ppu_320_to_34x_MMC3();
	}
}
void extcl_update_r2006_351(WORD new_r2006, WORD old_r2006) {
	if (m351.mapper == M351_MMC3) {
		extcl_update_r2006_MMC3(new_r2006, old_r2006);
	}
}

INLINE static void switch_mode(void) {
	switch (m351.reg[0] & 0x03) {
		default:
		case 0:
		case 1:
			m351.mapper = M351_MMC3;
			break;
		case 2:
			m351.mapper = M351_MMC1;
			nes.c.irq.high &= ~EXT_IRQ;
			break;
		case 3:
			m351.mapper = M351_VRC4;
			nes.c.irq.high &= ~EXT_IRQ;
			break;
	}
}
INLINE static void fix_all(void) {
	prg_fix();
	chr_fix();
	wram_mirroring_fix();
}
INLINE static void prg_fix(void) {
	WORD bank = 0;

	if (m351.reg[2] & 0x10) {
		if (m351.reg[2] & 0x04) {
			bank = m351.reg[1] >> 2;
			memmap_auto_16k(MMCPU(0x8000), bank);
			memmap_auto_16k(MMCPU(0xC000), bank);
		} else {
			bank = m351.reg[1] >> 3;
			memmap_auto_32k(MMCPU(0x8000), bank);
		}
	} else {
		if (m351.mapper == M351_MMC3) {
			MMC3_prg_fix();
		} else if (m351.mapper == M351_VRC4) {
			VRC2and4_prg_fix();
		} else if (m351.mapper == M351_MMC1) {
			MMC1_prg_fix();
		}
	}
}
INLINE static void chr_fix(void) {
	if ((m351.reg[2] & 0x01) && wram_size()) {
		memmap_vram_8k(MMPPU(0x0000), 0);
	} else if (m351.reg[2] & 0x40) {
		memmap_auto_8k(MMPPU(0x0000), (m351.reg[0] >> 2));
	} else {
		if (m351.mapper == M351_MMC3) {
			MMC3_chr_fix();
		} else if (m351.mapper == M351_VRC4) {
			VRC2and4_chr_fix();
		} else if (m351.mapper == M351_MMC1) {
			MMC1_chr_fix();
		}
	}
}
INLINE static void wram_mirroring_fix(void) {
	switch (m351.mapper) {
		default:
		case M351_MMC3:
			MMC3_wram_fix();
			MMC3_mirroring_fix();
			break;
		case M351_VRC4:
			VRC2and4_wram_fix();
			VRC2and4_mirroring_fix();
			break;
		case M351_MMC1:
			MMC1_wram_fix();
			MMC1_mirroring_fix();
			break;
	}
}
INLINE static WORD prg_base(void) {
	return (m351.reg[1] >> 1);
}
INLINE static WORD prg_mask(void) {
	return (0x1F >> ((m351.reg[2] & 0x04) >> 2));
}
INLINE static WORD chr_base(void) {
	return (m351.reg[0] << 1);
}
INLINE static WORD chr_mask(void) {
	return ((m351.reg[2] & 0x10) ? 0x1F : (m351.reg[2] & 0x20 ? 0x7F : 0xFF));
}

void prg_swap_mmc3_351(WORD address, WORD value) {
	WORD base = prg_base();
	WORD mask = prg_mask();

	prg_swap_MMC3_base(address, ((base & ~mask) | (value & mask)));
}
void chr_swap_mmc3_351(WORD address, WORD value) {
	WORD base = chr_base();
	WORD mask = chr_mask();

	chr_swap_MMC3_base(address, ((base & ~mask) | (value & mask)));
}

void prg_swap_vrc2and4_351(WORD address, WORD value) {
	WORD base = prg_base();
	WORD mask = prg_mask();

	prg_swap_VRC2and4_base(address, ((base & ~mask) | (value & mask)));
}
void chr_swap_vrc2and4_351(WORD address, WORD value) {
	WORD base = chr_base();
	WORD mask = chr_mask();

	chr_swap_VRC2and4_base(address, ((base & ~mask) | (value & mask)));
}

void prg_swap_mmc1_351(WORD address, WORD value) {
	WORD base = prg_base() >> 1;
	WORD mask = prg_mask() >> 1;

	prg_swap_MMC1_base(address, (base & ~mask) | (value & mask));
}
void chr_swap_mmc1_351(WORD address, WORD value) {
	WORD base = chr_base() >> 2;
	WORD mask = chr_mask() >> 2;

	chr_swap_MMC1_base(address, (base & ~mask) | (value & mask));
}
