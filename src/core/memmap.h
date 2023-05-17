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

#ifndef MEMMAP_H_
#define MEMMAP_H_

#include <stdio.h>
#include "common.h"

enum _sizes_types {
	S256B = 0x100,
	S512B = 0x200,
	S1K = 0x400,
	S2K = 0x800,
	S4K = 0x1000,
	S8K = 0x2000,
	S16K = 0x4000,
	S32K = 0x8000,
};
enum _memmap_bank_types {
	MEMMAP_BANK_NONE,
	MEMMAP_BANK_RAM,
	MEMMAP_BANK_PRGROM,
	MEMMAP_BANK_OTHER
};
enum _memmap_types {
//	MEMMAP_WRAM_CHUNK_SIZE = S4K,
	// imposto 0x100 per la mapper 347
	MEMMAP_WRAM_CHUNK_SIZE = S256B,
	MEMMAP_WRAM_CHUNK_BANKS = S16K / MEMMAP_WRAM_CHUNK_SIZE,

//	MEMMAP_PRGROM_CHUNK_SIZE = S8K,
//	MEMMAP_PRGROM_CHUNK_SIZE = S4K,
	MEMMAP_PRGROM_CHUNK_SIZE = MEMMAP_WRAM_CHUNK_SIZE,
	MEMMAP_PRGROM_CHUNK_BANKS = S32K / MEMMAP_PRGROM_CHUNK_SIZE
};
enum _memmap_misc { MAX_CHIPS = 8 };

#define prgrom_size() prgrom.size
#define prgrom_pnt() prgrom.data
#define prgrom_pnt_byte(byte) &prgrom.data[byte]
#define prgrom_byte(byte) prgrom.data[(byte)]
#define prgrom_calc_chunk(address) ((address) / MEMMAP_PRGROM_CHUNK_SIZE)

#define wram_size() wram.size
#define wram_pnt() wram.data
#define wram_pnt_byte(byte) &wram.data[byte]
#define wram_byte(byte) wram.data[(byte)]
#define wram_calc_chunk(address) ((address) / MEMMAP_WRAM_CHUNK_SIZE)

#define wram_ram_size() wram.ram.size
#define wram_ram_pnt() wram.ram.pnt
#define wram_ram_pnt_byte(byte) &wram.ram.pnt[byte]
#define wram_ram_byte(byte) wram.ram.pnt[(byte)]

#define wram_nvram_size() wram.nvram.size
#define wram_nvram_pnt() wram.nvram.pnt
#define wram_nvram_pnt_byte(byte) &wram.nvram.pnt[byte]
#define wram_nvram_byte(byte) wram.nvram.pnt[(byte)]

#define miscrom_size() miscrom.size
#define miscrom_pnt() miscrom.data
#define miscrom_pnt_byte(byte) &miscrom.data[byte]
#define miscrom_byte(byte) miscrom.data[(byte)]
#define miscrom_trainer_dst() miscrom.trainer.dst



// TUTTI da rinomiare e poi ELIMINARE
#if defined WRAM_OLD_HANDLER
#define prg_size() prg.rom.size
#define prg_byte(index) prg_rom()[index]
#define prg_pnt(index) &prg_byte(index)
#define prg_chip_rom(chip_rom) prg.chip[chip_rom].data
#define prg_chip_size(chip_rom) prg.chip[chip_rom].size
#define prg_rom_rd(address) prg.rom_8k[((address) >> 13) & 0x03][(address) & 0x1FFF]
#else
#define prg_rom() prgrom_pnt()
#define prg_size() prgrom_size()
#define prg_byte(index) prg_rom()[index]
#define prg_pnt(index) &prg_byte(index)
#define prg_chip_rom(chip_rom) prgrom.chips.chunk[chip_rom].pnt
#define prg_chip_size(chip_rom) prgrom.chips.chunk[chip_rom].size
#define prg_rom_rd(address) prgrom_rd(address)
#endif





typedef struct _memmap_info {
	WORD shift;
	struct chunk {
		size_t size;
		size_t items;
	} chunk;
} _memmap_info;
typedef struct _memmap_chunk {
	enum _memmap_bank_types type;
	BYTE writable;
	BYTE readable;
	BYTE *pnt;
	WORD mask;
	struct _memmap_chunk_mem_region {
		BYTE *start;
		BYTE *end;
	} mem_region;
} _memmap_chunk;
typedef struct _memmap_region {
	_memmap_info info;
	_memmap_chunk *chunks;
} _memmap_region;
typedef struct _memmap {
	_memmap_region wram;
	_memmap_region prgrom;
} _memmap;
typedef struct _prgrom {
	size_t size;
	size_t real_size;
	BYTE *data;
	struct _prgrom_chips {
		WORD amount;
		struct _prgrom_chip {
			size_t size;
			BYTE *pnt;
		} chunk[MAX_CHIPS];
	} chips;
} _prgrom;
typedef struct _wram {
	size_t size;
	size_t real_size;
	BYTE *data;
	struct _wram_battery {
		BYTE in_use;
	} battery;
	struct _wram_ram {
		size_t size;
		BYTE *pnt;
	} ram;
	struct _wram_nvram {
		size_t size;
		BYTE *pnt;
	} nvram;
} _wram;
//typedef struct _trainer {
//	size_t size;
//	BYTE *data;
//} _trainer;
typedef struct _miscrom {
	size_t size;
	size_t real_size;
	BYTE *data;
	BYTE chips;
	struct _miscrom_trainer {
		BYTE in_use;
		BYTE *dst;
	} trainer;
} _miscrom;

extern _memmap memmap;
extern _prgrom prgrom;
extern _wram wram;
//extern _trainer trainer;
extern _miscrom miscrom;

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

// memmap ----------------------------------------------------------------------------

EXTERNC BYTE memmap_init(void);
EXTERNC void memmap_quit(void);
EXTERNC BYTE memmap_adr_is_readable(WORD address);
EXTERNC BYTE memmap_adr_is_writable(WORD address);
EXTERNC BYTE *memmap_chunk_pnt(WORD address);

EXTERNC void memmap_auto_wp_256b(WORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_auto_wp_512b(WORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_auto_wp_1k(WORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_auto_wp_2k(WORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_auto_wp_4k(WORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_auto_wp_8k(WORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_auto_wp_16k(WORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_auto_wp_32k(WORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_auto_wp_custom_size(WORD address, DBWORD chunk, BYTE rd, BYTE wr, size_t size);

EXTERNC void memmap_auto_256b(WORD address, DBWORD value);
EXTERNC void memmap_auto_512b(WORD address, DBWORD value);
EXTERNC void memmap_auto_1k(WORD address, DBWORD value);
EXTERNC void memmap_auto_2k(WORD address, DBWORD value);
EXTERNC void memmap_auto_4k(WORD address, DBWORD value);
EXTERNC void memmap_auto_8k(WORD address, DBWORD value);
EXTERNC void memmap_auto_16k(WORD address, DBWORD value);
EXTERNC void memmap_auto_32k(WORD address, DBWORD value);
EXTERNC void memmap_auto_custom_size(WORD address, DBWORD chunk, size_t size);

EXTERNC void memmap_disable_256b(WORD address);
EXTERNC void memmap_disable_512b(WORD address);
EXTERNC void memmap_disable_1k(WORD address);
EXTERNC void memmap_disable_2k(WORD address);
EXTERNC void memmap_disable_4k(WORD address);
EXTERNC void memmap_disable_8k(WORD address);
EXTERNC void memmap_disable_16k(WORD address);
EXTERNC void memmap_disable_32k(WORD address);
EXTERNC void memmap_disable_custom_size(WORD address, size_t size);

EXTERNC void memmap_other_256b(WORD address, DBWORD value, BYTE *dst, size_t dst_size, BYTE rd, BYTE wr);
EXTERNC void memmap_other_512b(WORD address, DBWORD value, BYTE *dst, size_t dst_size, BYTE rd, BYTE wr);
EXTERNC void memmap_other_1k(WORD address, DBWORD value, BYTE *dst, size_t dst_size, BYTE rd, BYTE wr);
EXTERNC void memmap_other_2k(WORD address, DBWORD value, BYTE *dst, size_t dst_size, BYTE rd, BYTE wr);
EXTERNC void memmap_other_4k(WORD address, DBWORD value, BYTE *dst, size_t dst_size, BYTE rd, BYTE wr);
EXTERNC void memmap_other_8k(WORD address, DBWORD value, BYTE *dst, size_t dst_size, BYTE rd, BYTE wr);
EXTERNC void memmap_other_16k(WORD address, DBWORD value, BYTE *dst, size_t dst_size, BYTE rd, BYTE wr);
EXTERNC void memmap_other_32k(WORD address, DBWORD value, BYTE *dst, size_t dst_size, BYTE rd, BYTE wr);
EXTERNC void memmap_other_custom_size(WORD address, DBWORD initial_chunk, BYTE *dst, size_t dst_size, BYTE rd, BYTE wr, size_t custom_size);

// wram ------------------------------------------------------------------------------

EXTERNC BYTE wram_init(void);
EXTERNC void wram_quit(void);
EXTERNC void wram_set_ram_size(size_t size);
EXTERNC void wram_set_nvram_size(size_t size);
EXTERNC void wram_reset(void);
EXTERNC void wram_memset(void);

EXTERNC BYTE wram_rd(WORD address);
EXTERNC void wram_wr(WORD address, BYTE value);

EXTERNC BYTE wram_direct_rd(WORD address, BYTE openbus);
EXTERNC void wram_direct_wr(WORD address, BYTE value);

EXTERNC void memmap_wram_256b(WORD address, DBWORD value);
EXTERNC void memmap_wram_512b(WORD address, DBWORD value);
EXTERNC void memmap_wram_1k(WORD address, DBWORD value);
EXTERNC void memmap_wram_2k(WORD address, DBWORD value);
EXTERNC void memmap_wram_4k(WORD address, DBWORD value);
EXTERNC void memmap_wram_8k(WORD address, DBWORD value);
EXTERNC void memmap_wram_16k(WORD address, DBWORD value);
EXTERNC void memmap_wram_32k(WORD address, DBWORD value);
EXTERNC void memmap_wram_custom_size(WORD address, DBWORD chunk, size_t size);

EXTERNC void memmap_wram_ram_wp_256b(WORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_wram_ram_wp_512b(WORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_wram_ram_wp_1k(WORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_wram_ram_wp_2k(WORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_wram_ram_wp_4k(WORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_wram_ram_wp_8k(WORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_wram_ram_wp_16k(WORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_wram_ram_wp_32k(WORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_wram_ram_wp_custom_size(WORD address, DBWORD chunk, BYTE rd, BYTE wr, size_t size);

EXTERNC void memmap_wram_nvram_wp_256b(WORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_wram_nvram_wp_512b(WORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_wram_nvram_wp_1k(WORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_wram_nvram_wp_2k(WORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_wram_nvram_wp_4k(WORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_wram_nvram_wp_8k(WORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_wram_nvram_wp_16k(WORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_wram_nvram_wp_32k(WORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_wram_nvram_custom_size(WORD address, DBWORD chunk, BYTE rd, BYTE wr, size_t size);

EXTERNC void wram_save_nvram_file(void);

// prgrom ----------------------------------------------------------------------------

EXTERNC BYTE prgrom_init(BYTE set_value);
EXTERNC void prgrom_quit(void);
EXTERNC void prgrom_set_size(size_t size);
EXTERNC void prgrom_reset(void);
EXTERNC WORD prgrom_banks(enum _sizes_types size);
EXTERNC WORD prgrom_control_bank(enum _sizes_types size, WORD bank);

EXTERNC BYTE prgrom_rd(WORD address);
EXTERNC void prgrom_wr(WORD address, BYTE value);

EXTERNC void memmap_prgrom_256b(WORD address, DBWORD value);
EXTERNC void memmap_prgrom_512b(WORD address, DBWORD value);
EXTERNC void memmap_prgrom_1k(WORD address, DBWORD value);
EXTERNC void memmap_prgrom_2k(WORD address, DBWORD value);
EXTERNC void memmap_prgrom_4k(WORD address, DBWORD value);
EXTERNC void memmap_prgrom_8k(WORD address, DBWORD value);
EXTERNC void memmap_prgrom_16k(WORD address, DBWORD value);
EXTERNC void memmap_prgrom_32k(WORD address, DBWORD value);
EXTERNC void memmap_prgrom_custom_size(WORD address, DBWORD chunk, size_t size);

// miscrom ---------------------------------------------------------------------------

EXTERNC BYTE miscrom_init(void);
EXTERNC void miscrom_quit(void);
EXTERNC void miscrom_set_size(size_t size);
EXTERNC void miscrom_trainer_init(void);

#undef EXTERNC

#endif /* MEMMAP_H_ */
