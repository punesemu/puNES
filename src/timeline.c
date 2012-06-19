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
#include "memmap.h"
#include "clock.h"
#include "cpu6502.h"
#include "ppu.h"
#include "apu.h"
#include "mappers.h"
#include "irqA12.h"
#include "irql2f.h"
#include "fds.h"
#include "sdlgfx.h"
#include "gui.h"

enum {
	TLSAVE,
	TLREAD,
	TLCOUNT
};

#define onStruct(mode, strct)\
	switch(mode) {\
	case TLSAVE:\
		memcpy(tl.snaps[snap] + index, &strct, sizeof(strct));\
		index += sizeof(strct);\
		break;\
	case TLREAD:\
		memcpy(&strct, tl.snaps[snap] + index, sizeof(strct));\
		index += sizeof(strct);\
		break;\
	case TLCOUNT:\
		tl.snapSize += sizeof(strct);\
		break;\
	}
#define onMem(mode, mem, size)\
	switch(mode) {\
	case TLSAVE:\
		memcpy(tl.snaps[snap] + index, mem, size);\
		index += size;\
		break;\
	case TLREAD:\
		memcpy(mem, tl.snaps[snap] + index, size);\
		index += size;\
		break;\
	case TLCOUNT:\
		tl.snapSize += size;\
		break;\
	}

void tlOperation(BYTE mode, BYTE snap);

BYTE timelineInit(void) {
	/* in caso di riavvio del timeline */
	timelineQuit();

	memset(&tl, 0, sizeof(tl));

	tl.framesSnap = machine.fps * TLSNAPSEC;
	tl.frames = tl.framesSnap - 1;
	tl.snap = TLSNAPS - 1;

	tlOperation(TLCOUNT, 0);

	tl.start = malloc(tl.snapSize * TLSNAPSTOT);
	if (!tl.start) {
		fprintf(stderr, "timeline : Out of memory\n");
		return (EXIT_ERROR);
	}

	memset(tl.start, 0, tl.snapSize * TLSNAPSTOT);

	{
		BYTE index;

		for (index = 0; index < TLSNAPSTOT; index++) {
			tl.snaps[index] = tl.start + (index * tl.snapSize);
		}
	}

	return (EXIT_OK);
}
void timelineSnap(BYTE mode) {
	BYTE snap = 0;

	/* se non ci sono rom caricate, non faccio niente */
	if (!info.romFile[0]) {
		return;
	}

	if (mode == TLNORMAL) {
		if (++tl.snap == TLSNAPS) {
			tl.snap = 0;
		}

		if (++tl.snapsFill >= TLSNAPS) {
			tl.snapsFill = TLSNAPS;
		}

		tl.frames = 0;

		snap = tl.snap;
	} else {
		snap = TLSNAPFREE;
	}

	tlOperation(TLSAVE, snap);

	if (mode == TLNORMAL) {
		guiTimeline();
	}
}
void timelinePreview(BYTE snap) {
	DBWORD diff = (tl.snapsFill - 1) - snap;
	SWORD snapPrw = tl.snap - diff;

	if (snapPrw < 0) {
		snapPrw += TLSNAPS;
	}

	memcpy(screen.data, tl.snaps[snapPrw] + tl.preview, screenSize());
	gfxDrawScreen(TRUE);
}
void timelineBack(BYTE mode, BYTE snap) {
	DBWORD diff;

	if (mode == TLNORMAL) {
		diff = (tl.snapsFill - 1) - snap;
		tl.snapsFill -= diff;
		tl.snap -= diff;

		if (tl.snap < 0) {
			tl.snap += TLSNAPS;
		}

		tl.frames = 0;

		snap = tl.snap;
	} else {
		snap = TLSNAPFREE;
	}

	tlOperation(TLREAD, snap);
}
void timelineQuit(void) {
	if (tl.start) {
		free(tl.start);
	}
}

void tlOperation(BYTE mode, BYTE snap) {
	BYTE i;
	DBWORD index = 0;

	/* CPU */
	onStruct(mode, cpu);
	onStruct(mode, irq);
	onStruct(mode, nmi);

	/* PPU */
	onStruct(mode, ppu);
	onStruct(mode, ppuOpenbus);
	onStruct(mode, r2000);
	onStruct(mode, r2001);
	onStruct(mode, r2002);
	onStruct(mode, r2003);
	onStruct(mode, r2004);
	onStruct(mode, r2006);
	onStruct(mode, r2007);
	onStruct(mode, sprEv);
	onStruct(mode, sprite);
	onStruct(mode, spritePlus);
	onStruct(mode, tileRender);
	onStruct(mode, tileFetch);

	/* APU */
	onStruct(mode, apu);
	onStruct(mode, r4011);
	onStruct(mode, r4015);
	onStruct(mode, r4017);
	onStruct(mode, S1);
	onStruct(mode, S2);
	onStruct(mode, TR);
	onStruct(mode, NS);
	onStruct(mode, DMC);

	/* mem map */
	onStruct(mode, mmcpu);
	onStruct(mode, prg);
	if (info.mapper == FDS_MAPPER) {
		onMem(mode, prg.ram, 0x8000);
	} else {
		onMem(mode, prg.ram, 0x2000);
	}
	if (prg.ramPlus) {
		onMem(mode, prg.ramPlus, prgRamPlusSize());
	}
	onStruct(mode, chr);
	if (mapper.writeVRAM) {
		onMem(mode, chr.data, chrRamSize());
	}
	onStruct(mode, ntbl);
	onStruct(mode, palette);
	onStruct(mode, oam);

	/* mapper */
	onStruct(mode, mapper);
	for (i = 0; i < LENGTH(mapper.intStruct); i++) {
		if (mapper.intStruct[i]) {
			onMem(mode, mapper.intStruct[i], mapper.intStructSize[i]);
		}
	}

	/* irqA12 */
	if (irqA12.present) {
		onStruct(mode, irqA12);
	}

	/* irqA12 */
	if (irql2f.present) {
		onStruct(mode, irql2f);
	}

	/* FDS */
	if (fds.info.enabled) {
		BYTE old_side_inserted = fds.drive.side_inserted;

		onStruct(mode, fds.drive);
		onStruct(mode, fds.snd);

		/*
		 * in caso di ripristino di una snapshot, se era caricato
		 * un'altro side del disco, devo ricaricarlo.
		 */
		if ((mode == TLREAD) && (old_side_inserted != fds.drive.side_inserted)) {
			fds_disk_op(FDS_DISK_TIMELINE_SELECT, fds.drive.side_inserted);
			guiUpdate();
		}
	}

	if (mode == TLCOUNT) {
		tl.preview = tl.snapSize;
	}
	onMem(mode, screen.data, screenSize());
}
