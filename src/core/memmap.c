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
#include <stdlib.h>
#include <errno.h>
#include "info.h"
#include "gui.h"
#include "tas.h"
#include "cpu.h"
#include "mappers.h"

INLINE static unsigned int slot_from_address(_memmap_info *info, DBWORD address);
INLINE static size_t calc_mask(size_t size);
INLINE static size_t mask_address_with_size(size_t value, size_t size);
INLINE static unsigned int shift_from_size(size_t size);
INLINE static size_t pow_of_2(size_t size);
INLINE static void set_size(size_t *s, size_t *rs, size_t cs, size_t size);

_memmap_palette memmap_palette;
_oam oam;

_memmap memmap;
_prgrom prgrom;
_chrrom chrrom;
_wram wram;
_vram vram;
_ram ram;
_nmt nmt;
_miscrom miscrom;

// memmap ----------------------------------------------------------------------------

typedef struct _memmap_bank {
	BYTE type;
	BYTE rd;
	BYTE wr;
	size_t size;
	BYTE translate_value;
	_memmap_chunk_dst dst;
	struct _memmap_region_data {
		size_t address;
		size_t value;
		size_t slot;
		size_t bank;
	} region;
} _memmap_bank;

BYTE memmap_malloc(BYTE **dst, size_t size);
BYTE memmap_region_init(_memmap_region *region, size_t size, size_t chunk_size);
void memmap_region_quit(_memmap_region *region);

INLINE static void memmap_auto_wp(DBWORD adr, DBWORD value, size_t size, BYTE rd, BYTE wr, BYTE tvalue);
INLINE static void memmap_auto(DBWORD adr, DBWORD value, size_t size, BYTE tvalue);
INLINE static void memmap_disable(DBWORD adr, size_t size, BYTE tvalue);
INLINE static void memmap_other(DBWORD adr, DBWORD value, size_t bsize, BYTE *dst, size_t dsize, BYTE rd, BYTE wr, BYTE tvalue);
INLINE static void memmap_prgrom(DBWORD adr, DBWORD value, size_t size, BYTE rd, BYTE wr, BYTE tvalue);
INLINE static void memmap_wram(DBWORD adr, DBWORD value, size_t size, BYTE rd, BYTE wr, BYTE tvalue);
INLINE static void memmap_chrrom(DBWORD adr, DBWORD value, size_t size, BYTE rd, BYTE wr, BYTE tvalue);
INLINE static void memmap_chrrom_nmt(DBWORD adr, DBWORD value, size_t size, BYTE tvalue);
INLINE static void memmap_wram_wp(DBWORD adr, DBWORD value, size_t size, BYTE rd, BYTE wr, BYTE tvalue);
INLINE static void memmap_wram_ram_wp(DBWORD adr, DBWORD value, size_t size, BYTE rd, BYTE wr, BYTE tvalue);
INLINE static void memmap_wram_nvram_wp(DBWORD adr, DBWORD value, size_t size, BYTE rd, BYTE wr, BYTE tvalue);
INLINE static void memmap_vram(DBWORD adr, DBWORD value, size_t size, BYTE rd, BYTE wr, BYTE tvalue);
INLINE static void memmap_vram_wp(DBWORD adr, DBWORD value, size_t size, BYTE rd, BYTE wr, BYTE tvalue);
INLINE static void memmap_ram(DBWORD adr, DBWORD value, size_t size, BYTE rd, BYTE wr, BYTE tvalue);
INLINE static void memmap_nmt(DBWORD adr, DBWORD value, size_t size, BYTE rd, BYTE wr, BYTE tvalue);
INLINE static void memmap_nmt_wp(DBWORD adr, DBWORD value, size_t size, BYTE rd, BYTE wr, BYTE tvalue);
INLINE static void memmap_nmt_chrrom(DBWORD adr, DBWORD value, size_t size, BYTE tvalue);

INLINE static void memmap_wp_chunk(_memmap_region *region);
INLINE static void memmap_wp_set_chunks(void);
INLINE static _memmap_region *memmap_get_region(DBWORD address);
INLINE static WORD memmap_banks(enum _sizes_types bsize, size_t size);
INLINE static WORD memmap_control_bank(enum _sizes_types bsize, size_t size, WORD bank);
INLINE static size_t memmap_region_address(_memmap_region *region, WORD address);

static _memmap_bank smmbank;

BYTE memmap_init(void) {
	if (memmap_prg_region_init(MEMMAP_PRG_CHUNK_SIZE_DEFAULT) == EXIT_ERROR) return (EXIT_ERROR);
	if (memmap_chr_region_init(MEMMAP_CHR_CHUNK_SIZE_DEFAULT) == EXIT_ERROR) return (EXIT_ERROR);
	if (memmap_wram_region_init(MEMMAP_WRAM_CHUNK_SIZE_DEFAULT) == EXIT_ERROR) return (EXIT_ERROR);
	if (memmap_ram_region_init(MEMMAP_RAM_CHUNK_SIZE_DEFAULT) == EXIT_ERROR) return (EXIT_ERROR);
	if (memmap_nmt_region_init(MEMMAP_NMT_CHUNK_SIZE_DEFAULT) == EXIT_ERROR) return (EXIT_ERROR);
	return (EXIT_OK);
}
void memmap_quit(void) {
	memmap_region_quit(&memmap.prg);
	memmap_region_quit(&memmap.chr);
	memmap_region_quit(&memmap.wram);
	memmap_region_quit(&memmap.ram);
	memmap_region_quit(&memmap.nmt);
}
BYTE memmap_adr_is_readable(DBWORD address) {
	_memmap_region *region = memmap_get_region(address);

	return (region ? region->chunks[slot_from_address(&region->info, address)].readable : FALSE);
}
BYTE memmap_adr_is_writable(DBWORD address) {
	_memmap_region *region = memmap_get_region(address);

	return (region ? region->chunks[slot_from_address(&region->info, address)].writable : FALSE);
}
WORD memmap_chunk_actual_bank(DBWORD address) {
	_memmap_region *region = memmap_get_region(address);

	if (region) {
		BYTE slot = slot_from_address(&region->info, address);

		if (!region->chunks[slot].pnt) {
			return (0);
		}
		return (region->chunks[slot].actual_bank);
	}
	return (0);
}
BYTE *memmap_chunk_pnt(DBWORD address) {
	_memmap_region *region = memmap_get_region(address);

	if (region) {
		BYTE slot = slot_from_address(&region->info, address);
		BYTE *pnt = NULL;

		if (!region->chunks[slot].pnt) {
			return (NULL);
		}
		pnt = region->chunks[slot].pnt + (address & region->chunks[slot].mask);
		return (pnt > region->chunks[slot].mem_region.end ? NULL : pnt);
	}
	return (NULL);
}

BYTE memmap_prg_region_init(size_t chunk_size) {
	return (memmap_region_init(&memmap.prg, MEMMAP_PRG_SIZE, chunk_size));
}
BYTE memmap_chr_region_init(size_t chunk_size) {
	return (memmap_region_init(&memmap.chr, MEMMAP_CHR_SIZE, chunk_size));
}
BYTE memmap_wram_region_init(size_t chunk_size) {
	return (memmap_region_init(&memmap.wram, MEMMAP_WRAM_SIZE, chunk_size));
}
BYTE memmap_ram_region_init(size_t chunk_size) {
	return (memmap_region_init(&memmap.ram, MEMMAP_RAM_SIZE, chunk_size));
}
BYTE memmap_nmt_region_init(size_t chunk_size) {
	return (memmap_region_init(&memmap.nmt, MEMMAP_NMT_SIZE, chunk_size));
}

// with permissions
INLINE static void memmap_auto_wp(DBWORD adr, DBWORD value, size_t size, BYTE rd, BYTE wr, BYTE tvalue) {
	WORD in_region_adr = adr & 0xFFFF;

	if (adr & PPUMM) {
		// PPU
		if (in_region_adr < 0x2000) {
			if (chrrom_size()) {
				memmap_chrrom(adr, value, size, rd, wr, tvalue);
			} else {
				memmap_vram(adr, value, size, rd, wr, tvalue);
			}
		} else if (in_region_adr < 0x3F00) {
			memmap_nmt(adr, value, size, rd, wr, tvalue);
		}
	} else if (adr & CPUMM) {
		// CPU
		if (in_region_adr >= 0x8000) {
			memmap_prgrom(adr, value, size, rd, wr, tvalue);
		} else if (in_region_adr >= 0x4000) {
			memmap_wram(adr, value, size, rd, wr, tvalue);
		} else if (in_region_adr >= 0x2000) {

		} else {
			memmap_ram(adr, value, size, rd, wr, tvalue);
		}
	}
}
void memmap_auto_wp_128b(DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_auto_wp(address, value, S128B, rd, wr, TRUE);
}
void memmap_auto_wp_256b(DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_auto_wp(address, value, S256B, rd, wr, TRUE);
}
void memmap_auto_wp_512b(DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_auto_wp(address, value, S512B, rd, wr, TRUE);
}
void memmap_auto_wp_1k(DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_auto_wp(address, value, S1K, rd, wr, TRUE);
}
void memmap_auto_wp_2k(DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_auto_wp(address, value, S2K, rd, wr, TRUE);
}
void memmap_auto_wp_4k(DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_auto_wp(address, value, S4K, rd, wr, TRUE);
}
void memmap_auto_wp_8k(DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_auto_wp(address, value, S8K, rd, wr, TRUE);
}
void memmap_auto_wp_16k(DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_auto_wp(address, value, S16K, rd, wr, TRUE);
}
void memmap_auto_wp_32k(DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_auto_wp(address, value, S32K, rd, wr, TRUE);
}
void memmap_auto_wp_custom_size(DBWORD address, DBWORD chunk, size_t size, BYTE rd, BYTE wr) {
	memmap_auto_wp(address, chunk, size, rd, wr, FALSE);
}

// permissions :
// ram    : rd = TRUE, wr = TRUE
// prgrom : rd = TRUE, wr = FALSE
// wram   : rd = TRUE, wr = TRUE
// chrrom : rd = TRUE, wr = FALSE
// vram   : rd = TRUE, wr = TRUE
// nmt    : rd = TRUE, wr = TRUE
INLINE static void memmap_auto(DBWORD adr, DBWORD value, size_t size, BYTE tvalue) {
	WORD in_region_adr = adr & 0xFFFF;

	if (adr & PPUMM) {
		// PPU
		if (in_region_adr < 0x2000) {
			if (chrrom_size()) {
				memmap_chrrom(adr, value, size, TRUE, FALSE, tvalue);
			} else {
				memmap_vram(adr, value, size, TRUE, TRUE, tvalue);
			}
		} else if (in_region_adr < 0x3F00) {
			memmap_nmt(adr, value, size, TRUE, TRUE, tvalue);
		}
	} else if (adr & CPUMM) {
		// CPU
		if (in_region_adr >= 0x8000) {
			memmap_prgrom(adr, value, size, TRUE, FALSE, tvalue);
		} else if (in_region_adr >= 0x4000) {
			memmap_wram(adr, value, size, TRUE, TRUE, tvalue);
		} else if (in_region_adr >= 0x2000) {

		} else {
			memmap_ram(adr, value, size, TRUE, TRUE, tvalue);
		}
	}
}
void memmap_auto_128b(DBWORD address, DBWORD value) {
	memmap_auto(address, value, S128B, TRUE);
}
void memmap_auto_256b(DBWORD address, DBWORD value) {
	memmap_auto(address, value, S256B, TRUE);
}
void memmap_auto_512b(DBWORD address, DBWORD value) {
	memmap_auto(address, value, S512B, TRUE);
}
void memmap_auto_1k(DBWORD address, DBWORD value) {
	memmap_auto(address, value, S1K, TRUE);
}
void memmap_auto_2k(DBWORD address, DBWORD value) {
	memmap_auto(address, value, S2K, TRUE);
}
void memmap_auto_4k(DBWORD address, DBWORD value) {
	memmap_auto(address, value, S4K, TRUE);
}
void memmap_auto_8k(DBWORD address, DBWORD value) {
	memmap_auto(address, value, S8K, TRUE);
}
void memmap_auto_16k(DBWORD address, DBWORD value) {
	memmap_auto(address, value, S16K, TRUE);
}
void memmap_auto_32k(DBWORD address, DBWORD value) {
	memmap_auto(address, value, S32K, TRUE);
}
void memmap_auto_custom_size(DBWORD address, DBWORD chunk, size_t size) {
	memmap_auto(address, chunk, size, FALSE);
}

// permissions : rd = FALSE, wr = FALSE
INLINE static void memmap_disable(DBWORD adr, size_t size, BYTE tvalue) {
	smmbank.region.address = adr;
	smmbank.region.value = 0;
	smmbank.region.slot = 0;
	smmbank.region.bank = 0;
	smmbank.dst.pnt = NULL;
	smmbank.dst.size = 0;
	smmbank.dst.mask = 0;
	smmbank.type = MEMMAP_BANK_NONE;
	smmbank.rd = FALSE;
	smmbank.wr = FALSE;
	smmbank.size = size;
	smmbank.translate_value = tvalue;
	memmap_wp_set_chunks();
}
void memmap_disable_128b(DBWORD address) {
	memmap_disable( address, S128B, TRUE);
}
void memmap_disable_256b(DBWORD address) {
	memmap_disable( address, S256B, TRUE);
}
void memmap_disable_512b(DBWORD address) {
	memmap_disable( address, S512B, TRUE);
}
void memmap_disable_1k(DBWORD address) {
	memmap_disable( address, S1K, TRUE);
}
void memmap_disable_2k(DBWORD address) {
	memmap_disable( address, S2K, TRUE);
}
void memmap_disable_4k(DBWORD address) {
	memmap_disable( address, S4K, TRUE);
}
void memmap_disable_8k(DBWORD address) {
	memmap_disable( address, S8K, TRUE);
}
void memmap_disable_16k(DBWORD address) {
	memmap_disable( address, S16K, TRUE);
}
void memmap_disable_32k(DBWORD address) {
	memmap_disable( address, S32K, TRUE);
}
void memmap_disable_custom_size(DBWORD address, size_t size) {
	memmap_disable(address, size, FALSE);
}

// with permissions
INLINE static void memmap_other(DBWORD adr, DBWORD value, size_t bsize, BYTE *dst, size_t dsize, BYTE rd, BYTE wr, BYTE tvalue) {
	smmbank.region.address = adr;
	smmbank.region.value = value;
	smmbank.region.slot = 0;
	smmbank.region.bank = 0;
	smmbank.dst.pnt = dst;
	smmbank.dst.size = dsize;
	smmbank.dst.mask = calc_mask(dsize);
	smmbank.type = MEMMAP_BANK_OTHER;
	smmbank.rd = rd;
	smmbank.wr = wr;
	smmbank.size = bsize;
	smmbank.translate_value = tvalue;
	memmap_wp_set_chunks();
}
void memmap_other_128b(DBWORD address, DBWORD value, BYTE *dst, size_t dst_size, BYTE rd, BYTE wr) {
	memmap_other(address,  value, S128B, dst, dst_size, rd, wr, TRUE);
}
void memmap_other_256b(DBWORD address, DBWORD value, BYTE *dst, size_t dst_size, BYTE rd, BYTE wr) {
	memmap_other(address,  value, S256B, dst, dst_size, rd, wr, TRUE);
}
void memmap_other_512b(DBWORD address, DBWORD value, BYTE *dst, size_t dst_size, BYTE rd, BYTE wr) {
	memmap_other(address,  value, S512B, dst, dst_size, rd, wr, TRUE);
}
void memmap_other_1k(DBWORD address, DBWORD value, BYTE *dst, size_t dst_size, BYTE rd, BYTE wr) {
	memmap_other(address,  value, S1K, dst, dst_size, rd, wr, TRUE);
}
void memmap_other_2k(DBWORD address, DBWORD value, BYTE *dst, size_t dst_size, BYTE rd, BYTE wr) {
	memmap_other(address,  value, S2K, dst, dst_size, rd, wr, TRUE);
}
void memmap_other_4k(DBWORD address, DBWORD value, BYTE *dst, size_t dst_size, BYTE rd, BYTE wr) {
	memmap_other(address,  value, S4K, dst, dst_size, rd, wr, TRUE);
}
void memmap_other_8k(DBWORD address, DBWORD value, BYTE *dst, size_t dst_size, BYTE rd, BYTE wr) {
	memmap_other(address,  value, S8K, dst, dst_size, rd, wr, TRUE);
}
void memmap_other_16k(DBWORD address, DBWORD value, BYTE *dst, size_t dst_size, BYTE rd, BYTE wr) {
	memmap_other(address,  value, S16K, dst, dst_size, rd, wr, TRUE);
}
void memmap_other_32k(DBWORD address, DBWORD value, BYTE *dst, size_t dst_size, BYTE rd, BYTE wr) {
	memmap_other(address,  value, S32K, dst, dst_size, rd, wr, TRUE);
}
void memmap_other_custom_size(DBWORD address, DBWORD chunk, size_t size, BYTE *dst, size_t dsize, BYTE rd, BYTE wr) {
	memmap_other(address, chunk, size, dst, dsize, rd, wr, FALSE);
}

BYTE memmap_malloc(BYTE **dst, size_t size) {
	(*dst) = !size ? NULL : (BYTE *)malloc(size);
	return ((*dst) ? EXIT_OK : EXIT_ERROR);
}
BYTE memmap_region_init(_memmap_region *region, size_t size, size_t chunk_size) {
	memmap_region_quit(region);

	region->info.shift = shift_from_size(chunk_size);
	region->info.size = size;
	region->info.chunk.size = chunk_size;
	region->info.chunk.items = size / chunk_size;
	region->chunks = malloc(sizeof(_memmap_chunk ) * region->info.chunk.items);
	if (!region->chunks) {
		return (EXIT_ERROR);
	}
	return (EXIT_OK);
}
void memmap_region_quit(_memmap_region *region) {
	if (region->chunks) {
		free(region->chunks);
		region->chunks = NULL;
	}
}

INLINE static void memmap_wp_chunk(_memmap_region *region) {
	region->chunks[smmbank.region.slot].type = !smmbank.dst.pnt ? MEMMAP_BANK_NONE : smmbank.type;
	// per le mapper che permettono il controllo sulla WRAM (tipo MMC3), posso scrivere (o leggere)
	// nella regione interessata anche se non c'è installata nessuan WRAM solo per valorizzare i
	// registri interni della mapper.
	region->chunks[smmbank.region.slot].writable = smmbank.wr;
	region->chunks[smmbank.region.slot].readable = smmbank.rd;
	region->chunks[smmbank.region.slot].permit.wr = smmbank.dst.pnt && smmbank.wr;
	region->chunks[smmbank.region.slot].permit.rd = smmbank.dst.pnt && smmbank.rd;
	region->chunks[smmbank.region.slot].pnt = !smmbank.dst.pnt
		? NULL
		: &smmbank.dst.pnt[(smmbank.region.bank << region->info.shift) & smmbank.dst.mask];
	region->chunks[smmbank.region.slot].mem_region.start = !smmbank.dst.pnt
		? NULL
		: smmbank.dst.pnt;
	region->chunks[smmbank.region.slot].mem_region.end = !smmbank.dst.pnt
		? NULL
		: &smmbank.dst.pnt[smmbank.dst.size];
	region->chunks[smmbank.region.slot].mask = !smmbank.dst.pnt
		? 0
		: mask_address_with_size(region->info.chunk.size - 1, smmbank.dst.size);
	region->chunks[smmbank.region.slot].actual_bank = !smmbank.dst.pnt
		? 0
		: memmap_control_bank(smmbank.size, smmbank.dst.size, smmbank.region.value);
}
INLINE static void memmap_wp_set_chunks(void) {
	_memmap_region *region = memmap_get_region(smmbank.region.address);

	smmbank.region.address &= 0xFFFF;

	if (region) {
		unsigned int slot = slot_from_address(&region->info, smmbank.region.address);
		size_t chunks = smmbank.size / region->info.chunk.size;
		size_t bank = smmbank.translate_value ? smmbank.region.value * chunks : smmbank.region.value;

		for (size_t i = 0; i < chunks; i++) {
			smmbank.region.slot = slot + i;
			smmbank.region.bank = bank + i;
			if (smmbank.region.slot < region->info.chunk.items) {
				memmap_wp_chunk(region);
			}
		}
	}
}
INLINE static _memmap_region *memmap_get_region(DBWORD address) {
	WORD real_address = address & 0xFFFF;

	if (address & PPUMM) {
		// PPU
		if (real_address < 0x2000) {
			return (&memmap.chr);
		} else if (real_address < 0x3F00) {
			return (&memmap.nmt);
		}
	} else if (address & CPUMM) {
		// CPU
		if (real_address >= 0x8000) {
			return (&memmap.prg);
		} else if (real_address >= 0x4000) {
			return (&memmap.wram);
		} else if (real_address >= 0x2000) {

		} else {
			return (&memmap.ram);
		}
	}
	return (NULL);
}
INLINE static WORD memmap_banks(enum _sizes_types bsize, size_t size) {
	return (size / (size_t)bsize) + ((size % (size_t)bsize) ? 1 : 0);
}
INLINE static WORD memmap_control_bank(enum _sizes_types bsize, size_t size, WORD bank) {
	unsigned int max = memmap_banks(bsize, size);

	return (!max ? 0 : bank < max ? bank : bank & (max - 1));
}
INLINE static size_t memmap_region_address(_memmap_region *region, WORD address) {
	const unsigned int slot = slot_from_address(&region->info, address);

	return (region->chunks[slot].pnt - region->chunks[slot].mem_region.start) + (address & region->chunks[slot].mask);
}

// prgrom ----------------------------------------------------------------------------

BYTE prgrom_malloc(void);

BYTE prgrom_init(BYTE set_value) {
	if ((info.reset == CHANGE_ROM) || (info.reset == POWER_UP)) {
		if (prgrom_pnt()) {
			free(prgrom_pnt());
			prgrom_pnt() = NULL;
		}
		if (prgrom_malloc() == EXIT_ERROR) {
			return (EXIT_ERROR);
		}
		memset(prgrom_pnt(), set_value, prgrom.real_size);
		prgrom_mask() = calc_mask(prgrom_size());
		prgrom_reset_chunks();
	}
	return (EXIT_OK);
}
void prgrom_quit(void) {
	if (prgrom_pnt()) {
		free(prgrom_pnt());
	}
	memset(&prgrom, 0x00, sizeof(prgrom));
	memmap_disable_32k(MMCPU(0x8000));
}
void prgrom_set_size(size_t size) {
	set_size(&prgrom_size(), &prgrom.real_size, memmap.prg.info.chunk.size, size);
}
void prgrom_reset_chunks(void) {
	memmap_auto_16k(MMCPU(0x8000), 0);
	memmap_auto_16k(MMCPU(0xC000), ~0);
}
WORD prgrom_banks(enum _sizes_types size) {
	return (memmap_banks(size, prgrom_size()));
}
WORD prgrom_control_bank(enum _sizes_types size, WORD bank) {
	return (memmap_control_bank(size, prgrom_size(), bank));
}
size_t prgrom_region_address(WORD address) {
	return (memmap_region_address(&memmap.prg, address));
}

BYTE prgrom_rd(WORD address) {
	const unsigned int slot = slot_from_address(&memmap.prg.info, address);

	return (memmap.prg.chunks[slot].permit.rd
		? memmap.prg.chunks[slot].pnt[address & memmap.prg.chunks[slot].mask]
		: address >> 8);
}
void prgrom_wr(WORD address, BYTE value) {
	const unsigned int slot = slot_from_address(&memmap.prg.info, address);

	if (memmap.prg.chunks[slot].permit.wr) {
		memmap.prg.chunks[slot].pnt[address & memmap.prg.chunks[slot].mask] = value;
	}
}

// permissions : rd = TRUE, wr = FALSE
INLINE static void memmap_prgrom(DBWORD adr, DBWORD value, size_t size, BYTE rd, BYTE wr, BYTE tvalue) {
	if (adr & CPUMM) {
		smmbank.region.address = adr;
		smmbank.region.value = value;
		smmbank.region.slot = 0;
		smmbank.region.bank = 0;
		smmbank.dst.pnt = prgrom_pnt();
		smmbank.dst.size = prgrom_size();
		smmbank.dst.mask = prgrom_mask();
		smmbank.type = MEMMAP_BANK_PRGROM;
		smmbank.rd = rd;
		smmbank.wr = wr;
		smmbank.size = size;
		smmbank.translate_value = tvalue;
		memmap_wp_set_chunks();
	}
}
void memmap_prgrom_128b(DBWORD address, DBWORD value) {
	memmap_prgrom(address,  value, S128B, TRUE, FALSE, TRUE);
}
void memmap_prgrom_256b(DBWORD address, DBWORD value) {
	memmap_prgrom(address,  value, S256B, TRUE, FALSE, TRUE);
}
void memmap_prgrom_512b(DBWORD address, DBWORD value) {
	memmap_prgrom(address,  value, S512B, TRUE, FALSE, TRUE);
}
void memmap_prgrom_1k(DBWORD address, DBWORD value) {
	memmap_prgrom(address,  value, S1K, TRUE, FALSE, TRUE);
}
void memmap_prgrom_2k(DBWORD address, DBWORD value) {
	memmap_prgrom(address,  value, S2K, TRUE, FALSE, TRUE);
}
void memmap_prgrom_4k(DBWORD address, DBWORD value) {
	memmap_prgrom(address,  value, S4K, TRUE, FALSE, TRUE);
}
void memmap_prgrom_8k(DBWORD address, DBWORD value) {
	memmap_prgrom(address,  value, S8K, TRUE, FALSE, TRUE);
}
void memmap_prgrom_16k(DBWORD address, DBWORD value) {
	memmap_prgrom(address,  value, S16K, TRUE, FALSE, TRUE);
}
void memmap_prgrom_32k(DBWORD address, DBWORD value) {
	memmap_prgrom(address,  value, S32K, TRUE, FALSE, TRUE);
}
void memmap_prgrom_custom_size(DBWORD address, DBWORD chunk, size_t size) {
	memmap_prgrom(address, chunk, size, TRUE, FALSE, FALSE);
}

BYTE prgrom_malloc(void) {
	if (memmap_malloc(&prgrom_pnt(), prgrom.real_size) == EXIT_ERROR) {
		log_error(uL("prgrom malloc;out of memory"));
		return (EXIT_ERROR);
	}
	return (EXIT_OK);
}

// chrom -----------------------------------------------------------------------------

BYTE chrrom_malloc(void);

BYTE chrrom_init(void) {
	if ((info.reset == CHANGE_ROM) || (info.reset == POWER_UP)) {
		if (chrrom_size()) {
			if (chrrom_pnt()) {
				free(chrrom_pnt());
				chrrom_pnt() = NULL;
			}
			if (chrrom_malloc() == EXIT_ERROR) {
				return (EXIT_ERROR);
			}
			memset(chrrom_pnt(), 0x00, chrrom.real_size);
			chrrom_mask() = calc_mask(chrrom_size());
		}
	}
	return (EXIT_OK);
}
void chrrom_quit(void) {
	if (chrrom_pnt()) {
		free(chrrom_pnt());
	}
	memset(&chrrom, 0x00, sizeof(chrrom));
}
void chrrom_set_size(size_t size) {
	set_size(&chrrom_size(), &chrrom.real_size, memmap.chr.info.chunk.size, size);
}
void chrrom_reset_chunks(void) {
	memmap_auto_8k(MMPPU(0x0000), 0);
}
WORD chrrom_banks(enum _sizes_types size) {
	return (memmap_banks(size, chrrom_size()));
}
WORD chrrom_control_bank(enum _sizes_types size, WORD bank) {
	return (memmap_control_bank(size, chrrom_size(), bank));
}

BYTE chr_rd(WORD address) {
	const unsigned int slot = slot_from_address(&memmap.chr.info, address);

	if (memmap.chr.chunks[slot].permit.rd) {
		return (memmap.chr.chunks[slot].pnt[address & memmap.chr.chunks[slot].mask]);
	}
	return (cpu.openbus);
}
void chr_wr(WORD address, BYTE value) {
	const unsigned int slot = slot_from_address(&memmap.chr.info, address);

	if (memmap.chr.chunks[slot].permit.wr) {
		memmap.chr.chunks[slot].pnt[address & memmap.chr.chunks[slot].mask] = value;
	}
}
void chr_disable_write(void) {
	for (size_t i = 0; i < memmap.chr.info.chunk.items; i++) {
		memmap.chr.chunks[i].writable = FALSE;
		memmap.chr.chunks[i].permit.wr = FALSE;
	}
}

// permissions :
// chrrom : rd = TRUE, wr = FALSE
INLINE static void memmap_chrrom(DBWORD adr, DBWORD value, size_t size, BYTE rd, BYTE wr, BYTE tvalue) {
	if (adr & PPUMM) {
		smmbank.region.address = adr;
		smmbank.region.value = value;
		smmbank.region.slot = 0;
		smmbank.region.bank = 0;
		smmbank.dst.pnt = chrrom_pnt();
		smmbank.dst.size = chrrom_size();
		smmbank.dst.mask = chrrom_mask();
		smmbank.type = MEMMAP_BANK_CHRROM;
		smmbank.rd = rd;
		smmbank.wr = wr;
		smmbank.size = size;
		smmbank.translate_value = tvalue;
		memmap_wp_set_chunks();
	}
}
void memmap_chrrom_128b(DBWORD address, DBWORD value) {
	memmap_chrrom(address,  value, S128B, TRUE, FALSE, TRUE);
}
void memmap_chrrom_256b(DBWORD address, DBWORD value) {
	memmap_chrrom(address,  value, S256B, TRUE, FALSE, TRUE);
}
void memmap_chrrom_512b(DBWORD address, DBWORD value) {
	memmap_chrrom(address,  value, S512B, TRUE, FALSE, TRUE);
}
void memmap_chrrom_1k(DBWORD address, DBWORD value) {
	memmap_chrrom(address,  value, S1K, TRUE, FALSE, TRUE);
}
void memmap_chrrom_2k(DBWORD address, DBWORD value) {
	memmap_chrrom(address,  value, S2K, TRUE, FALSE, TRUE);
}
void memmap_chrrom_4k(DBWORD address, DBWORD value) {
	memmap_chrrom(address,  value, S4K, TRUE, FALSE, TRUE);
}
void memmap_chrrom_8k(DBWORD address, DBWORD value) {
	memmap_chrrom(address,  value, S8K, TRUE, FALSE, TRUE);
}
void memmap_chrrom_16k(DBWORD address, DBWORD value) {
	memmap_chrrom(address,  value, S16K, TRUE, FALSE, TRUE);
}
void memmap_chrrom_32k(DBWORD address, DBWORD value) {
	memmap_chrrom(address,  value, S32K, TRUE, FALSE, TRUE);
}
void memmap_chrrom_custom_size(DBWORD address, DBWORD chunk, size_t size) {
	memmap_chrrom(address, chunk, size, TRUE, FALSE, FALSE);
}

INLINE static void memmap_chrrom_nmt(DBWORD adr, DBWORD value, size_t size, BYTE tvalue) {
	if (adr & PPUMM) {
		memmap_other(adr, value, size, nmt_pnt(), nmt_size(), TRUE, TRUE, tvalue);
	}
}
void memmap_chrrom_nmt_128b(DBWORD address, DBWORD value) {
	memmap_chrrom_nmt(address, value, S128B, TRUE);
}
void memmap_chrrom_nmt_256b(DBWORD address, DBWORD value) {
	memmap_chrrom_nmt(address, value, S256B, TRUE);
}
void memmap_chrrom_nmt_512b(DBWORD address, DBWORD value) {
	memmap_chrrom_nmt(address, value, S512B, TRUE);
}
void memmap_chrrom_nmt_1k(DBWORD address, DBWORD value) {
	memmap_chrrom_nmt(address, value, S1K, TRUE);
}
void memmap_chrrom_nmt_2k(DBWORD address, DBWORD value) {
	memmap_chrrom_nmt(address, value, S2K, TRUE);
}
void memmap_chrrom_nmt_4k(DBWORD address, DBWORD value) {
	memmap_chrrom_nmt(address, value, S4K, TRUE);
}
void memmap_chrrom_nmt_8k(DBWORD address, DBWORD value) {
	memmap_chrrom_nmt(address, value, S8K, TRUE);
}

BYTE chrrom_malloc(void) {
	if (memmap_malloc(&chrrom_pnt(), chrrom.real_size) == EXIT_ERROR) {
		log_error(uL("chrrom malloc;out of memory"));
		return (EXIT_ERROR);
	}
	return (EXIT_OK);
}

// wram ------------------------------------------------------------------------------

BYTE wram_malloc(void);

BYTE wram_init(void) {
	if ((info.reset == CHANGE_ROM) || (info.reset == POWER_UP)) {
		if (wram_size()) {
			// alloco la memoria necessaria
			if (wram_malloc() == EXIT_ERROR) {
				return (EXIT_ERROR);
			}
			// inizializzo il pointer della ram
			wram.ram.pnt = wram_nvram_size() ? wram_pnt_byte(wram_nvram_size()) : wram_pnt_byte(0);
			// inizializzo il pointer della nvram
			wram.nvram.pnt = wram_nvram_size() ? wram_pnt_byte(0) : NULL;
			// inizializzom le mask
			wram_mask() = calc_mask(wram_size());
			wram_ram_mask() = calc_mask(wram_ram_size());
			wram_nvram_mask() = calc_mask(wram_nvram_size());

			// resetto la wram
			wram_reset_chunks();
		}
	}
	wram_memset();
	return (EXIT_OK);
}
void wram_quit(void) {
	if (wram_pnt()) {
		free(wram_pnt());
	}
	memset(&wram, 0x00, sizeof(wram));
	memmap_disable_16k(MMCPU(0x4000));
}
void wram_set_ram_size(size_t size) {
	wram.ram.size = pow_of_2(size);
	wram_size() = wram_ram_size() + wram_nvram_size();
	wram.real_size = pow_of_2(wram_size());
}
void wram_set_nvram_size(size_t size) {
	wram.nvram.size = pow_of_2(size);
	wram_size() = wram_ram_size() + wram_nvram_size();
	wram.real_size = pow_of_2(wram_size());
}
void wram_reset_chunks(void) {
	// disabilito la wram da 0x4000 a 0x5FFF
	memmap_disable_8k(MMCPU(0x4000));
	// setto i primi 8k su 0x6000
	memmap_auto_8k(MMCPU(0x6000), 0);
}
void wram_memset(void) {
	if (wram_size()) {
		if (info.mapper.id == FDS_MAPPER) {
			memset(wram_pnt(), 0xEA, wram.real_size);
		} else {
			BYTE *dst = NULL;
			size_t size = 0;

			// Note :
			// 1 - non devo inizializzare la nvram
			// 2 - Idemitsu - Space College - Kikenbutsu no Yasashii Butsuri to Kagaku (Japan).nes
			//     se la ram non è valorizzata a 0 da errore 3 al controllo della memoria
			if ((info.reset == CHANGE_ROM) || (info.reset == POWER_UP)) {
				dst = wram_pnt();
				size = wram.real_size;
			} else if (info.reset >= HARD) {
				dst = wram_ram_pnt();
				size = wram_ram_size();
			}
			if (dst) {
				//memset(dst, 0x00, size);
				emu_initial_ram(dst, size);
			}
		}
	}
}

BYTE wram_rd(WORD address) {
	const unsigned int slot = slot_from_address(&memmap.wram.info, address);

//	if (address < 0x6000) {
//		if ((vs_system.enabled) && (address >= 0x4020)) {
//			if ((address & 0x4020) == 0x4020) {
//				vs_system_r4020_clock(rd, openbus)
//			}
//			if (vs_system.special_mode.r5e0x) {
//				if (address == 0x5E00) {
//					vs_system.special_mode.index = 0;
//				} else if (address == 0x5E01) {
//					return (vs_system.special_mode.r5e0x[(vs_system.special_mode.index++) & 0x1F]);
//				}
//			} else if (vs_system.special_mode.type == VS_SM_Super_Xevious) {
//				if (address == 0x54FF) {
//					return (0x05);
//				} else if (address == 0x5678) {
//					return (vs_system.special_mode.index ? 0x00 : 0x01);
//				} else if (address == 0x578F) {
//					return(vs_system.special_mode.index ? 0xD1 : 0x89);
//				} else if (address == 0x5567) {
//					vs_system.special_mode.index ^= 1;
//					return (vs_system.special_mode.index ? 0x37 : 0x3E);
//				}
//			}
//		}
//	}
//	if (!memmap.wram.chunks[slot].pnt) {
//		// TODO : devo controllare se lo fa davvero
//		if (vs_system.enabled && vs_system.shared_mem) {
//			return (prg.ram.data[address & 0x07FF]);
//		}
//	} else

	return (memmap.wram.chunks[slot].permit.rd)
		? memmap.wram.chunks[slot].pnt[address & memmap.wram.chunks[slot].mask]
		: (address >> 8);
}
void wram_wr(WORD address, BYTE value) {
	const unsigned int slot = slot_from_address(&memmap.wram.info, address);

//	if (address < 0x6000) {
//		if (vs_system.enabled) {
//			if ((address >= 0x4020) && ((address & 0x4020) == 0x4020)) {
//				vs_system_r4020_clock(wr, value)
//			}
//		}
//	}
//	if (!memmap.wram.chunks[slot].pnt) {
//		// TODO : devo controllare se lo fa davvero
//		if (vs_system.enabled && vs_system.shared_mem) {
//			prg.ram.data[address & 0x07FF] = value;
//		}
//	} else
	if (memmap.wram.chunks[slot].permit.wr) {
		memmap.wram.chunks[slot].pnt[address & memmap.wram.chunks[slot].mask] = value;
	}
}

BYTE wram_direct_rd(WORD address, BYTE openbus) {
	return (wram_pnt() ? wram_byte(mask_address_with_size(address, wram_size())) : openbus);
}
void wram_direct_wr(WORD address, BYTE value) {
	if (wram_pnt()) {
		wram_byte(mask_address_with_size(address, wram_size())) = value;
	}
}

// permissions : rd = TRUE, wr = TRUE
INLINE static void memmap_wram(DBWORD adr, DBWORD value, size_t size, BYTE rd, BYTE wr, BYTE tvalue) {
	if (adr & CPUMM) {
		smmbank.region.address = adr;
		smmbank.region.value = value;
		smmbank.region.slot = 0;
		smmbank.region.bank = 0;
		smmbank.dst.pnt = wram_pnt();
		smmbank.dst.size = wram_size();
		smmbank.dst.mask = wram_mask();
		smmbank.type = MEMMAP_BANK_WRAM;
		smmbank.rd = rd;
		smmbank.wr = wr;
		smmbank.size = size;
		smmbank.translate_value = tvalue;
		memmap_wp_set_chunks();
	}
}
void memmap_wram_128b(DBWORD address, DBWORD value) {
	memmap_wram(address,  value, S128B, TRUE, TRUE, TRUE);
}
void memmap_wram_256b(DBWORD address, DBWORD value) {
	memmap_wram(address,  value, S256B, TRUE, TRUE, TRUE);
}
void memmap_wram_512b(DBWORD address, DBWORD value) {
	memmap_wram(address,  value, S512B, TRUE, TRUE, TRUE);
}
void memmap_wram_1k(DBWORD address, DBWORD value) {
	memmap_wram(address,  value, S1K, TRUE, TRUE, TRUE);
}
void memmap_wram_2k(DBWORD address, DBWORD value) {
	memmap_wram(address,  value, S2K, TRUE, TRUE, TRUE);
}
void memmap_wram_4k(DBWORD address, DBWORD value) {
	memmap_wram(address,  value, S4K, TRUE, TRUE, TRUE);
}
void memmap_wram_8k(DBWORD address, DBWORD value) {
	memmap_wram(address,  value, S8K, TRUE, TRUE, TRUE);
}
void memmap_wram_16k(DBWORD address, DBWORD value) {
	memmap_wram(address,  value, S16K, TRUE, TRUE, TRUE);
}
void memmap_wram_32k(DBWORD address, DBWORD value) {
	memmap_wram(address,  value, S32K, TRUE, TRUE, TRUE);
}
void memmap_wram_custom_size(DBWORD address, DBWORD chunk, size_t size) {
	memmap_wram(address, chunk, size, TRUE, TRUE, FALSE);
}

// with permissions
INLINE static void memmap_wram_wp(DBWORD adr, DBWORD value, size_t size, BYTE rd, BYTE wr, BYTE tvalue) {
	memmap_wram(adr, value, size, rd, wr, tvalue);
}
void memmap_wram_wp_128b(DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_wram_wp(address, value, S128B, rd, wr, TRUE);
}
void memmap_wram_wp_256b(DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_wram_wp(address, value, S256B, rd, wr, TRUE);
}
void memmap_wram_wp_512b(DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_wram_wp(address, value, S512B, rd, wr, TRUE);
}
void memmap_wram_wp_1k(DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_wram_wp(address, value, S1K, rd, wr, TRUE);
}
void memmap_wram_wp_2k(DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_wram_wp(address, value, S2K, rd, wr, TRUE);
}
void memmap_wram_wp_4k(DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_wram_wp(address, value, S4K, rd, wr, TRUE);
}
void memmap_wram_wp_8k(DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_wram_wp(address, value, S8K, rd, wr, TRUE);
}
void memmap_wram_wp_16k(DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_wram_wp(address, value, S16K, rd, wr, TRUE);
}
void memmap_wram_wp_32k(DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_wram_wp(address, value, S32K, rd, wr, TRUE);
}
void memmap_wram_wp_custom_size(DBWORD address, DBWORD chunk, size_t size, BYTE rd, BYTE wr) {
	memmap_wram_wp(address, chunk, size, rd, wr, FALSE);
}

// with permissions
INLINE static void memmap_wram_ram_wp(DBWORD adr, DBWORD value, size_t size, BYTE rd, BYTE wr, BYTE tvalue) {
	if (adr & CPUMM) {
		smmbank.region.address = adr;
		smmbank.region.value = value;
		smmbank.region.slot = 0;
		smmbank.region.bank = 0;
		smmbank.dst.pnt = wram_ram_pnt();
		smmbank.dst.size = wram_ram_size();
		smmbank.dst.mask = wram_ram_mask();
		smmbank.type = MEMMAP_BANK_WRAM;
		smmbank.rd = rd;
		smmbank.wr = wr;
		smmbank.size = size;
		smmbank.translate_value = tvalue;
		memmap_wp_set_chunks();
	}
}
void memmap_wram_ram_wp_128b(DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_wram_ram_wp(address, value, S128B, rd, wr, TRUE);
}
void memmap_wram_ram_wp_256b(DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_wram_ram_wp(address, value, S256B, rd, wr, TRUE);
}
void memmap_wram_ram_wp_512b(DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_wram_ram_wp(address, value, S512B, rd, wr, TRUE);
}
void memmap_wram_ram_wp_1k(DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_wram_ram_wp(address, value, S1K, rd, wr, TRUE);
}
void memmap_wram_ram_wp_2k(DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_wram_ram_wp(address, value, S2K, rd, wr, TRUE);
}
void memmap_wram_ram_wp_4k(DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_wram_ram_wp(address, value, S4K, rd, wr, TRUE);
}
void memmap_wram_ram_wp_8k(DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_wram_ram_wp(address, value, S8K, rd, wr, TRUE);
}
void memmap_wram_ram_wp_16k(DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_wram_ram_wp(address, value, S16K, rd, wr, TRUE);
}
void memmap_wram_ram_wp_32k(DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_wram_ram_wp(address, value, S32K, rd, wr, TRUE);
}
void memmap_wram_ram_wp_custom_size(DBWORD address, DBWORD chunk, size_t size, BYTE rd, BYTE wr) {
	memmap_wram_ram_wp(address, chunk, size, rd, wr, FALSE);
}

// with permissions
INLINE static void memmap_wram_nvram_wp(DBWORD adr, DBWORD value, size_t size, BYTE rd, BYTE wr, BYTE tvalue) {
	if (adr & CPUMM) {
		smmbank.region.address = adr;
		smmbank.region.value = value;
		smmbank.region.slot = 0;
		smmbank.region.bank = 0;
		smmbank.dst.pnt = wram_nvram_pnt();
		smmbank.dst.size = wram_nvram_size();
		smmbank.dst.mask = wram_nvram_mask();
		smmbank.type = MEMMAP_BANK_WRAM;
		smmbank.rd = rd;
		smmbank.wr = wr;
		smmbank.size = size;
		smmbank.translate_value = tvalue;
		memmap_wp_set_chunks();
	}
}
void memmap_wram_nvram_wp_128b(DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_wram_nvram_wp(address,  value, S128B, rd, wr, TRUE);
}
void memmap_wram_nvram_wp_256b(DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_wram_nvram_wp(address,  value, S256B, rd, wr, TRUE);
}
void memmap_wram_nvram_wp_512b(DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_wram_nvram_wp(address,  value, S512B, rd, wr, TRUE);
}
void memmap_wram_nvram_wp_1k(DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_wram_nvram_wp(address,  value, S1K, rd, wr, TRUE);
}
void memmap_wram_nvram_wp_2k(DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_wram_nvram_wp(address,  value, S2K, rd, wr, TRUE);
}
void memmap_wram_nvram_wp_4k(DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_wram_nvram_wp(address,  value, S4K, rd, wr, TRUE);
}
void memmap_wram_nvram_wp_8k(DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_wram_nvram_wp(address,  value, S8K, rd, wr, TRUE);
}
void memmap_wram_nvram_wp_16k(DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_wram_nvram_wp(address,  value, S16K, rd, wr, TRUE);
}
void memmap_wram_nvram_wp_32k(DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_wram_nvram_wp(address,  value, S32K, rd, wr, TRUE);
}
void memmap_wram_nvram_wp_custom_size(DBWORD address, DBWORD chunk, size_t size, BYTE rd, BYTE wr) {
	memmap_wram_nvram_wp(address, chunk, size, rd, wr, FALSE);
}

BYTE wram_malloc(void) {
	if (memmap_malloc(&wram_pnt(), wram.real_size) == EXIT_ERROR) {
		log_error(uL("wram malloc;out of memory"));
		return (EXIT_ERROR);
	}
	return (EXIT_OK);
}

// nvram -----------------------------------------------------------------------------

BYTE vram_malloc(void);

BYTE vram_init(void) {
	if ((info.reset == CHANGE_ROM) || (info.reset == POWER_UP)) {
		if (vram_size()) {
			// alloco la memoria necessaria
			if (vram_malloc() == EXIT_ERROR) {
				return (EXIT_ERROR);
			}
			// inizializzo il pointer della ram
			vram.ram.pnt = vram_nvram_size() ? vram_pnt_byte(vram_nvram_size()) : vram_pnt_byte(0);
			// inizializzo il pointer della nvram
			vram.nvram.pnt = vram_nvram_size() ? vram_pnt_byte(0) : NULL;
			// inizializzom le mask
			vram_mask() = calc_mask(vram_size());
			vram_ram_mask() = calc_mask(vram_ram_size());
			vram_nvram_mask() = calc_mask(vram_nvram_size());
		}
	}
	vram_memset();
	return (EXIT_OK);
}
void vram_quit(void) {
	if (vram_pnt()) {
		free(vram_pnt());
	}
	memset(&vram, 0x00, sizeof(vram));
}
void vram_set_ram_size(size_t size) {
	vram.ram.size = pow_of_2(size);
	vram_size() = vram_ram_size() + vram_nvram_size();
	vram.real_size = pow_of_2(vram_size());
}
void vram_set_nvram_size(size_t size) {
	vram.nvram.size = pow_of_2(size);
	vram_size() = vram_ram_size() + vram_nvram_size();
	vram.real_size = pow_of_2(vram_size());
}
void vram_memset(void) {
	if (vram_size()) {
		BYTE *dst = NULL;
		size_t size = 0;

		if ((info.reset == CHANGE_ROM) || (info.reset == POWER_UP)) {
			dst = vram_pnt();
			size = vram.real_size;
		} else if (info.reset >= HARD) {
			dst = vram_ram_pnt();
			size = vram_ram_size();
		}
		if (dst) {
			memset(dst, 0x00, size);
			//emu_initial_ram(dst, size);
		}
	}
}

// permissions : rd = TRUE, wr = TRUE
INLINE static void memmap_vram(DBWORD adr, DBWORD value, size_t size, BYTE rd, BYTE wr, BYTE tvalue) {
	if (adr & PPUMM) {
		smmbank.region.address = adr;
		smmbank.region.value = value;
		smmbank.region.slot = 0;
		smmbank.region.bank = 0;
		smmbank.dst.pnt = vram_pnt();
		smmbank.dst.size = vram_size();
		smmbank.dst.mask = vram_mask();
		smmbank.type = MEMMAP_BANK_VRAM;
		smmbank.rd = rd;
		smmbank.wr = wr;
		smmbank.size = size;
		smmbank.translate_value = tvalue;
		memmap_wp_set_chunks();
	}
}
void memmap_vram_128b(DBWORD address, DBWORD value) {
	memmap_vram(address,  value, S128B, TRUE, TRUE, TRUE);
}
void memmap_vram_256b(DBWORD address, DBWORD value) {
	memmap_vram(address,  value, S256B, TRUE, TRUE, TRUE);
}
void memmap_vram_512b(DBWORD address, DBWORD value) {
	memmap_vram(address,  value, S512B, TRUE, TRUE, TRUE);
}
void memmap_vram_1k(DBWORD address, DBWORD value) {
	memmap_vram(address,  value, S1K, TRUE, TRUE, TRUE);
}
void memmap_vram_2k(DBWORD address, DBWORD value) {
	memmap_vram(address,  value, S2K, TRUE, TRUE, TRUE);
}
void memmap_vram_4k(DBWORD address, DBWORD value) {
	memmap_vram(address,  value, S4K, TRUE, TRUE, TRUE);
}
void memmap_vram_8k(DBWORD address, DBWORD value) {
	memmap_vram(address,  value, S8K, TRUE, TRUE, TRUE);
}
void memmap_vram_custom_size(DBWORD address, DBWORD chunk, size_t size) {
	memmap_vram(address, chunk, size, TRUE, TRUE, FALSE);
}

INLINE static void memmap_vram_wp(DBWORD adr, DBWORD value, size_t size, BYTE rd, BYTE wr, BYTE tvalue) {
	memmap_vram(adr, value, size, rd, wr, tvalue);
}
void memmap_vram_wp_128b(DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_vram_wp(address,  value, S128B, rd, wr, TRUE);
}
void memmap_vram_wp_256b(DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_vram_wp(address,  value, S256B, rd, wr, TRUE);
}
void memmap_vram_wp_512b(DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_vram_wp(address,  value, S512B, rd, wr, TRUE);
}
void memmap_vram_wp_1k(DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_vram_wp(address,  value, S1K, rd, wr, TRUE);
}
void memmap_vram_wp_2k(DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_vram_wp(address,  value, S2K, rd, wr, TRUE);
}
void memmap_vram_wp_4k(DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_vram_wp(address,  value, S4K, rd, wr, TRUE);
}
void memmap_vram_wp_8k(DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_vram_wp(address,  value, S8K, rd, wr, TRUE);
}
void memmap_vram_wp_custom_size(DBWORD address, DBWORD chunk, size_t size, BYTE rd, BYTE wr) {
	memmap_vram_wp(address, chunk, size, rd, wr, FALSE);
}

BYTE vram_malloc(void) {
	if (memmap_malloc(&vram_pnt(), vram.real_size) == EXIT_ERROR) {
		log_error(uL("vram malloc;out of memory"));
		return (EXIT_ERROR);
	}
	return (EXIT_OK);
}

// ram -------------------------------------------------------------------------------

BYTE ram_malloc(void);

BYTE ram_init(void) {
	if ((info.reset == CHANGE_ROM) || (info.reset == POWER_UP)) {
		if (ram_size()) {
			// alloco la memoria necessaria
			if (ram_malloc() == EXIT_ERROR) {
				return (EXIT_ERROR);
			}
			// inizializzom la mask
			ram_mask() = calc_mask(ram_size());
		}
	}
	ram_memset();
	return (EXIT_OK);
}
void ram_quit(void) {
	if (ram_pnt()) {
		free(ram_pnt());
	}
	memset(&ram, 0x00, sizeof(ram));
}
void ram_set_size(size_t size) {
	set_size(&ram_size(), &ram.real_size, memmap.ram.info.chunk.size, size);
}
void ram_reset_chunks(void) {
	memmap_auto_2k(MMCPU(0x0000), 0);
	memmap_auto_2k(MMCPU(0x0800), 0);
	memmap_auto_2k(MMCPU(0x1000), 0);
	memmap_auto_2k(MMCPU(0x1800), 0);
}
void ram_memset(void) {
	if (ram_size()) {
		//memset(ram_pnt(), 0x00, ram.real_size);
		emu_initial_ram(ram_pnt(), ram.real_size);
	}
}

BYTE ram_rd(WORD address) {
	unsigned int slot = slot_from_address(&memmap.ram.info, address);

	if (memmap.ram.chunks[slot].permit.rd) {
		return (memmap.ram.chunks[slot].pnt[address & memmap.ram.chunks[slot].mask]);
	}
	return (cpu.openbus);
}
void ram_wr(WORD address, BYTE value) {
	unsigned int slot = slot_from_address(&memmap.ram.info, address);

	if (memmap.ram.chunks[slot].permit.wr) {
		memmap.ram.chunks[slot].pnt[address & memmap.ram.chunks[slot].mask] = value;
	}
}

// permissions : rd = TRUE, wr = TRUE
INLINE static void memmap_ram(DBWORD adr, DBWORD value, size_t size, BYTE rd, BYTE wr, BYTE tvalue) {
	if (adr & CPUMM) {
		smmbank.region.address = adr;
		smmbank.region.value = value;
		smmbank.region.slot = 0;
		smmbank.region.bank = 0;
		smmbank.dst.pnt = ram_pnt();
		smmbank.dst.size = ram_size();
		smmbank.dst.mask = ram_mask();
		smmbank.type = MEMMAP_BANK_RAM;
		smmbank.rd = rd;
		smmbank.wr = wr;
		smmbank.size = size;
		smmbank.translate_value = tvalue;
		memmap_wp_set_chunks();
	}
}
void memmap_ram_128b(DBWORD address, DBWORD value) {
	memmap_ram(address,  value, S128B, TRUE, TRUE, TRUE);
}
void memmap_ram_256b(DBWORD address, DBWORD value) {
	memmap_ram(address,  value, S256B, TRUE, TRUE, TRUE);
}
void memmap_ram_512b(DBWORD address, DBWORD value) {
	memmap_ram(address,  value, S512B, TRUE, TRUE, TRUE);
}
void memmap_ram_1k(DBWORD address, DBWORD value) {
	memmap_ram(address,  value, S1K, TRUE, TRUE, TRUE);
}
void memmap_ram_2k(DBWORD address, DBWORD value) {
	memmap_ram(address,  value, S2K, TRUE, TRUE, TRUE);
}
void memmap_ram_4k(DBWORD address, DBWORD value) {
	memmap_ram(address,  value, S4K, TRUE, TRUE, TRUE);
}
void memmap_ram_8k(DBWORD address, DBWORD value) {
	memmap_ram(address,  value, S8K, TRUE, TRUE, TRUE);
}

BYTE ram_malloc(void) {
	if (memmap_malloc(&ram_pnt(), ram.real_size) == EXIT_ERROR) {
		log_error(uL("ram malloc;out of memory"));
		return (EXIT_ERROR);
	}
	return (EXIT_OK);
}

// nmt -------------------------------------------------------------------------------

BYTE nmt_malloc(void);

BYTE nmt_init(void) {
	if ((info.reset == CHANGE_ROM) || (info.reset == POWER_UP)) {
		if (nmt_size()) {
			// alloco la memoria necessaria
			if (nmt_malloc() == EXIT_ERROR) {
				return (EXIT_ERROR);
			}
			// inizializzom la mask
			nmt_mask() = calc_mask(nmt_size());
		}
	}
	nmt_memset();
	return (EXIT_OK);
}
void nmt_quit(void) {
	if (nmt_pnt()) {
		free(nmt_pnt());
	}
	memset(&nmt, 0x00, sizeof(nmt));
}
void nmt_set_size(size_t size) {
	set_size(&nmt_size(), &nmt.real_size, memmap.nmt.info.chunk.size, size);
}
void nmt_reset_chunks(void) {
	switch (info.mapper.mirroring) {
		case MIRRORING_HORIZONTAL:
			mirroring_H();
			return;
		case MIRRORING_VERTICAL:
			mirroring_V();
			return;
		case MIRRORING_FOURSCR:
			mirroring_FSCR();
			return;
		case MIRRORING_SINGLE_SCR0:
			mirroring_SCR0();
			return;
		case MIRRORING_SINGLE_SCR1:
			mirroring_SCR1();
			return;
		default:
			mirroring_V();
			return;
	}
}
void nmt_memset(void) {
	if (nmt_size()) {
		memset(nmt_pnt(), 0x00, nmt.real_size);
	}
}

BYTE nmt_rd(WORD address) {
	unsigned int slot = slot_from_address(&memmap.nmt.info, address);

	if (memmap.nmt.chunks[slot].permit.rd) {
		return (memmap.nmt.chunks[slot].pnt[address & memmap.nmt.chunks[slot].mask]);
	}
	return (cpu.openbus);
}
void nmt_wr(WORD address, BYTE value) {
	unsigned int slot = slot_from_address(&memmap.nmt.info, address);

	if (memmap.nmt.chunks[slot].permit.wr) {
		memmap.nmt.chunks[slot].pnt[address & memmap.nmt.chunks[slot].mask] = value;
	}
}
void nmt_disable_write(void) {
	for (size_t i = 0; i < memmap.nmt.info.chunk.items; i++) {
		memmap.nmt.chunks[i].writable = FALSE;
		memmap.nmt.chunks[i].permit.wr = FALSE;
	}
}

// permissions : rd = TRUE, wr = TRUE
INLINE static void memmap_nmt(DBWORD adr, DBWORD value, size_t size, BYTE rd, BYTE wr, BYTE tvalue) {
	if (adr & PPUMM) {
		smmbank.region.address = adr;
		smmbank.region.value = value;
		smmbank.region.slot = 0;
		smmbank.region.bank = 0;
		smmbank.dst.pnt = nmt_pnt();
		smmbank.dst.size = nmt_size();
		smmbank.dst.mask = nmt_mask();
		smmbank.type = MEMMAP_BANK_NMT;
		smmbank.rd = rd;
		smmbank.wr = wr;
		smmbank.size = size;
		smmbank.translate_value = tvalue;
		memmap_wp_set_chunks();
	}
}
void memmap_nmt_128b(DBWORD address, DBWORD value) {
	memmap_nmt(address,  value, S256B, TRUE, TRUE, TRUE);
}
void memmap_nmt_256b(DBWORD address, DBWORD value) {
	memmap_nmt(address,  value, S128B, TRUE, TRUE, TRUE);
}
void memmap_nmt_512b(DBWORD address, DBWORD value) {
	memmap_nmt(address,  value, S512B, TRUE, TRUE, TRUE);
}
void memmap_nmt_1k(DBWORD address, DBWORD value) {
	memmap_nmt(address,  value, S1K, TRUE, TRUE, TRUE);
}
void memmap_nmt_2k(DBWORD address, DBWORD value) {
	memmap_nmt(address,  value, S2K, TRUE, TRUE, TRUE);
}
void memmap_nmt_4k(DBWORD address, DBWORD value) {
	memmap_nmt(address,  value, S4K, TRUE, TRUE, TRUE);
}
void memmap_nmt_8k(DBWORD address, DBWORD value) {
	memmap_nmt(address,  value, S8K, TRUE, TRUE, TRUE);
}

INLINE static void memmap_nmt_wp(DBWORD adr, DBWORD value, size_t size, BYTE rd, BYTE wr, BYTE tvalue) {
	memmap_nmt(adr, value, size, rd, wr, tvalue);
}
void memmap_nmt_wp_128b(DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_nmt_wp(address,  value, S128B, rd, wr, TRUE);
}
void memmap_nmt_wp_256b(DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_nmt_wp(address,  value, S256B, rd, wr, TRUE);
}
void memmap_nmt_wp_512b(DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_nmt_wp(address,  value, S512B, rd, wr, TRUE);
}
void memmap_nmt_wp_1k(DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_nmt_wp(address,  value, S1K, rd, wr, TRUE);
}
void memmap_nmt_wp_2k(DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_nmt_wp(address,  value, S2K, rd, wr, TRUE);
}
void memmap_nmt_wp_4k(DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_nmt_wp(address,  value, S4K, rd, wr, TRUE);
}
void memmap_nmt_wp_8k(DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_nmt_wp(address,  value, S8K, rd, wr, TRUE);
}

INLINE static void memmap_nmt_chrrom(DBWORD adr, DBWORD value, size_t size, BYTE tvalue) {
	BYTE *dst = NULL;
	size_t dsize = 0;
	BYTE rd = TRUE;
	BYTE wr = FALSE;

	if (adr & PPUMM) {
		if (chrrom_size()) {
			dst = chrrom_pnt();
			dsize = chrrom_size();
		} else if (vram_size()) {
			dst = vram_pnt();
			dsize = vram_size();
			wr = TRUE;
		}
		if (dst) {
			memmap_other(adr, value, size, dst, dsize, rd, wr, tvalue);
		}
	}
}
void memmap_nmt_chrrom_128b(DBWORD address, DBWORD value) {
	memmap_nmt_chrrom(address, value, S128B, TRUE);
}
void memmap_nmt_chrrom_256b(DBWORD address, DBWORD value) {
	memmap_nmt_chrrom(address, value, S256B, TRUE);
}
void memmap_nmt_chrrom_512b(DBWORD address, DBWORD value) {
	memmap_nmt_chrrom(address, value, S512B, TRUE);
}
void memmap_nmt_chrrom_1k(DBWORD address, DBWORD value) {
	memmap_nmt_chrrom(address, value, S1K, TRUE);
}
void memmap_nmt_chrrom_2k(DBWORD address, DBWORD value) {
	memmap_nmt_chrrom(address, value, S2K, TRUE);
}
void memmap_nmt_chrrom_4k(DBWORD address, DBWORD value) {
	memmap_nmt_chrrom(address, value, S4K, TRUE);
}
void memmap_nmt_chrrom_8k(DBWORD address, DBWORD value) {
	memmap_nmt_chrrom(address, value, S8K, TRUE);
}

INLINE static void memmap_nmt_vram(DBWORD adr, DBWORD value, size_t size, BYTE tvalue) {
	if (adr & PPUMM) {
		memmap_other(adr, value, size, vram_pnt(), vram_size(), TRUE, TRUE, tvalue);
	}
}
void memmap_nmt_vram_128b(DBWORD address, DBWORD value) {
	memmap_nmt_vram(address, value, S128B, TRUE);
}
void memmap_nmt_vram_256b(DBWORD address, DBWORD value) {
	memmap_nmt_vram(address, value, S256B, TRUE);
}
void memmap_nmt_vram_512b(DBWORD address, DBWORD value) {
	memmap_nmt_vram(address, value, S512B, TRUE);
}
void memmap_nmt_vram_1k(DBWORD address, DBWORD value) {
	memmap_nmt_vram(address, value, S1K, TRUE);
}
void memmap_nmt_vram_2k(DBWORD address, DBWORD value) {
	memmap_nmt_vram(address, value, S2K, TRUE);
}
void memmap_nmt_vram_4k(DBWORD address, DBWORD value) {
	memmap_nmt_vram(address, value, S4K, TRUE);
}
void memmap_nmt_vram_8k(DBWORD address, DBWORD value) {
	memmap_nmt_vram(address, value, S8K, TRUE);
}

void mirroring_H(void) {
	mapper.mirroring = MIRRORING_HORIZONTAL;
	memmap_nmt_1k(MMPPU(0x2000), 0);
	memmap_nmt_1k(MMPPU(0x2400), 0);
	memmap_nmt_1k(MMPPU(0x2800), 1);
	memmap_nmt_1k(MMPPU(0x2C00), 1);

	memmap_nmt_1k(MMPPU(0x3000), 0);
	memmap_nmt_1k(MMPPU(0x3400), 0);
	memmap_nmt_1k(MMPPU(0x3800), 1);
	memmap_nmt_1k(MMPPU(0x3C00), 1);
}
void mirroring_V(void) {
	mapper.mirroring = MIRRORING_VERTICAL;
	memmap_nmt_2k(MMPPU(0x2000), 0);
	memmap_nmt_2k(MMPPU(0x2800), 0);

	memmap_nmt_2k(MMPPU(0x3000), 0);
	memmap_nmt_2k(MMPPU(0x3800), 0);
}
void mirroring_SCR0(void) {
	mapper.mirroring = MIRRORING_SINGLE_SCR0;
	memmap_nmt_1k(MMPPU(0x2000), 0);
	memmap_nmt_1k(MMPPU(0x2400), 0);
	memmap_nmt_1k(MMPPU(0x2800), 0);
	memmap_nmt_1k(MMPPU(0x2C00), 0);

	memmap_nmt_1k(MMPPU(0x3000), 0);
	memmap_nmt_1k(MMPPU(0x3400), 0);
	memmap_nmt_1k(MMPPU(0x3800), 0);
	memmap_nmt_1k(MMPPU(0x3C00), 0);
}
void mirroring_SCR1(void) {
	mapper.mirroring = MIRRORING_SINGLE_SCR1;
	memmap_nmt_1k(MMPPU(0x2000), 1);
	memmap_nmt_1k(MMPPU(0x2400), 1);
	memmap_nmt_1k(MMPPU(0x2800), 1);
	memmap_nmt_1k(MMPPU(0x2C00), 1);

	memmap_nmt_1k(MMPPU(0x3000), 1);
	memmap_nmt_1k(MMPPU(0x3400), 1);
	memmap_nmt_1k(MMPPU(0x3800), 1);
	memmap_nmt_1k(MMPPU(0x3C00), 1);
}
void mirroring_FSCR(void) {
	mapper.mirroring = MIRRORING_FOURSCR;
	memmap_nmt_4k(MMPPU(0x2000), 0);

	memmap_nmt_4k(MMPPU(0x3000), 0);
}
void mirroring_SCR0x1_SCR1x3(void) {
	mapper.mirroring = MIRRORING_SCR0x1_SCR1x3;
	memmap_nmt_1k(MMPPU(0x2000), 0);
	memmap_nmt_1k(MMPPU(0x2400), 1);
	memmap_nmt_1k(MMPPU(0x2800), 1);
	memmap_nmt_1k(MMPPU(0x2C00), 1);

	memmap_nmt_1k(MMPPU(0x3000), 0);
	memmap_nmt_1k(MMPPU(0x3400), 1);
	memmap_nmt_1k(MMPPU(0x3800), 1);
	memmap_nmt_1k(MMPPU(0x3C00), 1);
}
void mirroring_SCR0x3_SCR1x1(void) {
	mapper.mirroring = MIRRORING_SCR0x3_SCR1x1;
	memmap_nmt_1k(MMPPU(0x2000), 0);
	memmap_nmt_1k(MMPPU(0x2400), 0);
	memmap_nmt_1k(MMPPU(0x2800), 0);
	memmap_nmt_1k(MMPPU(0x2C00), 1);

	memmap_nmt_1k(MMPPU(0x3000), 0);
	memmap_nmt_1k(MMPPU(0x3400), 0);
	memmap_nmt_1k(MMPPU(0x3800), 0);
	memmap_nmt_1k(MMPPU(0x3C00), 1);
}

BYTE nmt_malloc(void) {
	if (memmap_malloc(&nmt_pnt(), nmt.real_size) == EXIT_ERROR) {
		log_error(uL("nmt malloc;out of memory"));
		return (EXIT_ERROR);
	}
	return (EXIT_OK);
}

// miscrom ---------------------------------------------------------------------------

BYTE miscrom_malloc(void);

BYTE miscrom_init(void) {
	if ((info.reset == CHANGE_ROM) || (info.reset == POWER_UP)) {
		if (miscrom_pnt()) {
			free(miscrom_pnt());
			miscrom_pnt() = NULL;
		}
		if (miscrom_size()) {
			if (miscrom_malloc() == EXIT_ERROR) {
				return (EXIT_ERROR);
			}
			memset(miscrom_pnt(), 0x00, miscrom.real_size);
		}
	}
	return (EXIT_OK);
}
void miscrom_quit(void) {
	if (miscrom_pnt()) {
		free(miscrom_pnt());
	}
	memset(&miscrom, 0x00, sizeof(miscrom));
}
void miscrom_set_size(size_t size) {
	set_size(&miscrom_size(), &miscrom.real_size, memmap.prg.info.chunk.size, size);
}

BYTE miscrom_malloc(void) {
	if (memmap_malloc(&miscrom_pnt(), miscrom.real_size) == EXIT_ERROR) {
		log_error(uL("miscrom malloc;out of memory"));
		return (EXIT_ERROR);
	}
	return (EXIT_OK);
}
void miscrom_trainer_init(void) {
	if (info.reset >= HARD) {
		if (miscrom.trainer.in_use) {
			BYTE *dst = NULL;

			if (wram_size() >= S8K) {
				dst = wram_pnt() + S4K;
			}
			if (miscrom.trainer.dst) {
				dst = miscrom.trainer.dst;
			}
			if (dst) {
				memcpy(dst, miscrom_pnt(), miscrom_size());
			}
		}
	}
}

// misc ----------------------------------------------------------------------------

void nvram_file(uTCHAR *prg_ram_file);

void nvram_load_file(void) {
	if (tas.type != NOTAS) {
		return;
	}
	if ((info.reset == CHANGE_ROM) || (info.reset == POWER_UP)) {
		if (wram_nvram_size() || vram_nvram_size() || info.mapper.force_battery_io) {
			uTCHAR prg_ram_file[LENGTH_FILE_NAME_LONG];
			FILE *fp = NULL;

			// estraggo il nome del file
			nvram_file(&prg_ram_file[0]);

			// provo ad aprire il file
			fp = ufopen(prg_ram_file, uL("rb"));

			if (fp) {
				BYTE buffer[8];

				if (fread(&buffer[0], 8, 1, fp) < 1) {
					log_error(uL("mapper;error on read battery memory (%s)"), strerror(errno));
					fclose(fp);
					return;
				}
				if ((buffer[0] != 'F') ||
					(buffer[1] != 'H') ||
					(buffer[2] != 'p') ||
					(buffer[3] != 'u') ||
					(buffer[4] != 'N') ||
					(buffer[5] != 'E') ||
					(buffer[6] != 'S') ||
					(buffer[7] != 0x00)) {
					log_error(uL("mapper;error on read battery memory (unknow format)"));
					fclose(fp);
					return;
				}
				if (wram_nvram_size() && wram_nvram_pnt()) {
					// leggo il contenuto della nvram
					if (fread(wram_nvram_pnt(), wram_nvram_size(), 1, fp) < 1) {
						log_error(uL("mapper;error on read battery memory (%s)"), strerror(errno));
						fclose(fp);
						return;
					}
				}
				if (vram_nvram_size() && vram_nvram_pnt()) {
					// leggo il contenuto della nvram
					if (fread(vram_nvram_pnt(), vram_nvram_size(), 1, fp) < 1) {
						log_error(uL("mapper;error on read battery memory (%s)"), strerror(errno));
						fclose(fp);
						return;
					}
				}
				if (extcl_battery_io) {
					extcl_battery_io(RD_BAT, fp);
				}
				// chiudo il file
				fclose(fp);
			}
		}
	}
}
void nvram_save_file(void) {
	if (tas.type != NOTAS) {
		return;
	}
	if (wram_nvram_size() || vram_nvram_size() || info.mapper.force_battery_io) {
		uTCHAR prg_ram_file[LENGTH_FILE_NAME_LONG];
		FILE *fp = NULL;

		// estraggo il nome del file
		nvram_file(&prg_ram_file[0]);

		// apro il file
		fp = ufopen(prg_ram_file, uL("w+b"));

		if (fp) {
			const BYTE buffer[8] = { 'F', 'H', 'p', 'u', 'N', 'E', 'S', 0x00 };

			if (fwrite(&buffer[0], sizeof(buffer), 1, fp) < 1) {
				log_error(uL("mapper;error on write battery memory (%s)"), strerror(errno));
				fclose(fp);
				return;
			}
			if (wram_nvram_size() && wram_nvram_pnt()) {
				// scrivo il contenuto della nvram
				if (fwrite(wram_nvram_pnt(), wram_nvram_size(), 1, fp) < 1) {
					log_error(uL("mapper;error on write battery memory (%s)"), strerror(errno));
					fclose(fp);
					return;
				}
			}
			if (vram_nvram_size() && vram_nvram_pnt()) {
				// scrivo il contenuto della nvram
				if (fwrite(vram_nvram_pnt(), vram_nvram_size(), 1, fp) < 1) {
					log_error(uL("mapper;error on write battery memory (%s)"), strerror(errno));
					fclose(fp);
					return;
				}
			}
			if (extcl_battery_io) {
				extcl_battery_io(WR_BAT, fp);
			}
			// forzo la scrittura del file
			fflush(fp);
			// chiudo
			fclose(fp);
		}
	}
}
void nvram_file(uTCHAR *prg_ram_file) {
	uTCHAR basename[255], *fl = info.rom.file, *last_dot = NULL;

	gui_utf_basename(fl, basename, usizeof(basename));
	usnprintf(prg_ram_file, LENGTH_FILE_NAME_LONG, uL("" uPs("") PRB_FOLDER "/" uPs("")), gui_data_folder(), basename);

	// rintraccio l'ultimo '.' nel nome
	last_dot = ustrrchr(prg_ram_file, uL('.'));
	if (last_dot) {
		// elimino l'estensione
		(*last_dot) = 0x00;
	}
	// aggiungo l'estensione prb
	ustrcat(prg_ram_file, uL(".prb"));
}

// ---------------------------------------------------------------------------------

INLINE static unsigned int slot_from_address(_memmap_info *minfo, DBWORD address) {
	return ((address >> minfo->shift) & (minfo->chunk.items - 1));
}
INLINE static size_t calc_mask(size_t size) {
	size_t max = size - 1;
	size_t mask = 0;

	while (max > 0) {
		mask = (mask << 1) | 1;
		max >>= 1;
	}
	return (mask);
}
INLINE static size_t mask_address_with_size(size_t value, size_t size) {
	size_t max = size - 1;

	return (value > max ? value & max : value);
}
INLINE static unsigned int shift_from_size(size_t size) {
	unsigned int shift = 0, pot = 1;

	while (pot < size) {
		pot <<= 1;
		shift++;
	}
	return (shift);
}
INLINE static size_t pow_of_2(size_t size) {
	return (size <= 1 ? size : emu_power_of_two(size));
}
INLINE static void set_size(size_t *s, size_t *rs, size_t cs, size_t size) {
	(*s) = size;
	(*rs) = pow_of_2((size / cs) + ((size % cs) ? 1 : 0)) * cs;
}
