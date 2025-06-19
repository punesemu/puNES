/*
 *  Copyright (C) 2010-2026 Fabio Cavallo (aka FHorse)
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
 *  Thx to okaxaki for his project "A YM2413 emulator written in C."
 *  https://github.com/digital-sound-antiques/emu2413
 *  Thx to Nuke.YKT for his project "Cycle-accurate emulation of Yamaha OPLL":
 *  https://github.com/nukeykt/Nuked-OPLL
 */

#include "VRC7_snd.h"
#include "save_slot.h"
#include "snd.h"
#include "extra/emu2413/emu2413.h"

typedef struct _opll {
	SWORD out;
	BYTE tick;
} _opll;

OPLL *chip = NULL;
_opll opll;

void opll_quit(void) {
	if (chip) {
		OPLL_delete(chip);
		chip = NULL;
	}
}
void opll_reset(void) {
	opll_quit();
	chip = OPLL_new(3579545, snd.samplerate);
	OPLL_setChipType(chip, OPLL_VRC7_TONE);
	OPLL_set_rate(chip, 49716);
	OPLL_resetPatch(chip, OPLL_VRC7_TONE);
	opll.out = 0;
	opll.tick = 0;
}
void opll_write_reg(uint32_t reg, uint8_t value) {
	OPLL_writeIO(chip, reg, value);
}
void opll_update(void) {
	if (++opll.tick == 36) {
		opll.tick = 0;
		opll.out = OPLL_calc(chip);
		if (opll.out > 4096) {
			opll.out = 4096;
		} else if (opll.out < -4096) {
			opll.out = -4096;
		}
	}
}
BYTE opll_save(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, chip->adr);
	save_slot_ele(mode, slot, chip->inp_step);
	save_slot_ele(mode, slot, chip->out_step);
	save_slot_ele(mode, slot, chip->out_time);
	save_slot_ele(mode, slot, chip->reg);
	save_slot_ele(mode, slot, chip->test_flag);
	save_slot_ele(mode, slot, chip->slot_key_status);
	save_slot_ele(mode, slot, chip->rhythm_mode);
	save_slot_ele(mode, slot, chip->eg_counter);
	save_slot_ele(mode, slot, chip->pm_phase);
	save_slot_ele(mode, slot, chip->am_phase);
	save_slot_ele(mode, slot, chip->lfo_am);
	save_slot_ele(mode, slot, chip->noise);
	save_slot_ele(mode, slot, chip->short_noise);
	save_slot_ele(mode, slot, chip->patch_number);
	for (unsigned int i = 0; i < LENGTH(chip->slot); i++) {
		save_slot_ele(mode, slot, chip->slot[i].number);
		save_slot_ele(mode, slot, chip->slot[i].type);
		save_slot_ele(mode, slot, chip->slot[i].output);
		save_slot_ele(mode, slot, chip->slot[i].pg_phase);
		save_slot_ele(mode, slot, chip->slot[i].pg_out);
		save_slot_ele(mode, slot, chip->slot[i].pg_keep);
		save_slot_ele(mode, slot, chip->slot[i].blk_fnum);
		save_slot_ele(mode, slot, chip->slot[i].fnum);
		save_slot_ele(mode, slot, chip->slot[i].blk);
		save_slot_ele(mode, slot, chip->slot[i].eg_state);
		save_slot_ele(mode, slot, chip->slot[i].volume);
		save_slot_ele(mode, slot, chip->slot[i].key_flag);
		save_slot_ele(mode, slot, chip->slot[i].sus_flag);
		save_slot_ele(mode, slot, chip->slot[i].tll);
		save_slot_ele(mode, slot, chip->slot[i].rks);
		save_slot_ele(mode, slot, chip->slot[i].eg_rate_h);
		save_slot_ele(mode, slot, chip->slot[i].eg_rate_l);
		save_slot_ele(mode, slot, chip->slot[i].eg_shift);
		save_slot_ele(mode, slot, chip->slot[i].eg_out);
	}
	for (unsigned int i = 0; i < LENGTH(chip->patch); i++) {
		uint8_t patchrom[8] = { 0 };

		save_slot_ele(mode, slot, patchrom);
		if (mode == SAVE_SLOT_SAVE) {
			OPLL_patchToDump(&chip->patch[i], &patchrom[0]);
		} else if (mode == SAVE_SLOT_SAVE) {
			OPLL_dumpToPatch(&patchrom[0], &chip->patch[i]);
		}
	}
	save_slot_ele(mode, slot, chip->pan);
	save_slot_ele(mode, slot, chip->pan_fine);
	save_slot_ele(mode, slot, chip->mask);
	if (mode == SAVE_SLOT_READ) {
		OPLL_forceRefresh(chip);
	}
	save_slot_ele(mode, slot, opll.out);
	save_slot_ele(mode, slot, opll.tick);
	return (EXIT_OK);
}
SWORD opll_calc(void) {
	return (opll.out);
}
