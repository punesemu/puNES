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

#include "nsf.h"
#include "mappers.h"
#include "fds.h"
#include "clock.h"
#include "save_slot.h"
#include "gui.h"
#include "conf.h"
#include "video/gfx.h"
#include "fps.h"

INLINE static void select_region_NSF(void);

void map_init_NSF(void) {
	EXTCL_AFTER_MAPPER_INIT(NSF);
	EXTCL_SAVE_MAPPER(NSF);
	EXTCL_CPU_EVERY_CYCLE(NSF);
	EXTCL_LENGTH_CLOCK(NSF);
	EXTCL_ENVELOPE_CLOCK(NSF);
	EXTCL_APU_TICK(NSF);
	EXTCL_AUDIO_SAMPLES_MOD(nsf);
	map_internal_struct_init((BYTE *)&nsf, sizeof(nsf));

	if ((info.reset == CHANGE_ROM) || (info.reset == POWER_UP)) {
		memmap_prg_region_init(0, S4K);
	}

	if (info.reset >= HARD) {
		if (cfg->nsf_player_playlist && (nsf.playlist.count > 0)) {
			nsf.songs.current = nsf.playlist.starting;
			nsf.playlist.index = 0;

			nsf.curtain_title_song.index = 0;
			nsf.curtain_title_song.pause = FALSE;
			nsf.curtain_title_song.redraw.all = TRUE;
			nsf.curtain_title_song.timer = nsf.curtain_title_song.reload.r1;

			nsf.curtain_info.index = 0;
			nsf.curtain_info.pause = FALSE;
			nsf.curtain_info.redraw.all = TRUE;
			nsf.curtain_info.timer = nsf.curtain_info.reload.r1;
		} else {
			nsf.songs.current = nsf.songs.starting - 1;
		}
	}

	nsf.state = NSF_PLAY | NSF_CHANGE_SONG;
	nsf.timers.update_only_diff = FALSE;

	switch (machine.type) {
		default:
		case NTSC:
			nsf.type = NSF_NTSC_MODE;
			break;
		case PAL:
			nsf.type = NSF_PAL_MODE;
			break;
		case DENDY:
			nsf.type = NSF_DENDY_MODE;
			break;
	}

	select_region_NSF();
	nsf.rate.count = nsf.rate.reload;
	nsf.nmi.count = nsf.rate.reload;
	nsf.nmi.in_use = FALSE;

	nsf.adr.loop = NSF_ROUTINE_LOOP;

	if (nsf.sound_chips.vrc6) {
		map_internal_struct_init((BYTE *)&vrc6, sizeof(vrc6));
		init_NSF_VRC6(0x01, 0x02);
	}
	if (nsf.sound_chips.vrc7) {
		map_internal_struct_init((BYTE *)&vrc7, sizeof(vrc7));
		init_NSF_VRC7(0x10, 0x20);
	}
	if (nsf.sound_chips.fds) {
		map_internal_struct_init((BYTE *)&fds, sizeof(fds));
		map_init_NSF_FDS();
	}
	if (nsf.sound_chips.mmc5) {
		map_internal_struct_init((BYTE *)&m005, sizeof(m005));
		map_init_NSF_005();
	}
	if (nsf.sound_chips.namco163) {
		map_internal_struct_init((BYTE *)&m019, sizeof(m019));
		map_init_NSF_N163();
	}
	if (nsf.sound_chips.sunsoft5b) {
		map_internal_struct_init((BYTE *)&fme7, sizeof(fme7));
		init_NSF_FME7();
	}

	nsf2.irq.enable = 0;
	nsf2.irq.reload = 0;
	nsf2.irq.counter = 0;

	nsf_main_screen();
}
void extcl_after_mapper_init_NSF(void) {
	info.disable_tick_hw = TRUE;
	nsf_reset_prg();
	// For convenience, before INIT the host system should initialize the IRQ vector RAM with the starting contents of $FFFE-$FFFF
	nsf2.irq.vector[0] = prgrom_rd(0, 0xFFFE);
	nsf2.irq.vector[1] = prgrom_rd(0, 0xFFFF);
	info.disable_tick_hw = FALSE;
}
BYTE extcl_save_mapper_NSF(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, nsf.type);
	save_slot_ele(mode, slot, nsf.state);

	save_slot_ele(mode, slot, nsf.rate.count);

	save_slot_ele(mode, slot, nsf.nmi.in_use);
	save_slot_ele(mode, slot, nsf.nmi.count);

	save_slot_ele(mode, slot, nsf.songs.current);

	save_slot_ele(mode, slot, nsf.adr.loop);

	save_slot_ele(mode, slot, nsf.timers.button);
	save_slot_ele(mode, slot, nsf.timers.total_rom);
	save_slot_ele(mode, slot, nsf.timers.song);

	save_slot_ele(mode, slot, nsf.songs.started);

	save_slot_ele(mode, slot, nsf.timers.fadeout);
	save_slot_ele(mode, slot, nsf.timers.silence);

	save_slot_ele(mode, slot, nsf.playlist.index);

	if (nsf2.features.irq_support) {
		save_slot_ele(mode, slot, nsf2.irq.enable);
		save_slot_ele(mode, slot, nsf2.irq.reload);
		save_slot_ele(mode, slot, nsf2.irq.counter);
		save_slot_ele(mode, slot, nsf2.irq.vector);
	}

	if (mode == SAVE_SLOT_READ) {
		select_region_NSF();
		nsf_reset_song_title();
		nsf_reset_timers();
		nsf.curtain_title_song.redraw.all = TRUE;
		nsf.curtain_info.redraw.all = TRUE;
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
void extcl_cpu_every_cycle_NSF(BYTE nidx) {
	if (nsf.nmi.count && (--nsf.nmi.count == 0)) {
		nsf.nmi.count = nsf.nmi.reload;

		nes[0].c.nmi.high = !nsf.nmi.in_use;
		nes[0].p.ppu.odd_frame = !nes[0].p.ppu.odd_frame;
		nes[0].p.ppu.frames++;

		info.exec_cpu_op.b[0] = FALSE;

		nsf_main_screen_event();
		nsf_effect();
		gfx_draw_screen(0);
		fps_ppu_inc(0);
	}
	if (nsf.rate.count && (--nsf.rate.count == 0)) {
		if (!nsf2.features.non_returning_init) {
			nsf.adr.loop = NSF_ROUTINE_NORMAL;
		}
	}
	if (nsf2.irq.enable) {
		nsf2.irq.counter--;
		if (nsf2.irq.counter == 0) {
			nsf2.irq.counter = nsf2.irq.reload;
			nes[nidx].c.irq.high |= EXT_IRQ;
		}
	} else {
		nsf2.irq.counter = nsf2.irq.reload;
	}
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

INLINE static void select_region_NSF(void) {
	double rate = 0, nmi_rate = 0;

	if (machine.type == NTSC) {
		rate = nsf.play_speed.ntsc;
		nmi_rate = 0x40FF;
	} else if (machine.type == DENDY) {
		rate = nsf.play_speed.dendy;
		nmi_rate = 0x4E1D;
	} else {
		rate = nsf.play_speed.pal;
		nmi_rate = 0x4E1D;
	}
	nsf.rate.reload = (DBWORD)(machine.cpu_hz / (1000000.0f / rate));
	nsf.nmi.reload = (DBWORD)(machine.cpu_hz / (1000000.0f / nmi_rate));
}