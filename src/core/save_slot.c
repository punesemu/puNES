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

#include <stdlib.h>
#include <string.h>
#include "save_slot.h"
#include "mappers.h"
#include "conf.h"
#include "cpu.h"
#include "ppu.h"
#include "irqA12.h"
#include "irql2f.h"
#include "bck_states.h"
#include "rewind.h"
#include "video/gfx.h"
#include "video/gfx_thread.h"
#include "emu_thread.h"
#include "gui.h"
#include "tas.h"
#include "fds.h"
#include "nsf.h"
#include "cheat.h"

#define SAVE_VERSION 100

static BYTE mem_with_size(BYTE mode, BYTE slot, BYTE *mem, size_t msize, FILE *fp);
static void preview_image_from_ppu_screen(BYTE slot, _ppu_screen_buffer *sb, void **dst, size_t *size);
static void preview_image_from_png(BYTE slot, void *src, size_t size);
static uTCHAR *name_slot_file(BYTE slot);

typedef uint64_t postype;

_save_slot save_slot;

BYTE save_slot_save(BYTE slot) {
	uTCHAR *file = NULL;
	FILE *fp = NULL;

	// game genie
	if (info.mapper.id == GAMEGENIE_MAPPER) {
		gui_overlay_info_append_msg_precompiled(13, NULL);
		return (EXIT_ERROR);
	}

	if (slot < SAVE_SLOT_FILE) {
		file = name_slot_file(slot);
		if (file == NULL) {
			return (EXIT_ERROR);
		}
	} else {
		file = cfg->save_file;
	}

	fp = ufopen(file, uL("wb"));
	if (fp == NULL) {
		log_error(uL("save slot;error on write save state"));
		return (EXIT_ERROR);
	}

	// devo calcolarmi il tot_size
	save_slot_operation(SAVE_SLOT_COUNT, slot, NULL);

	save_slot.slot[slot].state = (save_slot_operation(SAVE_SLOT_SAVE, slot, fp) == EXIT_OK);

	fclose(fp);

	if (!save_slot.slot[slot].state) {
		gui_overlay_info_append_msg_precompiled(38, NULL);
		log_error(uL("save slot;error on write save state"));
		if (emu_file_exist(file) == EXIT_OK) {
			uremove(file);
		}
	} else if (slot < SAVE_SLOT_FILE) {
		gui_overlay_enable_save_slot(SAVE_SLOT_SAVE);
	}

	return (EXIT_OK);
}
BYTE save_slot_load(BYTE slot) {
	uTCHAR *file = NULL;
	FILE *fp = NULL;

	if (tas.type) {
		gui_overlay_info_append_msg_precompiled(14, NULL);
		tas_quit();
	}

	// game genie
	if (info.mapper.id == GAMEGENIE_MAPPER) {
		gamegenie_reset();
		gamegenie.phase = GG_LOAD_ROM;
		emu_reset(CHANGE_ROM);
		gamegenie.phase = GG_FINISH;
	}

	if (slot < SAVE_SLOT_FILE) {
		file = name_slot_file(slot);
		if (file == NULL) {
			return (EXIT_ERROR);
		}
	} else {
		file = cfg->save_file;
	}

	fp = ufopen(file, uL("rb"));
	if (fp == NULL) {
		gui_overlay_info_append_msg_precompiled(15, NULL);
		log_error(uL("save slot;error loading state"));
		return (EXIT_ERROR);
	}

	// mi salvo lo stato attuale da ripristinare in caso
	// di un file di salvataggio corrotto.
	rewind_save_state_snap(BCK_STATES_OP_SAVE_ON_MEM);

	save_slot_operation(SAVE_SLOT_COUNT, slot, fp);

	if (info.crc32.total != save_slot.crc32) {
		gui_overlay_info_append_msg_precompiled(16, NULL);
		log_error(uL("save slot;state file is not for this rom"));
		rewind_save_state_snap(BCK_STATES_OP_READ_FROM_MEM);
		fclose(fp);
		return (EXIT_ERROR);
	}

	if (save_slot_operation(SAVE_SLOT_READ, slot, fp)) {
		int corrupted = slot;

		gui_overlay_info_append_msg_precompiled(30, &corrupted);
		log_error(uL("save slot;error loading state, corrupted file"));
		rewind_save_state_snap(BCK_STATES_OP_READ_FROM_MEM);
		fclose(fp);
		return (EXIT_ERROR);
	}

	fclose(fp);

	if (slot < SAVE_SLOT_FILE) {
		gui_overlay_enable_save_slot(SAVE_SLOT_READ);
	}

	//riavvio il rewind
	rewind_init();

	return (EXIT_OK);
}
void save_slot_count_load(void) {
	uTCHAR *file = NULL;
	int i = 0;

	emu_thread_pause();
	gfx_thread_pause();

	for (i = 0; i < SAVE_SLOTS; i++) {
		save_slot.slot[i].tot_size = 0;

		save_slot.slot[i].state = FALSE;
		file = name_slot_file(i);

		if (file && (emu_file_exist(file) == EXIT_OK)) {
			FILE *fp = NULL;

			fp = ufopen(file, uL("rb"));
			if (fp == NULL) {
				continue;
			}

			save_slot.slot[i].state = (save_slot_operation(SAVE_SLOT_COUNT, i, fp) == EXIT_OK);

			fclose(fp);
		}

		if (!save_slot.slot[i].state) {
			gui_overlay_slot_preview_set_from_ppu_screen(i, NULL, NULL);
			gui_state_save_slot_set_tooltip(i);
		}
	}

	if (!save_slot.slot[save_slot.slot_in_use].state) {
		save_slot.slot_in_use = 0;

		for (i = 0; i < SAVE_SLOTS; i++) {
			if (save_slot.slot[i].state) {
				save_slot.slot_in_use = i;
			}
		}
	}

	gui_state_save_slot_set(save_slot.slot_in_use, FALSE);

	gfx_thread_continue();
	emu_thread_continue();
}
BYTE save_slot_operation(BYTE mode, BYTE slot, FILE *fp) {
	unsigned int i = 0;

	if (fp) {
		fseek(fp, 0L, SEEK_SET);
	}

	save_slot.version = SAVE_VERSION;

	if (mode == SAVE_SLOT_COUNT) {
		save_slot.slot[slot].tot_size = 0;
	} else if (mode == SAVE_SLOT_READ) {
		save_slot.slot[slot].tot_size = 0;
	} else {
		save_slot.crc32 = info.crc32.total;
		save_slot.size = save_slot.slot[slot].tot_size;
	}

	// forzo la lettura perche' devo sapere la
	// versione del file di salvataggio e le informazioni
	// della rom.
	save_slot_int(mode == SAVE_SLOT_COUNT ? SAVE_SLOT_READ : mode, slot, save_slot.version);
	save_slot_int(mode == SAVE_SLOT_COUNT ? SAVE_SLOT_READ : mode, slot, save_slot.crc32);
	save_slot_int(mode == SAVE_SLOT_COUNT ? SAVE_SLOT_READ : mode, slot, save_slot.size);

	if (fp && ((mode == SAVE_SLOT_COUNT) || (mode == SAVE_SLOT_READ))) {
		if ((save_slot.version < 100) || (info.crc32.total != save_slot.crc32)) {
			return (EXIT_ERROR);
		}
	}

	// cpu
	save_slot_ele(mode, slot, cpu.PC);
	save_slot_ele(mode, slot, cpu.SP);
	save_slot_ele(mode, slot, cpu.AR);
	save_slot_ele(mode, slot, cpu.XR);
	save_slot_ele(mode, slot, cpu.YR);
	save_slot_ele(mode, slot, cpu.SR);
	save_slot_ele(mode, slot, cpu.cf);
	save_slot_ele(mode, slot, cpu.zf);
	save_slot_ele(mode, slot, cpu.im);
	save_slot_ele(mode, slot, cpu.df);
	save_slot_ele(mode, slot, cpu.bf);
	save_slot_ele(mode, slot, cpu.of);
	save_slot_ele(mode, slot, cpu.sf);
	save_slot_ele(mode, slot, cpu.opcode);
	save_slot_ele(mode, slot, cpu.opcode_PC);
	save_slot_ele(mode, slot, cpu.odd_cycle);
	save_slot_ele(mode, slot, cpu.openbus);
	save_slot_ele(mode, slot, cpu.cycles);
	save_slot_ele(mode, slot, cpu.opcode_cycle);
	save_slot_ele(mode, slot, cpu.double_rd);
	save_slot_ele(mode, slot, cpu.double_wr);
	save_slot_ele(mode, slot, cpu.prg_ram_rd_active);
	save_slot_ele(mode, slot, cpu.prg_ram_wr_active);
	save_slot_ele(mode, slot, cpu.base_opcode_cycles);

	// irq
	save_slot_ele(mode, slot, irq.high);
	save_slot_ele(mode, slot, irq.delay);
	save_slot_ele(mode, slot, irq.before);
	save_slot_ele(mode, slot, irq.inhibit);
	// nmi
	save_slot_ele(mode, slot, nmi.high);
	save_slot_ele(mode, slot, nmi.delay);
	save_slot_ele(mode, slot, nmi.before);
	save_slot_ele(mode, slot, nmi.inhibit);
	save_slot_ele(mode, slot, nmi.frame_x);
	save_slot_ele(mode, slot, nmi.cpu_cycles_from_last_nmi);

	// ppu
	save_slot_ele(mode, slot, ppu.frame_x);
	save_slot_ele(mode, slot, ppu.frame_y);
	save_slot_ele(mode, slot, ppu.fine_x);
	save_slot_ele(mode, slot, ppu.screen_y);
	save_slot_ele(mode, slot, ppu.pixel_tile);
	save_slot_ele(mode, slot, ppu.sline_cycles);
	save_slot_ele(mode, slot, ppu.tmp_vram);
	save_slot_ele(mode, slot, ppu.spr_adr);
	save_slot_ele(mode, slot, ppu.bck_adr);
	save_slot_ele(mode, slot, ppu.openbus);
	save_slot_ele(mode, slot, ppu.odd_frame);
	save_slot_ele(mode, slot, ppu.cycles);
	save_slot_ele(mode, slot, ppu.frames);
	save_slot_ele(mode, slot, ppu.sf.actual);
	save_slot_ele(mode, slot, ppu.sf.prev);
	save_slot_ele(mode, slot, ppu.rnd_adr);
	// ppu_openbus
	save_slot_ele(mode, slot, ppu_openbus.bit0);
	save_slot_ele(mode, slot, ppu_openbus.bit1);
	save_slot_ele(mode, slot, ppu_openbus.bit2);
	save_slot_ele(mode, slot, ppu_openbus.bit3);
	save_slot_ele(mode, slot, ppu_openbus.bit4);
	save_slot_ele(mode, slot, ppu_openbus.bit5);
	save_slot_ele(mode, slot, ppu_openbus.bit6);
	save_slot_ele(mode, slot, ppu_openbus.bit7);
	// r2000
	save_slot_ele(mode, slot, r2000.value);
	save_slot_ele(mode, slot, r2000.nmi_enable);
	save_slot_ele(mode, slot, r2000.size_spr);
	save_slot_ele(mode, slot, r2000.r2006_inc);
	save_slot_ele(mode, slot, r2000.spt_adr);
	save_slot_ele(mode, slot, r2000.bpt_adr);
	save_slot_ele(mode, slot, r2000.race.ctrl);
	save_slot_ele(mode, slot, r2000.race.value);
	// r2001
	save_slot_ele(mode, slot, r2001.value);
	save_slot_ele(mode, slot, r2001.emphasis);
	save_slot_ele(mode, slot, r2001.visible);
	save_slot_ele(mode, slot, r2001.bck_visible);
	save_slot_ele(mode, slot, r2001.spr_visible);
	save_slot_ele(mode, slot, r2001.bck_clipping);
	save_slot_ele(mode, slot, r2001.spr_clipping);
	save_slot_ele(mode, slot, r2001.color_mode);
	save_slot_ele(mode, slot, r2001.race.ctrl);
	save_slot_ele(mode, slot, r2001.race.value);
	// r2002
	save_slot_ele(mode, slot, r2002.vblank);
	save_slot_ele(mode, slot, r2002.sprite0_hit);
	save_slot_ele(mode, slot, r2002.sprite_overflow);
	save_slot_ele(mode, slot, r2002.toggle);
	save_slot_ele(mode, slot, r2002.race.sprite_overflow);
	// r2003
	save_slot_ele(mode, slot, r2003.value);
	// r2004
	save_slot_ele(mode, slot, r2004.value);
	// r2006
	save_slot_ele(mode, slot, r2006.value);
	save_slot_ele(mode, slot, r2006.changed_from_op);
	save_slot_ele(mode, slot, r2006.race.ctrl);
	save_slot_ele(mode, slot, r2006.race.value);
	// r2007
	save_slot_ele(mode, slot, r2007.value);
	// spr_ev
	save_slot_ele(mode, slot, spr_ev.range);
	save_slot_ele(mode, slot, spr_ev.count);
	save_slot_ele(mode, slot, spr_ev.count_plus);
	save_slot_ele(mode, slot, spr_ev.tmp_spr_plus);
	save_slot_ele(mode, slot, spr_ev.evaluate);
	save_slot_ele(mode, slot, spr_ev.byte_OAM);
	save_slot_ele(mode, slot, spr_ev.index_plus);
	save_slot_ele(mode, slot, spr_ev.index);
	save_slot_ele(mode, slot, spr_ev.timing);
	save_slot_ele(mode, slot, spr_ev.phase);
	save_slot_ele(mode, slot, spr_ev.real);
	// sprite
	for (i = 0; i < LENGTH(sprite); i++) {
		save_slot_ele(mode, slot, sprite[i].y_C);
		save_slot_ele(mode, slot, sprite[i].tile);
		save_slot_ele(mode, slot, sprite[i].attrib);
		save_slot_ele(mode, slot, sprite[i].x_C);
		save_slot_ele(mode, slot, sprite[i].number);
		save_slot_ele(mode, slot, sprite[i].flip_v);
		save_slot_ele(mode, slot, sprite[i].l_byte);
		save_slot_ele(mode, slot, sprite[i].h_byte);
	}
	// sprite_plus
	for (i = 0; i < LENGTH(sprite_plus); i++) {
		save_slot_ele(mode, slot, sprite_plus[i].y_C);
		save_slot_ele(mode, slot, sprite_plus[i].tile);
		save_slot_ele(mode, slot, sprite_plus[i].attrib);
		save_slot_ele(mode, slot, sprite_plus[i].x_C);
		save_slot_ele(mode, slot, sprite_plus[i].number);
		save_slot_ele(mode, slot, sprite_plus[i].flip_v);
		save_slot_ele(mode, slot, sprite_plus[i].l_byte);
		save_slot_ele(mode, slot, sprite_plus[i].h_byte);
	}
	// tile_render
	save_slot_ele(mode, slot, tile_render.attrib);
	save_slot_ele(mode, slot, tile_render.l_byte);
	save_slot_ele(mode, slot, tile_render.h_byte);
	// tile_fetch
	save_slot_ele(mode, slot, tile_fetch.attrib);
	save_slot_ele(mode, slot, tile_fetch.l_byte);
	save_slot_ele(mode, slot, tile_fetch.h_byte);

	// apu
	save_slot_ele(mode, slot, apu.mode);
	save_slot_ele(mode, slot, apu.type);
	save_slot_ele(mode, slot, apu.step);
	save_slot_ele(mode, slot, apu.length_clocked);
	save_slot_ele(mode, slot, apu.DMC);
	save_slot_ele(mode, slot, apu.cycles);
	// r4015
	save_slot_ele(mode, slot, r4015.value);
	// r4017
	save_slot_ele(mode, slot, r4017.value);
	save_slot_ele(mode, slot, r4017.jitter.value);
	save_slot_ele(mode, slot, r4017.jitter.delay);
	save_slot_ele(mode, slot, r4017.reset_frame_delay);
	// S1
	save_slot_square(S1, slot);
	// S2
	save_slot_square(S2, slot);
	// TR
	save_slot_ele(mode, slot, TR.timer);
	save_slot_ele(mode, slot, TR.frequency);
	save_slot_ele(mode, slot, TR.linear.value);
	save_slot_ele(mode, slot, TR.linear.reload);
	save_slot_ele(mode, slot, TR.linear.halt);
	save_slot_ele(mode, slot, TR.length.value);
	save_slot_ele(mode, slot, TR.length.enabled);
	save_slot_ele(mode, slot, TR.length.halt);
	save_slot_ele(mode, slot, TR.sequencer);
	save_slot_ele(mode, slot, TR.output);
	// NS
	save_slot_ele(mode, slot, NS.timer);
	save_slot_ele(mode, slot, NS.frequency);
	save_slot_ele(mode, slot, NS.envelope.enabled);
	save_slot_ele(mode, slot, NS.envelope.divider);
	save_slot_ele(mode, slot, NS.envelope.counter);
	save_slot_ele(mode, slot, NS.envelope.constant_volume);
	save_slot_ele(mode, slot, NS.envelope.delay);
	save_slot_ele(mode, slot, NS.mode);
	save_slot_ele(mode, slot, NS.volume);
	save_slot_ele(mode, slot, NS.shift);
	save_slot_ele(mode, slot, NS.length.value);
	save_slot_ele(mode, slot, NS.length.enabled);
	save_slot_ele(mode, slot, NS.length.halt);
	save_slot_ele(mode, slot, NS.sequencer);
	save_slot_ele(mode, slot, NS.output);
	// DMC
	save_slot_ele(mode, slot, DMC.frequency);
	save_slot_ele(mode, slot, DMC.remain);
	save_slot_ele(mode, slot, DMC.irq_enabled);
	save_slot_ele(mode, slot, DMC.loop);
	save_slot_ele(mode, slot, DMC.rate_index);
	save_slot_ele(mode, slot, DMC.address_start);
	save_slot_ele(mode, slot, DMC.address);
	save_slot_ele(mode, slot, DMC.length);
	save_slot_ele(mode, slot, DMC.counter);
	save_slot_ele(mode, slot, DMC.empty);
	save_slot_ele(mode, slot, DMC.buffer);
	save_slot_ele(mode, slot, DMC.dma_cycle);
	save_slot_ele(mode, slot, DMC.silence);
	save_slot_ele(mode, slot, DMC.shift);
	save_slot_ele(mode, slot, DMC.counter_out);
	save_slot_ele(mode, slot, DMC.output);
	save_slot_ele(mode, slot, DMC.tick_type);

	// mem map
	if (mem_with_size(mode, slot, ram_pnt(), ram_size(), fp) == EXIT_ERROR) {
		return (EXIT_ERROR);
	}
	if (mem_with_size(mode, slot, wram_pnt(), wram_size(), fp) == EXIT_ERROR) {
		return (EXIT_ERROR);
	}
	if (mem_with_size(mode, slot, vram_pnt(), vram_size(), fp) == EXIT_ERROR) {
		return (EXIT_ERROR);
	}
	if (mem_with_size(mode, slot, nmt_pnt(), nmt_size(), fp) == EXIT_ERROR) {
		return (EXIT_ERROR);
	}
	save_slot_ele(mode, slot, memmap_palette.color);
	save_slot_ele(mode, slot, oam.data);
	save_slot_ele(mode, slot, oam.plus);
	for (i = 0; i < LENGTH(oam.ele_plus); i++) {
		save_slot_pos(mode, slot, oam.plus, oam.ele_plus[i]);
	}

	// mapper
	save_slot_ele(mode, slot, mapper.mirroring);
	if (mapper.internal_struct[0]) {
		extcl_save_mapper(mode, slot, fp);
	}

	// irqA12
	if (irqA12.present) {
		save_slot_ele(mode, slot, irqA12.present);
		save_slot_ele(mode, slot, irqA12.delay);
		save_slot_ele(mode, slot, irqA12.counter);
		save_slot_ele(mode, slot, irqA12.latch);
		save_slot_ele(mode, slot, irqA12.reload);
		save_slot_ele(mode, slot, irqA12.enable);
		save_slot_ele(mode, slot, irqA12.save_counter);
		save_slot_ele(mode, slot, irqA12.a12BS);
		save_slot_ele(mode, slot, irqA12.a12SB);
		save_slot_ele(mode, slot, irqA12.b_adr_old);
		save_slot_ele(mode, slot, irqA12.s_adr_old);
		save_slot_ele(mode, slot, irqA12.cycles);
		save_slot_ele(mode, slot, irqA12.race.C001);
		save_slot_ele(mode, slot, irqA12.race.counter);
		save_slot_ele(mode, slot, irqA12.race.reload);
	}

	// irql2f
	if (irql2f.present) {
		save_slot_ele(mode, slot, irql2f.present);
		save_slot_ele(mode, slot, irql2f.enable);
		save_slot_ele(mode, slot, irql2f.counter);
		save_slot_ele(mode, slot, irql2f.scanline);
		save_slot_ele(mode, slot, irql2f.frame_x);
		save_slot_ele(mode, slot, irql2f.delay);
		save_slot_ele(mode, slot, irql2f.in_frame);
		save_slot_ele(mode, slot, irql2f.pending);
	}

	if (fds.info.enabled) {
		// libero la zona di memoria gia' occupata
		BYTE old_side_inserted = fds.drive.side_inserted;

		// salvo, leggo o conto quello che serve
		save_slot_ele(mode, slot, fds.drive.disk_position);
		save_slot_ele(mode, slot, fds.drive.delay);
		save_slot_ele(mode, slot, fds.drive.disk_ejected);
		save_slot_ele(mode, slot, fds.drive.side_inserted);
		save_slot_ele(mode, slot, fds.drive.gap_ended);
		save_slot_ele(mode, slot, fds.drive.scan);
		save_slot_ele(mode, slot, fds.drive.crc_char);
		save_slot_ele(mode, slot, fds.drive.enabled_dsk_reg);
		save_slot_ele(mode, slot, fds.drive.enabled_snd_reg);
		save_slot_ele(mode, slot, fds.drive.data_readed);
		save_slot_ele(mode, slot, fds.drive.data_to_write);
		save_slot_ele(mode, slot, fds.drive.transfer_flag);
		save_slot_ele(mode, slot, fds.drive.motor_on);
		save_slot_ele(mode, slot, fds.drive.transfer_reset);
		save_slot_ele(mode, slot, fds.drive.read_mode);
		save_slot_ele(mode, slot, fds.drive.mirroring);
		save_slot_ele(mode, slot, fds.drive.crc_control);
		save_slot_ele(mode, slot, fds.drive.unknow);
		save_slot_ele(mode, slot, fds.drive.drive_ready);
		save_slot_ele(mode, slot, fds.drive.irq_disk_enabled);
		save_slot_ele(mode, slot, fds.drive.irq_disk_high);
		save_slot_ele(mode, slot, fds.drive.irq_timer_enabled);
		save_slot_ele(mode, slot, fds.drive.irq_timer_reload_enabled);
		save_slot_ele(mode, slot, fds.drive.irq_timer_high);
		save_slot_ele(mode, slot, fds.drive.irq_timer_reload);
		save_slot_ele(mode, slot, fds.drive.irq_timer_counter);
		save_slot_ele(mode, slot, fds.drive.irq_timer_delay);
		save_slot_ele(mode, slot, fds.drive.data_external_connector);
		save_slot_ele(mode, slot, fds.drive.filler);

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

		save_slot_ele(mode, slot, fds.auto_insert.r4032.frames);
		save_slot_ele(mode, slot, fds.auto_insert.r4032.checks);

		save_slot_ele(mode, slot, fds.auto_insert.delay.eject);
		save_slot_ele(mode, slot, fds.auto_insert.delay.dummy);
		save_slot_ele(mode, slot, fds.auto_insert.delay.side);

		save_slot_ele(mode, slot, fds.auto_insert.rE445.in_run);
		save_slot_ele(mode, slot, fds.auto_insert.rE445.count);

		save_slot_ele(mode, slot, fds.auto_insert.disabled);
		save_slot_ele(mode, slot, fds.auto_insert.new_side);
		save_slot_ele(mode, slot, fds.auto_insert.in_game);

		save_slot_ele(mode, slot, r2006.second_write.delay);
		save_slot_ele(mode, slot, r2006.second_write.value);

		save_slot_ele(mode, slot, r2001.grayscale_bit.delay);

		save_slot_ele(mode, slot, info.lag_frame.consecutive);

		save_slot_ele(mode, slot, dipswitch.value);

		// in caso di ripristino di una salvataggio, se era caricato
		// un'altro side del disco, devo ricaricarlo.
		if ((mode == SAVE_SLOT_READ) && (old_side_inserted != fds.drive.side_inserted)) {
			fds_disk_op(FDS_DISK_SELECT_FROM_REWIND, fds.drive.side_inserted, FALSE);
			gui_update();
		}
	}

	if (save_slot.slot[slot].tot_size != save_slot.size) {
		return (EXIT_ERROR);
	}

	// salvo la preview in formato PNG
	save_slot_mem(mode, slot, NULL, 0, TRUE);

	if (mode == SAVE_SLOT_READ) {
		prgrom_reset_chunks();
		chrrom_reset_chunks();
		ram_reset_chunks();
		wram_reset_chunks();
		nmt_reset_chunks();
		// ripristino i puntatori
		extcl_after_mapper_init();
	}

	return (EXIT_OK);
}
BYTE save_slot_element(BYTE mode, BYTE slot, void *src, size_t size, FILE *fp, BYTE preview) {
	DBWORD bytes = 0;

	switch (mode) {
		case SAVE_SLOT_SAVE:
			if (preview) {
				preview_image_from_ppu_screen(slot, ppu_screen.rd, &src, &size);
				fwrite(src, size, 1, fp);
			} else {
				fwrite(src, size, 1, fp);
			}
			fflush(fp);
			break;
		case SAVE_SLOT_READ:
			if (preview) {
				break;
			}
			bytes = fp ? fread(src, size, 1, fp) : 1;
			save_slot.slot[slot].tot_size += size;
			if (bytes != 1) {
				return (EXIT_ERROR);
			}
			break;
		case SAVE_SLOT_COUNT:
			if (preview) {
				size_t pos = 0, end = 0;

				if (!fp) {
					break;
				}

				pos = ftell(fp);
				fseek(fp, 0L, SEEK_END);
				end = ftell(fp);

				if (save_slot.slot[slot].tot_size >= end) {
					return (EXIT_ERROR);
				};

				size = end - save_slot.slot[slot].tot_size;
				src = malloc(size);

				if (!src) {
					return (EXIT_ERROR);
				}

				fseek(fp, save_slot.slot[slot].tot_size, SEEK_SET);
				if (fread(src, size, 1, fp) == 1) {
					preview_image_from_png(slot, src, size);
				}
				fseek(fp, pos, SEEK_SET);

				free(src);
				break;
			}
			save_slot.slot[slot].tot_size += size;
			break;
		default:
			return (EXIT_ERROR);
	}
	return (EXIT_OK);
}
BYTE save_slot_position(BYTE mode, BYTE slot, void *start, void *end, FILE *fp) {
	postype bank = 0;

	switch (mode) {
		case SAVE_SLOT_SAVE:
			bank = end - start;
			save_slot_ele(mode, slot, bank);
			break;
		case SAVE_SLOT_READ:
			save_slot_ele(mode, slot, bank);
			end = start + bank;
			break;
		case SAVE_SLOT_COUNT:
			save_slot_ele(mode, slot, bank);
			break;
		default:
			return (EXIT_ERROR);
	}
	return (EXIT_OK);
}

static BYTE mem_with_size(BYTE mode, BYTE slot, BYTE *mem, size_t msize, FILE *fp) {
	postype size = 0;
	int tmp = 0;

	if (mode == SAVE_SLOT_READ) {
		save_slot_int(mode, slot, tmp);
		if (tmp > TRUE) {
			return (EXIT_ERROR);
		}
		save_slot_ele(mode, slot, size);
		if (tmp) {
			if (size != msize) {
				return (EXIT_ERROR);
			}
			save_slot_mem(mode, slot, wram_pnt(), wram_size(), FALSE);
		}
	} else {
		if (size) {
			tmp = TRUE;
			size = msize;
			save_slot_int(mode, slot, tmp);
			save_slot_ele(mode, slot, size);
			save_slot_mem(mode, slot, mem, size, FALSE);
		} else {
			tmp = FALSE;
			size = 0;
			save_slot_int(mode, slot, tmp);
			save_slot_ele(mode, slot, size);
		}
	}
	return (EXIT_OK);
}
static void preview_image_from_ppu_screen(BYTE slot, _ppu_screen_buffer *sb, void **dst, size_t *size) {
	int stride = SCR_COLUMNS * sizeof(uint32_t);
	void *buffer = malloc((size_t)stride * SCR_ROWS);

	(*dst) = NULL;
	(*size) = 0;

	if (buffer) {
		scale_surface_preview_1x(sb, stride, buffer);
		gui_overlay_slot_preview_set_from_ppu_screen(slot, buffer, name_slot_file(slot));
		if (slot != SAVE_SLOT_FILE) {
			gui_state_save_slot_set_tooltip(slot);
		}
		free(buffer);
		gui_save_slot_preview_to_png(slot, dst, size);
	}
}
static void preview_image_from_png(BYTE slot, void *src, size_t size) {
	if (src) {
		gui_overlay_slot_preview_set_from_png(slot, src, size, name_slot_file(slot));
		if (slot != SAVE_SLOT_FILE) {
			gui_state_save_slot_set_tooltip(slot);
		}
	}
}
static uTCHAR *name_slot_file(BYTE slot) {
	static uTCHAR file[LENGTH_FILE_NAME_LONG];
	uTCHAR ext[10], bname[255], *last_dot = NULL, *fl = NULL;

	umemset(&file[0], 0x00, LENGTH_FILE_NAME_LONG);

	// game genie
	if (info.mapper.id == GAMEGENIE_MAPPER) {
		fl = gamegenie.rom;
	}

	if (!fl) {
		fl = info.rom.file;
	}

	if (!fl[0]) {
		return (NULL);
	}

	gui_utf_basename(fl, bname, usizeof(bname));
	usnprintf(&file[0], usizeof(file), uL("" uPs("") SAVE_FOLDER "/" uPs("")), gui_data_folder(), bname);

	if (nsf.enabled) {
		usnprintf(ext, usizeof(ext), uL(".n%02X"), slot);
	} else {
		usnprintf(ext, usizeof(ext), uL(".p%02X"), slot);
	}

	// rintraccio l'ultimo '.' nel nome
	last_dot = ustrrchr(file, uL('.'));
	if (last_dot) {
		// elimino l'estensione
		(*last_dot) = 0x00;
	}
	// aggiungo l'estensione
	ustrcat(&file[0], ext);

	return (&file[0]);
}
