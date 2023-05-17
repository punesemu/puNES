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
#include "mem_map.h"
#include "gui.h"
#include "tas.h"
#include "cpu.h"
#include "mappers.h"
#include "vs_system.h"

INLINE static unsigned int slot_from_address(_memmap_info *info, WORD address);
INLINE static size_t mask_address_with_size(size_t value, size_t size);
INLINE static unsigned int shift_from_size(size_t size);
INLINE static size_t pow_of_2(size_t size);

_memmap memmap;
_prgrom prgrom;
_wram wram;
//_trainer trainer;
_miscrom miscrom;

// memmap ----------------------------------------------------------------------------

typedef struct _memmap_bank {
	BYTE type;
	BYTE rd;
	BYTE wr;
	size_t size;
	BYTE translate_value;
	struct _memmap_cpu_data {
		size_t address;
		size_t value;
	} cpu;
	struct _memmap_wp_chunk_dst {
		BYTE *pnt;
		size_t size;
	} dst;
} _memmap_bank;

BYTE memmap_malloc(BYTE **dst, size_t size);
BYTE memmap_region_init(_memmap_region *region, size_t size, size_t items);
void memmap_region_quit(_memmap_region *region);

INLINE static void memmap_auto_wp(WORD adr, DBWORD value, BYTE rd, BYTE wr, BYTE tvalue, size_t size);
INLINE static void memmap_auto(WORD adr, DBWORD value, BYTE tvalue, size_t size);
INLINE static void memmap_disable(WORD adr, BYTE tvalue, size_t size);
INLINE static void memmap_wram(WORD adr, DBWORD value, BYTE rd, BYTE wr, BYTE tvalue, size_t size);
INLINE static void memmap_prgrom(WORD adr, DBWORD value, BYTE rd, BYTE wr, BYTE tvalue, size_t size);
INLINE static void memmap_other(WORD adr, DBWORD value, BYTE *dst, size_t dsize, BYTE rd, BYTE wr, BYTE tvalue, size_t bsize);

INLINE static void memmap_wp_chunk(_memmap_region *region);
INLINE static void memmap_wp_set_chunks(void);
INLINE static _memmap_region *memmap_get_region(WORD address);

static _memmap_bank smmbank;

BYTE memmap_init(void) {
	if (memmap_region_init(&memmap.wram, MEMMAP_WRAM_CHUNK_SIZE, MEMMAP_WRAM_CHUNK_BANKS) == EXIT_ERROR) {
		return (EXIT_ERROR);
	};
	if (memmap_region_init(&memmap.prgrom, MEMMAP_PRGROM_CHUNK_SIZE, MEMMAP_PRGROM_CHUNK_BANKS) == EXIT_ERROR) {
		return (EXIT_ERROR);
	};
	return (EXIT_OK);
}
void memmap_quit(void) {
	memmap_region_quit(&memmap.wram);
	memmap_region_quit(&memmap.prgrom);
}
BYTE memmap_adr_is_readable(WORD address) {
	_memmap_region *region = memmap_get_region(address);

	return (region->chunks[slot_from_address(&region->info, address)].readable);
}
BYTE memmap_adr_is_writable(WORD address) {
	_memmap_region *region = memmap_get_region(address);

	return (region->chunks[slot_from_address(&region->info, address)].writable);
}
BYTE *memmap_chunk_pnt(WORD address) {
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

// with permissions
INLINE static void memmap_auto_wp(WORD adr, DBWORD value, BYTE rd, BYTE wr, BYTE tvalue, size_t size) {
	if (adr >= 0x8000) {
		memmap_prgrom(adr, value, rd, wr, tvalue, size);
	} else if (adr >= 0x4000) {
		memmap_wram(adr, value, rd, wr, tvalue, size);
	}
}
void memmap_auto_wp_256b(WORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_auto_wp(address, value, rd, wr, TRUE, S256B);
}
void memmap_auto_wp_512b(WORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_auto_wp(address, value, rd, wr, TRUE, S512B);
}
void memmap_auto_wp_1k(WORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_auto_wp(address, value, rd, wr, TRUE, S1K);
}
void memmap_auto_wp_2k(WORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_auto_wp(address, value, rd, wr, TRUE, S2K);
}
void memmap_auto_wp_4k(WORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_auto_wp(address, value, rd, wr, TRUE, S4K);
}
void memmap_auto_wp_8k(WORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_auto_wp(address, value, rd, wr, TRUE, S8K);
}
void memmap_auto_wp_16k(WORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_auto_wp(address, value, rd, wr, TRUE, S16K);
}
void memmap_auto_wp_32k(WORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_auto_wp(address, value, rd, wr, TRUE, S32K);
}
void memmap_auto_wp_custom_size(WORD address, DBWORD chunk, BYTE rd, BYTE wr, size_t size) {
	memmap_auto_wp(address, chunk, rd, wr, FALSE, size);
}

// permissions :
// prgrom : rd = TRUE, wr = FALSE
// wram   : rd = TRUE, wr = TRUE
INLINE static void memmap_auto(WORD adr, DBWORD value, BYTE tvalue, size_t size) {
	if (adr >= 0x8000) {
		memmap_prgrom(adr, value, TRUE, FALSE, tvalue, size);
	} else if (adr >= 0x4000) {
		memmap_wram(adr, value, TRUE, TRUE, tvalue, size);
	}
}
void memmap_auto_256b(WORD address, DBWORD value) {
	memmap_auto(address, value, TRUE, S256B);
}
void memmap_auto_512b(WORD address, DBWORD value) {
	memmap_auto(address, value, TRUE, S512B);
}
void memmap_auto_1k(WORD address, DBWORD value) {
	memmap_auto(address, value, TRUE, S1K);
}
void memmap_auto_2k(WORD address, DBWORD value) {
	memmap_auto(address, value, TRUE, S2K);
}
void memmap_auto_4k(WORD address, DBWORD value) {
	memmap_auto(address, value, TRUE, S4K);
}
void memmap_auto_8k(WORD address, DBWORD value) {
	memmap_auto(address, value, TRUE, S8K);
}
void memmap_auto_16k(WORD address, DBWORD value) {
	memmap_auto(address, value, TRUE, S16K);
}
void memmap_auto_32k(WORD address, DBWORD value) {
	memmap_auto(address, value, TRUE, S32K);
}
void memmap_auto_custom_size(WORD address, DBWORD chunk, size_t size) {
	memmap_auto(address, chunk, FALSE, size);
}

// permissions : rd = FALSE, wr = FALSE
INLINE static void memmap_disable(WORD adr, BYTE tvalue, size_t size) {
	smmbank.cpu.address = adr;
	smmbank.cpu.value = 0;
	smmbank.dst.pnt = NULL;
	smmbank.dst.size = 0;
	smmbank.type = MEMMAP_BANK_NONE;
	smmbank.rd = FALSE;
	smmbank.wr = FALSE;
	smmbank.size = size;
	smmbank.translate_value = tvalue;
	memmap_wp_set_chunks();
}
void memmap_disable_256b(WORD address) {
	memmap_disable(address, TRUE, S256B);
}
void memmap_disable_512b(WORD address) {
	memmap_disable(address, TRUE, S512B);
}
void memmap_disable_1k(WORD address) {
	memmap_disable(address, TRUE, S1K);
}
void memmap_disable_2k(WORD address) {
	memmap_disable(address, TRUE, S2K);
}
void memmap_disable_4k(WORD address) {
	memmap_disable(address, TRUE, S4K);
}
void memmap_disable_8k(WORD address) {
	memmap_disable(address, TRUE, S8K);
}
void memmap_disable_16k(WORD address) {
	memmap_disable(address, TRUE, S16K);
}
void memmap_disable_32k(WORD address) {
	memmap_disable(address, TRUE, S32K);
}
void memmap_disable_custom_size(WORD address, size_t size) {
	memmap_disable(address, FALSE, size);
}

// with permissions
INLINE static void memmap_other(WORD adr, DBWORD value, BYTE *dst, size_t dsize, BYTE rd, BYTE wr, BYTE tvalue, size_t bsize) {
	smmbank.cpu.address = adr;
	smmbank.cpu.value = value;
	smmbank.dst.pnt = dst;
	smmbank.dst.size = dsize;
	smmbank.type = MEMMAP_BANK_OTHER;
	smmbank.rd = rd;
	smmbank.wr = wr;
	smmbank.size = bsize;
	smmbank.translate_value = tvalue;
	memmap_wp_set_chunks();
}
void memmap_other_256b(WORD address, DBWORD value, BYTE *dst, size_t dst_size, BYTE rd, BYTE wr) {
	memmap_other(address, value, dst, dst_size, rd, wr, TRUE, S256B);
}
void memmap_other_512b(WORD address, DBWORD value, BYTE *dst, size_t dst_size, BYTE rd, BYTE wr) {
	memmap_other(address, value, dst, dst_size, rd, wr, TRUE, S512B);
}
void memmap_other_1k(WORD address, DBWORD value, BYTE *dst, size_t dst_size, BYTE rd, BYTE wr) {
	memmap_other(address, value, dst, dst_size, rd, wr, TRUE, S1K);
}
void memmap_other_2k(WORD address, DBWORD value, BYTE *dst, size_t dst_size, BYTE rd, BYTE wr) {
	memmap_other(address, value, dst, dst_size, rd, wr, TRUE, S2K);
}
void memmap_other_4k(WORD address, DBWORD value, BYTE *dst, size_t dst_size, BYTE rd, BYTE wr) {
	memmap_other(address, value, dst, dst_size, rd, wr, TRUE, S4K);
}
void memmap_other_8k(WORD address, DBWORD value, BYTE *dst, size_t dst_size, BYTE rd, BYTE wr) {
	memmap_other(address, value, dst, dst_size, rd, wr, TRUE, S8K);
}
void memmap_other_16k(WORD address, DBWORD value, BYTE *dst, size_t dst_size, BYTE rd, BYTE wr) {
	memmap_other(address, value, dst, dst_size, rd, wr, TRUE, S16K);
}
void memmap_other_32k(WORD address, DBWORD value, BYTE *dst, size_t dst_size, BYTE rd, BYTE wr) {
	memmap_other(address, value, dst, dst_size, rd, wr, TRUE, S32K);
}
void memmap_other_custom_size(WORD address, DBWORD chunk, BYTE *dst, size_t dsize, BYTE rd, BYTE wr, size_t size) {
	memmap_other(address, chunk, dst, dsize, rd, wr, FALSE, size);
}

BYTE memmap_malloc(BYTE **dst, size_t size) {
	(*dst) = !size ? NULL : (BYTE *)malloc(size);
	return ((*dst) ? EXIT_OK : EXIT_ERROR);
}
BYTE memmap_region_init(_memmap_region *region, size_t size, size_t items) {
	region->info.shift = shift_from_size(size);
	region->info.chunk.size = size;
	region->info.chunk.items = items;
	region->chunks = malloc(sizeof(_memmap_chunk ) * items);
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
	region->chunks[smmbank.cpu.address].type = !smmbank.dst.pnt ? MEMMAP_BANK_NONE : smmbank.type;
	// per le mapper che permettono il controllo sulla WRAM (tipo MMC3), posso scrivere (o leggere)
	// nella regione interessata anche se non c'è installata nessuan WRAM solo per valorizzare i
	// registri interni della mapper.
	region->chunks[smmbank.cpu.address].writable = smmbank.wr;
	region->chunks[smmbank.cpu.address].readable = smmbank.rd;
	region->chunks[smmbank.cpu.address].pnt = !smmbank.dst.pnt
		? NULL
		: &smmbank.dst.pnt[mask_address_with_size(smmbank.cpu.value << region->info.shift, smmbank.dst.size)];
	region->chunks[smmbank.cpu.address].mem_region.start = !smmbank.dst.pnt
		? NULL
		: smmbank.dst.pnt;
	region->chunks[smmbank.cpu.address].mem_region.end = !smmbank.dst.pnt
		? NULL
		: &smmbank.dst.pnt[smmbank.dst.size];
	region->chunks[smmbank.cpu.address].mask = !smmbank.dst.pnt
		? 0
		: mask_address_with_size(region->info.chunk.size - 1, smmbank.dst.size);
}
INLINE static void memmap_wp_set_chunks(void) {
	_memmap_region *region = memmap_get_region(smmbank.cpu.address);

	if (region) {
		unsigned int slot = slot_from_address(&region->info, smmbank.cpu.address);
		size_t chunks = smmbank.size / region->info.chunk.size;
		size_t bank = smmbank.translate_value ? smmbank.cpu.value * chunks : smmbank.cpu.value;

		for (size_t i = 0; i < chunks; i++) {
			smmbank.cpu.address = slot + i;
			smmbank.cpu.value = bank + i;
			memmap_wp_chunk(region);
		}
	}
}
INLINE static _memmap_region *memmap_get_region(WORD address) {
	if (address >= 0x8000) {
		return(&memmap.prgrom);
	} else if (address >= 0x4000) {
		return(&memmap.wram);
	}
	return (NULL);
}

// wram ------------------------------------------------------------------------------

void wram_load_nvram_file(void);
void wram_file_nvram(uTCHAR *prg_ram_file);
BYTE wram_malloc(void);

INLINE static void memmap_wram_ram_wp(WORD adr, DBWORD value, BYTE rd, BYTE wr, BYTE tvalue, size_t size);
INLINE static void memmap_wram_nvram_wp(WORD adr, DBWORD value, BYTE rd, BYTE wr, BYTE tvalue, size_t size);

BYTE wram_init(void) {
	// se non ci sono stati settaggi particolari della mapper
	// devono esserci banchi di PRG Ram extra allora li assegno.
	if ((info.reset == CHANGE_ROM) || (info.reset == POWER_UP)) {
		if (wram_size()) {
			// alloco la memoria necessaria
			if (wram_malloc() == EXIT_ERROR) {
				return (EXIT_ERROR);
			}
			// resetto la wram
			wram_reset();
			// inizializzo il pointer della ram
			wram.ram.pnt = wram_nvram_size() ? wram_pnt_byte(wram_nvram_size()) : wram_pnt_byte(0);
			// inizializzo il pointer della nvram
			wram.nvram.pnt = wram_nvram_size() ? wram_pnt_byte(0) : NULL;
			// se esiste nvram la carico
			wram_load_nvram_file();
		}
	}
	wram_memset();
	return (EXIT_OK);
}
void wram_quit(void) {
	wram_save_nvram_file();
	if (wram_pnt()) {
		free(wram_pnt());
	}
	memset(&wram, 0x00, sizeof(wram));
	memmap_disable_16k(0x4000);
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
void wram_reset(void) {
	// disabilito la wram da 0x4000 a 0x5FFF
	memmap_disable_8k(0x4000);
	// setto i primi 8k su 0x6000
	memmap_auto_8k(0x6000, 0);
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
				memset(dst, 0x00, size);
				//emu_initial_ram(dst, size));
			}
		}
	}
}

BYTE wram_rd(WORD address) {
	const unsigned int slot = slot_from_address(&memmap.wram.info, address);
	BYTE openbus = cpu.openbus.before;

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
	if (memmap.wram.chunks[slot].readable) {
		openbus = (memmap.wram.chunks[slot].pnt[address & memmap.wram.chunks[slot].mask]);
	}
	return (openbus);
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
	if (memmap.wram.chunks[slot].writable) {
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
INLINE static void memmap_wram(WORD adr, DBWORD value, BYTE rd, BYTE wr, BYTE tvalue, size_t size) {
	smmbank.cpu.address = adr;
	smmbank.cpu.value = value;
	smmbank.dst.pnt = wram_pnt();
	smmbank.dst.size = wram_size();
	smmbank.type = MEMMAP_BANK_RAM;
	smmbank.rd = rd;
	smmbank.wr = wr;
	smmbank.size = size;
	smmbank.translate_value = tvalue;
	memmap_wp_set_chunks();
}
void memmap_wram_256b(WORD address, DBWORD value) {
	memmap_wram(address, value, TRUE, TRUE, TRUE, S256B);
}
void memmap_wram_512b(WORD address, DBWORD value) {
	memmap_wram(address, value, TRUE, TRUE, TRUE, S512B);
}
void memmap_wram_1k(WORD address, DBWORD value) {
	memmap_wram(address, value, TRUE, TRUE, TRUE, S1K);
}
void memmap_wram_2k(WORD address, DBWORD value) {
	memmap_wram(address, value, TRUE, TRUE, TRUE, S2K);
}
void memmap_wram_4k(WORD address, DBWORD value) {
	memmap_wram(address, value, TRUE, TRUE, TRUE, S4K);
}
void memmap_wram_8k(WORD address, DBWORD value) {
	memmap_wram(address, value, TRUE, TRUE, TRUE, S8K);
}
void memmap_wram_16k(WORD address, DBWORD value) {
	memmap_wram(address, value, TRUE, TRUE, TRUE, S16K);
}
void memmap_wram_32k(WORD address, DBWORD value) {
	memmap_wram(address, value, TRUE, TRUE, TRUE, S32K);
}
void memmap_wram_custom_size(WORD address, DBWORD chunk, size_t size) {
	memmap_wram(address, chunk, TRUE, TRUE, FALSE, size);
}

// with permissions
INLINE static void memmap_wram_ram_wp(WORD adr, DBWORD value, BYTE rd, BYTE wr, BYTE tvalue, size_t size) {
	smmbank.cpu.address = adr;
	smmbank.cpu.value = value;
	smmbank.dst.pnt = wram_ram_pnt();
	smmbank.dst.size = wram_ram_size();
	smmbank.type = MEMMAP_BANK_RAM;
	smmbank.rd = rd;
	smmbank.wr = wr;
	smmbank.size = size;
	smmbank.translate_value = tvalue;
	memmap_wp_set_chunks();
}
void memmap_wram_ram_wp_256b(WORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_wram_ram_wp(address, value, rd, wr, TRUE, S256B);
}
void memmap_wram_ram_wp_512b(WORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_wram_ram_wp(address, value, rd, wr, TRUE, S512B);
}
void memmap_wram_ram_wp_1k(WORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_wram_ram_wp(address, value, rd, wr, TRUE, S1K);
}
void memmap_wram_ram_wp_2k(WORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_wram_ram_wp(address, value, rd, wr, TRUE, S2K);
}
void memmap_wram_ram_wp_4k(WORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_wram_ram_wp(address, value, rd, wr, TRUE, S4K);
}
void memmap_wram_ram_wp_8k(WORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_wram_ram_wp(address, value, rd, wr, TRUE, S8K);
}
void memmap_wram_ram_wp_16k(WORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_wram_ram_wp(address, value, rd, wr, TRUE, S16K);
}
void memmap_wram_ram_wp_32k(WORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_wram_ram_wp(address, value, rd, wr, TRUE, S32K);
}
void memmap_wram_ram_wp_custom_size(WORD address, DBWORD chunk, BYTE rd, BYTE wr, size_t size) {
	memmap_wram_ram_wp(address, chunk, rd, wr, FALSE, size);
}

// with permissions
INLINE static void memmap_wram_nvram_wp(WORD adr, DBWORD value, BYTE rd, BYTE wr, BYTE tvalue, size_t size) {
	smmbank.cpu.address = adr;
	smmbank.cpu.value = value;
	smmbank.dst.pnt = wram_nvram_pnt();
	smmbank.dst.size = wram_nvram_size();
	smmbank.type = MEMMAP_BANK_RAM;
	smmbank.rd = rd;
	smmbank.wr = wr;
	smmbank.size = size;
	smmbank.translate_value = tvalue;
	memmap_wp_set_chunks();
}
void memmap_wram_nvram_wp_256b(WORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_wram_nvram_wp(address, value, rd, wr, TRUE, S256B);
}
void memmap_wram_nvram_wp_512b(WORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_wram_nvram_wp(address, value, rd, wr, TRUE, S512B);
}
void memmap_wram_nvram_wp_1k(WORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_wram_nvram_wp(address, value, rd, wr, TRUE, S1K);
}
void memmap_wram_nvram_wp_2k(WORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_wram_nvram_wp(address, value, rd, wr, TRUE, S2K);
}
void memmap_wram_nvram_wp_4k(WORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_wram_nvram_wp(address, value, rd, wr, TRUE, S4K);
}
void memmap_wram_nvram_wp_8k(WORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_wram_nvram_wp(address, value, rd, wr, TRUE, S8K);
}
void memmap_wram_nvram_wp_16k(WORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_wram_nvram_wp(address, value, rd, wr, TRUE, S16K);
}
void memmap_wram_nvram_wp_32k(WORD address, DBWORD value, BYTE rd, BYTE wr) {
	memmap_wram_nvram_wp(address, value, rd, wr, TRUE, S32K);
}
void memmap_wram_nvram_wp_custom_size(WORD address, DBWORD chunk, BYTE rd, BYTE wr, size_t size) {
	memmap_wram_nvram_wp(address, chunk, rd, wr, FALSE, size);
}

void wram_load_nvram_file(void) {
	if (wram_nvram_size() || info.mapper.force_battery_io) {
		uTCHAR prg_ram_file[LENGTH_FILE_NAME_LONG];
		FILE *fp = NULL;

		// estraggo il nome del file
		wram_file_nvram(&prg_ram_file[0]);

		// provo ad aprire il file
		fp = ufopen(prg_ram_file, uL("rb"));
		if (fp) {
			if (tas.type == NOTAS) {
				if (wram_nvram_pnt()) {
					// leggo il contenuto della nvram
					if (fread(wram_nvram_pnt(), wram_nvram_size(), 1, fp) < 1) {
						log_error(uL("mapper;error on read battery memory (%s)"), strerror(errno));
						fclose(fp);
						return;
					}
				}
				if (extcl_battery_io) {
					extcl_battery_io(RD_BAT, fp);
				}
			}
			// chiudo il file
			fclose(fp);
		}
	}
}
void wram_save_nvram_file(void) {
	if (wram_nvram_size() || info.mapper.force_battery_io) {
		uTCHAR prg_ram_file[LENGTH_FILE_NAME_LONG];
		FILE *fp = NULL;

		// estraggo il nome del file
		wram_file_nvram(&prg_ram_file[0]);

		// apro il file
		fp = ufopen(prg_ram_file, uL("w+b"));
		if (fp) {
			if (tas.type == NOTAS) {
				if (wram_nvram_pnt()) {
					// scrivo il contenuto della nvram
					if (fwrite(wram_nvram_pnt(), wram_nvram_size(), 1, fp) < 1) {
						log_error(uL("mapper;error on write battery memory (%s)"), strerror(errno));
						fclose(fp);
						return;
					}
				}
				if (extcl_battery_io) {
					extcl_battery_io(WR_BAT, fp);
				}
			}
			// forzo la scrittura del file
			fflush(fp);
			// chiudo
			fclose(fp);
		}
	}
}
void wram_file_nvram(uTCHAR *prg_ram_file) {
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

BYTE wram_malloc(void) {
	if (memmap_malloc(&wram_pnt(), wram.real_size) == EXIT_ERROR) {
		log_error(uL("wram malloc;out of memory"));
		return (EXIT_ERROR);
	}
	return (EXIT_OK);
}

// prgrom ----------------------------------------------------------------------------

BYTE prgrom_malloc(void);

BYTE prgrom_init(BYTE set_value) {
	// se non ci sono stati settaggi particolari della mapper
	// devono esserci banchi di PRG Ram extra allora li assegno.
	if ((info.reset == CHANGE_ROM) || (info.reset == POWER_UP)) {
		if (prgrom_pnt()) {
			free(prgrom_pnt());
			prgrom_pnt() = NULL;
		}
		if (prgrom_malloc() == EXIT_ERROR) {
			return (EXIT_ERROR);
		}
		memset(prgrom_pnt(), set_value, prgrom.real_size);
		prgrom_reset();
	}
	return (EXIT_OK);
}
void prgrom_quit(void) {
	if (prgrom_pnt()) {
		free(prgrom_pnt());
	}
	memset(&prgrom, 0x00, sizeof(prgrom));
	memmap_disable_32k(0x8000);
}
void prgrom_set_size(size_t size) {
	prgrom.size = size;
	prgrom.real_size = pow_of_2((prgrom.size / MEMMAP_PRGROM_CHUNK_SIZE) +
		((prgrom.size % MEMMAP_PRGROM_CHUNK_SIZE) ? 1 : 0)) * MEMMAP_PRGROM_CHUNK_SIZE;
}
void prgrom_reset(void) {
	memmap_auto_16k(0x8000, 0);
	memmap_auto_16k(0xC000, ~0);
}
WORD prgrom_banks(enum _sizes_types size) {
	return (prgrom_size() / (size_t)size) + ((prgrom_size() % (size_t)size) ? 1 : 0);
}
WORD prgrom_control_bank(enum _sizes_types size, WORD bank) {
	unsigned int max = prgrom_banks(size);

	return (!max ? 0 : bank > max ? bank & (max - 1) : bank);
}

BYTE prgrom_rd(WORD address) {
	const unsigned int slot = slot_from_address(&memmap.prgrom.info, address);
	BYTE openbus = cpu.openbus.actual;

	if (memmap.prgrom.chunks[slot].pnt && memmap.prgrom.chunks[slot].readable) {
		openbus = (memmap.prgrom.chunks[slot].pnt[address & memmap.prgrom.chunks[slot].mask]);
	}
	return (openbus);
}
void prgrom_wr(WORD address, BYTE value) {
	const unsigned int slot = slot_from_address(&memmap.prgrom.info, address);

	if (memmap.prgrom.chunks[slot].pnt && memmap.prgrom.chunks[slot].writable) {
		memmap.prgrom.chunks[slot].pnt[address & memmap.prgrom.chunks[slot].mask] = value;
	}
}

// permissions : rd = TRUE, wr = FALSE
INLINE static void memmap_prgrom(WORD adr, DBWORD value, BYTE rd, BYTE wr, BYTE tvalue, size_t size) {
	smmbank.cpu.address = adr;
	smmbank.cpu.value = value;
	smmbank.dst.pnt = prgrom_pnt();
	smmbank.dst.size = prgrom_size();
	smmbank.type = MEMMAP_BANK_PRGROM;
	smmbank.rd = rd;
	smmbank.wr = wr;
	smmbank.size = size;
	smmbank.translate_value = tvalue;
	memmap_wp_set_chunks();
}
void memmap_prgrom_256b(WORD address, DBWORD value) {
	memmap_prgrom(address, value, TRUE, FALSE, TRUE, S256B);
}
void memmap_prgrom_512b(WORD address, DBWORD value) {
	memmap_prgrom(address, value, TRUE, FALSE, TRUE, S512B);
}
void memmap_prgrom_1k(WORD address, DBWORD value) {
	memmap_prgrom(address, value, TRUE, FALSE, TRUE, S1K);
}
void memmap_prgrom_2k(WORD address, DBWORD value) {
	memmap_prgrom(address, value, TRUE, FALSE, TRUE, S2K);
}
void memmap_prgrom_4k(WORD address, DBWORD value) {
	memmap_prgrom(address, value, TRUE, FALSE, TRUE, S4K);
}
void memmap_prgrom_8k(WORD address, DBWORD value) {
	memmap_prgrom(address, value, TRUE, FALSE, TRUE, S8K);
}
void memmap_prgrom_16k(WORD address, DBWORD value) {
	memmap_prgrom(address, value, TRUE, FALSE, TRUE, S16K);
}
void memmap_prgrom_32k(WORD address, DBWORD value) {
	memmap_prgrom(address, value, TRUE, FALSE, TRUE, S32K);
}
void memmap_prgrom_custom_size(WORD address, DBWORD chunk, size_t size) {
	memmap_prgrom(address, chunk, TRUE, FALSE, FALSE, size);
}

BYTE prgrom_malloc(void) {
	if (memmap_malloc(&prgrom_pnt(), prgrom.real_size) == EXIT_ERROR) {
		log_error(uL("prgrom malloc;out of memory"));
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
	miscrom.size = size;
	miscrom.real_size = pow_of_2((miscrom.size / MEMMAP_PRGROM_CHUNK_SIZE) +
		((miscrom.size % MEMMAP_PRGROM_CHUNK_SIZE) ? 1 : 0)) * MEMMAP_PRGROM_CHUNK_SIZE;
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
				memcpy(dst, miscrom.data, miscrom.size);
			}
		}
	}
}

// misc ----------------------------------------------------------------------------

INLINE static unsigned int slot_from_address(_memmap_info *minfo, WORD address) {
	return ((address >> minfo->shift) & (minfo->chunk.items - 1));
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
	return (size < 1 ? size : emu_power_of_two(size));
}

