/*
 *  Copyright (C) 2010-2016 Fabio Cavallo (aka FHorse)
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

#include "mappers.h"
#include "info.h"
#include "cheat.h"

void map_init_GameGenie(void) {
	EXTCL_CPU_WR_MEM(GameGenie);
}
void extcl_cpu_wr_mem_GameGenie(WORD address, BYTE value) {
	_cheat *ch = NULL;

	if ((address >= 0x8001) && (address <= 0x8004)) {
		address -= 0x8001;
		ch = &gamegenie.cheat[0];
	} else if ((address >= 0x8005) && (address <= 0x8008)) {
		address -= 0x8005;
		ch = &gamegenie.cheat[1];
	} else if ((address >= 0x8009) && (address <= 0x800C)) {
		address -= 0x8009;
		ch = &gamegenie.cheat[2];
	}

	if (ch) {
		switch (address) {
			case GG_ADDRESS_HIGH:
				ch->address = ((value | 0x80) << 8) | (ch->address & 0x00FF);
				break;
			case GG_ADDRESS_LOW:
				ch->address = (ch->address & 0xFF00) | value;
				break;
			case GG_COMPARE:
				ch->compare = value;
				break;
			case GG_REPLACE:
				ch->replace = value;
				break;
		}
	}

	if (address == 0x8000) {
		if (value) {
			gamegenie.value = value;
		} else {
			BYTE i;

			gamegenie.phase = GG_LOAD_ROM;
			info.execute_cpu = FALSE;

			/* la rom ne supporta solo 3 */
			for (i = 0; i < 3; i++) {
				ch = &gamegenie.cheat[i];

				if (!(ch->disabled = (gamegenie.value >> (4 + i)) & 0x01)) {
					ch->enabled_compare = (gamegenie.value >> (1 + i)) & 0x01;
					gamegenie.counter++;
				}
			}
		}
	}
}
