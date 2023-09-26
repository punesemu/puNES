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
#include "vs_system.h"

INLINE static unsigned int slot_from_address(_memmap_info *info, DBWORD address);
INLINE static size_t calc_mask(size_t size);
INLINE static size_t mask_address_with_size(size_t value, size_t size);
INLINE static unsigned int shift_from_size(size_t size);
INLINE static size_t pow_of_2(size_t size);
INLINE static void set_size(size_t *s, size_t *rs, size_t cs, size_t size);

// memmap ----------------------------------------------------------------------------

typedef struct _memmap_bank {
	BYTE nidx;
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

INLINE static void memmap_auto_wp(BYTE nidx, DBWORD adr, DBWORD value, size_t size, BYTE rd, BYTE wr, BYTE tvalue);
INLINE static void memmap_auto(BYTE nidx, DBWORD adr, DBWORD value, size_t size, BYTE tvalue);
INLINE static void memmap_disable(BYTE nidx, DBWORD adr, size_t size, BYTE tvalue);
INLINE static void memmap_other(BYTE nidx, DBWORD adr, DBWORD value, size_t bsize, BYTE *dst, size_t dsize, BYTE rd, BYTE wr, BYTE tvalue);
INLINE static void memmap_prgrom(BYTE nidx, DBWORD adr, DBWORD value, size_t size, BYTE rd, BYTE wr, BYTE tvalue);
INLINE static void memmap_chrrom(BYTE nidx, DBWORD adr, DBWORD value, size_t size, BYTE rd, BYTE wr, BYTE tvalue);
INLINE static void memmap_chrrom_nmt(BYTE nidx, DBWORD adr, DBWORD value, size_t size, BYTE tvalue);
INLINE static void memmap_wram(BYTE nidx, DBWORD adr, DBWORD value, size_t size, BYTE rd, BYTE wr, BYTE tvalue);
INLINE static void memmap_wram_wp(BYTE nidx, DBWORD adr, DBWORD value, size_t size, BYTE rd, BYTE wr, BYTE tvalue);
INLINE static void memmap_wram_ram_wp(BYTE nidx, DBWORD adr, DBWORD value, size_t size, BYTE rd, BYTE wr, BYTE tvalue);
INLINE static void memmap_wram_nvram_wp(BYTE nidx, DBWORD adr, DBWORD value, size_t size, BYTE rd, BYTE wr, BYTE tvalue);
INLINE static void memmap_vram(BYTE nidx, DBWORD adr, DBWORD value, size_t size, BYTE rd, BYTE wr, BYTE tvalue);
INLINE static void memmap_vram_wp(BYTE nidx, DBWORD adr, DBWORD value, size_t size, BYTE rd, BYTE wr, BYTE tvalue);
INLINE static void memmap_ram(BYTE nidx, DBWORD adr, DBWORD value, size_t size, BYTE rd, BYTE wr, BYTE tvalue);
INLINE static void memmap_nmt(BYTE nidx, DBWORD adr, DBWORD value, size_t size, BYTE rd, BYTE wr, BYTE tvalue);
INLINE static void memmap_nmt_wp(BYTE nidx, DBWORD adr, DBWORD value, size_t size, BYTE rd, BYTE wr, BYTE tvalue);
INLINE static void memmap_nmt_chrrom(BYTE nidx, DBWORD adr, DBWORD value, size_t size, BYTE tvalue);

INLINE static void memmap_wp_chunk(_memmap_bank *mbank, _memmap_region *region);
INLINE static void memmap_wp_set_chunks(_memmap_bank *mbank);
INLINE static _memmap_region *memmap_get_region(BYTE nidx, DBWORD address);
INLINE static WORD memmap_banks(enum _sizes_types bsize, size_t size);
INLINE static WORD memmap_control_bank(enum _sizes_types bsize, size_t size, WORD bank);
INLINE static size_t memmap_region_address(_memmap_region *region, WORD address);

BYTE memmap_init(void) {
	for (int nesidx = 0; nesidx < NES_CHIPS_MAX; nesidx++) {
		if (memmap_prg_region_init(nesidx, MEMMAP_PRG_CHUNK_SIZE_DEFAULT) == EXIT_ERROR) return (EXIT_ERROR);
		if (memmap_chr_region_init(nesidx, MEMMAP_CHR_CHUNK_SIZE_DEFAULT) == EXIT_ERROR) return (EXIT_ERROR);
		if (memmap_wram_region_init(nesidx, MEMMAP_WRAM_CHUNK_SIZE_DEFAULT) == EXIT_ERROR) return (EXIT_ERROR);
		if (memmap_ram_region_init(nesidx, MEMMAP_RAM_CHUNK_SIZE_DEFAULT) == EXIT_ERROR) return (EXIT_ERROR);
		if (memmap_nmt_region_init(nesidx, MEMMAP_NMT_CHUNK_SIZE_DEFAULT) == EXIT_ERROR) return (EXIT_ERROR);
	}
	return (EXIT_OK);
}
void memmap_quit(void) {
	for (int nesidx = 0; nesidx < NES_CHIPS_MAX; nesidx++) {
		memmap_region_quit(&nes[nesidx].m.memmap.prg);
		memmap_region_quit(&nes[nesidx].m.memmap.chr);
		memmap_region_quit(&nes[nesidx].m.memmap.wram);
		memmap_region_quit(&nes[nesidx].m.memmap.ram);
		memmap_region_quit(&nes[nesidx].m.memmap.nmt);
	}
}
BYTE memmap_adr_is_readable(BYTE nidx, DBWORD address) {
	_memmap_region *region = memmap_get_region(nidx, address);

	return (region ? region->chunks[slot_from_address(&region->info, address)].readable : FALSE);
}
BYTE memmap_adr_is_writable(BYTE nidx, DBWORD address) {
	_memmap_region *region = memmap_get_region(nidx, address);

	return (region ? region->chunks[slot_from_address(&region->info, address)].writable : FALSE);
}
WORD memmap_chunk_actual_bank(BYTE nidx, DBWORD address) {
	_memmap_region *region = memmap_get_region(nidx, address);

	if (region) {
		BYTE slot = slot_from_address(&region->info, address);

		if (!region->chunks[slot].pnt) {
			return (0);
		}
		return (region->chunks[slot].actual_bank);
	}
	return (0);
}
BYTE *memmap_chunk_pnt(BYTE nidx, DBWORD address) {
	_memmap_region *region = memmap_get_region(nidx, address);

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

BYTE memmap_prg_region_init(BYTE nidx, size_t chunk_size) {
	return (memmap_region_init(&nes[nidx].m.memmap.prg, MEMMAP_PRG_SIZE, chunk_size));
}
BYTE memmap_chr_region_init(BYTE nidx, size_t chunk_size) {
	return (memmap_region_init(&nes[nidx].m.memmap.chr, MEMMAP_CHR_SIZE, chunk_size));
}
BYTE memmap_wram_region_init(BYTE nidx, size_t chunk_size) {
	return (memmap_region_init(&nes[nidx].m.memmap.wram, MEMMAP_WRAM_SIZE, chunk_size));
}
BYTE memmap_ram_region_init(BYTE nidx, size_t chunk_size) {
	return (memmap_region_init(&nes[nidx].m.memmap.ram, MEMMAP_RAM_SIZE, chunk_size));
}
BYTE memmap_nmt_region_init(BYTE nidx, size_t chunk_size) {
	return (memmap_region_init(&nes[nidx].m.memmap.nmt, MEMMAP_NMT_SIZE, chunk_size));
}

// with permissions
INLINE static void memmap_auto_wp(BYTE nidx, DBWORD adr, DBWORD value, size_t size, BYTE rd, BYTE wr, BYTE tvalue) {
	WORD in_region_adr = adr & 0xFFFF;

	if (adr & PPUMM) {
		// PPU
		if (in_region_adr < 0x2000) {
			if (chrrom_size()) {
				memmap_chrrom(nidx, adr, value, size, rd, wr, tvalue);
			} else {
				memmap_vram(nidx, adr, value, size, rd, wr, tvalue);
			}
		} else if (in_region_adr < 0x3F00) {
			memmap_nmt(nidx, adr, value, size, rd, wr, tvalue);
		}
	} else if (adr & CPUMM) {
		// CPU
		if (in_region_adr >= 0x8000) {
			memmap_prgrom(nidx, adr, value, size, rd, wr, tvalue);
		} else if (in_region_adr >= 0x4000) {
			memmap_wram(nidx, adr, value, size, rd, wr, tvalue);
		} else if (in_region_adr >= 0x2000) {
		} else {
			memmap_ram(nidx, adr, value, size, rd, wr, tvalue);
		}
	}
}
void memmap_auto_wp_128b(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_auto_wp(nidx, address, value, S128B, rd, wr, TRUE);
}
void memmap_auto_wp_256b(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_auto_wp(nidx, address, value, S256B, rd, wr, TRUE);
}
void memmap_auto_wp_512b(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_auto_wp(nidx, address, value, S512B, rd, wr, TRUE);
}
void memmap_auto_wp_1k(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_auto_wp(nidx, address, value, S1K, rd, wr, TRUE);
}
void memmap_auto_wp_2k(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_auto_wp(nidx, address, value, S2K, rd, wr, TRUE);
}
void memmap_auto_wp_4k(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_auto_wp(nidx, address, value, S4K, rd, wr, TRUE);
}
void memmap_auto_wp_8k(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_auto_wp(nidx, address, value, S8K, rd, wr, TRUE);
}
void memmap_auto_wp_16k(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_auto_wp(nidx, address, value, S16K, rd, wr, TRUE);
}
void memmap_auto_wp_32k(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_auto_wp(nidx, address, value, S32K, rd, wr, TRUE);
}
void memmap_auto_wp_custom_size(BYTE nidx, DBWORD address, DBWORD chunk, size_t size, BYTE rd, BYTE wr) {
	memmap_auto_wp(nidx, address, chunk, size, rd, wr, FALSE);
}

// permissions :
// ram    : rd = TRUE, wr = TRUE
// prgrom : rd = TRUE, wr = FALSE
// wram   : rd = TRUE, wr = TRUE
// chrrom : rd = TRUE, wr = FALSE
// vram   : rd = TRUE, wr = TRUE
// nmt    : rd = TRUE, wr = TRUE
INLINE static void memmap_auto(BYTE nidx, DBWORD adr, DBWORD value, size_t size, BYTE tvalue) {
	WORD in_region_adr = adr & 0xFFFF;

	if (adr & PPUMM) {
		// PPU
		if (in_region_adr < 0x2000) {
			if (chrrom_size()) {
				memmap_chrrom(nidx, adr, value, size, TRUE, FALSE, tvalue);
			} else {
				memmap_vram(nidx, adr, value, size, TRUE, TRUE, tvalue);
			}
		} else if (in_region_adr < 0x3F00) {
			memmap_nmt(nidx, adr, value, size, TRUE, TRUE, tvalue);
		}
	} else if (adr & CPUMM) {
		// CPU
		if (in_region_adr >= 0x8000) {
			memmap_prgrom(nidx, adr, value, size, TRUE, FALSE, tvalue);
		} else if (in_region_adr >= 0x4000) {
			memmap_wram(nidx, adr, value, size, TRUE, TRUE, tvalue);
		} else if (in_region_adr >= 0x2000) {
		} else {
			memmap_ram(nidx, adr, value, size, TRUE, TRUE, tvalue);
		}
	}
}
void memmap_auto_128b(BYTE nidx, DBWORD address, DBWORD value) {
	memmap_auto(nidx, address, value, S128B, TRUE);
}
void memmap_auto_256b(BYTE nidx, DBWORD address, DBWORD value) {
	memmap_auto(nidx, address, value, S256B, TRUE);
}
void memmap_auto_512b(BYTE nidx, DBWORD address, DBWORD value) {
	memmap_auto(nidx, address, value, S512B, TRUE);
}
void memmap_auto_1k(BYTE nidx, DBWORD address, DBWORD value) {
	memmap_auto(nidx, address, value, S1K, TRUE);
}
void memmap_auto_2k(BYTE nidx, DBWORD address, DBWORD value) {
	memmap_auto(nidx, address, value, S2K, TRUE);
}
void memmap_auto_4k(BYTE nidx, DBWORD address, DBWORD value) {
	memmap_auto(nidx, address, value, S4K, TRUE);
}
void memmap_auto_8k(BYTE nidx, DBWORD address, DBWORD value) {
	memmap_auto(nidx, address, value, S8K, TRUE);
}
void memmap_auto_16k(BYTE nidx, DBWORD address, DBWORD value) {
	memmap_auto(nidx, address, value, S16K, TRUE);
}
void memmap_auto_32k(BYTE nidx, DBWORD address, DBWORD value) {
	memmap_auto(nidx, address, value, S32K, TRUE);
}
void memmap_auto_custom_size(BYTE nidx, DBWORD address, DBWORD chunk, size_t size) {
	memmap_auto(nidx, address, chunk, size, FALSE);
}

// permissions : rd = FALSE, wr = FALSE
INLINE static void memmap_disable(BYTE nidx, DBWORD adr, size_t size, BYTE tvalue) {
	_memmap_bank mbank;

	mbank.region.address = adr;
	mbank.region.value = 0;
	mbank.region.slot = 0;
	mbank.region.bank = 0;
	mbank.dst.pnt = NULL;
	mbank.dst.size = 0;
	mbank.dst.mask = 0;
	mbank.type = MEMMAP_BANK_NONE;
	mbank.rd = FALSE;
	mbank.wr = FALSE;
	mbank.size = size;
	mbank.translate_value = tvalue;
	mbank.nidx = nidx;
	memmap_wp_set_chunks(&mbank);
}
void memmap_disable_128b(BYTE nidx, DBWORD address) {
	memmap_disable(nidx, address, S128B, TRUE);
}
void memmap_disable_256b(BYTE nidx, DBWORD address) {
	memmap_disable(nidx, address, S256B, TRUE);
}
void memmap_disable_512b(BYTE nidx, DBWORD address) {
	memmap_disable(nidx, address, S512B, TRUE);
}
void memmap_disable_1k(BYTE nidx, DBWORD address) {
	memmap_disable(nidx, address, S1K, TRUE);
}
void memmap_disable_2k(BYTE nidx, DBWORD address) {
	memmap_disable(nidx, address, S2K, TRUE);
}
void memmap_disable_4k(BYTE nidx, DBWORD address) {
	memmap_disable(nidx, address, S4K, TRUE);
}
void memmap_disable_8k(BYTE nidx, DBWORD address) {
	memmap_disable(nidx, address, S8K, TRUE);
}
void memmap_disable_16k(BYTE nidx, DBWORD address) {
	memmap_disable(nidx, address, S16K, TRUE);
}
void memmap_disable_32k(BYTE nidx, DBWORD address) {
	memmap_disable(nidx, address, S32K, TRUE);
}
void memmap_disable_custom_size(BYTE nidx, DBWORD address, size_t size) {
	memmap_disable(nidx, address, size, FALSE);
}

// with permissions
INLINE static void memmap_other(BYTE nidx, DBWORD adr, DBWORD value, size_t bsize, BYTE *dst, size_t dsize, BYTE rd, BYTE wr, BYTE tvalue) {
	_memmap_bank mbank;

	mbank.region.address = adr;
	mbank.region.value = value;
	mbank.region.slot = 0;
	mbank.region.bank = 0;
	mbank.dst.pnt = dst;
	mbank.dst.size = dsize;
	mbank.dst.mask = calc_mask(dsize);
	mbank.type = MEMMAP_BANK_OTHER;
	mbank.rd = rd;
	mbank.wr = wr;
	mbank.size = bsize;
	mbank.translate_value = tvalue;
	mbank.nidx = nidx;
	memmap_wp_set_chunks(&mbank);
}
void memmap_other_128b(BYTE nidx, DBWORD address, DBWORD value, BYTE *dst, size_t dst_size, BYTE rd, BYTE wr) {
	memmap_other(nidx, address, value, S128B, dst, dst_size, rd, wr, TRUE);
}
void memmap_other_256b(BYTE nidx, DBWORD address, DBWORD value, BYTE *dst, size_t dst_size, BYTE rd, BYTE wr) {
	memmap_other(nidx, address, value, S256B, dst, dst_size, rd, wr, TRUE);
}
void memmap_other_512b(BYTE nidx, DBWORD address, DBWORD value, BYTE *dst, size_t dst_size, BYTE rd, BYTE wr) {
	memmap_other(nidx, address, value, S512B, dst, dst_size, rd, wr, TRUE);
}
void memmap_other_1k(BYTE nidx, DBWORD address, DBWORD value, BYTE *dst, size_t dst_size, BYTE rd, BYTE wr) {
	memmap_other(nidx, address, value, S1K, dst, dst_size, rd, wr, TRUE);
}
void memmap_other_2k(BYTE nidx, DBWORD address, DBWORD value, BYTE *dst, size_t dst_size, BYTE rd, BYTE wr) {
	memmap_other(nidx, address, value, S2K, dst, dst_size, rd, wr, TRUE);
}
void memmap_other_4k(BYTE nidx, DBWORD address, DBWORD value, BYTE *dst, size_t dst_size, BYTE rd, BYTE wr) {
	memmap_other(nidx, address, value, S4K, dst, dst_size, rd, wr, TRUE);
}
void memmap_other_8k(BYTE nidx, DBWORD address, DBWORD value, BYTE *dst, size_t dst_size, BYTE rd, BYTE wr) {
	memmap_other(nidx, address, value, S8K, dst, dst_size, rd, wr, TRUE);
}
void memmap_other_16k(BYTE nidx, DBWORD address, DBWORD value, BYTE *dst, size_t dst_size, BYTE rd, BYTE wr) {
	memmap_other(nidx, address, value, S16K, dst, dst_size, rd, wr, TRUE);
}
void memmap_other_32k(BYTE nidx, DBWORD address, DBWORD value, BYTE *dst, size_t dst_size, BYTE rd, BYTE wr) {
	memmap_other(nidx, address, value, S32K, dst, dst_size, rd, wr, TRUE);
}
void memmap_other_custom_size(BYTE nidx, DBWORD address, DBWORD chunk, size_t size, BYTE *dst, size_t dsize, BYTE rd, BYTE wr) {
	memmap_other(nidx, address, chunk, size, dst, dsize, rd, wr, FALSE);
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

INLINE static void memmap_wp_chunk(_memmap_bank *mbank, _memmap_region *region) {
	region->chunks[mbank->region.slot].type = !mbank->dst.pnt ? MEMMAP_BANK_NONE : mbank->type;
	// per le mapper che permettono il controllo sulla WRAM (tipo MMC3), posso scrivere (o leggere)
	// nella regione interessata anche se non c'è installata nessuan WRAM solo per valorizzare i
	// registri interni della mapper.
	region->chunks[mbank->region.slot].writable = mbank->wr;
	region->chunks[mbank->region.slot].readable = mbank->rd;
	region->chunks[mbank->region.slot].permit.wr = mbank->dst.pnt && mbank->wr;
	region->chunks[mbank->region.slot].permit.rd = mbank->dst.pnt && mbank->rd;
	region->chunks[mbank->region.slot].pnt = !mbank->dst.pnt
		? NULL
		: &mbank->dst.pnt[(mbank->region.bank << region->info.shift) & mbank->dst.mask];
	region->chunks[mbank->region.slot].mem_region.start = !mbank->dst.pnt
		? NULL
		: mbank->dst.pnt;
	region->chunks[mbank->region.slot].mem_region.end = !mbank->dst.pnt
		? NULL
		: &mbank->dst.pnt[mbank->dst.size];
	region->chunks[mbank->region.slot].mask = !mbank->dst.pnt
		? 0
		: mask_address_with_size(region->info.chunk.size - 1, mbank->dst.size);
	region->chunks[mbank->region.slot].actual_bank = !mbank->dst.pnt
		? 0
		: memmap_control_bank(mbank->size, mbank->dst.size, mbank->region.value);
}
INLINE static void memmap_wp_set_chunks(_memmap_bank *mbank) {
	_memmap_region *region = memmap_get_region(mbank->nidx, mbank->region.address);

	mbank->region.address &= 0xFFFF;

	if (region) {
		unsigned int slot = slot_from_address(&region->info, mbank->region.address);
		size_t chunks = mbank->size / region->info.chunk.size;
		size_t bank = mbank->translate_value ? mbank->region.value * chunks : mbank->region.value;

		for (size_t i = 0; i < chunks; i++) {
			mbank->region.slot = slot + i;
			mbank->region.bank = bank + i;
			if (mbank->region.slot < region->info.chunk.items) {
				memmap_wp_chunk(mbank, region);
			}
		}
	}
}
INLINE static _memmap_region *memmap_get_region(BYTE nidx, DBWORD address) {
	WORD real_address = address & 0xFFFF;

	if (address & PPUMM) {
		// PPU
		if (real_address < 0x2000) {
			return (&nes[nidx].m.memmap.chr);
		} else if (real_address < 0x3F00) {
			return (&nes[nidx].m.memmap.nmt);
		}
	} else if (address & CPUMM) {
		// CPU
		if (real_address >= 0x8000) {
			return (&nes[nidx].m.memmap.prg);
		} else if (real_address >= 0x4000) {
			return (&nes[nidx].m.memmap.wram);
		} else if (real_address >= 0x2000) {
		} else {
			return (&nes[nidx].m.memmap.ram);
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
	for (int nesidx = 0; nesidx < NES_CHIPS_MAX; nesidx++) {
		memmap_disable_32k(nesidx, MMCPU(0x8000));
	}
}
void prgrom_set_size(size_t size) {
	set_size(&prgrom_size(), &prgrom.real_size, nes[0].m.memmap.prg.info.chunk.size, size);
}
void prgrom_reset_chunks(void) {
	for (int nesidx = 0; nesidx < info.number_of_nes; nesidx++) {
		memmap_auto_16k(nesidx, MMCPU(0x8000), 0);
		memmap_auto_16k(nesidx, MMCPU(0xC000), ~0);
	}
}
WORD prgrom_banks(enum _sizes_types size) {
	return (memmap_banks(size, prgrom_size()));
}
WORD prgrom_control_bank(enum _sizes_types size, WORD bank) {
	return (memmap_control_bank(size, prgrom_size(), bank));
}
size_t prgrom_region_address(BYTE nidx, WORD address) {
	return (memmap_region_address(&nes[nidx].m.memmap.prg, address));
}

BYTE prgrom_rd(BYTE nidx, WORD address) {
	const unsigned int slot = slot_from_address(&nes[nidx].m.memmap.prg.info, address);

	return (nes[nidx].m.memmap.prg.chunks[slot].permit.rd
		? nes[nidx].m.memmap.prg.chunks[slot].pnt[address & nes[nidx].m.memmap.prg.chunks[slot].mask]
		: address >> 8);
}
void prgrom_wr(BYTE nidx, WORD address, BYTE value) {
	const unsigned int slot = slot_from_address(&nes[nidx].m.memmap.prg.info, address);

	if (nes[nidx].m.memmap.prg.chunks[slot].permit.wr) {
		nes[nidx].m.memmap.prg.chunks[slot].pnt[address & nes[nidx].m.memmap.prg.chunks[slot].mask] = value;
	}
}

// permissions : rd = TRUE, wr = FALSE
INLINE static void memmap_prgrom(BYTE nidx, DBWORD adr, DBWORD value, size_t size, BYTE rd, BYTE wr, BYTE tvalue) {
	if (adr & CPUMM) {
		_memmap_bank mbank;

		mbank.region.address = adr;
		mbank.region.value = value;
		mbank.region.slot = 0;
		mbank.region.bank = 0;
		mbank.dst.pnt = prgrom_pnt();
		mbank.dst.size = prgrom_size();
		mbank.dst.mask = prgrom_mask();
		mbank.type = MEMMAP_BANK_PRGROM;
		mbank.rd = rd;
		mbank.wr = wr;
		mbank.size = size;
		mbank.translate_value = tvalue;
		mbank.nidx = nidx;
		memmap_wp_set_chunks(&mbank);
	}
}
void memmap_prgrom_128b(BYTE nidx, DBWORD address, DBWORD value) {
	memmap_prgrom(nidx, address, value, S128B, TRUE, FALSE, TRUE);
}
void memmap_prgrom_256b(BYTE nidx, DBWORD address, DBWORD value) {
	memmap_prgrom(nidx, address, value, S256B, TRUE, FALSE, TRUE);
}
void memmap_prgrom_512b(BYTE nidx, DBWORD address, DBWORD value) {
	memmap_prgrom(nidx, address, value, S512B, TRUE, FALSE, TRUE);
}
void memmap_prgrom_1k(BYTE nidx, DBWORD address, DBWORD value) {
	memmap_prgrom(nidx, address, value, S1K, TRUE, FALSE, TRUE);
}
void memmap_prgrom_2k(BYTE nidx, DBWORD address, DBWORD value) {
	memmap_prgrom(nidx, address, value, S2K, TRUE, FALSE, TRUE);
}
void memmap_prgrom_4k(BYTE nidx, DBWORD address, DBWORD value) {
	memmap_prgrom(nidx, address, value, S4K, TRUE, FALSE, TRUE);
}
void memmap_prgrom_8k(BYTE nidx, DBWORD address, DBWORD value) {
	memmap_prgrom(nidx, address, value, S8K, TRUE, FALSE, TRUE);
}
void memmap_prgrom_16k(BYTE nidx, DBWORD address, DBWORD value) {
	memmap_prgrom(nidx, address, value, S16K, TRUE, FALSE, TRUE);
}
void memmap_prgrom_32k(BYTE nidx, DBWORD address, DBWORD value) {
	memmap_prgrom(nidx, address, value, S32K, TRUE, FALSE, TRUE);
}
void memmap_prgrom_custom_size(BYTE nidx, DBWORD address, DBWORD chunk, size_t size) {
	memmap_prgrom(nidx, address, chunk, size, TRUE, FALSE, FALSE);
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
	set_size(&chrrom_size(), &chrrom.real_size, nes[0].m.memmap.chr.info.chunk.size, size);
}
void chrrom_reset_chunks(void) {
	for (int nesidx = 0; nesidx < info.number_of_nes; nesidx++) {
		memmap_auto_8k(nesidx, MMPPU(0x0000), 0);
	}
}
WORD chrrom_banks(enum _sizes_types size) {
	return (memmap_banks(size, chrrom_size()));
}
WORD chrrom_control_bank(enum _sizes_types size, WORD bank) {
	return (memmap_control_bank(size, chrrom_size(), bank));
}

BYTE chr_rd(BYTE nidx, WORD address) {
	const unsigned int slot = slot_from_address(&nes[nidx].m.memmap.chr.info, address);

	if (nes[nidx].m.memmap.chr.chunks[slot].permit.rd) {
		return (nes[nidx].m.memmap.chr.chunks[slot].pnt[address & nes[nidx].m.memmap.chr.chunks[slot].mask]);
	}
	return (nes[nidx].c.cpu.openbus);
}
void chr_wr(BYTE nidx, WORD address, BYTE value) {
	const unsigned int slot = slot_from_address(&nes[nidx].m.memmap.chr.info, address);

	if (nes[nidx].m.memmap.chr.chunks[slot].permit.wr) {
		nes[nidx].m.memmap.chr.chunks[slot].pnt[address & nes[nidx].m.memmap.chr.chunks[slot].mask] = value;
	}
}
void chr_disable_write(BYTE nidx) {
	for (size_t i = 0; i < nes[nidx].m.memmap.chr.info.chunk.items; i++) {
		nes[nidx].m.memmap.chr.chunks[i].writable = FALSE;
		nes[nidx].m.memmap.chr.chunks[i].permit.wr = FALSE;
	}
}

// permissions :
// chrrom : rd = TRUE, wr = FALSE
INLINE static void memmap_chrrom(BYTE nidx, DBWORD adr, DBWORD value, size_t size, BYTE rd, BYTE wr, BYTE tvalue) {
	if (adr & PPUMM) {
		_memmap_bank mbank;

		mbank.region.address = adr;
		mbank.region.value = value;
		mbank.region.slot = 0;
		mbank.region.bank = 0;
		mbank.dst.pnt = chrrom_pnt();
		mbank.dst.size = chrrom_size();
		mbank.dst.mask = chrrom_mask();
		mbank.type = MEMMAP_BANK_CHRROM;
		mbank.rd = rd;
		mbank.wr = wr;
		mbank.size = size;
		mbank.translate_value = tvalue;
		mbank.nidx = nidx;
		memmap_wp_set_chunks(&mbank);
	}
}
void memmap_chrrom_128b(BYTE nidx, DBWORD address, DBWORD value) {
	memmap_chrrom(nidx, address, value, S128B, TRUE, FALSE, TRUE);
}
void memmap_chrrom_256b(BYTE nidx, DBWORD address, DBWORD value) {
	memmap_chrrom(nidx, address, value, S256B, TRUE, FALSE, TRUE);
}
void memmap_chrrom_512b(BYTE nidx, DBWORD address, DBWORD value) {
	memmap_chrrom(nidx, address, value, S512B, TRUE, FALSE, TRUE);
}
void memmap_chrrom_1k(BYTE nidx, DBWORD address, DBWORD value) {
	memmap_chrrom(nidx, address, value, S1K, TRUE, FALSE, TRUE);
}
void memmap_chrrom_2k(BYTE nidx, DBWORD address, DBWORD value) {
	memmap_chrrom(nidx, address, value, S2K, TRUE, FALSE, TRUE);
}
void memmap_chrrom_4k(BYTE nidx, DBWORD address, DBWORD value) {
	memmap_chrrom(nidx, address, value, S4K, TRUE, FALSE, TRUE);
}
void memmap_chrrom_8k(BYTE nidx, DBWORD address, DBWORD value) {
	memmap_chrrom(nidx, address, value, S8K, TRUE, FALSE, TRUE);
}
void memmap_chrrom_16k(BYTE nidx, DBWORD address, DBWORD value) {
	memmap_chrrom(nidx, address, value, S16K, TRUE, FALSE, TRUE);
}
void memmap_chrrom_32k(BYTE nidx, DBWORD address, DBWORD value) {
	memmap_chrrom(nidx, address, value, S32K, TRUE, FALSE, TRUE);
}
void memmap_chrrom_custom_size(BYTE nidx, DBWORD address, DBWORD chunk, size_t size) {
	memmap_chrrom(nidx, address, chunk, size, TRUE, FALSE, FALSE);
}

INLINE static void memmap_chrrom_nmt(BYTE nidx, DBWORD adr, DBWORD value, size_t size, BYTE tvalue) {
	if (adr & PPUMM) {
		memmap_other(nidx, adr, value, size, nmt_pnt(nidx), nmt_size(nidx), TRUE, TRUE, tvalue);
	}
}
void memmap_chrrom_nmt_128b(BYTE nidx, DBWORD address, DBWORD value) {
	memmap_chrrom_nmt(nidx, address, value, S128B, TRUE);
}
void memmap_chrrom_nmt_256b(BYTE nidx, DBWORD address, DBWORD value) {
	memmap_chrrom_nmt(nidx, address, value, S256B, TRUE);
}
void memmap_chrrom_nmt_512b(BYTE nidx, DBWORD address, DBWORD value) {
	memmap_chrrom_nmt(nidx, address, value, S512B, TRUE);
}
void memmap_chrrom_nmt_1k(BYTE nidx, DBWORD address, DBWORD value) {
	memmap_chrrom_nmt(nidx, address, value, S1K, TRUE);
}
void memmap_chrrom_nmt_2k(BYTE nidx, DBWORD address, DBWORD value) {
	memmap_chrrom_nmt(nidx, address, value, S2K, TRUE);
}
void memmap_chrrom_nmt_4k(BYTE nidx, DBWORD address, DBWORD value) {
	memmap_chrrom_nmt(nidx, address, value, S4K, TRUE);
}
void memmap_chrrom_nmt_8k(BYTE nidx, DBWORD address, DBWORD value) {
	memmap_chrrom_nmt(nidx, address, value, S8K, TRUE);
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
			wram.ram.pnt = wram_nvram_size()
				? wram_ram_size() ? wram_pnt_byte(wram_nvram_size()) : NULL
				: wram_pnt_byte(0);
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
	for (int nesidx = 0; nesidx < NES_CHIPS_MAX; nesidx++) {
		memmap_disable_16k(nesidx, MMCPU(0x4000));
	}
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
	for (int nesidx = 0; nesidx < info.number_of_nes; nesidx++) {
		// disabilito la wram da 0x4000 a 0x5FFF
		memmap_disable_8k(nesidx, MMCPU(0x4000));
		// setto i primi 8k su 0x6000
		memmap_auto_8k(nesidx, MMCPU(0x6000), 0);
	}
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

BYTE wram_rd(BYTE nidx, WORD address) {
	const unsigned int slot = slot_from_address(&nes[nidx].m.memmap.wram.info, address);

	if (vs_system.enabled) {
		if (address < 0x6000) {
			if (address >= 0x4020) {
				if ((address & 0x4020) == 0x4020) {
					vs_system_r4020_clock(rd, (address >> 8))
				}
				if (vs_system.special_mode.r5e0x) {
					if (address == 0x5E00) {
						vs_system.special_mode.index = 0;
					} else if (address == 0x5E01) {
						return (vs_system.special_mode.r5e0x[(vs_system.special_mode.index++) & 0x1F]);
					}
				} else if (vs_system.special_mode.type == VS_SM_Super_Xevious) {
					if (address == 0x54FF) {
						return (0x05);
					} else if (address == 0x5678) {
						return (vs_system.special_mode.index ? 0x00 : 0x01);
					} else if (address == 0x578F) {
						return (vs_system.special_mode.index ? 0xD1 : 0x89);
					} else if (address == 0x5567) {
						vs_system.special_mode.index ^= 1;
						return (vs_system.special_mode.index ? 0x37 : 0x3E);
					}
				}
			}
		} else if (vs_system.shared_mem != nidx) {
			return (address >> 8);
		}
	}
	return (nes[nidx].m.memmap.wram.chunks[slot].permit.rd)
		? nes[nidx].m.memmap.wram.chunks[slot].pnt[address & nes[nidx].m.memmap.wram.chunks[slot].mask]
		: (address >> 8);
}
void wram_wr(BYTE nidx, WORD address, BYTE value) {
	const unsigned int slot = slot_from_address(&nes[nidx].m.memmap.wram.info, address);

	if (vs_system.enabled) {
		if (address < 0x6000) {
			if ((address >= 0x4020) && ((address & 0x4020) == 0x4020)) {
				vs_system_r4020_clock(wr, value)
			}
		} else if (vs_system.shared_mem != nidx) {
			printf("wram_wr NO : %d %d - 0x%04X - %d %d %d - %d %d %d\n",
				   nidx,
				   vs_system.shared_mem,

				   address,

				   nes[0].p.ppu.frames,
				   nes[0].p.ppu.frame_y,
				   nes[0].p.ppu.frame_x,

				   nes[1].p.ppu.frames,
				   nes[1].p.ppu.frame_y,
				   nes[1].p.ppu.frame_x
			);
			return;
		}
	}
	if (nes[nidx].m.memmap.wram.chunks[slot].permit.wr) {
		nes[nidx].m.memmap.wram.chunks[slot].pnt[address & nes[nidx].m.memmap.wram.chunks[slot].mask] = value;
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
INLINE static void memmap_wram(BYTE nidx, DBWORD adr, DBWORD value, size_t size, BYTE rd, BYTE wr, BYTE tvalue) {
	if (adr & CPUMM) {
		_memmap_bank mbank;

		mbank.region.address = adr;
		mbank.region.value = value;
		mbank.region.slot = 0;
		mbank.region.bank = 0;
		mbank.dst.pnt = wram_pnt();
		mbank.dst.size = wram_size();
		mbank.dst.mask = wram_mask();
		mbank.type = MEMMAP_BANK_WRAM;
		mbank.rd = rd;
		mbank.wr = wr;
		mbank.size = size;
		mbank.translate_value = tvalue;
		mbank.nidx = nidx;
		memmap_wp_set_chunks(&mbank);
	}
}
void memmap_wram_128b(BYTE nidx, DBWORD address, DBWORD value) {
	memmap_wram(nidx, address, value, S128B, TRUE, TRUE, TRUE);
}
void memmap_wram_256b(BYTE nidx, DBWORD address, DBWORD value) {
	memmap_wram(nidx, address, value, S256B, TRUE, TRUE, TRUE);
}
void memmap_wram_512b(BYTE nidx, DBWORD address, DBWORD value) {
	memmap_wram(nidx, address, value, S512B, TRUE, TRUE, TRUE);
}
void memmap_wram_1k(BYTE nidx, DBWORD address, DBWORD value) {
	memmap_wram(nidx, address, value, S1K, TRUE, TRUE, TRUE);
}
void memmap_wram_2k(BYTE nidx, DBWORD address, DBWORD value) {
	memmap_wram(nidx, address, value, S2K, TRUE, TRUE, TRUE);
}
void memmap_wram_4k(BYTE nidx, DBWORD address, DBWORD value) {
	memmap_wram(nidx, address, value, S4K, TRUE, TRUE, TRUE);
}
void memmap_wram_8k(BYTE nidx, DBWORD address, DBWORD value) {
	memmap_wram(nidx, address, value, S8K, TRUE, TRUE, TRUE);
}
void memmap_wram_16k(BYTE nidx, DBWORD address, DBWORD value) {
	memmap_wram(nidx, address, value, S16K, TRUE, TRUE, TRUE);
}
void memmap_wram_32k(BYTE nidx, DBWORD address, DBWORD value) {
	memmap_wram(nidx, address, value, S32K, TRUE, TRUE, TRUE);
}
void memmap_wram_custom_size(BYTE nidx, DBWORD address, DBWORD chunk, size_t size) {
	memmap_wram(nidx, address, chunk, size, TRUE, TRUE, FALSE);
}

// with permissions
INLINE static void memmap_wram_wp(BYTE nidx, DBWORD adr, DBWORD value, size_t size, BYTE rd, BYTE wr, BYTE tvalue) {
	memmap_wram(nidx, adr, value, size, rd, wr, tvalue);
}
void memmap_wram_wp_128b(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_wram_wp(nidx, address, value, S128B, rd, wr, TRUE);
}
void memmap_wram_wp_256b(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_wram_wp(nidx, address, value, S256B, rd, wr, TRUE);
}
void memmap_wram_wp_512b(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_wram_wp(nidx, address, value, S512B, rd, wr, TRUE);
}
void memmap_wram_wp_1k(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_wram_wp(nidx, address, value, S1K, rd, wr, TRUE);
}
void memmap_wram_wp_2k(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_wram_wp(nidx, address, value, S2K, rd, wr, TRUE);
}
void memmap_wram_wp_4k(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_wram_wp(nidx, address, value, S4K, rd, wr, TRUE);
}
void memmap_wram_wp_8k(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_wram_wp(nidx, address, value, S8K, rd, wr, TRUE);
}
void memmap_wram_wp_16k(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_wram_wp(nidx, address, value, S16K, rd, wr, TRUE);
}
void memmap_wram_wp_32k(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_wram_wp(nidx, address, value, S32K, rd, wr, TRUE);
}
void memmap_wram_wp_custom_size(BYTE nidx, DBWORD address, DBWORD chunk, size_t size, BYTE rd, BYTE wr) {
	memmap_wram_wp(nidx, address, chunk, size, rd, wr, FALSE);
}

// with permissions
INLINE static void memmap_wram_ram_wp(BYTE nidx, DBWORD adr, DBWORD value, size_t size, BYTE rd, BYTE wr, BYTE tvalue) {
	if (adr & CPUMM) {
		_memmap_bank mbank;

		mbank.region.address = adr;
		mbank.region.value = value;
		mbank.region.slot = 0;
		mbank.region.bank = 0;
		mbank.dst.pnt = wram_ram_pnt();
		mbank.dst.size = wram_ram_size();
		mbank.dst.mask = wram_ram_mask();
		mbank.type = MEMMAP_BANK_WRAM;
		mbank.rd = rd;
		mbank.wr = wr;
		mbank.size = size;
		mbank.translate_value = tvalue;
		mbank.nidx = nidx;
		memmap_wp_set_chunks(&mbank);
	}
}
void memmap_wram_ram_wp_128b(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_wram_ram_wp(nidx, address, value, S128B, rd, wr, TRUE);
}
void memmap_wram_ram_wp_256b(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_wram_ram_wp(nidx, address, value, S256B, rd, wr, TRUE);
}
void memmap_wram_ram_wp_512b(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_wram_ram_wp(nidx, address, value, S512B, rd, wr, TRUE);
}
void memmap_wram_ram_wp_1k(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_wram_ram_wp(nidx, address, value, S1K, rd, wr, TRUE);
}
void memmap_wram_ram_wp_2k(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_wram_ram_wp(nidx, address, value, S2K, rd, wr, TRUE);
}
void memmap_wram_ram_wp_4k(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_wram_ram_wp(nidx, address, value, S4K, rd, wr, TRUE);
}
void memmap_wram_ram_wp_8k(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_wram_ram_wp(nidx, address, value, S8K, rd, wr, TRUE);
}
void memmap_wram_ram_wp_16k(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_wram_ram_wp(nidx, address, value, S16K, rd, wr, TRUE);
}
void memmap_wram_ram_wp_32k(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_wram_ram_wp(nidx, address, value, S32K, rd, wr, TRUE);
}
void memmap_wram_ram_wp_custom_size(BYTE nidx, DBWORD address, DBWORD chunk, size_t size, BYTE rd, BYTE wr) {
	memmap_wram_ram_wp(nidx, address, chunk, size, rd, wr, FALSE);
}

// with permissions
INLINE static void memmap_wram_nvram_wp(BYTE nidx, DBWORD adr, DBWORD value, size_t size, BYTE rd, BYTE wr, BYTE tvalue) {
	if (adr & CPUMM) {
		_memmap_bank mbank;

		mbank.region.address = adr;
		mbank.region.value = value;
		mbank.region.slot = 0;
		mbank.region.bank = 0;
		mbank.dst.pnt = wram_nvram_pnt();
		mbank.dst.size = wram_nvram_size();
		mbank.dst.mask = wram_nvram_mask();
		mbank.type = MEMMAP_BANK_WRAM;
		mbank.rd = rd;
		mbank.wr = wr;
		mbank.size = size;
		mbank.translate_value = tvalue;
		mbank.nidx = nidx;
		memmap_wp_set_chunks(&mbank);
	}
}
void memmap_wram_nvram_wp_128b(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_wram_nvram_wp(nidx, address, value, S128B, rd, wr, TRUE);
}
void memmap_wram_nvram_wp_256b(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_wram_nvram_wp(nidx, address, value, S256B, rd, wr, TRUE);
}
void memmap_wram_nvram_wp_512b(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_wram_nvram_wp(nidx, address, value, S512B, rd, wr, TRUE);
}
void memmap_wram_nvram_wp_1k(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_wram_nvram_wp(nidx, address, value, S1K, rd, wr, TRUE);
}
void memmap_wram_nvram_wp_2k(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_wram_nvram_wp(nidx, address, value, S2K, rd, wr, TRUE);
}
void memmap_wram_nvram_wp_4k(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_wram_nvram_wp(nidx, address, value, S4K, rd, wr, TRUE);
}
void memmap_wram_nvram_wp_8k(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_wram_nvram_wp(nidx, address, value, S8K, rd, wr, TRUE);
}
void memmap_wram_nvram_wp_16k(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_wram_nvram_wp(nidx, address, value, S16K, rd, wr, TRUE);
}
void memmap_wram_nvram_wp_32k(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_wram_nvram_wp(nidx, address, value, S32K, rd, wr, TRUE);
}
void memmap_wram_nvram_wp_custom_size(BYTE nidx, DBWORD address, DBWORD chunk, size_t size, BYTE rd, BYTE wr) {
	memmap_wram_nvram_wp(nidx, address, chunk, size, rd, wr, FALSE);
}

BYTE wram_malloc(void) {
	if (memmap_malloc(&wram_pnt(), wram.real_size) == EXIT_ERROR) {
		log_error(uL("wram malloc;out of memory"));
		return (EXIT_ERROR);
	}
	return (EXIT_OK);
}

// nvram -----------------------------------------------------------------------------

BYTE vram_malloc(BYTE nidx);

BYTE vram_init(void) {
	if ((info.reset == CHANGE_ROM) || (info.reset == POWER_UP)) {
		for (int nesidx = 0; nesidx < info.number_of_nes; nesidx++) {
			if (vram_size(nesidx)) {
				// alloco la memoria necessaria
				if (vram_malloc(nesidx) == EXIT_ERROR) {
					return (EXIT_ERROR);
				}
				// inizializzo il pointer della ram
				nes[nesidx].m.vram.ram.pnt = vram_nvram_size(nesidx)
					? vram_ram_size(nesidx) ? vram_pnt_byte(nesidx, vram_nvram_size(nesidx)) : NULL
					: vram_pnt_byte(nesidx, 0);
				// inizializzo il pointer della nvram
				nes[nesidx].m.vram.nvram.pnt = vram_nvram_size(nesidx) ? vram_pnt_byte(nesidx, 0) : NULL;
				// inizializzom le mask
				vram_mask(nesidx) = calc_mask(vram_size(nesidx));
				vram_ram_mask(nesidx) = calc_mask(vram_ram_size(nesidx));
				vram_nvram_mask(nesidx) = calc_mask(vram_nvram_size(nesidx));
			}
		}
	}
	vram_memset();
	return (EXIT_OK);
}
void vram_quit(void) {
	for (int nesidx = 0; nesidx < info.number_of_nes; nesidx++) {
		if (vram_pnt(nesidx)) {
			free(vram_pnt(nesidx));
		}
		memset(&nes[nesidx].m.vram, 0x00, sizeof(nes[nesidx].m.vram));
	}
}
void vram_set_ram_size(BYTE nidx, size_t size) {
	nes[nidx].m.vram.ram.size = pow_of_2(size);
	vram_size(nidx) = vram_ram_size(nidx) + vram_nvram_size(nidx);
	nes[nidx].m.vram.real_size = pow_of_2(vram_size(nidx));
}
void vram_set_nvram_size(BYTE nidx, size_t size) {
	nes[nidx].m.vram.nvram.size = pow_of_2(size);
	vram_size(nidx) = vram_ram_size(nidx) + vram_nvram_size(nidx);
	nes[nidx].m.vram.real_size = pow_of_2(vram_size(nidx));
}
void vram_memset(void) {
	for (int nesidx = 0; nesidx < info.number_of_nes; nesidx++) {
		if (vram_size(nesidx)) {
			BYTE *dst = NULL;
			size_t size = 0;

			if ((info.reset == CHANGE_ROM) || (info.reset == POWER_UP)) {
				dst = vram_pnt(nesidx);
				size = nes[nesidx].m.vram.real_size;
			} else if (info.reset >= HARD) {
				dst = vram_ram_pnt(nesidx);
				size = vram_ram_size(nesidx);
			}
			if (dst) {
				memset(dst, 0x00, size);
				//emu_initial_ram(dst, size);
			}
		}
	}
}

// permissions : rd = TRUE, wr = TRUE
INLINE static void memmap_vram(BYTE nidx, DBWORD adr, DBWORD value, size_t size, BYTE rd, BYTE wr, BYTE tvalue) {
	if (adr & PPUMM) {
		_memmap_bank mbank;

		mbank.region.address = adr;
		mbank.region.value = value;
		mbank.region.slot = 0;
		mbank.region.bank = 0;
		mbank.dst.pnt = vram_pnt(nidx);
		mbank.dst.size = vram_size(nidx);
		mbank.dst.mask = vram_mask(nidx);
		mbank.type = MEMMAP_BANK_VRAM;
		mbank.rd = rd;
		mbank.wr = wr;
		mbank.size = size;
		mbank.translate_value = tvalue;
		mbank.nidx = nidx;
		memmap_wp_set_chunks(&mbank);
	}
}
void memmap_vram_128b(BYTE nidx, DBWORD address, DBWORD value) {
	memmap_vram(nidx, address, value, S128B, TRUE, TRUE, TRUE);
}
void memmap_vram_256b(BYTE nidx, DBWORD address, DBWORD value) {
	memmap_vram(nidx, address, value, S256B, TRUE, TRUE, TRUE);
}
void memmap_vram_512b(BYTE nidx, DBWORD address, DBWORD value) {
	memmap_vram(nidx, address, value, S512B, TRUE, TRUE, TRUE);
}
void memmap_vram_1k(BYTE nidx, DBWORD address, DBWORD value) {
	memmap_vram(nidx, address, value, S1K, TRUE, TRUE, TRUE);
}
void memmap_vram_2k(BYTE nidx, DBWORD address, DBWORD value) {
	memmap_vram(nidx, address, value, S2K, TRUE, TRUE, TRUE);
}
void memmap_vram_4k(BYTE nidx, DBWORD address, DBWORD value) {
	memmap_vram(nidx, address, value, S4K, TRUE, TRUE, TRUE);
}
void memmap_vram_8k(BYTE nidx, DBWORD address, DBWORD value) {
	memmap_vram(nidx, address, value, S8K, TRUE, TRUE, TRUE);
}
void memmap_vram_custom_size(BYTE nidx, DBWORD address, DBWORD chunk, size_t size) {
	memmap_vram(nidx, address, chunk, size, TRUE, TRUE, FALSE);
}

INLINE static void memmap_vram_wp(BYTE nidx, DBWORD adr, DBWORD value, size_t size, BYTE rd, BYTE wr, BYTE tvalue) {
	memmap_vram(nidx, adr, value, size, rd, wr, tvalue);
}
void memmap_vram_wp_128b(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_vram_wp(nidx, address, value, S128B, rd, wr, TRUE);
}
void memmap_vram_wp_256b(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_vram_wp(nidx, address, value, S256B, rd, wr, TRUE);
}
void memmap_vram_wp_512b(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_vram_wp(nidx, address, value, S512B, rd, wr, TRUE);
}
void memmap_vram_wp_1k(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_vram_wp(nidx, address, value, S1K, rd, wr, TRUE);
}
void memmap_vram_wp_2k(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_vram_wp(nidx, address, value, S2K, rd, wr, TRUE);
}
void memmap_vram_wp_4k(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_vram_wp(nidx, address, value, S4K, rd, wr, TRUE);
}
void memmap_vram_wp_8k(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_vram_wp(nidx, address, value, S8K, rd, wr, TRUE);
}
void memmap_vram_wp_custom_size(BYTE nidx, DBWORD address, DBWORD chunk, size_t size, BYTE rd, BYTE wr) {
	memmap_vram_wp(nidx, address, chunk, size, rd, wr, FALSE);
}

BYTE vram_malloc(BYTE nidx) {
	if (memmap_malloc(&vram_pnt(nidx), nes[nidx].m.vram.real_size) == EXIT_ERROR) {
		log_error(uL("vram malloc;out of memory"));
		return (EXIT_ERROR);
	}
	return (EXIT_OK);
}

// ram -------------------------------------------------------------------------------

BYTE ram_malloc(BYTE nidx);

BYTE ram_init(void) {
	if ((info.reset == CHANGE_ROM) || (info.reset == POWER_UP)) {
		for (int nesidx = 0; nesidx < info.number_of_nes; nesidx++) {
			if (ram_size(nesidx)) {
				// alloco la memoria necessaria
				if (ram_malloc(nesidx) == EXIT_ERROR) {
					return (EXIT_ERROR);
				}
				// inizializzom la mask
				ram_mask(nesidx) = calc_mask(ram_size(nesidx));
			}
		}
	}
	ram_memset();
	return (EXIT_OK);
}
void ram_quit(void) {
	for (int nesidx = 0; nesidx < info.number_of_nes; nesidx++) {
		if (ram_pnt(nesidx)) {
			free(ram_pnt(nesidx));
		}
		memset(&nes[nesidx].m.ram, 0x00, sizeof(nes[nesidx].m.ram));
	}
}
void ram_set_size(BYTE nidx, size_t size) {
	set_size(&ram_size(nidx), &nes[nidx].m.ram.real_size, nes[nidx].m.memmap.ram.info.chunk.size, size);
}
void ram_reset_chunks(void) {
	for (int nesidx = 0; nesidx < info.number_of_nes; nesidx++) {
		memmap_auto_2k(nesidx, MMCPU(0x0000), 0);
		memmap_auto_2k(nesidx, MMCPU(0x0800), 0);
		memmap_auto_2k(nesidx, MMCPU(0x1000), 0);
		memmap_auto_2k(nesidx, MMCPU(0x1800), 0);
	}
}
void ram_memset(void) {
	for (int nesidx = 0; nesidx < info.number_of_nes; nesidx++) {
		if (ram_size(nesidx)) {
			//memset(ram_pnt(nesidx), 0x00, nes[nesidx].m.ram.real_size);
			emu_initial_ram(ram_pnt(nesidx), nes[nesidx].m.ram.real_size);
		}
	}
}

BYTE ram_rd(BYTE nidx, WORD address) {
	unsigned int slot = slot_from_address(&nes[nidx].m.memmap.ram.info, address);

	if (nes[nidx].m.memmap.ram.chunks[slot].permit.rd) {
		return (nes[nidx].m.memmap.ram.chunks[slot].pnt[address & nes[nidx].m.memmap.ram.chunks[slot].mask]);
	}
	return (nes[nidx].c.cpu.openbus);
}
void ram_wr(BYTE nidx, WORD address, BYTE value) {
	unsigned int slot = slot_from_address(&nes[nidx].m.memmap.ram.info, address);

	if (nes[nidx].m.memmap.ram.chunks[slot].permit.wr) {
		nes[nidx].m.memmap.ram.chunks[slot].pnt[address & nes[nidx].m.memmap.ram.chunks[slot].mask] = value;
	}
}

// permissions : rd = TRUE, wr = TRUE
INLINE static void memmap_ram(BYTE nidx, DBWORD adr, DBWORD value, size_t size, BYTE rd, BYTE wr, BYTE tvalue) {
	if (adr & CPUMM) {
		_memmap_bank mbank;

		mbank.region.address = adr;
		mbank.region.value = value;
		mbank.region.slot = 0;
		mbank.region.bank = 0;
		mbank.dst.pnt = ram_pnt(nidx);
		mbank.dst.size = ram_size(nidx);
		mbank.dst.mask = ram_mask(nidx);
		mbank.type = MEMMAP_BANK_RAM;
		mbank.rd = rd;
		mbank.wr = wr;
		mbank.size = size;
		mbank.translate_value = tvalue;
		mbank.nidx = nidx;
		memmap_wp_set_chunks(&mbank);
	}
}
void memmap_ram_128b(BYTE nidx, DBWORD address, DBWORD value) {
	memmap_ram(nidx, address, value, S128B, TRUE, TRUE, TRUE);
}
void memmap_ram_256b(BYTE nidx, DBWORD address, DBWORD value) {
	memmap_ram(nidx, address, value, S256B, TRUE, TRUE, TRUE);
}
void memmap_ram_512b(BYTE nidx, DBWORD address, DBWORD value) {
	memmap_ram(nidx, address, value, S512B, TRUE, TRUE, TRUE);
}
void memmap_ram_1k(BYTE nidx, DBWORD address, DBWORD value) {
	memmap_ram(nidx, address, value, S1K, TRUE, TRUE, TRUE);
}
void memmap_ram_2k(BYTE nidx, DBWORD address, DBWORD value) {
	memmap_ram(nidx, address, value, S2K, TRUE, TRUE, TRUE);
}
void memmap_ram_4k(BYTE nidx, DBWORD address, DBWORD value) {
	memmap_ram(nidx, address, value, S4K, TRUE, TRUE, TRUE);
}
void memmap_ram_8k(BYTE nidx, DBWORD address, DBWORD value) {
	memmap_ram(nidx, address, value, S8K, TRUE, TRUE, TRUE);
}

BYTE ram_malloc(BYTE nidx) {
	if (memmap_malloc(&ram_pnt(nidx), nes[nidx].m.ram.real_size) == EXIT_ERROR) {
		log_error(uL("ram malloc;out of memory"));
		return (EXIT_ERROR);
	}
	return (EXIT_OK);
}

// nmt -------------------------------------------------------------------------------

BYTE nmt_malloc(BYTE nidx);

BYTE nmt_init(void) {
	if ((info.reset == CHANGE_ROM) || (info.reset == POWER_UP)) {
		for (int nesidx = 0; nesidx < info.number_of_nes; nesidx++) {
			if (nmt_size(nesidx)) {
				// alloco la memoria necessaria
				if (nmt_malloc(nesidx) == EXIT_ERROR) {
					return (EXIT_ERROR);
				}
				// inizializzom la mask
				nmt_mask(nesidx) = calc_mask(nmt_size(nesidx));
			}
		}
	}
	nmt_memset();
	return (EXIT_OK);
}
void nmt_quit(void) {
	for (int nesidx = 0; nesidx < info.number_of_nes; nesidx++) {
		if (nmt_pnt(nesidx)) {
			free(nmt_pnt(nesidx));
		}
		memset(&nes[nesidx].m.nmt, 0x00, sizeof(nes[nesidx].m.nmt));
	}
}
void nmt_set_size(BYTE nidx, size_t size) {
	set_size(&nmt_size(nidx), &nes[nidx].m.nmt.real_size, nes[nidx].m.memmap.nmt.info.chunk.size, size);
}
void nmt_reset_chunks(void) {
	for (int nesidx = 0; nesidx < info.number_of_nes; nesidx++) {
		switch (info.mapper.mirroring) {
			case MIRRORING_HORIZONTAL:
				mirroring_H(nesidx);
				break;
			default:
			case MIRRORING_VERTICAL:
				mirroring_V(nesidx);
				break;
			case MIRRORING_FOURSCR:
				mirroring_FSCR(nesidx);
				break;
			case MIRRORING_SINGLE_SCR0:
				mirroring_SCR0(nesidx);
				break;
			case MIRRORING_SINGLE_SCR1:
				mirroring_SCR1(nesidx);
				break;
		}
	}
}
void nmt_memset(void) {
	for (int nesidx = 0; nesidx < info.number_of_nes; nesidx++) {
		if (nmt_size(nesidx)) {
			memset(nmt_pnt(nesidx), 0x00, nes[nesidx].m.nmt.real_size);
		}
	}
}

BYTE nmt_rd(BYTE nidx, WORD address) {
	unsigned int slot = slot_from_address(&nes[nidx].m.memmap.nmt.info, address);

	if (nes[nidx].m.memmap.nmt.chunks[slot].permit.rd) {
		return (nes[nidx].m.memmap.nmt.chunks[slot].pnt[address & nes[nidx].m.memmap.nmt.chunks[slot].mask]);
	}
	return (nes[nidx].c.cpu.openbus);
}
void nmt_wr(BYTE nidx, WORD address, BYTE value) {
	unsigned int slot = slot_from_address(&nes[nidx].m.memmap.nmt.info, address);

	if (nes[nidx].m.memmap.nmt.chunks[slot].permit.wr) {
		nes[nidx].m.memmap.nmt.chunks[slot].pnt[address & nes[nidx].m.memmap.nmt.chunks[slot].mask] = value;
	}
}
void nmt_disable_write(BYTE nidx) {
	for (size_t i = 0; i < nes[nidx].m.memmap.nmt.info.chunk.items; i++) {
		nes[nidx].m.memmap.nmt.chunks[i].writable = FALSE;
		nes[nidx].m.memmap.nmt.chunks[i].permit.wr = FALSE;
	}
}

// permissions : rd = TRUE, wr = TRUE
INLINE static void memmap_nmt(BYTE nidx, DBWORD adr, DBWORD value, size_t size, BYTE rd, BYTE wr, BYTE tvalue) {
	if (adr & PPUMM) {
		_memmap_bank mbank;

		mbank.region.address = adr;
		mbank.region.value = value;
		mbank.region.slot = 0;
		mbank.region.bank = 0;
		mbank.dst.pnt = nmt_pnt(nidx);
		mbank.dst.size = nmt_size(nidx);
		mbank.dst.mask = nmt_mask(nidx);
		mbank.type = MEMMAP_BANK_NMT;
		mbank.rd = rd;
		mbank.wr = wr;
		mbank.size = size;
		mbank.translate_value = tvalue;
		mbank.nidx = nidx;
		memmap_wp_set_chunks(&mbank);
	}
}
void memmap_nmt_128b(BYTE nidx, DBWORD address, DBWORD value) {
	memmap_nmt(nidx, address, value, S256B, TRUE, TRUE, TRUE);
}
void memmap_nmt_256b(BYTE nidx, DBWORD address, DBWORD value) {
	memmap_nmt(nidx, address, value, S128B, TRUE, TRUE, TRUE);
}
void memmap_nmt_512b(BYTE nidx, DBWORD address, DBWORD value) {
	memmap_nmt(nidx, address, value, S512B, TRUE, TRUE, TRUE);
}
void memmap_nmt_1k(BYTE nidx, DBWORD address, DBWORD value) {
	memmap_nmt(nidx, address, value, S1K, TRUE, TRUE, TRUE);
}
void memmap_nmt_2k(BYTE nidx, DBWORD address, DBWORD value) {
	memmap_nmt(nidx, address, value, S2K, TRUE, TRUE, TRUE);
}
void memmap_nmt_4k(BYTE nidx, DBWORD address, DBWORD value) {
	memmap_nmt(nidx, address, value, S4K, TRUE, TRUE, TRUE);
}
void memmap_nmt_8k(BYTE nidx, DBWORD address, DBWORD value) {
	memmap_nmt(nidx, address, value, S8K, TRUE, TRUE, TRUE);
}

INLINE static void memmap_nmt_wp(BYTE nidx, DBWORD adr, DBWORD value, size_t size, BYTE rd, BYTE wr, BYTE tvalue) {
	memmap_nmt(nidx, adr, value, size, rd, wr, tvalue);
}
void memmap_nmt_wp_128b(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_nmt_wp(nidx, address, value, S128B, rd, wr, TRUE);
}
void memmap_nmt_wp_256b(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_nmt_wp(nidx, address, value, S256B, rd, wr, TRUE);
}
void memmap_nmt_wp_512b(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_nmt_wp(nidx, address, value, S512B, rd, wr, TRUE);
}
void memmap_nmt_wp_1k(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_nmt_wp(nidx, address, value, S1K, rd, wr, TRUE);
}
void memmap_nmt_wp_2k(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_nmt_wp(nidx, address, value, S2K, rd, wr, TRUE);
}
void memmap_nmt_wp_4k(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_nmt_wp(nidx, address, value, S4K, rd, wr, TRUE);
}
void memmap_nmt_wp_8k(BYTE nidx, DBWORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_nmt_wp(nidx, address, value, S8K, rd, wr, TRUE);
}

INLINE static void memmap_nmt_chrrom(BYTE nidx, DBWORD adr, DBWORD value, size_t size, BYTE tvalue) {
	BYTE *dst = NULL;
	size_t dsize = 0;
	BYTE rd = TRUE;
	BYTE wr = FALSE;

	if (adr & PPUMM) {
		if (chrrom_size()) {
			dst = chrrom_pnt();
			dsize = chrrom_size();
		} else if (vram_size(nidx)) {
			dst = vram_pnt(nidx);
			dsize = vram_size(nidx);
			wr = TRUE;
		}
		if (dst) {
			memmap_other(nidx, adr, value, size, dst, dsize, rd, wr, tvalue);
		}
	}
}
void memmap_nmt_chrrom_128b(BYTE nidx, DBWORD address, DBWORD value) {
	memmap_nmt_chrrom(nidx, address, value, S128B, TRUE);
}
void memmap_nmt_chrrom_256b(BYTE nidx, DBWORD address, DBWORD value) {
	memmap_nmt_chrrom(nidx, address, value, S256B, TRUE);
}
void memmap_nmt_chrrom_512b(BYTE nidx, DBWORD address, DBWORD value) {
	memmap_nmt_chrrom(nidx, address, value, S512B, TRUE);
}
void memmap_nmt_chrrom_1k(BYTE nidx, DBWORD address, DBWORD value) {
	memmap_nmt_chrrom(nidx, address, value, S1K, TRUE);
}
void memmap_nmt_chrrom_2k(BYTE nidx, DBWORD address, DBWORD value) {
	memmap_nmt_chrrom(nidx, address, value, S2K, TRUE);
}
void memmap_nmt_chrrom_4k(BYTE nidx, DBWORD address, DBWORD value) {
	memmap_nmt_chrrom(nidx, address, value, S4K, TRUE);
}
void memmap_nmt_chrrom_8k(BYTE nidx, DBWORD address, DBWORD value) {
	memmap_nmt_chrrom(nidx, address, value, S8K, TRUE);
}

INLINE static void memmap_nmt_vram(BYTE nidx, DBWORD adr, DBWORD value, size_t size, BYTE tvalue) {
	if (adr & PPUMM) {
		memmap_other(nidx, adr, value, size, vram_pnt(nidx), vram_size(nidx), TRUE, TRUE, tvalue);
	}
}
void memmap_nmt_vram_128b(BYTE nidx, DBWORD address, DBWORD value) {
	memmap_nmt_vram(nidx, address, value, S128B, TRUE);
}
void memmap_nmt_vram_256b(BYTE nidx, DBWORD address, DBWORD value) {
	memmap_nmt_vram(nidx, address, value, S256B, TRUE);
}
void memmap_nmt_vram_512b(BYTE nidx, DBWORD address, DBWORD value) {
	memmap_nmt_vram(nidx, address, value, S512B, TRUE);
}
void memmap_nmt_vram_1k(BYTE nidx, DBWORD address, DBWORD value) {
	memmap_nmt_vram(nidx, address, value, S1K, TRUE);
}
void memmap_nmt_vram_2k(BYTE nidx, DBWORD address, DBWORD value) {
	memmap_nmt_vram(nidx, address, value, S2K, TRUE);
}
void memmap_nmt_vram_4k(BYTE nidx, DBWORD address, DBWORD value) {
	memmap_nmt_vram(nidx, address, value, S4K, TRUE);
}
void memmap_nmt_vram_8k(BYTE nidx, DBWORD address, DBWORD value) {
	memmap_nmt_vram(nidx, address, value, S8K, TRUE);
}

void mirroring_H(BYTE nidx) {
	mapper.mirroring = MIRRORING_HORIZONTAL;
	memmap_nmt_1k(nidx, MMPPU(0x2000), 0);
	memmap_nmt_1k(nidx, MMPPU(0x2400), 0);
	memmap_nmt_1k(nidx, MMPPU(0x2800), 1);
	memmap_nmt_1k(nidx, MMPPU(0x2C00), 1);

	memmap_nmt_1k(nidx, MMPPU(0x3000), 0);
	memmap_nmt_1k(nidx, MMPPU(0x3400), 0);
	memmap_nmt_1k(nidx, MMPPU(0x3800), 1);
	memmap_nmt_1k(nidx, MMPPU(0x3C00), 1);
}
void mirroring_V(BYTE nidx) {
	mapper.mirroring = MIRRORING_VERTICAL;
	memmap_nmt_2k(nidx, MMPPU(0x2000), 0);
	memmap_nmt_2k(nidx, MMPPU(0x2800), 0);

	memmap_nmt_2k(nidx, MMPPU(0x3000), 0);
	memmap_nmt_2k(nidx, MMPPU(0x3800), 0);
}
void mirroring_SCR0(BYTE nidx) {
	mapper.mirroring = MIRRORING_SINGLE_SCR0;
	memmap_nmt_1k(nidx, MMPPU(0x2000), 0);
	memmap_nmt_1k(nidx, MMPPU(0x2400), 0);
	memmap_nmt_1k(nidx, MMPPU(0x2800), 0);
	memmap_nmt_1k(nidx, MMPPU(0x2C00), 0);

	memmap_nmt_1k(nidx, MMPPU(0x3000), 0);
	memmap_nmt_1k(nidx, MMPPU(0x3400), 0);
	memmap_nmt_1k(nidx, MMPPU(0x3800), 0);
	memmap_nmt_1k(nidx, MMPPU(0x3C00), 0);
}
void mirroring_SCR1(BYTE nidx) {
	mapper.mirroring = MIRRORING_SINGLE_SCR1;
	memmap_nmt_1k(nidx, MMPPU(0x2000), 1);
	memmap_nmt_1k(nidx, MMPPU(0x2400), 1);
	memmap_nmt_1k(nidx, MMPPU(0x2800), 1);
	memmap_nmt_1k(nidx, MMPPU(0x2C00), 1);

	memmap_nmt_1k(nidx, MMPPU(0x3000), 1);
	memmap_nmt_1k(nidx, MMPPU(0x3400), 1);
	memmap_nmt_1k(nidx, MMPPU(0x3800), 1);
	memmap_nmt_1k(nidx, MMPPU(0x3C00), 1);
}
void mirroring_FSCR(BYTE nidx) {
	mapper.mirroring = MIRRORING_FOURSCR;
	memmap_nmt_4k(nidx, MMPPU(0x2000), 0);

	memmap_nmt_4k(nidx, MMPPU(0x3000), 0);
}
void mirroring_SCR0x1_SCR1x3(BYTE nidx) {
	mapper.mirroring = MIRRORING_SCR0x1_SCR1x3;
	memmap_nmt_1k(nidx, MMPPU(0x2000), 0);
	memmap_nmt_1k(nidx, MMPPU(0x2400), 1);
	memmap_nmt_1k(nidx, MMPPU(0x2800), 1);
	memmap_nmt_1k(nidx, MMPPU(0x2C00), 1);

	memmap_nmt_1k(nidx, MMPPU(0x3000), 0);
	memmap_nmt_1k(nidx, MMPPU(0x3400), 1);
	memmap_nmt_1k(nidx, MMPPU(0x3800), 1);
	memmap_nmt_1k(nidx, MMPPU(0x3C00), 1);
}
void mirroring_SCR0x3_SCR1x1(BYTE nidx) {
	mapper.mirroring = MIRRORING_SCR0x3_SCR1x1;
	memmap_nmt_1k(nidx, MMPPU(0x2000), 0);
	memmap_nmt_1k(nidx, MMPPU(0x2400), 0);
	memmap_nmt_1k(nidx, MMPPU(0x2800), 0);
	memmap_nmt_1k(nidx, MMPPU(0x2C00), 1);

	memmap_nmt_1k(nidx, MMPPU(0x3000), 0);
	memmap_nmt_1k(nidx, MMPPU(0x3400), 0);
	memmap_nmt_1k(nidx, MMPPU(0x3800), 0);
	memmap_nmt_1k(nidx, MMPPU(0x3C00), 1);
}

BYTE nmt_malloc(BYTE nidx) {
	if (memmap_malloc(&nmt_pnt(nidx), nes[nidx].m.nmt.real_size) == EXIT_ERROR) {
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
	set_size(&miscrom_size(), &miscrom.real_size, nes[0].m.memmap.prg.info.chunk.size, size);
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
		if (wram_nvram_size() || vram_nvram_size(0) || vram_nvram_size(1) || info.mapper.force_battery_io) {
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
				for (int nesidx = 0; nesidx < info.number_of_nes; nesidx++) {
					if (vram_nvram_size(nesidx) && vram_nvram_pnt(nesidx)) {
						// leggo il contenuto della nvram
						if (fread(vram_nvram_pnt(nesidx), vram_nvram_size(nesidx), 1, fp) < 1) {
							log_error(uL("mapper;error on read battery memory (%s)"), strerror(errno));
							fclose(fp);
							return;
						}
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
	if (wram_nvram_size() || vram_nvram_size(0) || vram_nvram_size(1) || info.mapper.force_battery_io) {
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
			for (int nesidx = 0; nesidx < info.number_of_nes; nesidx++) {
				if (vram_nvram_size(nesidx) && vram_nvram_pnt(nesidx)) {
					// scrivo il contenuto della nvram
					if (fwrite(vram_nvram_pnt(nesidx), vram_nvram_size(nesidx), 1, fp) < 1) {
						log_error(uL("mapper;error on write battery memory (%s)"), strerror(errno));
						fclose(fp);
						return;
					}
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

	if (size) {
		while (max > 0) {
			mask = (mask << 1) | 1;
			max >>= 1;
		}
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
