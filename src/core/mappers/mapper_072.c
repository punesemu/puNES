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
#include "mappers.h"
#include "save_slot.h"
#include "wave_file_interface.h"
#include "upd7756_interface.h"

INLINE static void prg_fix_072(void);
INLINE static void chr_fix_072(void);

_m072 m072;

void map_init_072(void) {
	EXTCL_AFTER_MAPPER_INIT(072);
	EXTCL_MAPPER_QUIT(072);
	EXTCL_CPU_WR_MEM(072);
	EXTCL_SAVE_MAPPER(072);
	EXTCL_CPU_EVERY_CYCLE(072);
	mapper.internal_struct[0] = (BYTE *)&m072;
	mapper.internal_struct_size[0] = sizeof(m072);

	if (info.reset >= HARD) {
		memset(&m072, 0x00, sizeof(m072));
	}

	if ((info.reset == CHANGE_ROM) || (info.reset == POWER_UP)) {
		wavefiles_clear();
		if ((info.mapper.submapper == 0) && (miscrom_size() >= S32K)) {
			upd7756_load_sample_rom(miscrom_pnt(), miscrom_size());
		} else {
			// TODO: I need WAV files to implement their playback
		}
	}
}
void extcl_after_mapper_init_072(void) {
	prg_fix_072();
	chr_fix_072();
}
void extcl_mapper_quit_072(void) {
	wavefiles_clear();
}
void extcl_cpu_wr_mem_072(BYTE nidx, WORD address, BYTE value) {
	// bus conflict
	value &= prgrom_rd(nidx, address);

	m072.reg = (m072.reg ^ value) & value;
	if (m072.reg & 0x80) {
		m072.prg = value;
		prg_fix_072();
	}
	if (m072.reg & 0x40) {
		m072.chr = value;
		chr_fix_072();
	}
	if (!(value & 0x10)) {
		m072.snd.speech = address & 0x1F;
		wavefiles_restart(m072.snd.speech);
		m072.snd.playing = TRUE;
	}
	if (!(value & 0x20)) {
		m072.snd.playing = FALSE;
	}
}
BYTE extcl_save_mapper_072(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m072.prg);
	save_slot_ele(mode, slot, m072.chr);
	save_slot_ele(mode, slot, m072.reg);
	save_slot_ele(mode, slot, m072.snd.speech);
	save_slot_ele(mode, slot, m072.snd.playing);
	save_slot_ele(mode, slot, m072.snd.out);

	return (EXIT_OK);
}
void extcl_cpu_every_cycle_072(UNUSED(BYTE nidx)) {
	m072.snd.out = 0;
	if (m072.snd.playing) {
		m072.snd.out = (SWORD)(wavefiles_get_next_sample(m072.snd.speech) >> 7);
		m072.snd.playing = !wavefiles_is_finished(m072.snd.speech);
	}
}

INLINE static void prg_fix_072(void) {
	if (info.mapper.id == 92) {
		memmap_auto_16k(0, MMCPU(0x8000), 0x00);
		memmap_auto_16k(0, MMCPU(0xC000), m072.prg);
	} else {
		memmap_auto_16k(0, MMCPU(0x8000), m072.prg);
		memmap_auto_16k(0, MMCPU(0xC000), 0xFF);
	}
}
INLINE static void chr_fix_072(void) {
	memmap_auto_8k(0, MMPPU(0x0000), m072.chr);
}
