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

#ifndef NSFE_H_
#define NSFE_H_

#include "common.h"
#include "rom_mem.h"

enum nsfe_phase_type { NSFE_COUNT, NSFE_READ };

BYTE nsfe_load_rom(void);
void nsfe_info(void);

BYTE nsfe_NONE(_rom_mem *rom, BYTE phase);
BYTE nsfe_INFO(_rom_mem *rom, BYTE phase);
BYTE nsfe_DATA(_rom_mem *rom, BYTE phase);
BYTE nsfe_BANK(_rom_mem *rom, BYTE phase);
BYTE nsfe_RATE(_rom_mem *rom, BYTE phase);
BYTE nsfe_NSF2(_rom_mem *rom, BYTE phase);
BYTE nsfe_plst(_rom_mem *rom, BYTE phase);
BYTE nsfe_time(_rom_mem *rom, BYTE phase);
BYTE nsfe_fade(_rom_mem *rom, BYTE phase);
BYTE nsfe_tlbl(_rom_mem *rom, BYTE phase);
BYTE nsfe_taut(_rom_mem *rom, BYTE phase);
BYTE nsfe_auth(_rom_mem *rom, BYTE phase);
BYTE nsfe_text(_rom_mem *rom, BYTE phase);
BYTE nsfe_regn(_rom_mem *rom, BYTE phase);

#endif /* NSFE_H_ */
