/*
 * timeline.c
 *
 *  Created on: 05/ago/2011
 *      Author: fhorse
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "timeline.h"
#include "mem_map.h"
#include "clock.h"
#include "cpu.h"
#include "ppu.h"
#include "apu.h"
#include "mappers.h"
#include "irqA12.h"
#include "irql2f.h"
#include "fds.h"
#include "gfx.h"
#include "gui.h"

enum timeline_mode { TL_SAVE, TL_READ, TL_COUNT };

#define tl_on_struct(mode, strct)\
	switch(mode) {\
	case TL_SAVE:\
		memcpy(tl.snaps[snap] + index, &strct, sizeof(strct));\
		index += sizeof(strct);\
		break;\
	case TL_READ:\
		memcpy(&strct, tl.snaps[snap] + index, sizeof(strct));\
		index += sizeof(strct);\
		break;\
	case TL_COUNT:\
		tl.snap_size += sizeof(strct);\
		break;\
	}
#define tl_on_mem(mode, mem, size)\
	switch(mode) {\
	case TL_SAVE:\
		memcpy(tl.snaps[snap] + index, mem, size);\
		index += size;\
		break;\
	case TL_READ:\
		memcpy(mem, tl.snaps[snap] + index, size);\
		index += size;\
		break;\
	case TL_COUNT:\
		tl.snap_size += size;\
		break;\
	}

void tl_operation(BYTE mode, BYTE snap);

BYTE timeline_init(void) {
	/* in caso di riavvio del timeline */
	timeline_quit();

	memset(&tl, 0, sizeof(tl));

	tl.frames_snap = machine.fps * TL_SNAP_SEC;
	tl.frames = tl.frames_snap - 1;
	tl.snap = TL_SNAPS - 1;

	tl_operation(TL_COUNT, 0);

	tl.start = malloc(tl.snap_size * TL_SNAPS_TOT);
	if (!tl.start) {
		fprintf(stderr, "timeline : Out of memory\n");
		return (EXIT_ERROR);
	}

	memset(tl.start, 0, tl.snap_size * TL_SNAPS_TOT);

	{
		BYTE index;

		for (index = 0; index < TL_SNAPS_TOT; index++) {
			tl.snaps[index] = tl.start + (index * tl.snap_size);
		}
	}

	return (EXIT_OK);
}
void timeline_snap(BYTE mode) {
	BYTE snap = 0;

	/* se non ci sono rom caricate, non faccio niente */
	if (!info.rom_file[0]) {
		return;
	}

	if (mode == TL_NORMAL) {
		if (++tl.snap == TL_SNAPS) {
			tl.snap = 0;
		}

		if (++tl.snaps_fill >= TL_SNAPS) {
			tl.snaps_fill = TL_SNAPS;
		}

		tl.frames = 0;

		snap = tl.snap;
	} else {
		snap = TL_SNAP_FREE;
	}

	tl_operation(TL_SAVE, snap);

	if (mode == TL_NORMAL) {
		gui_timeline();
	}
}
void timeline_preview(BYTE snap) {
	DBWORD diff = (tl.snaps_fill - 1) - snap;
	SWORD snap_preview = tl.snap - diff;

	if (snap_preview < 0) {
		snap_preview += TL_SNAPS;
	}

	memcpy(screen.data, tl.snaps[snap_preview] + tl.preview, screen_size());
	gfx_draw_screen(TRUE);
}
void timeline_back(BYTE mode, BYTE snap) {
	DBWORD diff;

	if (mode == TL_NORMAL) {
		diff = (tl.snaps_fill - 1) - snap;
		tl.snaps_fill -= diff;
		tl.snap -= diff;

		if (tl.snap < 0) {
			tl.snap += TL_SNAPS;
		}

		tl.frames = 0;

		snap = tl.snap;
	} else {
		snap = TL_SNAP_FREE;
	}

	tl_operation(TL_READ, snap);
}
void timeline_quit(void) {
	if (tl.start) {
		free(tl.start);
	}
}

void tl_operation(BYTE mode, BYTE snap) {
	BYTE i;
	DBWORD index = 0;

	/* CPU */
	tl_on_struct(mode, cpu);
	tl_on_struct(mode, irq);
	tl_on_struct(mode, nmi);

	/* PPU */
	tl_on_struct(mode, ppu);
	tl_on_struct(mode, ppu_openbus);
	tl_on_struct(mode, r2000);
	tl_on_struct(mode, r2001);
	tl_on_struct(mode, r2002);
	tl_on_struct(mode, r2003);
	tl_on_struct(mode, r2004);
	tl_on_struct(mode, r2006);
	tl_on_struct(mode, r2007);
	tl_on_struct(mode, spr_ev);
	tl_on_struct(mode, sprite);
	tl_on_struct(mode, sprite_plus);
	tl_on_struct(mode, tile_render);
	tl_on_struct(mode, tile_fetch);

	/* APU */
	tl_on_struct(mode, apu);
	tl_on_struct(mode, r4011);
	tl_on_struct(mode, r4015);
	tl_on_struct(mode, r4017);
	tl_on_struct(mode, S1);
	tl_on_struct(mode, S2);
	tl_on_struct(mode, TR);
	tl_on_struct(mode, NS);
	tl_on_struct(mode, DMC);

	/* mem map */
	tl_on_struct(mode, mmcpu);
	tl_on_struct(mode, prg);
	if (info.mapper == FDS_MAPPER) {
		tl_on_mem(mode, prg.ram, 0x8000);
	} else {
		tl_on_mem(mode, prg.ram, 0x2000);
	}
	if (prg.ram_plus) {
		tl_on_mem(mode, prg.ram_plus, prg_ram_plus_size());
	}
	tl_on_struct(mode, chr);
	if (mapper.write_vram) {
		tl_on_mem(mode, chr.data, chr_ram_size());
	}
	tl_on_struct(mode, ntbl);
	tl_on_struct(mode, palette);
	tl_on_struct(mode, oam);

	/* mapper */
	tl_on_struct(mode, mapper);
	for (i = 0; i < LENGTH(mapper.internal_struct); i++) {
		if (mapper.internal_struct[i]) {
			tl_on_mem(mode, mapper.internal_struct[i], mapper.internal_struct_size[i]);
		}
	}

	/* irqA12 */
	if (irqA12.present) {
		tl_on_struct(mode, irqA12);
	}

	/* irqA12 */
	if (irql2f.present) {
		tl_on_struct(mode, irql2f);
	}

	/* FDS */
	if (fds.info.enabled) {
		BYTE old_side_inserted = fds.drive.side_inserted;

		tl_on_struct(mode, fds.drive);
		tl_on_struct(mode, fds.snd);

		/*
		 * in caso di ripristino di una snapshot, se era caricato
		 * un'altro side del disco, devo ricaricarlo.
		 */
		if ((mode == TL_READ) && (old_side_inserted != fds.drive.side_inserted)) {
			fds_disk_op(FDS_DISK_TIMELINE_SELECT, fds.drive.side_inserted);
			gui_update();
		}
	}

	if (mode == TL_COUNT) {
		tl.preview = tl.snap_size;
	}
	tl_on_mem(mode, screen.data, screen_size());
}
