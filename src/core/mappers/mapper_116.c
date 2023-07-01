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

enum _m116_mappers { M116_MMC3 = 1, M116_VRC2, M116_MMC1 };

INLINE static void switch_mode(void);
INLINE static void fix_all(void);
INLINE static WORD prg_base(void);
INLINE static WORD prg_mask(void);
INLINE static WORD chr_base(void);
INLINE static WORD chr_mask(void);

void prg_swap_mmc3_116(WORD address, WORD value);
void chr_swap_mmc3_116(WORD address, WORD value);

void prg_swap_vrc2and4_116(WORD address, WORD value);
void chr_swap_vrc2and4_116(WORD address, WORD value);

void prg_swap_mmc1_116(WORD address, WORD value);
void chr_swap_mmc1_116(WORD address, WORD value);

struct _m116 {
	BYTE mapper;
	WORD reg;
} m116;
struct _m116tmp {
	BYTE game;
} m116tmp;

void map_init_116(void) {
	EXTCL_AFTER_MAPPER_INIT(116);
	EXTCL_CPU_WR_MEM(116);
	EXTCL_CPU_RD_MEM(116);
	EXTCL_SAVE_MAPPER(116);
	EXTCL_CPU_EVERY_CYCLE(116);
	EXTCL_PPU_000_TO_34X(116);
	EXTCL_PPU_000_TO_255(116);
	EXTCL_PPU_256_TO_319(116);
	EXTCL_PPU_320_TO_34X(116);
	EXTCL_UPDATE_R2006(116);
	mapper.internal_struct[0] = (BYTE *)&m116;
	mapper.internal_struct_size[0] = sizeof(m116);
	mapper.internal_struct[1] = (BYTE *)&mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);
	mapper.internal_struct[2] = (BYTE *)&vrc2and4;
	mapper.internal_struct_size[2] = sizeof(vrc2and4);
	mapper.internal_struct[1] = (BYTE *)&mmc1;
	mapper.internal_struct_size[1] = sizeof(mmc1);

	memset(&irqA12, 0x00, sizeof(irqA12));
	memset(&m116, 0x00, sizeof(m116));

	init_MMC3(info.reset);
	MMC3_prg_swap = prg_swap_mmc3_116;
	MMC3_chr_swap = chr_swap_mmc3_116;

	init_VRC2and4(VRC24_VRC2, 0x01, 0x02, TRUE, info.reset);
	VRC2and4_prg_swap = prg_swap_vrc2and4_116;
	VRC2and4_chr_swap = chr_swap_vrc2and4_116;

	init_MMC1(MMC1A, info.reset);
	MMC1_prg_swap = prg_swap_mmc1_116;
	MMC1_chr_swap = chr_swap_mmc1_116;

	// AV Kyuukyoku Mahjong 2 (Asia) (Ja) (Ge De) (Unl).nes
	if ((prgrom_size() == S128K) && (prgrom_size() == chrrom_size())) {
		info.mapper.submapper = 2;
	};

	if (info.reset >= HARD) {
		vrc2and4.chr[0] = 0xFF;
		vrc2and4.chr[1] = 0xFF;
		vrc2and4.chr[2] = 0xFF;
		vrc2and4.chr[3] = 0xFF;

		m116.reg = 0x01;

		if (info.mapper.submapper == 3) {
			m116tmp.game = (info.reset == CHANGE_ROM) || (info.reset == POWER_UP)
				? 0
				: (m116tmp.game + 1) % 4;
		}
	}

	info.mapper.extend_wr = TRUE;

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_after_mapper_init_116(void) {
	switch_mode();
	fix_all();
}
void extcl_cpu_wr_mem_116(WORD address, BYTE value) {
	if ((address >= 0x4000) && (address <= 0x5FFF)) {
		if (address & 0x0100) {
			m116.reg = value;
			extcl_after_mapper_init_116();
		}
		return;
	}
	if (m116.mapper == M116_MMC3) {
		extcl_cpu_wr_mem_MMC3(address, value);
	} else if (m116.mapper == M116_VRC2) {
		extcl_cpu_wr_mem_VRC2and4(address, value);
	} else if (m116.mapper == M116_MMC1) {
		extcl_cpu_wr_mem_MMC1(address, value);
	}
}
BYTE extcl_cpu_rd_mem_116(WORD address, UNUSED(BYTE openbus)) {
	if (m116.mapper == M116_VRC2) {
		return (extcl_cpu_rd_mem_VRC2and4(address, wram_rd(address)));
	}
	return (wram_rd(address));
}
BYTE extcl_save_mapper_116(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m116.mapper);
	save_slot_ele(mode, slot, m116.reg);
	save_slot_ele(mode, slot, m116tmp.game);
	if (extcl_save_mapper_MMC3(mode, slot, fp) == EXIT_ERROR) return (EXIT_ERROR);
	if (extcl_save_mapper_VRC2and4(mode, slot, fp) == EXIT_ERROR) return (EXIT_ERROR);
	return (extcl_save_mapper_MMC1(mode, slot, fp));
}
void extcl_cpu_every_cycle_116(void) {
	if (m116.mapper == M116_MMC3) {
		extcl_cpu_every_cycle_MMC3();
	}
}
void extcl_ppu_000_to_34x_116(void) {
	if (m116.mapper == M116_MMC3) {
		extcl_ppu_000_to_34x_MMC3();
	}
}
void extcl_ppu_000_to_255_116(void) {
	if (m116.mapper == M116_MMC3) {
		extcl_ppu_000_to_255_MMC3();
	}
}
void extcl_ppu_256_to_319_116(void) {
	if (m116.mapper == M116_MMC3) {
		extcl_ppu_256_to_319_MMC3();
	}
}
void extcl_ppu_320_to_34x_116(void) {
	if (m116.mapper == M116_MMC3) {
		extcl_ppu_320_to_34x_MMC3();
	}
}
void extcl_update_r2006_116(WORD new_r2006, WORD old_r2006) {
	if (m116.mapper == M116_MMC3) {
		extcl_update_r2006_MMC3(new_r2006, old_r2006);
	}
}

INLINE static void switch_mode(void) {
	switch (m116.reg & 0x03) {
		case 1:
			m116.mapper = M116_MMC3;
			break;
		case 2:
		case 3:
			m116.mapper = M116_MMC1;
			irq.high &= ~EXT_IRQ;
			if (info.mapper.submapper != 1) {
				extcl_cpu_wr_mem_MMC1(0x8000, 0x80);
			}
			break;
		case 0:
			m116.mapper = M116_VRC2;
			irq.high &= ~EXT_IRQ;
			break;
	}
}
INLINE static void fix_all(void) {
	if (m116.mapper == M116_MMC3) {
		extcl_after_mapper_init_MMC3();
	} else if (m116.mapper == M116_VRC2) {
		extcl_after_mapper_init_VRC2and4();
	} else if (m116.mapper == M116_MMC1) {
		extcl_after_mapper_init_MMC1();
	}
}
INLINE static WORD prg_base(void) {
	return (m116tmp.game ? (m116tmp.game + 1) * 0x10 : 0x00);
}
INLINE static WORD prg_mask(void) {
	return (info.mapper.submapper != 3 ? 0x3F : m116tmp.game ? 0x0F : 0x1F);
}
INLINE static WORD chr_base(void) {
	return (m116tmp.game ? (m116tmp.game + 1) * 0x80 : 0x00);
}
INLINE static WORD chr_mask(void) {
	return (m116tmp.game ? 0x7F : 0xFF);
}

void prg_swap_mmc3_116(WORD address, WORD value) {
	WORD base = prg_base();
	WORD mask = prg_mask();

	prg_swap_MMC3_base(address, ((base & ~mask) | (value & mask)));
}
void chr_swap_mmc3_116(WORD address, WORD value) {
	WORD base = ((m116.reg & 0x04) << 6) | chr_base();
	WORD mask = chr_mask();

	chr_swap_MMC3_base(address, ((base & ~mask) | (value & mask)));
}

void prg_swap_vrc2and4_116(WORD address, WORD value) {
	WORD base = prg_base();
	WORD mask = prg_mask();

	prg_swap_VRC2and4_base(address, ((base & ~mask) | (value & mask)));
}
void chr_swap_vrc2and4_116(WORD address, WORD value) {
	WORD base = ((m116.reg & 0x04) << 6) | chr_base();
	WORD mask = chr_mask();

	chr_swap_VRC2and4_base(address, ((base & ~mask) | (value & mask)));
}

void prg_swap_mmc1_116(WORD address, WORD value) {
	if (info.mapper.submapper == 2) {
		value >>= 1;
	} else {
		WORD base = prg_base() >> 1;
		WORD mask = prg_mask() >> 1;

		value = (base & ~mask) | (value & mask);
	}
	prg_swap_MMC1_base(address, value);
}
void chr_swap_mmc1_116(WORD address, WORD value) {
	WORD base = chr_base() >> 2;
	WORD mask = chr_mask() >> 2;

	chr_swap_MMC1_base(address, (base & ~mask) | (value & mask));
}
