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

#include "overscan.h"

_overscan_borders overscan_borders[2];
_overscan overscan;

BYTE overscan_set_mode(BYTE mode) {
	_overscan_borders save = (*overscan.borders);

	if (mode == NTSC) {
		overscan.borders = &overscan_borders[0];
	} else {
		overscan.borders = &overscan_borders[1];
	}

	{
		BYTE *src = (BYTE *)&save, *dst = (BYTE *)overscan.borders;
		unsigned int i;

		for (i = 0; i < sizeof(_overscan_borders); i++) {
			if ((*(src + i)) != (*(dst + i))) {
				return (TRUE);
			}
		}
	}

	return (FALSE);
}
