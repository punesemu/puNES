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

INLINE static void prg_fix_KS106C(void);

struct _ks106ctmp {
	BYTE chip;
} ks106ctmp;

void map_init_KS106C(void) {
	EXTCL_CPU_WR_MEM(KS106C);

	if (info.reset >= HARD) {
		ks106ctmp.chip = 0;
	} else {
		ks106ctmp.chip++;
	}
	prg_fix_KS106C();
}
void extcl_cpu_wr_mem_KS106C(UNUSED(WORD address), UNUSED(BYTE value)) {}

INLINE static void prg_fix_KS106C(void) {
	DBWORD bank;

	bank = ks106ctmp.chip;
	_control_bank(bank, info.prg.rom.max.banks_32k)
	map_prg_rom_8k(4, 0, bank);
	map_prg_rom_8k_update();

	bank = ks106ctmp.chip;
	_control_bank(bank, info.chr.rom.max.banks_8k)
	bank <<= 13;
	chr.bank_1k[0] = chr_pnt(bank | 0x0000);
	chr.bank_1k[1] = chr_pnt(bank | 0x0400);
	chr.bank_1k[2] = chr_pnt(bank | 0x0800);
	chr.bank_1k[3] = chr_pnt(bank | 0x0C00);
	chr.bank_1k[4] = chr_pnt(bank | 0x1000);
	chr.bank_1k[5] = chr_pnt(bank | 0x1400);
	chr.bank_1k[6] = chr_pnt(bank | 0x1800);
	chr.bank_1k[7] = chr_pnt(bank | 0x1C00);

	if (ks106ctmp.chip & 0x01) {
		mirroring_V();
	} else {
		mirroring_H();
	}
}
