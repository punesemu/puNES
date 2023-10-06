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

#ifndef NES20DB_HPP_
#define NES20DB_HPP_

#include "common.h"

typedef struct _nes20db {
	struct _nes20db_prgrom {
		uint32_t size;
		uint32_t crc32;
	} prgrom;
	struct _nes20db_chrrom {
		uint32_t size;
		uint32_t crc32;
	} chrrom;
	struct _nes20db_trainer {
		uint32_t size;
		uint32_t crc32;
	} trainer;
	struct _nes20db_miscrom {
		uint32_t size;
		uint32_t crc32;
		uint32_t number;
	} miscrom;
	struct _nes20db_rom {
		uint32_t size;
		uint32_t crc32;
	} rom;
	struct _nes20db_prgram {
		uint32_t size;
	} prgram;
	struct _nes20db_prgnvram {
		uint32_t size;
	} prgnvram;
	struct _nes20db_chrram {
		uint32_t size;
	} chrram;
	struct _nes20db_chrnvram {
		uint32_t size;
	} chrnvram;
	struct _nes20db_pcb {
		uint32_t mapper;
		uint32_t submapper;
		uint32_t mirroring;
		uint32_t battery;
	} pcb;
	struct _nes20db_console {
		uint32_t type;
		uint32_t region;
	} console;
	struct _nes20db_vs {
		uint32_t hardware;
		uint32_t ppu;
	} vs;
	struct _nes20db_expansion {
		uint32_t type;
	} expansion;
} _nes20db;

extern _nes20db nes20db;

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC void nes20db_reset(void);
EXTERNC BYTE nes20db_search(void);

#undef EXTERNC

#endif /* NES20DB_HPP_ */
