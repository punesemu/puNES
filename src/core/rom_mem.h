/*
 *  Copyright (C) 2010-2020 Fabio Cavallo (aka FHorse)
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

#ifndef ROM_MEM_H_
#define ROM_MEM_H_

#include <stddef.h>
#include "common.h"

typedef struct _rom_mem {
	BYTE *data;
	size_t size;
	size_t position;
} _rom_mem;

void rom_mem_memcpy(void *dst, _rom_mem *rom, size_t increment);
BYTE rom_mem_ctrl_memcpy(void *dst, _rom_mem *rom, size_t increment);

#endif /* ROM_MEM_H_ */
