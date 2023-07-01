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
#include "crc.h"
#include "Crc32.h"

uint32_t emu_crc32(const void *data, size_t length) {
	return (crc32_fast(data, length));
}
uint32_t emu_crc32_continue(const void *data, size_t length, uint32_t previous) {
	return (crc32_fast(data, length, previous));
}
uint32_t emu_crc32_zeroes(size_t length, uint32_t previous) {
	if (length) {
		BYTE zeroes[length];

		memset(&zeroes[0], 0x00, sizeof(zeroes));
		return (crc32_fast(&zeroes[0], length, previous));
	}
	return (previous);
}