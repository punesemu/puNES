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

#define mirroring(data)\
	switch (data & 0x03) {\
		case 0:\
			mirroring_V();\
			break;\
		case 1:\
			mirroring_H();\
			break;\
		case 2:\
			mirroring_SCR0();\
			break;\
		case 3:\
			mirroring_SCR1();\
			break;\
		}
#define chr_rom_2k_swap(slot)\
{\
	DBWORD bank;\
	control_bank(info.chr.rom.max.banks_2k)\
	bank = value << 11;\
	chr.bank_1k[slot] = chr_pnt(bank);\
	chr.bank_1k[slot + 1] = chr_pnt(bank | 0x400);\
}
#define s4_mirroring()\
	if (!s4.mode) {\
		switch (s4.mirroring & 0x03) {\
		case 0:\
			mirroring_V();\
			break;\
		case 1:\
			mirroring_H();\
			break;\
		case 2:\
			mirroring_SCR0();\
			break;\
		case 3:\
			mirroring_SCR1();\
			break;\
		}\
	} else {\
		switch (s4.mirroring & 0x03) {\
		case 0:\
			ntbl.bank_1k[0] = ntbl.bank_1k[2] = chr_pnt(s4.chr_nmt[0]);\
			ntbl.bank_1k[1] = ntbl.bank_1k[3] = chr_pnt(s4.chr_nmt[1]);\
			break;\
		case 1:\
			ntbl.bank_1k[0] = ntbl.bank_1k[1] = chr_pnt(s4.chr_nmt[0]);\
			ntbl.bank_1k[2] = ntbl.bank_1k[3] = chr_pnt(s4.chr_nmt[1]);\
			break;\
		case 2:\
			ntbl.bank_1k[0] = ntbl.bank_1k[1] = chr_pnt(s4.chr_nmt[0]);\
			ntbl.bank_1k[2] = ntbl.bank_1k[3] = chr_pnt(s4.chr_nmt[0]);\
			break;\
		case 3:\
			ntbl.bank_1k[0] = ntbl.bank_1k[1] = chr_pnt(s4.chr_nmt[1]);\
			ntbl.bank_1k[2] = ntbl.bank_1k[3] = chr_pnt(s4.chr_nmt[1]);\
			break;\
		}\
	}

struct _sunsoft3 {
	BYTE enable;
	BYTE toggle;
	WORD count;
	BYTE delay;
} s3;
struct _sunsoft4 {
	uint32_t chr_nmt[2];
	BYTE mode;
	BYTE mirroring;
} s4;
struct _sunsofttmp {
	BYTE type;
} sunsofttmp;

void map_init_Sunsoft(BYTE model) {
	switch (model) {
		default:
		case SUN2A:
		case SUN2B:
			EXTCL_CPU_WR_MEM(Sunsoft_S2);
			break;
		case SUN3:
			EXTCL_CPU_WR_MEM(Sunsoft_S3);
			EXTCL_SAVE_MAPPER(Sunsoft_S3);
			EXTCL_CPU_EVERY_CYCLE(Sunsoft_S3);
			mapper.internal_struct[0] = (BYTE *)&s3;
			mapper.internal_struct_size[0] = sizeof(s3);

			if (info.reset >= HARD) {
				memset(&s3, 0x00, sizeof(s3));
			}
			break;
		case SUN4:
			EXTCL_CPU_WR_MEM(Sunsoft_S4);
			EXTCL_SAVE_MAPPER(Sunsoft_S4);
			mapper.internal_struct[0] = (BYTE *)&s4;
			mapper.internal_struct_size[0] = sizeof(s4);

			if (info.reset >= HARD) {
				memset(&s4, 0x00, sizeof(s4));
				s4.chr_nmt[0] = 0x80 << 10;
				s4.chr_nmt[1] = 0x80 << 10;
			}

			if (info.id == MAHARAJA) {
				info.prg.ram.banks_8k_plus = 1;
				info.prg.ram.bat.banks = 1;
			}
			break;
	}

	sunsofttmp.type = model;
}

void extcl_cpu_wr_mem_Sunsoft_S2(UNUSED(WORD address), BYTE value) {
	const BYTE save = value;
	DBWORD bank = 0;

	if (sunsofttmp.type == SUN2B) {
		if (value & 0x08) {
			mirroring_SCR1();
		} else {
			mirroring_SCR0();
		}
	}

	value = (save >> 4) & 0x07;
	control_bank(info.prg.rom.max.banks_16k)
	map_prg_rom_8k(2, 0, value);
	map_prg_rom_8k_update();

	value = ((save & 0x80) >> 4) | (save & 0x07);
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
}

void extcl_cpu_wr_mem_Sunsoft_S3(WORD address, BYTE value) {
	switch (address & 0xF800) {
		case 0x8800:
			chr_rom_2k_swap(0)
			return;
		case 0x9800:
			chr_rom_2k_swap(2)
			return;
		case 0xA800:
			chr_rom_2k_swap(4)
			return;
		case 0xB800:
			chr_rom_2k_swap(6)
			return;
		case 0xC000:
		case 0xC800:
			s3.toggle ^= 1;
			if (s3.toggle) {
				s3.count = (s3.count & 0x00FF) | (value << 8);
			} else {
				s3.count = (s3.count & 0xFF00) | value;
			}
			return;
		case 0xD800:
			s3.toggle = 0;
			s3.enable = value & 0x10;
			irq.high &= ~EXT_IRQ;
			return;
		case 0xE800:
			mirroring(value)
			return;
		case 0xF800:
			control_bank(info.prg.rom.max.banks_16k)
			map_prg_rom_8k(2, 0, value);
			map_prg_rom_8k_update();
			return;
	}
}
BYTE extcl_save_mapper_Sunsoft_S3(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, s3.enable);
	save_slot_ele(mode, slot, s3.toggle);
	save_slot_ele(mode, slot, s3.count);
	save_slot_ele(mode, slot, s3.delay);

	return (EXIT_OK);
}
void extcl_cpu_every_cycle_Sunsoft_S3(void) {
	if (s3.delay && !(--s3.delay)) {
		irq.high |= EXT_IRQ;
	}

	if (s3.enable && s3.count && !(--s3.count)) {
		s3.enable = FALSE;
		s3.count = 0xFFFF;
		s3.delay = 1;
	}
}

void extcl_cpu_wr_mem_Sunsoft_S4(WORD address, BYTE value) {
	switch (address & 0xF000) {
		case 0x8000:
			chr_rom_2k_swap(0)
			return;
		case 0x9000:
			chr_rom_2k_swap(2)
			return;
		case 0xA000:
			chr_rom_2k_swap(4)
			return;
		case 0xB000:
			chr_rom_2k_swap(6)
			return;
		case 0xC000:
			s4.chr_nmt[0] = (value | 0x80) << 10;
			s4_mirroring()
			return;
		case 0xD000:
			s4.chr_nmt[1] = (value | 0x80) << 10;
			s4_mirroring()
			return;
		case 0xE000:
			s4.mode = value & 0x10;
			s4.mirroring = value & 0x03;
			s4_mirroring()
			return;
		case 0xF000:
			control_bank(info.prg.rom.max.banks_16k)
			map_prg_rom_8k(2, 0, value);
			map_prg_rom_8k_update();
			return;
	}
}
BYTE extcl_save_mapper_Sunsoft_S4(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, s4.chr_nmt);
	save_slot_ele(mode, slot, s4.mirroring);
	save_slot_ele(mode, slot, s4.mode);
	if ((mode == SAVE_SLOT_READ) && s4.mode) {
		s4_mirroring()
	}

	return (EXIT_OK);
}
