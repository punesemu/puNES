/*
 * mapper_GameGenie.c
 *
 *  Created on: 16/apr/2012
 *      Author: fhorse
 */

#include "mappers.h"
#include "gamegenie.h"

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
