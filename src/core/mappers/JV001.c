/*
 *  Copyright (C) 2010-2026 Fabio Cavallo (aka FHorse)
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

void (*JV001_prg_fix)(void);
void (*JV001_chr_fix)(void);
void (*JV001_wram_fix)(void);
void (*JV001_mirroring_fix)(void);

_jv001 jv001;

// promemoria
//void map_init_JV001(void) {
//	EXTCL_AFTER_MAPPER_INIT(JV001);
//	EXTCL_CPU_WR_MEM(JV001);
//	EXTCL_CPU_RD_MEM(JV001);
//	EXTCL_SAVE_MAPPER(JV001);
//}

void extcl_after_mapper_init_JV001(void) {
	JV001_prg_fix();
	JV001_chr_fix();
	JV001_wram_fix();
	JV001_mirroring_fix();
}
void extcl_cpu_wr_mem_JV001(UNUSED(BYTE nidx), WORD address, BYTE value) {
	switch (address & 0xE000) {
		case 0x4000:
		case 0x6000:
			switch (address & 0x0103) {
				case 0x100:
					jv001.accumulator = jv001.increase
						? jv001.accumulator + 1
						: (jv001.accumulator & 0xF0) | ((jv001.staging ^ jv001.invert) & 0x0F);
					break;
				case 0x101:
					jv001.invert = 0xFF * (value & 0x01);
					break;
				case 0x102:
					jv001.staging = value & 0x0F;
					jv001.inverter = value & 0xF0;
					break;
				case 0x103:
					jv001.increase = value & 0x01;
					break;
			}
			break;
		case 0x8000:
		case 0xA000:
		case 0xC000:
		case 0xE000:
			jv001.output = (jv001.inverter & 0xF0) | (jv001.accumulator & 0x0F);
			break;
	}
	jv001.X = jv001.invert ? jv001.A : jv001.B;
	JV001_prg_fix();
	JV001_chr_fix();
	JV001_wram_fix();
	JV001_mirroring_fix();
}
BYTE extcl_cpu_rd_mem_JV001(UNUSED(BYTE nidx), WORD address, BYTE openbus) {
	if ((address & 0x0103) == 0x0100) {
		openbus = ((jv001.inverter ^ jv001.invert) & 0xF0) | (jv001.accumulator & 0x0F);
		JV001_prg_fix();
		JV001_chr_fix();
		JV001_wram_fix();
		JV001_mirroring_fix();
	}
	return (openbus);
}
BYTE extcl_save_mapper_JV001(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, jv001.increase);
	save_slot_ele(mode, slot, jv001.output);
	save_slot_ele(mode, slot, jv001.invert);
	save_slot_ele(mode, slot, jv001.staging);
	save_slot_ele(mode, slot, jv001.accumulator);
	save_slot_ele(mode, slot, jv001.inverter);
	save_slot_ele(mode, slot, jv001.A);
	save_slot_ele(mode, slot, jv001.B);
	save_slot_ele(mode, slot, jv001.X);
	return (EXIT_OK);
}

void init_JV001(BYTE reset) {
	if (reset >= HARD) {
		memset(&jv001, 0x00, sizeof(jv001));

		jv001.invert = 0xFF;
		jv001.B = 1;
	}

	info.mapper.extend_wr = TRUE;

	JV001_prg_fix = prg_fix_JV001_base;
	JV001_chr_fix = chr_fix_JV001_base;
	JV001_wram_fix = wram_fix_JV001_base;
	JV001_mirroring_fix = mirroring_fix_JV001_base;
}
void prg_fix_JV001_base(void) {}
void chr_fix_JV001_base(void) {}
void wram_fix_JV001_base(void) {}
void mirroring_fix_JV001_base(void) {}
