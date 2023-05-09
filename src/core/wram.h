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

#ifndef WRAM_H_
#define WRAM_H_

#include <stdio.h>
#include "common.h"

enum _wram_bank_types {
	WRAM_BANK_NONE,
	WRAM_BANK_RAM,
	WRAM_BANK_PRGROM,
	WRAM_BANK_OTHER,
	WRAM_CHUNK_SIZE = 0x100,
	WRAM_CHUNK_BANKS = 0x4000 / WRAM_CHUNK_SIZE
};

#define prg_wram_size() wram.size
#define prg_wram_ram_size() wram.ram.size
#define prg_wram_ram_pnt() wram.ram.pnt
#define prg_wram_nvram_size() wram.nvram.size
#define prg_wram_nvram_pnt() wram.nvram.pnt

typedef struct _wram {
	size_t size;
	BYTE *data;
	BYTE battery_present;
	WORD shift;
	struct _wram_ram {
		size_t size;
		BYTE *pnt;
	} ram;
	struct _wram_nvram {
		size_t size;
		BYTE *pnt;
	} nvram;
	struct _wram_chunk {
		enum _wram_bank_types type;
		BYTE writable;
		BYTE readable;
		BYTE *pnt;
		WORD mask;
	} chunk[WRAM_CHUNK_BANKS];
} _wram;
typedef struct _trainer {
	size_t size;
	BYTE *data;
	BYTE *where_to_copy ;
} _trainer;

extern _wram wram;
extern _trainer trainer;

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC BYTE wram_init(void);
EXTERNC void wram_quit(void);
EXTERNC void wram_set_ram_size(size_t size);
EXTERNC void wram_set_nvram_size(size_t size);
EXTERNC BYTE wram_is_writable(WORD address);
EXTERNC void wram_reset(void);

EXTERNC BYTE wram_rd(const WORD address, const BYTE before);
EXTERNC void wram_wr(const WORD address, const BYTE value);

EXTERNC BYTE wram_direct_rd(const WORD address, const BYTE openbus);
EXTERNC void wram_direct_wr(const WORD address, const BYTE value);

EXTERNC void wram_save_nvram_file(void);

EXTERNC void wram_map_auto_wp_256b(const WORD address, const DBWORD value, const BYTE rd, const BYTE wr);
EXTERNC void wram_map_auto_wp_512b(const WORD address, const DBWORD value, const BYTE rd, const BYTE wr);
EXTERNC void wram_map_auto_wp_1k(const WORD address, const DBWORD value, const BYTE rd, const BYTE wr);
EXTERNC void wram_map_auto_wp_2k(const WORD address, const DBWORD value, const BYTE rd, const BYTE wr);
EXTERNC void wram_map_auto_wp_4k(const WORD address, const DBWORD value, const BYTE rd, const BYTE wr);
EXTERNC void wram_map_auto_wp_8k(const WORD address, const DBWORD value, const BYTE rd, const BYTE wr);
EXTERNC void wram_map_auto_wp_16k(const WORD address, const DBWORD value, const BYTE rd, const BYTE wr);

EXTERNC void wram_map_auto_256b(const WORD address, const DBWORD value);
EXTERNC void wram_map_auto_512b(const WORD address, const DBWORD value);
EXTERNC void wram_map_auto_1k(const WORD address, const DBWORD value);
EXTERNC void wram_map_auto_2k(const WORD address, const DBWORD value);
EXTERNC void wram_map_auto_4k(const WORD address, const DBWORD value);
EXTERNC void wram_map_auto_8k(const WORD address, const DBWORD value);
EXTERNC void wram_map_auto_16k(const WORD address, const DBWORD value);

EXTERNC void wram_map_prg_rom_256b(const WORD address, DBWORD value);
EXTERNC void wram_map_prg_rom_512b(const WORD address, DBWORD value);
EXTERNC void wram_map_prg_rom_1k(const WORD address, DBWORD value);
EXTERNC void wram_map_prg_rom_2k(const WORD address, const DBWORD value);
EXTERNC void wram_map_prg_rom_4k(const WORD address, const DBWORD value);
EXTERNC void wram_map_prg_rom_8k(const WORD address, const DBWORD value);
EXTERNC void wram_map_prg_rom_16k(const WORD address, const DBWORD value);

EXTERNC void wram_map_disable_256b(const WORD address);
EXTERNC void wram_map_disable_512b(const WORD address);
EXTERNC void wram_map_disable_1k(const WORD address);
EXTERNC void wram_map_disable_2k(const WORD address);
EXTERNC void wram_map_disable_4k(const WORD address);
EXTERNC void wram_map_disable_8k(const WORD address);
EXTERNC void wram_map_disable_16k(const WORD address);

EXTERNC void wram_map_other_256b(const WORD address, DBWORD value, BYTE *dst, const size_t size, const BYTE rd, const BYTE wr);
EXTERNC void wram_map_other_512b(const WORD address, DBWORD value, BYTE *dst, const size_t size, const BYTE rd, const BYTE wr);
EXTERNC void wram_map_other_1k(const WORD address, DBWORD value, BYTE *dst, const size_t size, const BYTE rd, const BYTE wr);
EXTERNC void wram_map_other_2k(const WORD address, DBWORD value, BYTE *dst, const size_t size, const BYTE rd, const BYTE wr);
EXTERNC void wram_map_other_4k(const WORD address, DBWORD value, BYTE *dst, const size_t size, const BYTE rd, const BYTE wr);
EXTERNC void wram_map_other_8k(const WORD address, DBWORD value, BYTE *dst, const size_t size, const BYTE rd, const BYTE wr);
EXTERNC void wram_map_other_16k(const WORD address, DBWORD value, BYTE *dst, const size_t size, const BYTE rd, const BYTE wr);

EXTERNC void wram_map_ram_wp_256b(const WORD address, const DBWORD value, const BYTE rd, const BYTE wr);
EXTERNC void wram_map_ram_wp_512b(const WORD address, const DBWORD value, const BYTE rd, const BYTE wr);
EXTERNC void wram_map_ram_wp_1k(const WORD address, const DBWORD value, const BYTE rd, const BYTE wr);
EXTERNC void wram_map_ram_wp_2k(const WORD address, const DBWORD value, const BYTE rd, const BYTE wr);
EXTERNC void wram_map_ram_wp_4k(const WORD address, const DBWORD value, const BYTE rd, const BYTE wr);
EXTERNC void wram_map_ram_wp_8k(const WORD address, const DBWORD value, const BYTE rd, const BYTE wr);
EXTERNC void wram_map_ram_wp_16k(const WORD address, const DBWORD value, const BYTE rd, const BYTE wr);

EXTERNC void wram_map_nvram_wp_256b(const WORD address, const DBWORD value, const BYTE rd, const BYTE wr);
EXTERNC void wram_map_nvram_wp_512b(const WORD address, const DBWORD value, const BYTE rd, const BYTE wr);
EXTERNC void wram_map_nvram_wp_1k(const WORD address, const DBWORD value, const BYTE rd, const BYTE wr);
EXTERNC void wram_map_nvram_wp_2k(const WORD address, const DBWORD value, const BYTE rd, const BYTE wr);
EXTERNC void wram_map_nvram_wp_4k(const WORD address, const DBWORD value, const BYTE rd, const BYTE wr);
EXTERNC void wram_map_nvram_wp_8k(const WORD address, const DBWORD value, const BYTE rd, const BYTE wr);
EXTERNC void wram_map_nvram_wp_16k(const WORD address, const DBWORD value, const BYTE rd, const BYTE wr);

EXTERNC void wram_trainer_init(void);
EXTERNC void wram_trainer_quit(void);
EXTERNC BYTE wram_trainer_malloc(uint32_t size);

#undef EXTERNC

#endif /* WRAM_H_ */
