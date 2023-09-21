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

#include "nsf.h"
#include "mappers.h"
#include "fds.h"
#include "clock.h"
#include "save_slot.h"
#include "gui.h"
#include "conf.h"
#include "draw_on_screen.h"

void map_init_NSF(void) {
	BYTE internal_struct = 0;

	EXTCL_AFTER_MAPPER_INIT(NSF);
	EXTCL_SAVE_MAPPER(NSF);
	EXTCL_LENGTH_CLOCK(NSF);
	EXTCL_ENVELOPE_CLOCK(NSF);
	EXTCL_APU_TICK(NSF);
	EXTCL_AUDIO_SAMPLES_MOD(nsf);
	mapper.internal_struct[internal_struct] = (BYTE *)&nsf;
	mapper.internal_struct_size[internal_struct] = sizeof(nsf);
	internal_struct++;

	if ((info.reset == CHANGE_ROM) || (info.reset == POWER_UP)) {
		memmap_prg_region_init(0, S4K);
	}

	nsf.made_tick = FALSE;

	if (info.reset >= HARD) {
		if (cfg->nsf_player_nsfe_playlist && (nsf.playlist.count > 0)) {
			nsf.songs.current = nsf.playlist.starting;
			nsf.playlist.index = 0;

			nsf.scroll_info_nsf.pixel = 0;

			nsf.scroll_title_song.pixel = 0;

			nsf.curtain_title_song.index = 0;
			nsf.curtain_title_song.pause = FALSE;
			nsf.curtain_title_song.redraw.all = TRUE;
			nsf.curtain_title_song.redraw.bottom = 0;
			nsf.curtain_title_song.timer = nsf.curtain_title_song.reload.r1;
			nsf.curtain_title_song.borders.bottom = 0;

			nsf.curtain_info.index = 0;
			nsf.curtain_info.pause = FALSE;
			nsf.curtain_info.redraw.all = TRUE;
			nsf.curtain_info.redraw.bottom = 0;
			nsf.curtain_info.timer = nsf.curtain_info.reload.r1;
			nsf.curtain_info.borders.bottom = 0;
		} else {
			nsf.songs.current = nsf.songs.starting - 1;
		}
	}

	nsf.frames = 0;

	nsf.made_tick = TRUE;
	nsf.state = NSF_PLAY | NSF_CHANGE_SONG;

	nsf.routine.INT_NMI = 2;
	nsf.routine.INT_RESET = 2;

	nsf.draw_mask_frames = 2;

	if (machine.type == NTSC) {
		nsf.rate.reload = (DBWORD)(machine.cpu_hz / (1000000.0f / (double)nsf.play_speed.ntsc));
	} else {
		nsf.rate.reload = (DBWORD)(machine.cpu_hz / (1000000.0f / (double)nsf.play_speed.pal));
	}

	nsf.rate.count = nsf.rate.reload;

	if (nsf.sound_chips.vrc6) {
		mapper.internal_struct[internal_struct] = (BYTE *)&vrc6;
		mapper.internal_struct_size[internal_struct] = sizeof(vrc6);
		internal_struct++;

		init_NSF_VRC6(0x01, 0x02);
	}
	if (nsf.sound_chips.vrc7) {
		mapper.internal_struct[internal_struct] = (BYTE *)&vrc7;
		mapper.internal_struct_size[internal_struct] = sizeof(vrc7);
		internal_struct++;

		init_NSF_VRC7(0x10, 0x20);
	}
	if (nsf.sound_chips.fds) {
		mapper.internal_struct[internal_struct] = (BYTE *)&fds;
		mapper.internal_struct_size[internal_struct] = sizeof(fds);
		internal_struct++;

		map_init_NSF_FDS();
	}
	if (nsf.sound_chips.mmc5) {
		mapper.internal_struct[internal_struct] = (BYTE *)&m005;
		mapper.internal_struct_size[internal_struct] = sizeof(m005);
		internal_struct++;

		map_init_NSF_005();
	}
	if (nsf.sound_chips.namco163) {
		mapper.internal_struct[internal_struct] = (BYTE *)&m019;
		mapper.internal_struct_size[internal_struct] = sizeof(m019);
		internal_struct++;

		map_init_NSF_N163();
	}
	if (nsf.sound_chips.sunsoft5b) {
		mapper.internal_struct[internal_struct] = (BYTE *)&fme7;
		mapper.internal_struct_size[internal_struct] = sizeof(fme7);
		internal_struct++;

		init_NSF_FME7();
	}

	nsf_main_screen();
}
void extcl_after_mapper_init_NSF(void) {
	nsf_reset_prg();
}
BYTE extcl_save_mapper_NSF(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, nsf.type);
	save_slot_ele(mode, slot, nsf.state);
	save_slot_ele(mode, slot, nsf.made_tick);
	save_slot_ele(mode, slot, nsf.frames);

	save_slot_ele(mode, slot, nsf.rate.count);

	save_slot_ele(mode, slot, nsf.songs.current);

	save_slot_ele(mode, slot, nsf.routine.prg);
	save_slot_ele(mode, slot, nsf.routine.INT_NMI);
	save_slot_ele(mode, slot, nsf.routine.INT_RESET);

	save_slot_ele(mode, slot, nsf.timers.button);
	save_slot_ele(mode, slot, nsf.timers.total_rom);
	save_slot_ele(mode, slot, nsf.timers.song);

	save_slot_ele(mode, slot, nsf.songs.started);

	save_slot_ele(mode, slot, nsf.timers.fadeout);
	save_slot_ele(mode, slot, nsf.timers.silence);

	save_slot_ele(mode, slot, nsf.playlist.index);

	if (mode == SAVE_SLOT_READ) {
		nsf_reset_song_title();
		nsf_reset_timers();

		nsf.curtain_title_song.redraw.all = TRUE;
		nsf.curtain_title_song.redraw.bottom = nsf.curtain_title_song.borders.bottom;
		if (nsf.curtain_title_song.pause) {
			nsf.curtain_title_song.redraw.bottom = dospf(1);
		}

		nsf.curtain_info.redraw.all = TRUE;
		nsf.curtain_info.redraw.bottom = nsf.curtain_info.borders.bottom;
		if (nsf.curtain_info.pause) {
			nsf.curtain_info.redraw.bottom = dospf(1);
		}
	}

	if (nsf.sound_chips.vrc6) {
		if (extcl_save_mapper_VRC6(mode, slot, fp) == EXIT_ERROR) return (EXIT_ERROR);
	}
	if (nsf.sound_chips.vrc7) {
		if (extcl_save_mapper_VRC7(mode, slot, fp) == EXIT_ERROR) return (EXIT_ERROR);
	}
	if (nsf.sound_chips.fds) {
		save_slot_ele(mode, slot, fds.snd.wave.data);
		save_slot_ele(mode, slot, fds.snd.wave.writable);
		save_slot_ele(mode, slot, fds.snd.wave.volume);
		save_slot_ele(mode, slot, fds.snd.wave.index);
		save_slot_ele(mode, slot, fds.snd.wave.counter);

		save_slot_ele(mode, slot, fds.snd.envelope.speed);
		save_slot_ele(mode, slot, fds.snd.envelope.disabled);

		save_slot_ele(mode, slot, fds.snd.main.silence);
		save_slot_ele(mode, slot, fds.snd.main.frequency);
		save_slot_ele(mode, slot, fds.snd.main.output);

		save_slot_ele(mode, slot, fds.snd.volume.speed);
		save_slot_ele(mode, slot, fds.snd.volume.mode);
		save_slot_ele(mode, slot, fds.snd.volume.increase);
		save_slot_ele(mode, slot, fds.snd.volume.gain);
		save_slot_ele(mode, slot, fds.snd.volume.counter);

		save_slot_ele(mode, slot, fds.snd.sweep.bias);
		save_slot_ele(mode, slot, fds.snd.sweep.mode);
		save_slot_ele(mode, slot, fds.snd.sweep.increase);
		save_slot_ele(mode, slot, fds.snd.sweep.speed);
		save_slot_ele(mode, slot, fds.snd.sweep.gain);
		save_slot_ele(mode, slot, fds.snd.sweep.counter);

		save_slot_ele(mode, slot, fds.snd.modulation.data);
		save_slot_ele(mode, slot, fds.snd.modulation.frequency);
		save_slot_ele(mode, slot, fds.snd.modulation.disabled);
		save_slot_ele(mode, slot, fds.snd.modulation.index);
		save_slot_ele(mode, slot, fds.snd.modulation.counter);
		save_slot_ele(mode, slot, fds.snd.modulation.mod);
	}
	if (nsf.sound_chips.mmc5) {
		if (extcl_save_mapper_005(mode, slot, fp) == EXIT_ERROR) return (EXIT_ERROR);
	}
	if (nsf.sound_chips.namco163) {
		if (extcl_save_mapper_019(mode, slot, fp) == EXIT_ERROR) return (EXIT_ERROR);
	}
	if (nsf.sound_chips.sunsoft5b) {
		if (extcl_save_mapper_FME7(mode, slot, fp) == EXIT_ERROR) return (EXIT_ERROR);
	}

	return (EXIT_OK);
}
void extcl_length_clock_NSF(void) {
	if (nsf.sound_chips.mmc5) {
		extcl_length_clock_005();
	}
}
void extcl_envelope_clock_NSF(void) {
	if (nsf.sound_chips.mmc5) {
		extcl_envelope_clock_005();
	}
}
void extcl_apu_tick_NSF(void) {
	if (nsf.sound_chips.vrc6) {
		extcl_apu_tick_VRC6();
	}
	if (nsf.sound_chips.vrc7) {
		extcl_apu_tick_VRC7();
	}
	if (nsf.sound_chips.fds) {
		extcl_apu_tick_FDS();
	}
	if (nsf.sound_chips.mmc5) {
		extcl_apu_tick_005();
	}
	if (nsf.sound_chips.namco163) {
		extcl_apu_tick_019();
	}
	if (nsf.sound_chips.sunsoft5b) {
		extcl_apu_tick_FME7();
	}
}
