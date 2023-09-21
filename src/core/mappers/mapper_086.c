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
#include "hc55516_interface.h"
#include "wave_file_interface.h"
#include "upd7756_interface.h"

INLINE static void prg_fix_086(void);
INLINE static void chr_fix_086(void);

_m086 m086;
struct _m086tmp {
	BYTE snd_type;
	hc55516 *um_chip;
	SWORD um_output[256];
} m086tmp;

void map_init_086(void) {
	EXTCL_AFTER_MAPPER_INIT(086);
	EXTCL_MAPPER_QUIT(086);
	EXTCL_CPU_WR_MEM(086);
	EXTCL_SAVE_MAPPER(086);
	EXTCL_CPU_EVERY_CYCLE(086);
	mapper.internal_struct[0] = (BYTE *)&m086;
	mapper.internal_struct_size[0] = sizeof(m086);

	if (info.reset >= HARD) {
		memset(&m086, 0x00, sizeof(m086));
	}

	if ((info.reset == CHANGE_ROM) || (info.reset == POWER_UP)) {
		memset(&m086tmp, 0x00, sizeof(m086tmp));
		wavefiles_clear();
		if ((info.mapper.submapper == 1) && (miscrom_size() >= S32K)) {
			m086tmp.snd_type = 1;
			m086tmp.um_chip = hc55516_create(0);
		} else if ((info.mapper.submapper == 0) && (miscrom_size() >= S32K)) {
			m086tmp.snd_type = 2;
			upd7756_load_sample_rom(miscrom_pnt(), miscrom_size());
		} else {
			// TODO: I need WAV files to implement their playback
		}
	}

	info.mapper.extend_wr = TRUE;
}
void extcl_after_mapper_init_086(void) {
	prg_fix_086();
	chr_fix_086();
}
void extcl_mapper_quit_086(void) {
	if (m086tmp.um_chip) {
		hc55516_free(m086tmp.um_chip);
		m086tmp.um_chip = NULL;
	}
	wavefiles_clear();
}
void extcl_cpu_wr_mem_086(UNUSED(BYTE cidx), WORD address, BYTE value) {
	switch (address & 0xF000) {
		case 0x5000:
			if (m086tmp.snd_type == 1) {
				m086.snd.um_rate = value;
			}
			return;
		case 0x6000:
			m086.reg = value;
			prg_fix_086();
			chr_fix_086();
			return;
		case 0x7000:
			if (m086tmp.snd_type == 1) {
				m086.snd.speech = value;
				if (!(m086.snd.speech & 0x10)) {
					hc55516_start(m086tmp.um_chip);
					hc55516_reset(m086tmp.um_chip);
					m086.snd.um_sample = 0;
					m086.snd.playing = TRUE;
				}
			} else if (m086tmp.snd_type == 2) {
				m086.snd.speech = value;
				if (!(m086.snd.speech & 0x10)) {
					wavefiles_restart(m086.snd.speech & 0x0F);
					m086.snd.playing = TRUE;
				}
				if (!(m086.snd.speech & 0x20)) {
					m086.snd.playing = FALSE;
				}
			}
			return;
		default:
			return;
	}
}
BYTE extcl_save_mapper_086(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m086.reg);
	save_slot_ele(mode, slot, m086.snd.speech);
	save_slot_ele(mode, slot, m086.snd.playing);
	save_slot_ele(mode, slot, m086.snd.out);
	if (m086tmp.snd_type == 1) {
		save_slot_ele(mode, slot, m086.snd.um_rate);
		save_slot_ele(mode, slot, m086.snd.um_sample);
		save_slot_ele(mode, slot, m086.snd.um_count);
	}

	return (EXIT_OK);
}
void extcl_cpu_every_cycle_086(UNUSED(BYTE cidx)) {
	if (m086tmp.snd_type == 1) {
		int period = (m086.snd.um_rate & 0x80 ? 176 : 88);

		if (m086.snd.playing) {
			if (!(m086.snd.um_count % period)) {
				int digit = miscrom_byte(((m086.snd.speech & 0x0F) << 11) | (m086.snd.um_sample >> 3)) >> (~m086.snd.um_sample & 0x07);

				hc55516_digit_w(m086tmp.um_chip, digit);
				hc55516_clock_w(m086tmp.um_chip, 1);
				hc55516_clock_w(m086tmp.um_chip, 0);
				hc55516_sound_stream_update(m086tmp.um_chip, &m086tmp.um_output[0], period);
				if (++m086.snd.um_sample & 0x4000) {
					m086.snd.playing = FALSE;
				}
			}
		}
		m086.snd.out = m086.snd.playing ? (SWORD)(m086tmp.um_output[(m086.snd.um_count++ % period)] >> 7): 0;
	} else if (m086tmp.snd_type == 2) {
		m086.snd.out = 0;
		if (m086.snd.playing) {
			int message = (m086.snd.speech & 0x0F);

			m086.snd.out = (SWORD)(wavefiles_get_next_sample(message) >> 7);
			m086.snd.playing = !wavefiles_is_finished(message);
		}
	}
}

INLINE static void prg_fix_086(void) {
	memmap_auto_32k(0, MMCPU(0x8000), ((m086.reg & 0x30) >> 4));
}
INLINE static void chr_fix_086(void) {
	memmap_auto_8k(0, MMPPU(0x0000), ((m086.reg & 0x40) >> 4) | (m086.reg & 0x03));
}
