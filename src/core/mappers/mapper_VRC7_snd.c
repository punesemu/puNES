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
/*
 *  Thx to Nuke.YKT for his project "Cycle-accurate emulation of Yamaha OPLL":
 *  https://github.com/nukeykt/Nuked-OPLL
 *
 ***********************************************************************************
 */

#include "mapper_VRC7_snd.h"
#include "save_slot.h"
#include "clock.h"
#include "opll.h"

typedef struct _opll {
	opll_t chip;
	int32_t out;
	int32_t next;
	double tick;
	double freq;
} _opll;

_opll opll;

void opll_reset() {
	OPLL_Reset(&opll.chip, opll_type_ds1001);
	opll.out = 0;
	opll.next = 0;
	opll.freq = machine.cpu_hz / (double)3580000.0f;
	opll.tick = 0;
}
void opll_write_reg(uint32_t reg, uint8_t value) {
	OPLL_Write(&opll.chip, reg, value);
}
void opll_update(void) {
	opll.tick += opll.freq;

	while (opll.tick > 1.0f) {
		int32_t buffer[2];

		opll.tick -= 1.0f;
		OPLL_Clock(&opll.chip, buffer);
		opll.next += buffer[0];
		opll.next += buffer[1];
		if (opll.chip.cycles == 0) {
			opll.out = opll.next;
			opll.next = 0;
		}
	}
}
BYTE opll_save(BYTE mode, BYTE slot, FILE *fp) {
	opll_patch_t *patchrom = (opll_patch_t *)opll.chip.patchrom;

	opll.chip.patchrom = NULL;
	save_slot_ele(mode, slot, opll.chip);
	save_slot_ele(mode, slot, opll.out);
	save_slot_ele(mode, slot, opll.next);
	save_slot_ele(mode, slot, opll.tick);
	opll.chip.patchrom = patchrom;
	return (EXIT_OK);
}
SWORD opll_calc(void) {
	return ((SWORD)opll.out);
}
