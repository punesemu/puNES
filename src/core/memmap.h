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
	S128B = 0x80,
	S256B = 0x100,
	S512B = 0x200,
	S1K = 0x400,
	S2K = 0x800,
	S4K = 0x1000,
	S8K = 0x2000,
	S16K = 0x4000,
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
#define prgrom_calc_chunk(address) ((address) / memmap.prg.info.chunk.size)

#define chrrom_size() chrrom.data.size
#define chrrom_pnt() chrrom.data.pnt
#define chrrom_pnt_byte(byte) &chrrom.data.pnt[byte]
#define chrrom_byte(byte) chrrom.data.pnt[(byte)]
#define chrrom_mask() chrrom.data.mask

#define ram_size() ram.data.size
#define ram_pnt() ram.data.pnt
#define ram_pnt_byte(byte) &ram.data.pnt[byte]
#define ram_byte(byte) ram.data.pnt[(byte)]
#define ram_mask() ram.data.mask

#define wram_size() wram.data.size
#define wram_pnt() wram.data.pnt
#define wram_pnt_byte(byte) &wram.data.pnt[byte]
#define wram_byte(byte) wram.data.pnt[(byte)]
#define wram_mask() wram.data.mask
#define wram_calc_chunk(address) ((address) / memmap.wram.info.chunk.size)

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

#define vram_size() vram.data.size
#define vram_pnt() vram.data.pnt
#define vram_pnt_byte(byte) &vram.data.pnt[byte]
#define vram_byte(byte) vram.data.pnt[(byte)]
#define vram_mask() vram.data.mask
#define vram_calc_chunk(address) ((address) / memmap.vram.info.chunk.size)

#define vram_ram_size() vram.ram.size
#define vram_ram_pnt() vram.ram.pnt
#define vram_ram_pnt_byte(byte) &vram.ram.pnt[byte]
#define vram_ram_byte(byte) vram.ram.pnt[(byte)]
#define vram_ram_mask() vram.ram.mask

#define vram_nvram_size() vram.nvram.size
#define vram_nvram_pnt() vram.nvram.pnt
#define vram_nvram_pnt_byte(byte) &vram.nvram.pnt[byte]
#define vram_nvram_byte(byte) vram.nvram.pnt[(byte)]
#define vram_nvram_mask() vram.nvram.mask

#define nmt_size() nmt.data.size
#define nmt_pnt() nmt.data.pnt
#define nmt_pnt_byte(byte) &nmt.data.pnt[byte]
#define nmt_byte(byte) nmt.data.pnt[(byte)]
#define nmt_mask() nmt.data.mask

#define miscrom_size() miscrom.data.size
#define miscrom_pnt() miscrom.data.pnt
#define miscrom_pnt_byte(byte) &miscrom.data.pnt[byte]
#define miscrom_byte(byte) miscrom.data.pnt[(byte)]
#define miscrom_ram_mask() miscrom.data.mask
#define miscrom_trainer_dst() miscrom.trainer.dst



// TUTTI da rinomiare e poi ELIMINARE
#if defined WRAM_OLD_HANDLER
#define prg_size() prg.rom.size
#define prg_byte(index) prg_rom()[index]
#define prg_pnt(index) &prg_byte(index)
#define prg_chip_rom(chip_rom) prg.chip[chip_rom].data
#define prg_chip_size(chip_rom) prg.chip[chip_rom].size
#define prg_rom_rd(address) prg.rom_8k[((address) >> 13) & 0x03][(address) & 0x1FFF]

#define chr_rom() chr.rom.data
#define chr_size() chr.rom.size
#define chr_byte(index) chr_rom()[index]
#define chr_pnt(index) &chr_byte(index)
#define chr_chip_rom(chip_rom) chr.chip[chip_rom].data
#define chr_chip_size(chip_rom) chr.chip[chip_rom].size
#define chr_ram_size() info.chr.rom.banks_8k << 13

#else
#define prg_rom() prgrom_pnt()
#define prg_size() prgrom_size()
#define prg_byte(index) prg_rom()[index]
#define prg_pnt(index) &prg_byte(index)
#define prg_chip_rom(chip_rom) prgrom.chips.chunk[chip_rom].pnt
#define prg_chip_size(chip_rom) prgrom.chips.chunk[chip_rom].size
#define prg_rom_rd(address) prgrom_rd(address)

#define chr_rom() chrrom_pnt()
#define chr_size() chrrom_size()
#define chr_byte(index) chr_rom()[index]
#define chr_pnt(index) &chr_byte(index)
#define chr_chip_rom(chip_rom) chrrom.chips.chunk[chip_rom].pnt
#define chr_chip_size(chip_rom) chrrom.chips.chunk[chip_rom].size
#define chr_ram_size() vram_size()

#endif





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

extern _mmap_palette mmap_palette;
extern _oam oam;



extern _memmap memmap;
extern _prgrom prgrom;
extern _chrrom chrrom;
extern _wram wram;
extern _vram vram;
extern _ram ram;
extern _nmt nmt;
extern _miscrom miscrom;

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

// memmap ----------------------------------------------------------------------------

EXTERNC BYTE memmap_init(void);
EXTERNC void memmap_quit(void);
EXTERNC BYTE memmap_adr_is_readable(DBWORD address);
EXTERNC BYTE memmap_adr_is_writable(DBWORD address);
EXTERNC WORD memmap_chunk_actual_bank(DBWORD address);
EXTERNC BYTE *memmap_chunk_pnt(DBWORD address);

EXTERNC BYTE memmap_prg_region_init(size_t chunk_size);
EXTERNC BYTE memmap_chr_region_init(size_t chunk_size);
EXTERNC BYTE memmap_wram_region_init(size_t chunk_size);
EXTERNC BYTE memmap_ram_region_init(size_t chunk_size);
EXTERNC BYTE memmap_nmt_region_init(size_t chunk_size);

EXTERNC void memmap_auto_wp_128b(DBWORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_auto_wp_256b(DBWORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_auto_wp_512b(DBWORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_auto_wp_1k(DBWORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_auto_wp_2k(DBWORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_auto_wp_4k(DBWORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_auto_wp_8k(DBWORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_auto_wp_16k(DBWORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_auto_wp_32k(DBWORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_auto_wp_custom_size(DBWORD address, DBWORD chunk, size_t size, BYTE rd, BYTE wr);

EXTERNC void memmap_auto_128b(DBWORD address, DBWORD value);
EXTERNC void memmap_auto_256b(DBWORD address, DBWORD value);
EXTERNC void memmap_auto_512b(DBWORD address, DBWORD value);
EXTERNC void memmap_auto_1k(DBWORD address, DBWORD value);
EXTERNC void memmap_auto_2k(DBWORD address, DBWORD value);
EXTERNC void memmap_auto_4k(DBWORD address, DBWORD value);
EXTERNC void memmap_auto_8k(DBWORD address, DBWORD value);
EXTERNC void memmap_auto_16k(DBWORD address, DBWORD value);
EXTERNC void memmap_auto_32k(DBWORD address, DBWORD value);
EXTERNC void memmap_auto_custom_size(DBWORD address, DBWORD chunk, size_t size);

EXTERNC void memmap_disable_128b(DBWORD address);
EXTERNC void memmap_disable_256b(DBWORD address);
EXTERNC void memmap_disable_512b(DBWORD address);
EXTERNC void memmap_disable_1k(DBWORD address);
EXTERNC void memmap_disable_2k(DBWORD address);
EXTERNC void memmap_disable_4k(DBWORD address);
EXTERNC void memmap_disable_8k(DBWORD address);
EXTERNC void memmap_disable_16k(DBWORD address);
EXTERNC void memmap_disable_32k(DBWORD address);
EXTERNC void memmap_disable_custom_size(DBWORD address, size_t size);

EXTERNC void memmap_other_128b(DBWORD address, DBWORD value, BYTE *dst, size_t dst_size, BYTE rd, BYTE wr);
EXTERNC void memmap_other_256b(DBWORD address, DBWORD value, BYTE *dst, size_t dst_size, BYTE rd, BYTE wr);
EXTERNC void memmap_other_512b(DBWORD address, DBWORD value, BYTE *dst, size_t dst_size, BYTE rd, BYTE wr);
EXTERNC void memmap_other_1k(DBWORD address, DBWORD value, BYTE *dst, size_t dst_size, BYTE rd, BYTE wr);
EXTERNC void memmap_other_2k(DBWORD address, DBWORD value, BYTE *dst, size_t dst_size, BYTE rd, BYTE wr);
EXTERNC void memmap_other_4k(DBWORD address, DBWORD value, BYTE *dst, size_t dst_size, BYTE rd, BYTE wr);
EXTERNC void memmap_other_8k(DBWORD address, DBWORD value, BYTE *dst, size_t dst_size, BYTE rd, BYTE wr);
EXTERNC void memmap_other_16k(DBWORD address, DBWORD value, BYTE *dst, size_t dst_size, BYTE rd, BYTE wr);
EXTERNC void memmap_other_32k(DBWORD address, DBWORD value, BYTE *dst, size_t dst_size, BYTE rd, BYTE wr);
EXTERNC void memmap_other_custom_size(DBWORD address, DBWORD initial_chunk, size_t custom_size, BYTE *dst, size_t dst_size, BYTE rd, BYTE wr);

// prgrom ----------------------------------------------------------------------------

EXTERNC BYTE prgrom_init(BYTE set_value);
EXTERNC void prgrom_quit(void);
EXTERNC void prgrom_set_size(size_t size);
EXTERNC void prgrom_reset_chunks(void);
EXTERNC WORD prgrom_banks(enum _sizes_types size);
EXTERNC WORD prgrom_control_bank(enum _sizes_types size, WORD bank);
EXTERNC size_t prgrom_region_address(WORD address);

EXTERNC BYTE prgrom_rd(WORD address);
EXTERNC void prgrom_wr(WORD address, BYTE value);

EXTERNC void memmap_prgrom_128b(DBWORD address, DBWORD value);
EXTERNC void memmap_prgrom_256b(DBWORD address, DBWORD value);
EXTERNC void memmap_prgrom_512b(DBWORD address, DBWORD value);
EXTERNC void memmap_prgrom_1k(DBWORD address, DBWORD value);
EXTERNC void memmap_prgrom_2k(DBWORD address, DBWORD value);
EXTERNC void memmap_prgrom_4k(DBWORD address, DBWORD value);
EXTERNC void memmap_prgrom_8k(DBWORD address, DBWORD value);
EXTERNC void memmap_prgrom_16k(DBWORD address, DBWORD value);
EXTERNC void memmap_prgrom_32k(DBWORD address, DBWORD value);
EXTERNC void memmap_prgrom_custom_size(DBWORD address, DBWORD chunk, size_t size);

// chrom -----------------------------------------------------------------------------

EXTERNC BYTE chrrom_init(void);
EXTERNC void chrrom_quit(void);
EXTERNC void chrrom_set_size(size_t size);
EXTERNC void chrrom_reset_chunks(void);
EXTERNC WORD chrrom_banks(enum _sizes_types size);
EXTERNC WORD chrrom_control_bank(enum _sizes_types size, WORD bank);

EXTERNC BYTE chr_rd(WORD address);
EXTERNC void chr_wr(WORD address, BYTE value);
EXTERNC void chr_disable_write(void);

EXTERNC void memmap_chrrom_128b(DBWORD address, DBWORD value);
EXTERNC void memmap_chrrom_256b(DBWORD address, DBWORD value);
EXTERNC void memmap_chrrom_512b(DBWORD address, DBWORD value);
EXTERNC void memmap_chrrom_1k(DBWORD address, DBWORD value);
EXTERNC void memmap_chrrom_2k(DBWORD address, DBWORD value);
EXTERNC void memmap_chrrom_4k(DBWORD address, DBWORD value);
EXTERNC void memmap_chrrom_8k(DBWORD address, DBWORD value);
EXTERNC void memmap_chrrom_16k(DBWORD address, DBWORD value);
EXTERNC void memmap_chrrom_32k(DBWORD address, DBWORD value);
EXTERNC void memmap_chrrom_custom_size(DBWORD address, DBWORD chunk, size_t size);

EXTERNC void memmap_chrrom_nmt_128b(DBWORD address, DBWORD value);
EXTERNC void memmap_chrrom_nmt_256b(DBWORD address, DBWORD value);
EXTERNC void memmap_chrrom_nmt_512b(DBWORD address, DBWORD value);
EXTERNC void memmap_chrrom_nmt_1k(DBWORD address, DBWORD value);
EXTERNC void memmap_chrrom_nmt_2k(DBWORD address, DBWORD value);
EXTERNC void memmap_chrrom_nmt_4k(DBWORD address, DBWORD value);
EXTERNC void memmap_chrrom_nmt_8k(DBWORD address, DBWORD value);

// wram ------------------------------------------------------------------------------

EXTERNC BYTE wram_init(void);
EXTERNC void wram_quit(void);
EXTERNC void wram_set_ram_size(size_t size);
EXTERNC void wram_set_nvram_size(size_t size);
EXTERNC void wram_reset_chunks(void);
EXTERNC void wram_memset(void);

EXTERNC BYTE wram_rd(WORD address);
EXTERNC void wram_wr(WORD address, BYTE value);

EXTERNC BYTE wram_direct_rd(WORD address, BYTE openbus);
EXTERNC void wram_direct_wr(WORD address, BYTE value);

EXTERNC void memmap_wram_128b(DBWORD address, DBWORD value);
EXTERNC void memmap_wram_256b(DBWORD address, DBWORD value);
EXTERNC void memmap_wram_512b(DBWORD address, DBWORD value);
EXTERNC void memmap_wram_1k(DBWORD address, DBWORD value);
EXTERNC void memmap_wram_2k(DBWORD address, DBWORD value);
EXTERNC void memmap_wram_4k(DBWORD address, DBWORD value);
EXTERNC void memmap_wram_8k(DBWORD address, DBWORD value);
EXTERNC void memmap_wram_16k(DBWORD address, DBWORD value);
EXTERNC void memmap_wram_32k(DBWORD address, DBWORD value);
EXTERNC void memmap_wram_custom_size(DBWORD address, DBWORD chunk, size_t size);

EXTERNC void memmap_wram_ram_wp_128b(DBWORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_wram_ram_wp_256b(DBWORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_wram_ram_wp_512b(DBWORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_wram_ram_wp_1k(DBWORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_wram_ram_wp_2k(DBWORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_wram_ram_wp_4k(DBWORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_wram_ram_wp_8k(DBWORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_wram_ram_wp_16k(DBWORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_wram_ram_wp_32k(DBWORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_wram_ram_wp_custom_size(DBWORD address, DBWORD chunk, size_t size, BYTE rd, BYTE wr);

EXTERNC void memmap_wram_nvram_wp_128b(DBWORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_wram_nvram_wp_256b(DBWORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_wram_nvram_wp_512b(DBWORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_wram_nvram_wp_1k(DBWORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_wram_nvram_wp_2k(DBWORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_wram_nvram_wp_4k(DBWORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_wram_nvram_wp_8k(DBWORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_wram_nvram_wp_16k(DBWORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_wram_nvram_wp_32k(DBWORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_wram_nvram_custom_size(DBWORD address, DBWORD chunk, size_t size, BYTE rd, BYTE wr);

// nvram -----------------------------------------------------------------------------

EXTERNC BYTE vram_init(void);
EXTERNC void vram_quit(void);
EXTERNC void vram_set_ram_size(size_t size);
EXTERNC void vram_set_nvram_size(size_t size);
EXTERNC void vram_memset(void);

EXTERNC void memmap_vram_128b(DBWORD address, DBWORD value);
EXTERNC void memmap_vram_256b(DBWORD address, DBWORD value);
EXTERNC void memmap_vram_512b(DBWORD address, DBWORD value);
EXTERNC void memmap_vram_1k(DBWORD address, DBWORD value);
EXTERNC void memmap_vram_2k(DBWORD address, DBWORD value);
EXTERNC void memmap_vram_4k(DBWORD address, DBWORD value);
EXTERNC void memmap_vram_8k(DBWORD address, DBWORD value);
EXTERNC void memmap_vram_custom_size(DBWORD address, DBWORD chunk, size_t size);

EXTERNC void memmap_vram_wp_128b(DBWORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_vram_wp_256b(DBWORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_vram_wp_512b(DBWORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_vram_wp_1k(DBWORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_vram_wp_2k(DBWORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_vram_wp_4k(DBWORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_vram_wp_8k(DBWORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_vram_wp_custom_size(DBWORD address, DBWORD chunk, size_t size, BYTE rd, BYTE wr);

// ram -------------------------------------------------------------------------------

EXTERNC BYTE ram_init(void);
EXTERNC void ram_quit(void);
EXTERNC void ram_set_size(size_t size);
EXTERNC void ram_reset_chunks(void);
EXTERNC void ram_memset(void);
EXTERNC void ram_disable_write(void);

EXTERNC BYTE ram_rd(WORD address);
EXTERNC void ram_wr(WORD address, BYTE value);

EXTERNC void memmap_ram_128b(DBWORD address, DBWORD value);
EXTERNC void memmap_ram_256b(DBWORD address, DBWORD value);
EXTERNC void memmap_ram_512b(DBWORD address, DBWORD value);
EXTERNC void memmap_ram_1k(DBWORD address, DBWORD value);
EXTERNC void memmap_ram_2k(DBWORD address, DBWORD value);

// nmt -------------------------------------------------------------------------------

EXTERNC BYTE nmt_init(void);
EXTERNC void nmt_quit(void);
EXTERNC void nmt_set_size(size_t size);
EXTERNC void nmt_reset_chunks(void);
EXTERNC void nmt_memset(void);
EXTERNC void nmt_disable_write(void);

EXTERNC BYTE nmt_rd(WORD address);
EXTERNC void nmt_wr(WORD address, BYTE value);

EXTERNC void memmap_nmt_128b(DBWORD address, DBWORD value);
EXTERNC void memmap_nmt_256b(DBWORD address, DBWORD value);
EXTERNC void memmap_nmt_512b(DBWORD address, DBWORD value);
EXTERNC void memmap_nmt_1k(DBWORD address, DBWORD value);
EXTERNC void memmap_nmt_2k(DBWORD address, DBWORD value);
EXTERNC void memmap_nmt_4k(DBWORD address, DBWORD value);
EXTERNC void memmap_nmt_8k(DBWORD address, DBWORD value);

EXTERNC void memmap_nmt_wp_128b(DBWORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_nmt_wp_256b(DBWORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_nmt_wp_512b(DBWORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_nmt_wp_1k(DBWORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_nmt_wp_2k(DBWORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_nmt_wp_4k(DBWORD address, DBWORD value, BYTE rd, BYTE wr);
EXTERNC void memmap_nmt_wp_8k(DBWORD address, DBWORD value, BYTE rd, BYTE wr);

EXTERNC void memmap_nmt_chrrom_128b(DBWORD address, DBWORD value);
EXTERNC void memmap_nmt_chrrom_256b(DBWORD address, DBWORD value);
EXTERNC void memmap_nmt_chrrom_512b(DBWORD address, DBWORD value);
EXTERNC void memmap_nmt_chrrom_1k(DBWORD address, DBWORD value);
EXTERNC void memmap_nmt_chrrom_2k(DBWORD address, DBWORD value);
EXTERNC void memmap_nmt_chrrom_4k(DBWORD address, DBWORD value);
EXTERNC void memmap_nmt_chrrom_8k(DBWORD address, DBWORD value);

EXTERNC void memmap_nmt_vram_128b(DBWORD address, DBWORD value);
EXTERNC void memmap_nmt_vram_256b(DBWORD address, DBWORD value);
EXTERNC void memmap_nmt_vram_512b(DBWORD address, DBWORD value);
EXTERNC void memmap_nmt_vram_1k(DBWORD address, DBWORD value);
EXTERNC void memmap_nmt_vram_2k(DBWORD address, DBWORD value);
EXTERNC void memmap_nmt_vram_4k(DBWORD address, DBWORD value);
EXTERNC void memmap_nmt_vram_8k(DBWORD address, DBWORD value);


EXTERNC void mirroring_H(void);
EXTERNC void mirroring_V(void);
EXTERNC void mirroring_SCR0(void);
EXTERNC void mirroring_SCR1(void);
EXTERNC void mirroring_FSCR(void);
EXTERNC void mirroring_SCR0x1_SCR1x3(void);
EXTERNC void mirroring_SCR0x3_SCR1x1(void);

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
