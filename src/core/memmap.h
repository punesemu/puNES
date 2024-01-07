/*
 *  Copyright (C) 2010-2024 Fabio Cavallo (aka FHorse)
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

#include "common.h"

enum _sizes_types {
	S128B = 0x80,
	S256B = 0x100,
	S512B = 0x200,
	S1K = 0x400,
	S2K = 0x800,
	S4K = 0x1000,
	S8K = 0x2000,
	S16K = 0x4000,
	S24K = 0x6000,
	S48K = 0xC000,
	S32K = 0x8000,
	S64K = 0x10000,
	S128K = 0x20000,
	S256K = 0x40000,
	S512K = 0x80000,
	S1M = 0x100000,
	S2M = 0x200000,
	S4M = 0x400000,
	S8M = 0x800000,
	S16M = 0x1000000
};
enum mirroring_types {
	MIRRORING_HORIZONTAL,
	MIRRORING_VERTICAL,
	MIRRORING_SINGLE_SCR0,
	MIRRORING_SINGLE_SCR1,
	MIRRORING_FOURSCR,
	MIRRORING_SCR0x1_SCR1x3,
	MIRRORING_SCR0x3_SCR1x1
};
enum _memmap_bank_types {
	MEMMAP_BANK_NONE,
	MEMMAP_BANK_PRGROM,
	MEMMAP_BANK_CHRROM,
	MEMMAP_BANK_WRAM,
	MEMMAP_BANK_VRAM,
	MEMMAP_BANK_RAM,
	MEMMAP_BANK_NMT,
	MEMMAP_BANK_OTHER
};
enum _memmap_types {
	CPUMM = 0x10000,
	PPUMM = 0x20000,

	// imposto S256B per la mapper 539
	// per le mapper 347, 186 uso S1K
	//MEMMAP_CHUNK_SIZE_DEFAULT = S256B,

	MEMMAP_PRG_CHUNK_SIZE_DEFAULT = S8K,
	MEMMAP_PRG_SIZE = S32K,

	MEMMAP_CHR_CHUNK_SIZE_DEFAULT = S1K,
	MEMMAP_CHR_SIZE = S8K,

	MEMMAP_WRAM_CHUNK_SIZE_DEFAULT = S4K,
	MEMMAP_WRAM_SIZE = S16K,

	MEMMAP_RAM_CHUNK_SIZE_DEFAULT = S2K,
	MEMMAP_RAM_SIZE = S2K,

	MEMMAP_NMT_CHUNK_SIZE_DEFAULT = S1K,
	MEMMAP_NMT_SIZE = S8K,
};
enum _memmap_misc { MAX_CHIPS = 8 };

#define MMCPU(address) (CPUMM | (address))
#define MMPPU(address) (PPUMM | (address))

#define prgrom_size() prgrom.data.size
#define prgrom_pnt() prgrom.data.pnt
#define prgrom_pnt_byte(byte) &prgrom.data.pnt[byte]
#define prgrom_byte(byte) prgrom.data.pnt[(byte)]
#define prgrom_mask() prgrom.data.mask
#define prgrom_calc_chunk(idx, address) ((address) / nes[idx].m.memmap.prg.info.chunk.size)
#define prgrom_chip(chip_rom) prgrom.chips.chunk[chip_rom].pnt
#define prgrom_chip_size(chip_rom) prgrom.chips.chunk[chip_rom].size

#define chrrom_size() chrrom.data.size
#define chrrom_pnt() chrrom.data.pnt
#define chrrom_pnt_byte(byte) &chrrom.data.pnt[byte]
#define chrrom_byte(byte) chrrom.data.pnt[(byte)]
#define chrrom_mask() chrrom.data.mask
#define chrrom_chip(chip_rom) chrrom.chips.chunk[chip_rom].pnt
#define chrrom_chip_size(chip_rom) chrrom.chips.chunk[chip_rom].size

#define ram_size(idx) nes[idx].m.ram.data.size
#define ram_pnt(idx) nes[idx].m.ram.data.pnt
#define ram_pnt_byte(idx, byte) &nes[idx].m.ram.data.pnt[byte]
#define ram_byte(idx, byte) nes[idx].m.ram.data.pnt[(byte)]
#define ram_mask(idx) nes[idx].m.ram.data.mask

#define wram_size() wram.data.size
#define wram_pnt() wram.data.pnt
#define wram_pnt_byte(byte) &wram.data.pnt[byte]
#define wram_byte(byte) wram.data.pnt[(byte)]
#define wram_mask() wram.data.mask
#define wram_calc_chunk(idx, address) ((address) / nes[idx].m.memmap.wram.info.chunk.size)

#define wram_ram_size() wram.ram.size
#define wram_ram_pnt() wram.ram.pnt
#define wram_ram_pnt_byte(byte) &wram.ram.pnt[byte]
#define wram_ram_byte(byte) wram.ram.pnt[(byte)]
#define wram_ram_mask() wram.ram.mask

#define wram_nvram_size() wram.nvram.size
#define wram_nvram_pnt() wram.nvram.pnt
#define wram_nvram_pnt_byte(byte) &wram.nvram.pnt[byte]
#define wram_nvram_byte(byte) wram.nvram.pnt[(byte)]
#define wram_nvram_mask() wram.nvram.mask

#define vram_size(idx) nes[idx].m.vram.data.size
#define vram_pnt(idx) nes[idx].m.vram.data.pnt
#define vram_pnt_byte(idx, byte) &nes[idx].m.vram.data.pnt[byte]
#define vram_byte(idx, byte) nes[idx].m.vram.data.pnt[(byte)]
#define vram_mask(idx) nes[idx].m.vram.data.mask
#define vram_calc_chunk(idx, address) ((address) / nes[idx].m.memmap.vram.info.chunk.size)

#define vram_ram_size(idx) nes[idx].m.vram.ram.size
#define vram_ram_pnt(idx) nes[idx].m.vram.ram.pnt
#define vram_ram_pnt_byte(idx, byte) &nes[idx].m.vram.ram.pnt[byte]
#define vram_ram_byte(idx, byte) nes[idx].m.vram.ram.pnt[(byte)]
#define vram_ram_mask(idx) nes[idx].m.vram.ram.mask

#define vram_nvram_size(idx) nes[idx].m.vram.nvram.size
#define vram_nvram_pnt(idx) nes[idx].m.vram.nvram.pnt
#define vram_nvram_pnt_byte(idx, byte) &nes[idx].m.vram.nvram.pnt[byte]
#define vram_nvram_byte(idx, byte) nes[idx].m.vram.nvram.pnt[(byte)]
#define vram_nvram_mask(idx) nes[idx].m.vram.nvram.mask

#define nmt_size(idx) nes[idx].m.nmt.data.size
#define nmt_pnt(idx) nes[idx].m.nmt.data.pnt
#define nmt_pnt_byte(idx, byte) &nes[idx].m.nmt.data.pnt[byte]
#define nmt_byte(idx, byte) nes[idx].m.nmt.data.pnt[(byte)]
#define nmt_mask(idx) nes[idx].m.nmt.data.mask

#define miscrom_size() miscrom.data.size
#define miscrom_pnt() miscrom.data.pnt
#define miscrom_pnt_byte(byte) &miscrom.data.pnt[byte]
#define miscrom_byte(byte) miscrom.data.pnt[(byte)]
#define miscrom_ram_mask() miscrom.data.mask
#define miscrom_trainer_dst() miscrom.trainer.dst

typedef struct _memmap_info {
	size_t size;
	WORD shift;
	struct chunk {
		size_t size;
		size_t items;
	} chunk;
} _memmap_info;
typedef struct _memmap_chunk {
	enum _memmap_bank_types type;
	BYTE *pnt;
	BYTE writable;
	BYTE readable;
	WORD mask;
	WORD actual_bank;
	struct _memmap_chunck_permit {
		BYTE wr;
		BYTE rd;
	} permit;
	struct _memmap_chunk_mem_region {
		BYTE *start;
		BYTE *end;
	} mem_region;
} _memmap_chunk;
typedef struct _memmap_region {
	_memmap_info info;
	_memmap_chunk *chunks;
} _memmap_region;
typedef struct _memmap_chunk_dst {
	BYTE *pnt;
	size_t size;
	size_t mask;
} _memmap_chunk_dst;
typedef struct _memmap {
	_memmap_region ram;
	_memmap_region wram;
	_memmap_region prg;
	_memmap_region chr;
	_memmap_region nmt;
} _memmap;
typedef struct _prgrom {
	size_t real_size;
	_memmap_chunk_dst data;
	struct _prgrom_chips {
		WORD amount;
		struct _prgrom_chip {
			BYTE *pnt;
			size_t size;
		} chunk[MAX_CHIPS];
	} chips;
} _prgrom;
typedef struct _chrrom {
	size_t real_size;
	_memmap_chunk_dst data;
	struct _chrrom_chips {
		WORD amount;
		struct _chrrom_chip {
			BYTE *pnt;
			size_t size;
		} chunk[MAX_CHIPS];
	} chips;
} _chrrom;
typedef struct _wram {
	size_t real_size;
	_memmap_chunk_dst data;
	_memmap_chunk_dst ram;
	_memmap_chunk_dst nvram;
} _wram;
typedef struct _vram {
	size_t real_size;
	_memmap_chunk_dst data;
	_memmap_chunk_dst ram;
	_memmap_chunk_dst nvram;
} _vram;
typedef struct _ram {
	size_t real_size;
	_memmap_chunk_dst data;
} _ram;
typedef struct _nmt {
	size_t real_size;
	_memmap_chunk_dst data;
} _nmt;
typedef struct _miscrom {
	size_t real_size;
	BYTE chips;
	_memmap_chunk_dst data;
	struct _miscrom_trainer {
		BYTE in_use;
		BYTE *dst;
	} trainer;
} _miscrom;
typedef struct _memmap_palette {
	BYTE color[0x20];
} _memmap_palette;
typedef struct _memmap_data {
	_memmap memmap;
	_vram vram;
	_ram ram;
	_nmt nmt;
	_memmap_palette memmap_palette;
} _memmap_data;

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

// memmap ----------------------------------------------------------------------------

EXTERNC BYTE memmap_init(void);
EXTERNC void memmap_quit(void);
EXTERNC BYTE memmap_adr_is_readable(BYTE nidx, DBWORD address);
EXTERNC BYTE memmap_adr_is_writable(BYTE nidx, DBWORD address);
EXTERNC WORD memmap_chunk_actual_bank(BYTE nidx, DBWORD address);
EXTERNC BYTE *memmap_chunk_pnt(BYTE nidx, DBWORD address);

EXTERNC BYTE memmap_prg_region_init(BYTE nidx, size_t chunk_size);
EXTERNC BYTE memmap_chr_region_init(BYTE nidx, size_t chunk_size);
EXTERNC BYTE memmap_wram_region_init(BYTE nidx, size_t chunk_size);
EXTERNC BYTE memmap_ram_region_init(BYTE nidx, size_t chunk_size);
EXTERNC BYTE memmap_nmt_region_init(BYTE nidx, size_t chunk_size);

EXTERNC void memmap_auto_wp_128b(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_auto_wp_256b(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_auto_wp_512b(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_auto_wp_1k(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_auto_wp_2k(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_auto_wp_4k(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_auto_wp_8k(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_auto_wp_16k(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_auto_wp_32k(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_auto_wp_custom_size(BYTE nidx, DBWORD address, DBWORD chunk, size_t size, BYTE rd, BYTE wr);

EXTERNC void memmap_auto_128b(BYTE nidx, DBWORD address, DBWORD value);
EXTERNC void memmap_auto_256b(BYTE nidx, DBWORD address, DBWORD value);
EXTERNC void memmap_auto_512b(BYTE nidx, DBWORD address, DBWORD value);
EXTERNC void memmap_auto_1k(BYTE nidx, DBWORD address, DBWORD value);
EXTERNC void memmap_auto_2k(BYTE nidx, DBWORD address, DBWORD value);
EXTERNC void memmap_auto_4k(BYTE nidx, DBWORD address, DBWORD value);
EXTERNC void memmap_auto_8k(BYTE nidx, DBWORD address, DBWORD value);
EXTERNC void memmap_auto_16k(BYTE nidx, DBWORD address, DBWORD value);
EXTERNC void memmap_auto_32k(BYTE nidx, DBWORD address, DBWORD value);
EXTERNC void memmap_auto_custom_size(BYTE nidx, DBWORD address, DBWORD chunk, size_t size);

EXTERNC void memmap_disable_128b(BYTE nidx, DBWORD address);
EXTERNC void memmap_disable_256b(BYTE nidx, DBWORD address);
EXTERNC void memmap_disable_512b(BYTE nidx, DBWORD address);
EXTERNC void memmap_disable_1k(BYTE nidx, DBWORD address);
EXTERNC void memmap_disable_2k(BYTE nidx, DBWORD address);
EXTERNC void memmap_disable_4k(BYTE nidx, DBWORD address);
EXTERNC void memmap_disable_8k(BYTE nidx, DBWORD address);
EXTERNC void memmap_disable_16k(BYTE nidx, DBWORD address);
EXTERNC void memmap_disable_32k(BYTE nidx, DBWORD address);
EXTERNC void memmap_disable_custom_size(BYTE nidx, DBWORD address, size_t size);

EXTERNC void memmap_other_128b(BYTE nidx, DBWORD address, DBWORD value, BYTE *dst, size_t dst_size, BYTE rd, BYTE wr);
EXTERNC void memmap_other_256b(BYTE nidx, DBWORD address, DBWORD value, BYTE *dst, size_t dst_size, BYTE rd, BYTE wr);
EXTERNC void memmap_other_512b(BYTE nidx, DBWORD address, DBWORD value, BYTE *dst, size_t dst_size, BYTE rd, BYTE wr);
EXTERNC void memmap_other_1k(BYTE nidx, DBWORD address, DBWORD value, BYTE *dst, size_t dst_size, BYTE rd, BYTE wr);
EXTERNC void memmap_other_2k(BYTE nidx, DBWORD address, DBWORD value, BYTE *dst, size_t dst_size, BYTE rd, BYTE wr);
EXTERNC void memmap_other_4k(BYTE nidx, DBWORD address, DBWORD value, BYTE *dst, size_t dst_size, BYTE rd, BYTE wr);
EXTERNC void memmap_other_8k(BYTE nidx, DBWORD address, DBWORD value, BYTE *dst, size_t dst_size, BYTE rd, BYTE wr);
EXTERNC void memmap_other_16k(BYTE nidx, DBWORD address, DBWORD value, BYTE *dst, size_t dst_size, BYTE rd, BYTE wr);
EXTERNC void memmap_other_32k(BYTE nidx, DBWORD address, DBWORD value, BYTE *dst, size_t dst_size, BYTE rd, BYTE wr);
EXTERNC void memmap_other_custom_size(BYTE nidx, DBWORD address, DBWORD initial_chunk, size_t custom_size, BYTE *dst, size_t dst_size, BYTE rd, BYTE wr);

EXTERNC void memmap_prgrom_vs_8k(BYTE nidx, DBWORD address, DBWORD value);
EXTERNC void memmap_prgrom_vs_32k(BYTE nidx, DBWORD address, DBWORD value);

// prgrom ----------------------------------------------------------------------------

EXTERNC BYTE prgrom_init(BYTE set_value);
EXTERNC void prgrom_quit(void);
EXTERNC void prgrom_set_size(size_t size);
EXTERNC void prgrom_reset_chunks(void);
EXTERNC WORD prgrom_banks(enum _sizes_types size);
EXTERNC WORD prgrom_control_bank(enum _sizes_types size, WORD bank);
EXTERNC size_t prgrom_region_address(BYTE nidx, WORD address);

EXTERNC BYTE prgrom_rd(BYTE nidx, WORD address);
EXTERNC void prgrom_wr(BYTE nidx, WORD address, BYTE value);

EXTERNC void memmap_prgrom_128b(BYTE nidx, DBWORD address, DBWORD value);
EXTERNC void memmap_prgrom_256b(BYTE nidx, DBWORD address, DBWORD value);
EXTERNC void memmap_prgrom_512b(BYTE nidx, DBWORD address, DBWORD value);
EXTERNC void memmap_prgrom_1k(BYTE nidx, DBWORD address, DBWORD value);
EXTERNC void memmap_prgrom_2k(BYTE nidx, DBWORD address, DBWORD value);
EXTERNC void memmap_prgrom_4k(BYTE nidx, DBWORD address, DBWORD value);
EXTERNC void memmap_prgrom_8k(BYTE nidx, DBWORD address, DBWORD value);
EXTERNC void memmap_prgrom_16k(BYTE nidx, DBWORD address, DBWORD value);
EXTERNC void memmap_prgrom_32k(BYTE nidx, DBWORD address, DBWORD value);
EXTERNC void memmap_prgrom_custom_size(BYTE nidx, DBWORD address, DBWORD chunk, size_t size);

// chrom -----------------------------------------------------------------------------

EXTERNC BYTE chrrom_init(void);
EXTERNC void chrrom_quit(void);
EXTERNC void chrrom_set_size(size_t size);
EXTERNC void chrrom_reset_chunks(void);
EXTERNC WORD chrrom_banks(enum _sizes_types size);
EXTERNC WORD chrrom_control_bank(enum _sizes_types size, WORD bank);

EXTERNC BYTE chr_rd(BYTE nidx, WORD address);
EXTERNC void chr_wr(BYTE nidx, WORD address, BYTE value);
EXTERNC void chr_disable_write(BYTE nidx);

EXTERNC void memmap_chrrom_128b(BYTE nidx, DBWORD address, DBWORD value);
EXTERNC void memmap_chrrom_256b(BYTE nidx, DBWORD address, DBWORD value);
EXTERNC void memmap_chrrom_512b(BYTE nidx, DBWORD address, DBWORD value);
EXTERNC void memmap_chrrom_1k(BYTE nidx, DBWORD address, DBWORD value);
EXTERNC void memmap_chrrom_2k(BYTE nidx, DBWORD address, DBWORD value);
EXTERNC void memmap_chrrom_4k(BYTE nidx, DBWORD address, DBWORD value);
EXTERNC void memmap_chrrom_8k(BYTE nidx, DBWORD address, DBWORD value);
EXTERNC void memmap_chrrom_16k(BYTE nidx, DBWORD address, DBWORD value);
EXTERNC void memmap_chrrom_32k(BYTE nidx, DBWORD address, DBWORD value);
EXTERNC void memmap_chrrom_custom_size(BYTE nidx, DBWORD address, DBWORD chunk, size_t size);

EXTERNC void memmap_chrrom_nmt_128b(BYTE nidx, DBWORD address, DBWORD value);
EXTERNC void memmap_chrrom_nmt_256b(BYTE nidx, DBWORD address, DBWORD value);
EXTERNC void memmap_chrrom_nmt_512b(BYTE nidx, DBWORD address, DBWORD value);
EXTERNC void memmap_chrrom_nmt_1k(BYTE nidx, DBWORD address, DBWORD value);
EXTERNC void memmap_chrrom_nmt_2k(BYTE nidx, DBWORD address, DBWORD value);
EXTERNC void memmap_chrrom_nmt_4k(BYTE nidx, DBWORD address, DBWORD value);
EXTERNC void memmap_chrrom_nmt_8k(BYTE nidx, DBWORD address, DBWORD value);

EXTERNC void memmap_chrrom_auto_vs_8k(BYTE nidx, DBWORD address, DBWORD value);

// wram ------------------------------------------------------------------------------

EXTERNC BYTE wram_init(void);
EXTERNC void wram_quit(void);
EXTERNC void wram_set_ram_size(size_t size);
EXTERNC void wram_set_nvram_size(size_t size);
EXTERNC void wram_reset_chunks(void);
EXTERNC void wram_memset(void);

EXTERNC BYTE wram_rd(BYTE nidx, WORD address);
EXTERNC void wram_wr(BYTE nidx, WORD address, BYTE value);

EXTERNC BYTE wram_direct_rd(WORD address, BYTE openbus);
EXTERNC void wram_direct_wr(WORD address, BYTE value);

EXTERNC void memmap_wram_128b(BYTE nidx, DBWORD address, DBWORD value);
EXTERNC void memmap_wram_256b(BYTE nidx, DBWORD address, DBWORD value);
EXTERNC void memmap_wram_512b(BYTE nidx, DBWORD address, DBWORD value);
EXTERNC void memmap_wram_1k(BYTE nidx, DBWORD address, DBWORD value);
EXTERNC void memmap_wram_2k(BYTE nidx, DBWORD address, DBWORD value);
EXTERNC void memmap_wram_4k(BYTE nidx, DBWORD address, DBWORD value);
EXTERNC void memmap_wram_8k(BYTE nidx, DBWORD address, DBWORD value);
EXTERNC void memmap_wram_16k(BYTE nidx, DBWORD address, DBWORD value);
EXTERNC void memmap_wram_32k(BYTE nidx, DBWORD address, DBWORD value);
EXTERNC void memmap_wram_custom_size(BYTE nidx, DBWORD address, DBWORD chunk, size_t size);

EXTERNC void memmap_wram_wp_128b(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_wram_wp_256b(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_wram_wp_512b(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_wram_wp_1k(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_wram_wp_2k(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_wram_wp_4k(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_wram_wp_8k(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_wram_wp_16k(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_wram_wp_32k(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_wram_wp_custom_size(BYTE nidx, DBWORD address, DBWORD chunk, size_t size, BYTE rd, BYTE wr);

EXTERNC void memmap_wram_ram_wp_128b(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_wram_ram_wp_256b(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_wram_ram_wp_512b(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_wram_ram_wp_1k(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_wram_ram_wp_2k(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_wram_ram_wp_4k(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_wram_ram_wp_8k(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_wram_ram_wp_16k(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_wram_ram_wp_32k(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_wram_ram_wp_custom_size(BYTE nidx, DBWORD address, DBWORD chunk, size_t size, BYTE rd, BYTE wr);

EXTERNC void memmap_wram_nvram_wp_128b(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_wram_nvram_wp_256b(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_wram_nvram_wp_512b(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_wram_nvram_wp_1k(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_wram_nvram_wp_2k(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_wram_nvram_wp_4k(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_wram_nvram_wp_8k(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_wram_nvram_wp_16k(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_wram_nvram_wp_32k(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_wram_nvram_custom_size(BYTE nidx, DBWORD address, DBWORD chunk, size_t size, BYTE rd, BYTE wr);

// nvram -----------------------------------------------------------------------------

EXTERNC BYTE vram_init(void);
EXTERNC void vram_quit(void);
EXTERNC void vram_set_ram_size(BYTE nidx, size_t size);
EXTERNC void vram_set_nvram_size(BYTE nidx, size_t size);
EXTERNC void vram_memset(void);

EXTERNC void memmap_vram_128b(BYTE nidx, DBWORD address, DBWORD value);
EXTERNC void memmap_vram_256b(BYTE nidx, DBWORD address, DBWORD value);
EXTERNC void memmap_vram_512b(BYTE nidx, DBWORD address, DBWORD value);
EXTERNC void memmap_vram_1k(BYTE nidx, DBWORD address, DBWORD value);
EXTERNC void memmap_vram_2k(BYTE nidx, DBWORD address, DBWORD value);
EXTERNC void memmap_vram_4k(BYTE nidx, DBWORD address, DBWORD value);
EXTERNC void memmap_vram_8k(BYTE nidx, DBWORD address, DBWORD value);
EXTERNC void memmap_vram_custom_size(BYTE nidx, DBWORD address, DBWORD chunk, size_t size);

EXTERNC void memmap_vram_wp_128b(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_vram_wp_256b(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_vram_wp_512b(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_vram_wp_1k(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_vram_wp_2k(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_vram_wp_4k(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_vram_wp_8k(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_vram_wp_custom_size(BYTE nidx, DBWORD address, DBWORD chunk, size_t size, BYTE rd, BYTE wr);

// ram -------------------------------------------------------------------------------

EXTERNC BYTE ram_init(void);
EXTERNC void ram_quit(void);
EXTERNC void ram_set_size(BYTE nidx, size_t size);
EXTERNC void ram_reset_chunks(void);
EXTERNC void ram_memset(void);

EXTERNC BYTE ram_rd(BYTE nidx, WORD address);
EXTERNC void ram_wr(BYTE nidx, WORD address, BYTE value);

EXTERNC void memmap_ram_128b(BYTE nidx, DBWORD address, DBWORD value);
EXTERNC void memmap_ram_256b(BYTE nidx, DBWORD address, DBWORD value);
EXTERNC void memmap_ram_512b(BYTE nidx, DBWORD address, DBWORD value);
EXTERNC void memmap_ram_1k(BYTE nidx, DBWORD address, DBWORD value);
EXTERNC void memmap_ram_2k(BYTE nidx, DBWORD address, DBWORD value);

// nmt -------------------------------------------------------------------------------

EXTERNC BYTE nmt_init(void);
EXTERNC void nmt_quit(void);
EXTERNC void nmt_set_size(BYTE nidx, size_t size);
EXTERNC void nmt_reset_chunks(void);
EXTERNC void nmt_memset(void);

EXTERNC BYTE nmt_rd(BYTE nidx, WORD address);
EXTERNC void nmt_wr(BYTE nidx, WORD address, BYTE value);
EXTERNC void nmt_disable_write(BYTE nidx);

EXTERNC void memmap_nmt_128b(BYTE nidx, DBWORD address, DBWORD value);
EXTERNC void memmap_nmt_256b(BYTE nidx, DBWORD address, DBWORD value);
EXTERNC void memmap_nmt_512b(BYTE nidx, DBWORD address, DBWORD value);
EXTERNC void memmap_nmt_1k(BYTE nidx, DBWORD address, DBWORD value);
EXTERNC void memmap_nmt_2k(BYTE nidx, DBWORD address, DBWORD value);
EXTERNC void memmap_nmt_4k(BYTE nidx, DBWORD address, DBWORD value);
EXTERNC void memmap_nmt_8k(BYTE nidx, DBWORD address, DBWORD value);

EXTERNC void memmap_nmt_wp_128b(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_nmt_wp_256b(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_nmt_wp_512b(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_nmt_wp_1k(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_nmt_wp_2k(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_nmt_wp_4k(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_nmt_wp_8k(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr);

EXTERNC void memmap_nmt_chrrom_128b(BYTE nidx, DBWORD address, DBWORD value);
EXTERNC void memmap_nmt_chrrom_256b(BYTE nidx, DBWORD address, DBWORD value);
EXTERNC void memmap_nmt_chrrom_512b(BYTE nidx, DBWORD address, DBWORD value);
EXTERNC void memmap_nmt_chrrom_1k(BYTE nidx, DBWORD address, DBWORD value);
EXTERNC void memmap_nmt_chrrom_2k(BYTE nidx, DBWORD address, DBWORD value);
EXTERNC void memmap_nmt_chrrom_4k(BYTE nidx, DBWORD address, DBWORD value);
EXTERNC void memmap_nmt_chrrom_8k(BYTE nidx, DBWORD address, DBWORD value);

EXTERNC void memmap_nmt_vram_128b(BYTE nidx, DBWORD address, DBWORD value);
EXTERNC void memmap_nmt_vram_256b(BYTE nidx, DBWORD address, DBWORD value);
EXTERNC void memmap_nmt_vram_512b(BYTE nidx, DBWORD address, DBWORD value);
EXTERNC void memmap_nmt_vram_1k(BYTE nidx, DBWORD address, DBWORD value);
EXTERNC void memmap_nmt_vram_2k(BYTE nidx, DBWORD address, DBWORD value);
EXTERNC void memmap_nmt_vram_4k(BYTE nidx, DBWORD address, DBWORD value);
EXTERNC void memmap_nmt_vram_8k(BYTE nidx, DBWORD address, DBWORD value);

EXTERNC void mirroring_H(BYTE nidx);
EXTERNC void mirroring_V(BYTE nidx);
EXTERNC void mirroring_SCR0(BYTE nidx);
EXTERNC void mirroring_SCR1(BYTE nidx);
EXTERNC void mirroring_FSCR(BYTE nidx);
EXTERNC void mirroring_SCR0x1_SCR1x3(BYTE nidx);
EXTERNC void mirroring_SCR0x3_SCR1x1(BYTE nidx);

// miscrom ---------------------------------------------------------------------------

EXTERNC BYTE miscrom_init(void);
EXTERNC void miscrom_quit(void);
EXTERNC void miscrom_set_size(size_t size);
EXTERNC void miscrom_trainer_init(void);

// misc ------------------------------------------------------------------------------

EXTERNC void nvram_load_file(void);
EXTERNC void nvram_save_file(void);

#undef EXTERNC

#endif /* MEMMAP_H_ */
