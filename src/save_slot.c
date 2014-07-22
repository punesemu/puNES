/*
 * save_slot.c
 *
 *  Created on: 11/ago/2011
 *      Author: fhorse
 */

#include <libgen.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "save_slot.h"
#include "cfg_file.h"
#include "mem_map.h"
#include "cpu.h"
#include "ppu.h"
#include "apu.h"
#include "mappers.h"
#include "irqA12.h"
#include "irql2f.h"
#include "timeline.h"
#include "gfx.h"
#define __GUI_BASE__
#include "gui.h"
#undef __GUI_BASE__
#include "tas.h"
#include "text.h"
#include "fds.h"
#include "gamegenie.h"
#include "info.h"

#define SAVE_VERSION 13

BYTE slot_operation(BYTE mode, BYTE slot, FILE *fp);
BYTE name_slot_file(char *file, BYTE slot);

BYTE save_slot_save(void) {
	char file[LENGTH_FILE_NAME];
	FILE *fp;

	/* game genie */
	if (info.mapper.id == GAMEGENIE_MAPPER) {
		text_add_line_info(1, "[yellow]save is impossible in Game Genie menu");
		return (EXIT_ERROR);
	}

	if (name_slot_file(file, save_slot.slot)) {
		return (EXIT_ERROR);
	}

	if ((fp = fopen(file, "wb")) == NULL) {
		fprintf(stderr, "error on write save state\n");
		return (EXIT_ERROR);
	}

	slot_operation(SAVE_SLOT_SAVE, save_slot.slot, fp);
	/* aggiorno la posizione della preview e il totalsize */
	slot_operation(SAVE_SLOT_COUNT, save_slot.slot, fp);

	save_slot.state[save_slot.slot] = TRUE;

	fclose(fp);

	return (EXIT_OK);
}
BYTE save_slot_load(void) {
	char file[LENGTH_FILE_NAME];
	FILE *fp;

	if (tas.type) {
		text_add_line_info(1, "[yellow]movie playback interrupted[normal]");
		tas_quit();
	}

	/* game genie */
	if (info.mapper.id == GAMEGENIE_MAPPER) {
		gamegenie_reset();
		gamegenie.phase = GG_LOAD_ROM;
		emu_reset(CHANGE_ROM);
		gamegenie.phase = GG_FINISH;
	}

	if (name_slot_file(file, save_slot.slot)) {
		return (EXIT_ERROR);
	}

	if ((fp = fopen(file, "rb")) == NULL) {
		text_add_line_info(1, "[red]error[normal] loading state");
		fprintf(stderr, "error loading state\n");
		return (EXIT_ERROR);
	}

	/*
	 * mi salvo lo stato attuale da ripristinare in caso
	 * di un file di salvataggio corrotto.
	 */
	timeline_snap(TL_SAVE_SLOT);

	if (slot_operation(SAVE_SLOT_READ, save_slot.slot, fp)) {
		fprintf(stderr, "error loading state, corrupted file.\n");
		timeline_back(TL_SAVE_SLOT, 0);
	}

	fclose(fp);

	/* riavvio il timeline */
	timeline_init();

	return (EXIT_OK);
}
void save_slot_preview(BYTE slot) {
	char file[LENGTH_FILE_NAME];
	FILE *fp;

	if (!save_slot.preview_start) {
		memcpy(tl.snaps[TL_SNAP_FREE] + tl.preview, screen.data, screen_size());
		save_slot.preview_start = TRUE;
	}

	if (!save_slot.state[slot]) {
		memcpy(screen.data, tl.snaps[TL_SNAP_FREE] + tl.preview, screen_size());
		gfx_draw_screen(TRUE);
		return;
	}

	if (name_slot_file(file, slot)) {
		memcpy(screen.data, tl.snaps[TL_SNAP_FREE] + tl.preview, screen_size());
		gfx_draw_screen(TRUE);
		return;
	}

	if ((fp = fopen(file, "rb")) == NULL) {
		memcpy(screen.data, tl.snaps[TL_SNAP_FREE] + tl.preview, screen_size());
		gfx_draw_screen(TRUE);
		fprintf(stderr, "error on load preview\n");
		return;
	}

	fseek(fp, save_slot.preview[slot], SEEK_SET);

	{
		DBWORD bytes;

		bytes = fread(screen.data, screen_size(), 1, fp);

		if (bytes != 1) {
			memcpy(screen.data, tl.snaps[TL_SNAP_FREE] + tl.preview, screen_size());
		}
	}

	fclose(fp);
	gfx_draw_screen(TRUE);
}
void save_slot_count_load(void) {
	BYTE i;
	char file[LENGTH_FILE_NAME];

	for (i = 0; i < SAVE_SLOTS; i++) {
		save_slot.preview[i] = save_slot.tot_size[i] = 0;

		save_slot.state[i] = FALSE;
		name_slot_file(file, i);

		if (emu_file_exist(file) == EXIT_OK) {
			FILE *fp;

			save_slot.state[i] = TRUE;

			if ((fp = fopen(file, "rb")) == NULL) {
				continue;
			}

			slot_operation(SAVE_SLOT_COUNT, i, fp);
			fclose(fp);
		}
	}

	save_slot.preview_start = FALSE;

	if (!save_slot.state[save_slot.slot]) {
		BYTE i;

		save_slot.slot = 0;

		for (i = 0; i < SAVE_SLOTS; i++) {
			if (save_slot.state[i]) {
				save_slot.slot = i;
			}
		}
	}

	gui_save_slot(save_slot.slot);
}
BYTE save_slot_element_struct(BYTE mode, BYTE slot, uintptr_t *src, DBWORD size, FILE *fp,
        BYTE preview) {
	DBWORD bytes;

	switch (mode) {
		case SAVE_SLOT_SAVE:
			bytes = fwrite(src, size, 1, fp);
			save_slot.tot_size[slot] += size;
			break;
		case SAVE_SLOT_READ:
			bytes = fread(src, size, 1, fp);
			if ((bytes != 1) && !preview) {
				return (EXIT_ERROR);
			}
			break;
		case SAVE_SLOT_COUNT:
			save_slot.tot_size[slot] += size;
			break;
	}
	return (EXIT_OK);
}

BYTE slot_operation(BYTE mode, BYTE slot, FILE *fp) {
	uint32_t tmp = 0;
	WORD i = 0;

	save_slot.version = SAVE_VERSION;

	if (mode == SAVE_SLOT_COUNT) {
		save_slot.tot_size[slot] = 0;
		/*
		 * forzo la lettura perche' devo sapere la
		 * versione del file di salvataggio.
		 */
		save_slot_int(SAVE_SLOT_READ, slot, save_slot.version)
	}

	if (mode == SAVE_SLOT_READ) {
		save_slot_int(mode, slot, save_slot.version)
		i += sizeof(info.rom_file);
		i += sizeof(info.sha1sum.prg.value);
		i += sizeof(info.sha1sum.prg.string);
		if (save_slot.version >= 11) {
			i += sizeof(info.sha1sum.chr.value);
			i += sizeof(info.sha1sum.chr.string);
		}
		fseek(fp, i, SEEK_CUR);
	} else {
		save_slot_int(mode, slot, save_slot.version)
		save_slot_ele(mode, slot, info.rom_file)
		save_slot_ele(mode, slot, info.sha1sum.prg.value)
		save_slot_ele(mode, slot, info.sha1sum.prg.string)
		if (save_slot.version >= 11) {
			save_slot_ele(mode, slot, info.sha1sum.chr.value)
			save_slot_ele(mode, slot, info.sha1sum.chr.string)
		}
	}

	/* cpu */
	save_slot_ele(mode, slot, cpu.PC)
	save_slot_ele(mode, slot, cpu.SP)
	save_slot_ele(mode, slot, cpu.AR)
	save_slot_ele(mode, slot, cpu.XR)
	save_slot_ele(mode, slot, cpu.YR)
	save_slot_ele(mode, slot, cpu.SR)
	save_slot_ele(mode, slot, cpu.cf)
	save_slot_ele(mode, slot, cpu.zf)
	save_slot_ele(mode, slot, cpu.im)
	save_slot_ele(mode, slot, cpu.df)
	save_slot_ele(mode, slot, cpu.bf)
	save_slot_ele(mode, slot, cpu.of)
	save_slot_ele(mode, slot, cpu.sf)
	save_slot_ele(mode, slot, cpu.opcode)
	save_slot_ele(mode, slot, cpu.opcode_PC)
	save_slot_ele(mode, slot, cpu.odd_cycle)
	save_slot_ele(mode, slot, cpu.openbus)
	save_slot_ele(mode, slot, cpu.cycles)
	save_slot_ele(mode, slot, cpu.opcode_cycle)
	save_slot_ele(mode, slot, cpu.double_rd)
	save_slot_ele(mode, slot, cpu.double_wr)
	save_slot_ele(mode, slot, cpu.prg_ram_rd_active)
	save_slot_ele(mode, slot, cpu.prg_ram_wr_active)
	/* questo dato e' stato aggiunto solo dalla versione 9 in poi */
	if (save_slot.version >= 9) {
		save_slot_ele(mode, slot, cpu.base_opcode_cycles)
	}

	/* irq */
	save_slot_ele(mode, slot, irq.high)
	save_slot_ele(mode, slot, irq.delay)
	save_slot_ele(mode, slot, irq.before)
	save_slot_ele(mode, slot, irq.inhibit)
	/* nmi */
	save_slot_ele(mode, slot, nmi.high)
	save_slot_ele(mode, slot, nmi.delay)
	save_slot_ele(mode, slot, nmi.before)
	save_slot_ele(mode, slot, nmi.inhibit)
	save_slot_ele(mode, slot, nmi.frame_x)
	/* questo dato e' stato aggiunto solo dalla versione 9 in poi */
	if (save_slot.version >= 9) {
		save_slot_ele(mode, slot, nmi.cpu_cycles_from_last_nmi)
	}

	/* ppu */
	save_slot_ele(mode, slot, ppu.frame_x)
	save_slot_ele(mode, slot, ppu.frame_y)
	save_slot_ele(mode, slot, ppu.fine_x)
	save_slot_ele(mode, slot, ppu.screen_y)
	save_slot_ele(mode, slot, ppu.pixel_tile)
	save_slot_ele(mode, slot, ppu.sline_cycles)
	save_slot_ele(mode, slot, ppu.tmp_vram)
	save_slot_ele(mode, slot, ppu.spr_adr)
	save_slot_ele(mode, slot, ppu.bck_adr)
	save_slot_ele(mode, slot, ppu.openbus)
	save_slot_ele(mode, slot, ppu.odd_frame)
	save_slot_ele(mode, slot, ppu.cycles)
	save_slot_ele(mode, slot, ppu.frames)
	if (save_slot.version >= 13) {
		save_slot_ele(mode, slot, ppu.sf.actual)
		save_slot_ele(mode, slot, ppu.sf.prev)
		save_slot_ele(mode, slot, ppu.sf.first_of_tick)
	}
	/* ppu_openbus */
	save_slot_ele(mode, slot, ppu_openbus.bit0)
	save_slot_ele(mode, slot, ppu_openbus.bit1)
	save_slot_ele(mode, slot, ppu_openbus.bit2)
	save_slot_ele(mode, slot, ppu_openbus.bit3)
	save_slot_ele(mode, slot, ppu_openbus.bit4)
	save_slot_ele(mode, slot, ppu_openbus.bit5)
	save_slot_ele(mode, slot, ppu_openbus.bit6)
	save_slot_ele(mode, slot, ppu_openbus.bit7)
	/* r2000 */
	save_slot_ele(mode, slot, r2000.value)
	save_slot_ele(mode, slot, r2000.nmi_enable)
	save_slot_ele(mode, slot, r2000.size_spr)
	save_slot_ele(mode, slot, r2000.r2006_inc)
	save_slot_ele(mode, slot, r2000.spt_adr)
	save_slot_ele(mode, slot, r2000.bpt_adr)
	if (save_slot.version >= 13) {
		save_slot_ele(mode, slot, r2000.race.ctrl)
		save_slot_ele(mode, slot, r2000.race.value)
	}
	/* r2001 */
	save_slot_ele(mode, slot, r2001.value)
	save_slot_ele(mode, slot, r2001.emphasis)
	save_slot_ele(mode, slot, r2001.visible)
	save_slot_ele(mode, slot, r2001.bck_visible)
	save_slot_ele(mode, slot, r2001.spr_visible)
	save_slot_ele(mode, slot, r2001.bck_clipping)
	save_slot_ele(mode, slot, r2001.spr_clipping)
	save_slot_ele(mode, slot, r2001.color_mode)
	if (save_slot.version >= 13) {
		save_slot_ele(mode, slot, r2001.race.ctrl)
		save_slot_ele(mode, slot, r2001.race.value)
	}
	/* r2002 */
	save_slot_ele(mode, slot, r2002.vblank)
	save_slot_ele(mode, slot, r2002.sprite0_hit)
	save_slot_ele(mode, slot, r2002.sprite_overflow)
	save_slot_ele(mode, slot, r2002.toggle)
	/* r2003 */
	save_slot_ele(mode, slot, r2003.value)
	/* r2004 */
	save_slot_ele(mode, slot, r2004.value)
	/* r2006 */
	save_slot_ele(mode, slot, r2006.value)
	save_slot_ele(mode, slot, r2006.changed_from_op)
	if (save_slot.version >= 12) {
		save_slot_ele(mode, slot, r2006.race.ctrl)
		save_slot_ele(mode, slot, r2006.race.value)
	}
	/* r2007 */
	save_slot_ele(mode, slot, r2007.value)
	/* spr_ev */
	save_slot_ele(mode, slot, spr_ev.range)
	save_slot_ele(mode, slot, spr_ev.count)
	save_slot_ele(mode, slot, spr_ev.count_plus)
	save_slot_ele(mode, slot, spr_ev.tmp_spr_plus)
	save_slot_ele(mode, slot, spr_ev.evaluate)
	save_slot_ele(mode, slot, spr_ev.byte_OAM)
	save_slot_ele(mode, slot, spr_ev.index_plus)
	save_slot_ele(mode, slot, spr_ev.index)
	save_slot_ele(mode, slot, spr_ev.timing)
	save_slot_ele(mode, slot, spr_ev.phase)
	if (save_slot.version >= 13) {
		save_slot_ele(mode, slot, spr_ev.real)
	}
	/* sprite */
	for (i = 0; i < LENGTH(sprite); i++) {
		save_slot_ele(mode, slot, sprite[i].y_C)
		save_slot_ele(mode, slot, sprite[i].tile)
		save_slot_ele(mode, slot, sprite[i].attrib)
		save_slot_ele(mode, slot, sprite[i].x_C)
		save_slot_ele(mode, slot, sprite[i].number)
		save_slot_ele(mode, slot, sprite[i].flip_v)
		save_slot_ele(mode, slot, sprite[i].l_byte)
		save_slot_ele(mode, slot, sprite[i].h_byte)
	}
	/* sprite_plus */
	for (i = 0; i < LENGTH(sprite_plus); i++) {
		save_slot_ele(mode, slot, sprite_plus[i].y_C)
		save_slot_ele(mode, slot, sprite_plus[i].tile)
		save_slot_ele(mode, slot, sprite_plus[i].attrib)
		save_slot_ele(mode, slot, sprite_plus[i].x_C)
		save_slot_ele(mode, slot, sprite_plus[i].number)
		save_slot_ele(mode, slot, sprite_plus[i].flip_v)
		save_slot_ele(mode, slot, sprite_plus[i].l_byte)
		save_slot_ele(mode, slot, sprite_plus[i].h_byte)
	}
	/* tile_render */
	save_slot_ele(mode, slot, tile_render.attrib)
	save_slot_ele(mode, slot, tile_render.l_byte)
	save_slot_ele(mode, slot, tile_render.h_byte)
	/* tile_fetch */
	save_slot_ele(mode, slot, tile_fetch.attrib)
	save_slot_ele(mode, slot, tile_fetch.l_byte)
	save_slot_ele(mode, slot, tile_fetch.h_byte)

	/* apu */
	save_slot_ele(mode, slot, apu.mode)
	save_slot_ele(mode, slot, apu.type)
	save_slot_ele(mode, slot, apu.step)
	save_slot_ele(mode, slot, apu.length_clocked)
	save_slot_ele(mode, slot, apu.DMC)
	save_slot_ele(mode, slot, apu.cycles)
	/* r4015 */
	save_slot_ele(mode, slot, r4015.value)
	/* r4017 */
	save_slot_ele(mode, slot, r4017.value)
	save_slot_ele(mode, slot, r4017.jitter.value)
	save_slot_ele(mode, slot, r4017.jitter.delay)
	if (save_slot.version >= 13) {
		save_slot_ele(mode, slot, r4017.reset_frame_delay)
	}
	/* S1 */
	save_slot_square(S1, slot)
	/* S2 */
	save_slot_square(S2, slot)
	/* TR */
	save_slot_ele(mode, slot, TR.timer)
	save_slot_ele(mode, slot, TR.frequency)
	save_slot_ele(mode, slot, TR.linear.value)
	save_slot_ele(mode, slot, TR.linear.reload)
	save_slot_ele(mode, slot, TR.linear.halt)
	save_slot_ele(mode, slot, TR.length.value)
	save_slot_ele(mode, slot, TR.length.enabled)
	save_slot_ele(mode, slot, TR.length.halt)
	save_slot_ele(mode, slot, TR.sequencer)
	save_slot_ele(mode, slot, TR.output)
	/* NS */
	save_slot_ele(mode, slot, NS.timer)
	save_slot_ele(mode, slot, NS.frequency)
	save_slot_ele(mode, slot, NS.envelope.enabled)
	save_slot_ele(mode, slot, NS.envelope.divider)
	save_slot_ele(mode, slot, NS.envelope.counter)
	save_slot_ele(mode, slot, NS.envelope.constant_volume)
	save_slot_ele(mode, slot, NS.envelope.delay)
	save_slot_ele(mode, slot, NS.mode)
	save_slot_ele(mode, slot, NS.volume)
	/*
	 * ho portato da DBWORD a WORD NS.shift e per mantenere
	 * la compatibilita' con i vecchi salvataggi faccio questa
	 * conversione.
	 */
	if (save_slot.version < 7) {
		if (mode == SAVE_SLOT_READ) {
			DBWORD old_nsshift;

			save_slot_ele(mode, slot, old_nsshift)

			NS.shift = old_nsshift;
		} else if (mode == SAVE_SLOT_COUNT) {
			save_slot.tot_size[slot] += sizeof(DBWORD);
		}
	} else {
		save_slot_ele(mode, slot, NS.shift)
	}
	save_slot_ele(mode, slot, NS.length.value)
	save_slot_ele(mode, slot, NS.length.enabled)
	save_slot_ele(mode, slot, NS.length.halt)
	save_slot_ele(mode, slot, NS.sequencer)
	save_slot_ele(mode, slot, NS.output)
	/* DMC */
	save_slot_ele(mode, slot, DMC.frequency)
	save_slot_ele(mode, slot, DMC.remain)
	save_slot_ele(mode, slot, DMC.irq_enabled)
	save_slot_ele(mode, slot, DMC.loop)
	save_slot_ele(mode, slot, DMC.rate_index)
	save_slot_ele(mode, slot, DMC.address_start)
	save_slot_ele(mode, slot, DMC.address)
	save_slot_ele(mode, slot, DMC.length)
	save_slot_ele(mode, slot, DMC.counter)
	save_slot_ele(mode, slot, DMC.empty)
	save_slot_ele(mode, slot, DMC.buffer)
	save_slot_ele(mode, slot, DMC.dma_cycle)
	save_slot_ele(mode, slot, DMC.silence)
	save_slot_ele(mode, slot, DMC.shift)
	save_slot_ele(mode, slot, DMC.counter_out)
	save_slot_ele(mode, slot, DMC.output)
	save_slot_ele(mode, slot, DMC.tick_type)

	/* mem map */
	save_slot_ele(mode, slot, mmcpu.ram)
	if (info.mapper.id == FDS_MAPPER) {
		save_slot_mem(mode, slot, prg.ram, 0x8000, FALSE)
	} else {
		save_slot_mem(mode, slot, prg.ram, 0x2000, FALSE)
	}
	if (mode == SAVE_SLOT_READ) {
		save_slot_int(mode, slot, tmp)
		if (tmp) {
			save_slot_mem(mode, slot, prg.ram_plus, prg_ram_plus_size(), FALSE)
			save_slot_pos(mode, slot, prg.ram_plus, prg.ram_plus_8k)
			save_slot_int(mode, slot, tmp)
			if (tmp) {
				save_slot_pos(mode, slot, prg.ram_plus, prg.ram_battery)
			}
		}
	} else {
		if (prg.ram_plus) {
			tmp = TRUE;
			save_slot_int(mode, slot, tmp)
			save_slot_mem(mode, slot, prg.ram_plus, prg_ram_plus_size(), FALSE)
			save_slot_pos(mode, slot, prg.ram_plus, prg.ram_plus_8k)
			if (prg.ram_battery) {
				tmp = TRUE;
				save_slot_int(mode, slot, tmp)
				save_slot_pos(mode, slot, prg.ram_plus, prg.ram_battery)
			} else {
				tmp = FALSE;
				save_slot_int(mode, slot, tmp)
			}
		} else {
			tmp = FALSE;
			save_slot_int(mode, slot, tmp)
		}
	}
	for (i = 0; i < LENGTH(prg.rom_8k); i++) {
		if (mode == SAVE_SLOT_SAVE) {
			uint32_t bank = mapper.rom_map_to[i] << 13;
			save_slot_int(mode, slot, bank)
		} else {
			save_slot_pos(mode, slot, prg_chip(0), prg.rom_8k[i])
		}
	}
	save_slot_int(mode, slot, mapper.write_vram)
	if (mapper.write_vram) {
		save_slot_mem(mode, slot, chr_chip(0), chr_ram_size(), FALSE)
	}
	for (i = 0; i < LENGTH(chr.bank_1k); i++) {
		save_slot_pos(mode, slot, chr_chip(0), chr.bank_1k[i])
	}
	save_slot_ele(mode, slot, ntbl.data)
	for (i = 0; i < LENGTH(ntbl.bank_1k); i++) {
		if (mode == SAVE_SLOT_SAVE) {
			uint32_t diff = ntbl.bank_1k[i] - ntbl.data;
			if (diff > 0x1000) {
				tmp = 0;
				save_slot_int(mode, slot, tmp)
			} else {
				save_slot_pos(mode, slot, ntbl.data, ntbl.bank_1k[i])
			}
		} else {
			save_slot_pos(mode, slot, ntbl.data, ntbl.bank_1k[i])
		}
	}
	save_slot_ele(mode, slot, palette.color)
	save_slot_ele(mode, slot, oam.data)
	save_slot_ele(mode, slot, oam.plus)
	for (i = 0; i < LENGTH(oam.ele_plus); i++) {
		save_slot_pos(mode, slot, oam.plus, oam.ele_plus[i])
	}

	/* mapper */
	save_slot_ele(mode, slot, mapper.mirroring)
	/*
	 * ho portato da BYTE a WORD mapper.rom_map_to e per mantenere
	 * la compatibilita' con i vecchi salvataggi faccio questa
	 * conversione.
	 */
	if (save_slot.version < 2) {
		if (mode == SAVE_SLOT_READ) {
			BYTE old_romMapTo[4], i;

			save_slot_ele(mode, slot, old_romMapTo)

			for (i = 0; i < 4; i++) {
				mapper.rom_map_to[i] = old_romMapTo[i];
			}
		} else if (mode == SAVE_SLOT_COUNT) {
			save_slot.tot_size[slot] += sizeof(BYTE) * 4;
		}
	} else {
		save_slot_ele(mode, slot, mapper.rom_map_to)
	}

	if (mapper.internal_struct[0]) {
		extcl_save_mapper(mode, slot, fp);
	}

	/* irqA12 */
	if (irqA12.present) {
		save_slot_ele(mode, slot, irqA12.present)
		save_slot_ele(mode, slot, irqA12.delay)
		save_slot_ele(mode, slot, irqA12.counter)
		save_slot_ele(mode, slot, irqA12.latch)
		save_slot_ele(mode, slot, irqA12.reload)
		save_slot_ele(mode, slot, irqA12.enable)
		save_slot_ele(mode, slot, irqA12.save_counter)
		save_slot_ele(mode, slot, irqA12.a12BS)
		save_slot_ele(mode, slot, irqA12.a12SB)
		save_slot_ele(mode, slot, irqA12.b_adr_old)
		save_slot_ele(mode, slot, irqA12.s_adr_old)
		if (save_slot.version >= 10) {
			save_slot_ele(mode, slot, irqA12.cycles)
		}
	}

	/* irql2f */
	if (irql2f.present) {
		save_slot_ele(mode, slot, irql2f.present)
		save_slot_ele(mode, slot, irql2f.enable)
		save_slot_ele(mode, slot, irql2f.counter)
		save_slot_ele(mode, slot, irql2f.scanline)
		save_slot_ele(mode, slot, irql2f.frame_x)
		save_slot_ele(mode, slot, irql2f.delay)
		save_slot_ele(mode, slot, irql2f.in_frame)
		save_slot_ele(mode, slot, irql2f.pending)
	}

	if (fds.info.enabled) {
		/* libero la zona di memoria gia' occupata */
		BYTE old_side_inserted = fds.drive.side_inserted;

		/* salvo, leggo o conto quello che serve */
		save_slot_ele(mode, slot, fds.drive.disk_position)
		save_slot_ele(mode, slot, fds.drive.delay)
		save_slot_ele(mode, slot, fds.drive.disk_ejected)
		save_slot_ele(mode, slot, fds.drive.side_inserted)
		save_slot_ele(mode, slot, fds.drive.gap_ended)
		save_slot_ele(mode, slot, fds.drive.scan)
		save_slot_ele(mode, slot, fds.drive.crc_char)
		save_slot_ele(mode, slot, fds.drive.enabled_dsk_reg)
		save_slot_ele(mode, slot, fds.drive.enabled_snd_reg)
		save_slot_ele(mode, slot, fds.drive.data_readed)
		save_slot_ele(mode, slot, fds.drive.data_to_write)
		save_slot_ele(mode, slot, fds.drive.transfer_flag)
		save_slot_ele(mode, slot, fds.drive.motor_on)
		save_slot_ele(mode, slot, fds.drive.transfer_reset)
		save_slot_ele(mode, slot, fds.drive.read_mode)
		save_slot_ele(mode, slot, fds.drive.mirroring)
		save_slot_ele(mode, slot, fds.drive.crc_control)
		save_slot_ele(mode, slot, fds.drive.unknow)
		save_slot_ele(mode, slot, fds.drive.drive_ready)
		save_slot_ele(mode, slot, fds.drive.irq_disk_enabled)
		save_slot_ele(mode, slot, fds.drive.irq_disk_high)
		save_slot_ele(mode, slot, fds.drive.irq_timer_enabled)
		save_slot_ele(mode, slot, fds.drive.irq_timer_reload_enabled)
		save_slot_ele(mode, slot, fds.drive.irq_timer_high)
		save_slot_ele(mode, slot, fds.drive.irq_timer_reload)
		save_slot_ele(mode, slot, fds.drive.irq_timer_counter)
		save_slot_ele(mode, slot, fds.drive.irq_timer_delay)
		save_slot_ele(mode, slot, fds.drive.data_external_connector)
		save_slot_ele(mode, slot, fds.drive.filler)

		/*
		 * l'fds drive l'ho aggiunto nella versione 3, mentre il
		 * sound dalla 4 in poi.
		 */
		if (save_slot.version >= 4) {
			save_slot_ele(mode, slot, fds.snd.wave.data)
			save_slot_ele(mode, slot, fds.snd.wave.writable)
			save_slot_ele(mode, slot, fds.snd.wave.volume)
			save_slot_ele(mode, slot, fds.snd.wave.index)
			save_slot_ele(mode, slot, fds.snd.wave.counter)

			save_slot_ele(mode, slot, fds.snd.envelope.speed)
			save_slot_ele(mode, slot, fds.snd.envelope.disabled)

			save_slot_ele(mode, slot, fds.snd.main.silence)
			save_slot_ele(mode, slot, fds.snd.main.frequency)
			save_slot_ele(mode, slot, fds.snd.main.output)

			save_slot_ele(mode, slot, fds.snd.volume.speed)
			save_slot_ele(mode, slot, fds.snd.volume.mode)
			save_slot_ele(mode, slot, fds.snd.volume.increase)
			save_slot_ele(mode, slot, fds.snd.volume.gain)
			save_slot_ele(mode, slot, fds.snd.volume.counter)

			save_slot_ele(mode, slot, fds.snd.sweep.bias)
			save_slot_ele(mode, slot, fds.snd.sweep.mode)
			save_slot_ele(mode, slot, fds.snd.sweep.increase)
			save_slot_ele(mode, slot, fds.snd.sweep.speed)
			save_slot_ele(mode, slot, fds.snd.sweep.gain)
			save_slot_ele(mode, slot, fds.snd.sweep.counter)

			save_slot_ele(mode, slot, fds.snd.modulation.data)
			save_slot_ele(mode, slot, fds.snd.modulation.frequency)
			save_slot_ele(mode, slot, fds.snd.modulation.disabled)
			save_slot_ele(mode, slot, fds.snd.modulation.index)
			save_slot_ele(mode, slot, fds.snd.modulation.counter)
			save_slot_ele(mode, slot, fds.snd.modulation.mod)
		}

		/*
		 * in caso di ripristino di una salvataggio, se era caricato
		 * un'altro side del disco, devo ricaricarlo.
		 */
		if ((mode == SAVE_SLOT_READ) && (old_side_inserted != fds.drive.side_inserted)) {
			fds_disk_op(FDS_DISK_TIMELINE_SELECT, fds.drive.side_inserted);
			gui_update();
		}
	}

	if ((mode == SAVE_SLOT_COUNT) || (mode == SAVE_SLOT_SAVE)) {
		save_slot.preview[slot] = save_slot.tot_size[slot];
	}

	save_slot_mem(mode, slot, screen.data, screen_size(), TRUE)

	return (EXIT_OK);
}
BYTE name_slot_file(char *file, BYTE slot) {
	char ext[10], *last_dot, *fl;

	memset(file, 0x00, LENGTH_FILE_NAME);

	/* game genie */
	if (info.mapper.id == GAMEGENIE_MAPPER) {
		fl = info.load_rom_file;
	} else {
		fl = info.rom_file;
	}

	if (!fl[0]) {
		return (EXIT_ERROR);
	}

	sprintf(file, "%s" SAVE_FOLDER "/%s", info.base_folder, basename(fl));
	sprintf(ext, ".p%02d", slot);

	/* rintraccio l'ultimo '.' nel nome */
	last_dot = strrchr(file, '.');
	/* elimino l'estensione */
	*last_dot = 0x00;
	/* aggiungo l'estensione */
	strcat(file, ext);

	return (EXIT_OK);
}
