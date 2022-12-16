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
#include "patcher_xdelta3_wrap.h"
#include "extra/xdelta-3.1.0/xdelta3.c"

BYTE patcher_xdelta(_rom_mem *patch, _rom_mem *rom) {
	usize_t size = 32 * 1024 * 1024; // 32 MB
	BYTE *blk1, *blk2;

	if ((blk1 = (BYTE *)malloc(size)) == NULL) {
		return (EXIT_ERROR);
	}

	memset(blk1, 0x00, size);

	if (xd3_decode_memory(patch->data, patch->size, rom->data, rom->size, blk1, &size, size, 0) != 0) {
		free(blk1);
		return (EXIT_ERROR);
	}

	if ((blk2 = (BYTE *)malloc(size)) == NULL) {
		free(blk1);
		return (EXIT_ERROR);
	}

	memcpy(blk2, blk1, size);

	free(blk1);
	free(rom->data);

	rom->data = blk2;
	rom->size = size;

	return (EXIT_OK);
}
