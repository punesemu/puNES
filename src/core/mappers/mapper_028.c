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
#include "cpu.h"
#include "save_slot.h"

enum m28_reg { INNERBNK, MODEBNK, OUTERBNK };

INLINE static void nmt_setup_028(void);
INLINE static void prg_setup_028(void);
INLINE static BYTE calc_prg_bank_028(WORD address);

struct _m028 {
	BYTE index;
	BYTE mirroring;
	BYTE prg[3];
} m028;
static BYTE const inner_and[4] = { 0x01, 0x03, 0x07, 0x0F };

void map_init_028(void) {
	EXTCL_CPU_WR_MEM(028);
	EXTCL_CPU_RD_MEM(028);
	EXTCL_SAVE_MAPPER(028);
	mapper.internal_struct[0] = (BYTE *)&m028;
	mapper.internal_struct_size[0] = sizeof(m028);

	if (info.reset >= HARD) {
		memset(&m028, 0x00, sizeof(m028));
		m028.prg[OUTERBNK] = 0x3F;
		m028.prg[INNERBNK] = 0x0F;
	}

	info.mapper.extend_wr = TRUE;
}
void extcl_cpu_wr_mem_028(WORD address, BYTE value) {
	if (address < 0x5000) {
		return;
	}
	if (address < 0x6000) {
		m028.index = ((value & 0x80) >> 6) | (value & 0x01);
		return;
	}
	if (address < 0x8000) {
		return;
	}

	switch (m028.index) {
		case 0: {
			DBWORD bank;

			if (!(m028.mirroring & 0x02)) {
				m028.mirroring = (m028.mirroring & 0x02) | ((value & 0x10) >> 4);
				nmt_setup_028();
			}

			value &= 0x03;
			control_bank(info.chr.rom.max.banks_8k)
			bank = value << 13;
			chr.bank_1k[0] = chr_pnt(bank);
			chr.bank_1k[1] = chr_pnt(bank | 0x0400);
			chr.bank_1k[2] = chr_pnt(bank | 0x0800);
			chr.bank_1k[3] = chr_pnt(bank | 0x0C00);
			chr.bank_1k[4] = chr_pnt(bank | 0x1000);
			chr.bank_1k[5] = chr_pnt(bank | 0x1400);
			chr.bank_1k[6] = chr_pnt(bank | 0x1800);
			chr.bank_1k[7] = chr_pnt(bank | 0x1C00);
			return;
		}
		case 1:
			m028.prg[INNERBNK] = value & 0x0F;
			prg_setup_028();

			if (!(m028.mirroring & 0x02)) {
				m028.mirroring = (m028.mirroring & 0x02) | ((value & 0x10) >> 4);
				nmt_setup_028();
			}
			return;
		case 2:
			m028.prg[MODEBNK] = value & 0x3C;
			prg_setup_028();

			m028.mirroring = value & 0x03;
			nmt_setup_028();
			return;
		case 3:
			m028.prg[OUTERBNK] = value & 0x3F;
			prg_setup_028();
			return;
	}
}
BYTE extcl_cpu_rd_mem_028(WORD address, BYTE openbus) {
	if ((address > 0x4FFF) && (address < 0x6000)) {
		return (cpu.openbus.before);
	}
	return (openbus);
}
BYTE extcl_save_mapper_028(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m028.index);
	save_slot_ele(mode, slot, m028.mirroring);
	save_slot_ele(mode, slot, m028.prg);

	return (EXIT_OK);
}

INLINE static void nmt_setup_028(void) {
	switch (m028.mirroring) {
		case 0:
			mirroring_SCR0();
			break;
		case 1:
			mirroring_SCR1();
			break;
		case 2:
			mirroring_V();
			break;
		case 3:
			mirroring_H();
			break;
	}
}
INLINE static void prg_setup_028(void) {
	BYTE value;

	value = calc_prg_bank_028(0x8000);
	control_bank(info.prg.rom.max.banks_16k)
	map_prg_rom_8k(2, 0, value);

	value = calc_prg_bank_028(0xC000);
	control_bank(info.prg.rom.max.banks_16k)
	map_prg_rom_8k(2, 2, value);

	map_prg_rom_8k_update();
}
INLINE static BYTE calc_prg_bank_028(WORD address) {
	BYTE cpu_a14 = (address >> 14) & 0x01;
	BYTE outer_bank = m028.prg[OUTERBNK] << 1;
	BYTE bank_mode = m028.prg[MODEBNK] >> 2;
	BYTE current_bank = m028.prg[INNERBNK];

	if (((bank_mode ^ cpu_a14) & 0x03) == 0x02) {
		bank_mode = 0;
	}
	if ((bank_mode & 0x02) == 0) {
		current_bank = (current_bank << 1) | cpu_a14;
	}
	return (((current_bank ^ outer_bank) & inner_and[(bank_mode >> 2) & 0x03]) ^ outer_bank);
}