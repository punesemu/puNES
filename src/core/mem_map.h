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

#ifndef MEM_MAP_H_
#define MEM_MAP_H_

#include <stdio.h>
#include "common.h"

enum mirroring_types {
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
	map_nmt_1k(0, 0);\
	map_nmt_1k(1, 0);\
	map_nmt_1k(2, 1);\
	map_nmt_1k(3, 1)
#define mirroring_V()\
	mapper.mirroring = MIRRORING_VERTICAL;\
	map_nmt_1k(0, 0);\
	map_nmt_1k(1, 1);\
	map_nmt_1k(2, 0);\
	map_nmt_1k(3, 1)
#define mirroring_SCR0()\
	mapper.mirroring = MIRRORING_SINGLE_SCR0;\
	map_nmt_1k(0, 0);\
	map_nmt_1k(1, 0);\
	map_nmt_1k(2, 0);\
	map_nmt_1k(3, 0)
#define mirroring_SCR1()\
	mapper.mirroring = MIRRORING_SINGLE_SCR1;\
	map_nmt_1k(0, 1);\
	map_nmt_1k(1, 1);\
	map_nmt_1k(2, 1);\
	map_nmt_1k(3, 1)
#define mirroring_FSCR()\
	mapper.mirroring = MIRRORING_FOURSCR;\
	map_nmt_1k(0, 0);\
	map_nmt_1k(1, 1);\
	map_nmt_1k(2, 2);\
	map_nmt_1k(3, 3)
#define mirroring_SCR0x1_SCR1x3()\
	mapper.mirroring = MIRRORING_SCR0x1_SCR1x3;\
	map_nmt_1k(0, 0);\
	map_nmt_1k(1, 1);\
	map_nmt_1k(2, 1);\
	map_nmt_1k(3, 1)
#define mirroring_SCR0x3_SCR1x1()\
	mapper.mirroring = MIRRORING_SCR0x3_SCR1x1;\
	map_nmt_1k(0, 0);\
	map_nmt_1k(1, 0);\
	map_nmt_1k(2, 0);\
	map_nmt_1k(3, 1)

#define chr_rom() chr.rom.data
#define chr_size() chr.rom.size
#define chr_byte(index) chr_rom()[index]
#define chr_pnt(index) &chr_byte(index)
#define chr_chip_rom(chip_rom) chr.chip[chip_rom].data
#define chr_chip_size(chip_rom) chr.chip[chip_rom].size
#define chr_ram_size() info.chr.rom.banks_8k << 13

typedef struct _mmcpu {
	BYTE ram[0x800];   // Mirrored four times
} _mmcpu;
typedef struct _prg {
	struct _prg_rom {
		size_t size;
		BYTE *data;
	} rom;

	BYTE *rom_8k[4];   // 8k pages (0x2000)
} _prg;
typedef struct _chr {
	struct _chr_rom {
		size_t size;
		BYTE *data;
	} rom;
	struct _chr_chip {
		size_t size;
		BYTE *data;
	} chip[8 /*MAX_CHIPS*/];

	BYTE *bank_1k[8];

	struct _extra {
		size_t size;
		BYTE *data;
	} extra;
} _chr;
typedef struct _nametables {
	BYTE data[0x1000];
	BYTE *bank_1k[4];
	BYTE writable[4];
} _nametables;
typedef struct _mmap_palette {
	BYTE color[0x20];
} _mmap_palette;
typedef struct _oam {
	BYTE data[256];
	BYTE *element[64];
	BYTE plus[32];
	BYTE *ele_plus[8];
	// unlimited sprites
	BYTE plus_unl[224];
	BYTE *ele_plus_unl[56];
} _oam;

extern _mmcpu mmcpu;
extern _prg prg;
extern _chr chr;
extern _nametables ntbl;
extern _mmap_palette mmap_palette;
extern _oam oam;

#endif /* MEM_MAP_H_ */
