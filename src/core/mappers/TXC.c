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
#include "cpu.h"
#include "save_slot.h"

void (*TXC_prg_fix)(void);
void (*TXC_chr_fix)(void);
void (*TXC_wram_fix)(void);
void (*TXC_mirroring_fix)(void);

_txc txc;

// promemoria
//void map_init_TXC(void) {
//	EXTCL_AFTER_MAPPER_INIT(TXC);
//	EXTCL_CPU_WR_MEM(TXC);
//	EXTCL_CPU_RD_MEM(TXC);
//	EXTCL_SAVE_MAPPER(TXC);
//}

void extcl_after_mapper_init_TXC(void) {
	TXC_prg_fix();
	TXC_chr_fix();
	TXC_wram_fix();
	TXC_mirroring_fix();
}
void extcl_cpu_wr_mem_TXC(WORD address, BYTE value) {
	switch (address & 0xE000) {
		case 0x4000:
		case 0x6000:
			switch (address & 0x0103) {
				case 0x100:
					txc.accumulator = txc.increase
						? txc.accumulator + 1
						: (txc.accumulator & 0xF8) | ((txc.staging ^ txc.invert) & 0x07);
					break;
				case 0x101:
					txc.invert = 0xFF * (value & 0x01);
					break;
				case 0x102:
					txc.staging = value & 0x07;
					txc.inverter = value & 0xF8;
					break;
				case 0x103:
					txc.increase = value & 0x01;
					break;
			}
			break;
		case 0x8000:
		case 0xA000:
		case 0xC000:
		case 0xE000:
			txc.output = ((txc.inverter & 0x08) << 1) | (txc.accumulator & 0x0F);
			break;
	}
	txc.X = txc.invert ? txc.A : txc.B;
	txc.Y = txc.X | ((value & 0x10) >> 4);
	TXC_prg_fix();
	TXC_chr_fix();
	TXC_wram_fix();
	TXC_mirroring_fix();
}
BYTE extcl_cpu_rd_mem_TXC(WORD address, UNUSED(BYTE openbus)) {
	if ((address & 0x0103) == 0x0100) {
		openbus = ((txc.inverter ^ txc.invert) & 0xF8) | (txc.accumulator & 0x07);
		txc.Y = txc.X | ((openbus & 0x10) >> 4);
		TXC_prg_fix();
		TXC_chr_fix();
		TXC_wram_fix();
		TXC_mirroring_fix();
	}
	return (openbus);
}
BYTE extcl_save_mapper_TXC(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, txc.increase);
	save_slot_ele(mode, slot, txc.output);
	save_slot_ele(mode, slot, txc.invert);
	save_slot_ele(mode, slot, txc.staging);
	save_slot_ele(mode, slot, txc.accumulator);
	save_slot_ele(mode, slot, txc.inverter);
	save_slot_ele(mode, slot, txc.A);
	save_slot_ele(mode, slot, txc.B);
	save_slot_ele(mode, slot, txc.X);
	save_slot_ele(mode, slot, txc.Y);

	return (EXIT_OK);
}

void init_TXC(void) {
	if (info.reset >= HARD) {
		memset(&txc, 0x00, sizeof(txc));

		txc.B = 1;
	}

	info.mapper.extend_wr = TRUE;
	info.mapper.extend_rd = TRUE;

	TXC_prg_fix = prg_fix_TXC_base;
	TXC_chr_fix = chr_fix_TXC_base;
	TXC_wram_fix = wram_fix_TXC_base;
	TXC_mirroring_fix = mirroring_fix_TXC_base;
}

void prg_fix_TXC_base(void) {}
void chr_fix_TXC_base(void) {}
void wram_fix_TXC_base(void) {}
void mirroring_fix_TXC_base(void) {}
