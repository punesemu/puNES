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

enum m256_models {
	M256_NORMAL,
	M256_WAIXING_VT03,
	M256_POWER_JOY_SUPERMAX,
	M256_ZECHESS,
	M256_SPORTS_GAME,
	M256_WAIXING_VT02,
	M256_UNUSED_CUBE_TECH = 13,
	M256_UNUSED_KARAOTO,
	M256_UNUSED_JUNGLETAC,
	M256_SUBMAPPERS
};

typedef struct _m256_regs {
	BYTE cpu[4];
	BYTE mmc3[8];
	BYTE ppu[6];
} _m256_regs;

static const _m256_regs regs[M256_SUBMAPPERS] = {
//         ---- cpu -----  ---------- mmc3 ----------  -------- ppu -------
/*  0 */ { { 0, 1, 2, 3 }, { 0, 1, 2, 3, 4, 5, 6, 7 }, { 0, 1, 2, 3, 4, 5 } }, // M256_NORMAL
/*  1 */ { { 0, 1, 2, 3 }, { 5, 4, 3, 2, 1, 0, 6, 7 }, { 1, 0, 5, 4, 3, 2 } }, // M256_WAIXING_VT03
/*  2 */ { { 1, 0, 2, 3 }, { 0, 1, 2, 3, 4, 5, 7, 6 }, { 0, 1, 2, 3, 4, 5 } }, // M256_POWER_JOY_SUPERMAX
/*  3 */ { { 0, 1, 2, 3 }, { 0, 1, 2, 3, 4, 5, 6, 7 }, { 5, 4, 3, 2, 0, 1 } }, // M256_ZECHESS
/*  4 */ { { 0, 1, 2, 3 }, { 0, 1, 2, 3, 4, 5, 6, 7 }, { 2, 5, 0, 4, 3, 1 } }, // M256_SPORTS_GAME
/*  5 */ { { 0, 1, 2, 3 }, { 0, 1, 2, 3, 4, 5, 6, 7 }, { 1, 0, 5, 4, 3, 2 } }, // M256_WAIXING_VT02
/*  6 */ { { 0, 1, 2, 3 }, { 0, 1, 2, 3, 4, 5, 6, 7 }, { 0, 1, 2, 3, 4, 5 } }, // inutilizzato
/*  7 */ { { 0, 1, 2, 3 }, { 0, 1, 2, 3, 4, 5, 6, 7 }, { 0, 1, 2, 3, 4, 5 } }, // inutilizzato
/*  8 */ { { 0, 1, 2, 3 }, { 0, 1, 2, 3, 4, 5, 6, 7 }, { 0, 1, 2, 3, 4, 5 } }, // inutilizzato
/*  9 */ { { 0, 1, 2, 3 }, { 0, 1, 2, 3, 4, 5, 6, 7 }, { 0, 1, 2, 3, 4, 5 } }, // inutilizzato
/* 10 */ { { 0, 1, 2, 3 }, { 0, 1, 2, 3, 4, 5, 6, 7 }, { 0, 1, 2, 3, 4, 5 } }, // inutilizzato
/* 11 */ { { 0, 1, 2, 3 }, { 0, 1, 2, 3, 4, 5, 6, 7 }, { 0, 1, 2, 3, 4, 5 } }, // inutilizzato
/* 12 */ { { 0, 1, 2, 3 }, { 0, 1, 2, 3, 4, 5, 6, 7 }, { 0, 1, 2, 3, 4, 5 } }, // inutilizzato
/* 13 */ { { 0, 1, 2, 3 }, { 0, 1, 2, 3, 4, 5, 6, 7 }, { 0, 1, 2, 3, 4, 5 } }, // M256_UNUSED_CUBE_TECH
/* 14 */ { { 0, 1, 2, 3 }, { 0, 1, 2, 3, 4, 5, 6, 7 }, { 0, 1, 2, 3, 4, 5 } }, // M256_UNUSED_KARAOTO
/* 15 */ { { 0, 1, 2, 3 }, { 0, 1, 2, 3, 4, 5, 6, 7 }, { 0, 1, 2, 3, 4, 5 } }, // M256_UNUSED_JUNGLETAC
};

void map_init_256(void) {
	EXTCL_AFTER_MAPPER_INIT(256);
	EXTCL_MAPPER_QUIT(OneBus);
	EXTCL_CPU_WR_MEM(256);
	EXTCL_CPU_RD_MEM(OneBus);
	EXTCL_SAVE_MAPPER(OneBus);
	EXTCL_WR_PPU_REG(256);
	EXTCL_WR_APU(OneBus);
	EXTCL_RD_APU(OneBus);
	EXTCL_RD_CHR(OneBus);
	EXTCL_CPU_EVERY_CYCLE(OneBus);
	EXTCL_IRQ_A12_CLOCK(OneBus);
	EXTCL_PPU_000_TO_34X(OneBus);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *)&onebus;
	mapper.internal_struct_size[0] = sizeof(onebus);

	memset(&nes[0].irqA12, 0x00, sizeof(nes[0].irqA12));

	if (info.format != NES_2_0) {
		if (info.mapper.submapper == 0) {
			if ((info.crc32.prg == 0x947AC898) || // Power Joy Supermax 30-in-1 (Unl) [U][!].unf
				(info.crc32.prg == 0x1AB45228)) { // Power Joy Supermax 60-in-1 (Unl) [U][!].unf
				info.mapper.submapper = M256_POWER_JOY_SUPERMAX;
			} else if (info.crc32.prg == 0x1242DA7F) {
				// "Sports Game 69-in-1 (Unl) [U][!].unf" e "DreamGEAR 75-in-1 (Unl) [U][!].unf"
				// alcuni giochi (tipo SHARKS) di queste roms non funzionano correttamente perche' utilizzano l'extended mode
				// del VT03, lo si puo' notare dal fatto che viene settato il registro $2010 a 0x86 o 0x96 (ho dato un'occhiata
				// al src/devices/video/ppu2c0x_vt.cpp del mame per capire il significato dei bits di questo registro).
				info.mapper.submapper = M256_SPORTS_GAME;
			} else if (info.crc32.prg == 0xF7138980) {
				info.mapper.ext_console_type = VT09;
			}
		}
	}

	init_OneBus(info.reset);

	nes[0].irqA12.present = TRUE;
}
void extcl_after_mapper_init_256(void) {
	OneBus_prg_fix_8k(0x0FFF, 0);
	OneBus_chr_fix(0x7FFF, 0);
	OneBus_wram_fix(0x0FFF, 0);
	OneBus_mirroring_fix();
}
void extcl_cpu_wr_mem_256(BYTE nidx, WORD address, BYTE value) {
	if ((address >= 0x4107) && (address <= 0x410A)) {
		address = 0x4107 + regs[info.mapper.submapper].cpu[address - 0x4107];
	} else if ((address & 0xE001) == 0x8000) {
		value = (value & 0xF8) | regs[info.mapper.submapper].mmc3[value & 0x07];
	}
	extcl_cpu_wr_mem_OneBus(nidx, address, value);
}
BYTE extcl_wr_ppu_reg_256(BYTE nidx, WORD address, BYTE *value) {
	if ((address >= 0x2012) && (address <= 0x2017)) {
		address = 0x2012 + regs[info.mapper.submapper].ppu[address - 0x2012];
	}
	return (extcl_wr_ppu_reg_OneBus(nidx, address, value));
}
