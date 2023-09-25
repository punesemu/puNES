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

INLINE static void prg_fix_003(void);
INLINE static void chr_fix_003(void);
INLINE static void wram_fix_003(void);

_m003 m003;

void map_init_003() {
	EXTCL_AFTER_MAPPER_INIT(003);
	EXTCL_MAPPER_QUIT(003);
	EXTCL_CPU_WR_MEM(003);
	EXTCL_SAVE_MAPPER(003);
	EXTCL_CPU_EVERY_CYCLE(003);
	mapper.internal_struct[0] = (BYTE *)&m003;
	mapper.internal_struct_size[0] = sizeof(m003);

	if (info.reset >= HARD) {
		memset(&m003, 0x00, sizeof(m003));
	}

	if (info.crc32.prg == 0xF8DA2506) {
		// TODO: I need WAV files to implement their playback
		info.mapper.extend_wr = TRUE;
	}
}
void extcl_after_mapper_init_003(void) {
	prg_fix_003();
	chr_fix_003();
	wram_fix_003();
}
void extcl_mapper_quit_003(void) {
	wavefiles_clear();
}
void extcl_cpu_wr_mem_003(BYTE nidx, WORD address, BYTE value) {
	if ((address >= 0x6000) && (address <= 0x7FFF)) {
		m003.snd.speech = value;
		if (!(m003.snd.speech & 0x40) && !m003.snd.playing) {
			wavefiles_restart(m003.snd.speech & 0x07);
			m003.snd.playing = TRUE;
		}
		return;
	} else if (address >= 0x8000) {
		// bus conflict
		if (info.mapper.submapper == 2) {
			value &= prgrom_rd(nidx, address);
		}
		m003.reg = value;
		chr_fix_003();
	}
}
BYTE extcl_save_mapper_003(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m003.reg);
	save_slot_ele(mode, slot, m003.snd.speech);
	save_slot_ele(mode, slot, m003.snd.playing);
	save_slot_ele(mode, slot, m003.snd.out);

	return (EXIT_OK);
}
void extcl_cpu_every_cycle_003(UNUSED(BYTE nidx)) {
	m003.snd.out = 0;
	if (m003.snd.playing) {
		BYTE speech = m003.snd.speech & 0x07;

		m003.snd.out = (SWORD)(wavefiles_get_next_sample(speech) >> 7);
		m003.snd.playing = !wavefiles_is_finished(speech);
	}
}

INLINE static void prg_fix_003(void) {
	memmap_auto_32k(0, MMCPU(0x8000), 0);
}
INLINE static void chr_fix_003(void) {
	memmap_auto_8k(0, MMPPU(0x0000), m003.reg);
}
INLINE static void wram_fix_003(void) {
	memmap_wram_8k(0, MMCPU(0x6000), 0);
}

