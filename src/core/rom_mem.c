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
#include "rom_mem.h"

void rom_mem_memcpy(void *dst, _rom_mem *rom, size_t increment) {
	memcpy(dst, rom->data + rom->position, increment);
	rom->position += increment;
}
BYTE rom_mem_ctrl_memcpy(void *dst, _rom_mem *rom, size_t increment) {
	if ((rom->position + increment) > rom->size) {
		return (EXIT_ERROR);
	}
	rom_mem_memcpy(dst, rom, increment);
	return (EXIT_OK);
}
BYTE rom_mem_ctrl_memcpy_truncated(void *dst, _rom_mem *rom, size_t increment) {
	if ((rom->position + increment) > rom->size) {
		rom_mem_memcpy(dst, rom, rom->size - rom->position);
		return (EXIT_ERROR);
	}
	rom_mem_memcpy(dst, rom, increment);
	return (EXIT_OK);
}
