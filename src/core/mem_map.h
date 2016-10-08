/*
 *  Copyright (C) 2010-2016 Fabio Cavallo (aka FHorse)
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

#ifndef MEM_MAP_H_
#define MEM_MAP_H_

#include <stdio.h>
#include "common.h"

/* i vari mirroring */
enum mirroring_type {
	MIRRORING_HORIZONTAL,
	MIRRORING_VERTICAL,
	MIRRORING_SINGLE_SCR0,
	MIRRORING_SINGLE_SCR1,
	MIRRORING_FOURSCR,
	MIRRORING_SCR0x1_SCR1x3,
	MIRRORING_SCR0x3_SCR1x1
};

#define mirroring_H()\
	mapper.mirroring = MIRRORING_HORIZONTAL;\
	ntbl.bank_1k[0] = ntbl.bank_1k[1] = &ntbl.data[0];\
	ntbl.bank_1k[2] = ntbl.bank_1k[3] = &ntbl.data[0x0400]
#define mirroring_V()\
	mapper.mirroring = MIRRORING_VERTICAL;\
	ntbl.bank_1k[0] = ntbl.bank_1k[2] = &ntbl.data[0];\
	ntbl.bank_1k[1] = ntbl.bank_1k[3] = &ntbl.data[0x0400]
#define mirroring_SCR0()\
	mapper.mirroring = MIRRORING_SINGLE_SCR0;\
	ntbl.bank_1k[0] = ntbl.bank_1k[1] = &ntbl.data[0];\
	ntbl.bank_1k[2] = ntbl.bank_1k[3] = &ntbl.data[0]
#define mirroring_SCR1()\
	mapper.mirroring = MIRRORING_SINGLE_SCR1;\
	ntbl.bank_1k[0] = ntbl.bank_1k[1] = &ntbl.data[0x0400];\
	ntbl.bank_1k[2] = ntbl.bank_1k[3] = &ntbl.data[0x0400]
#define mirroring_FSCR()\
	mapper.mirroring = MIRRORING_FOURSCR;\
	ntbl.bank_1k[0] = &ntbl.data[0];\
	ntbl.bank_1k[1] = &ntbl.data[0x0400];\
	ntbl.bank_1k[2] = &ntbl.data[0x0800];\
	ntbl.bank_1k[3] = &ntbl.data[0x0C00]
#define mirroring_SCR0x1_SCR1x3()\
	mapper.mirroring = MIRRORING_SCR0x1_SCR1x3;\
	ntbl.bank_1k[0] = &ntbl.data[0];\
	ntbl.bank_1k[1] = \
	ntbl.bank_1k[2] = \
	ntbl.bank_1k[3] = &ntbl.data[0x0400]
#define mirroring_SCR0x3_SCR1x1()\
	mapper.mirroring = MIRRORING_SCR0x3_SCR1x1;\
	ntbl.bank_1k[0] = \
	ntbl.bank_1k[1] = \
	ntbl.bank_1k[2] = &ntbl.data[0];\
	ntbl.bank_1k[3] = &ntbl.data[0x0400]

#define prg_chip(chip_rom) prg.chip[chip_rom].rom
#define prg_chip_byte(chip_rom, index) prg_chip(chip_rom)[index]
#define prg_chip_byte_pnt(chip_rom, index) &prg_chip_byte(chip_rom, index)
#define prg_chip_size(chip_rom) prg.chip[chip_rom].size

#define prg_ram_plus_size() info.prg.ram.banks_8k_plus << 13

#define chr_chip(chip_rom) chr.chip[chip_rom].rom
#define chr_chip_byte(chip_rom, index) chr_chip(chip_rom)[index]
#define chr_chip_byte_pnt(chip_rom, index) &chr_chip_byte(chip_rom, index)
#define chr_chip_size(chip_rom) chr.chip[chip_rom].size

#define chr_ram_size() info.chr.rom[0].banks_8k << 13

struct _mmcpu {
	BYTE ram[0x800];   // Mirrored four times
} mmcpu;
struct _prg {
	struct _prg_chip {
		size_t size;
		BYTE *rom;
	} chip[MAX_CHIPS];

	WORD rom_chip[4];
	BYTE *rom_8k[4];   // 8k pages (0x2000)

	struct _prg_ram_chip { // Non Battery RAM
		size_t size;
		BYTE *data;
	} ram;

	BYTE *ram_plus;    // PRG Ram extra
	BYTE *ram_plus_8k;
	BYTE *ram_battery; // Battery RAM
} prg;
struct _chr {
	struct _chr_chip {
		size_t size;
		BYTE *rom;
	} chip[MAX_CHIPS];

	WORD rom_chip[8];
	BYTE *bank_1k[8];

	struct _extra {
		size_t size;
		BYTE *data;
	} extra;
} chr;
struct _nametables {
	BYTE data[0x1000];
	BYTE *bank_1k[4];
} ntbl;
struct _palette {
	BYTE color[0x20];
} palette;
struct _oam {
	BYTE data[256];
	BYTE *element[64];
	BYTE plus[32];
	BYTE *ele_plus[8];
	// unlimited sprites
	BYTE plus_unl[224];
	BYTE *ele_plus_unl[56];
} oam;

#endif /* MEM_MAP_H_ */
