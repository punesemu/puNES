/*
 * savestate.c
 *
 *  Created on: 11/ago/2011
 *      Author: fhorse
 */

#include <libgen.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include "savestate.h"
#include "cfg_file.h"
#include "memmap.h"
#include "cpu6502.h"
#include "ppu.h"
#include "apu.h"
#include "mappers.h"
#include "irqA12.h"
#include "irql2f.h"
#include "timeline.h"
#include "sdlgfx.h"
#include "gui.h"
#include "tas.h"
#include "sdltext.h"
#include "fds.h"
#include "gamegenie.h"

#define SAVEVERSION 8

BYTE stateOperation(BYTE mode, BYTE slot, FILE *fp);
BYTE nameStateFile(char *file, BYTE slot);

BYTE savestateSave(void) {
	char file[512];
	FILE *fp;

	/* game genie */
	if (info.mapper == GAMEGENIE_MAPPER) {
		textAddLineInfo(1, "[yellow]save is impossible in Game Genie menu");
		return (EXIT_ERROR);
	}

	if (nameStateFile(file, savestate.slot)) {
		return (EXIT_ERROR);
	}

	if ((fp = fopen(file, "wb")) == NULL) {
		fprintf(stderr, "error on write save state\n");
		return (EXIT_ERROR);
	}

	stateOperation(SSSAVE, savestate.slot, fp);
	/* aggiorno la posizione della preview e il totalsize */
	stateOperation(SSCOUNT, savestate.slot, fp);

	savestate.slotState[savestate.slot] = TRUE;

	fclose(fp);

	return (EXIT_OK);
}
BYTE savestateLoad(void) {
	char file[512];
	FILE *fp;

	if (tas.type) {
		textAddLineInfo(1, "[yellow]movie playback interrupted[normal]");
		tasQuit();
	}

	/* game genie */
	if (info.mapper == GAMEGENIE_MAPPER) {
		gamegenie_reset(FALSE);
		gamegenie.phase = GG_LOAD_ROM;
		emuReset(CHANGEROM);
		gamegenie.phase = GG_FINISH;
	}

	if (nameStateFile(file, savestate.slot)) {
		return (EXIT_ERROR);
	}

	if ((fp = fopen(file, "rb")) == NULL) {
		textAddLineInfo(1, "[red]error[normal] on load state");
		fprintf(stderr, "error on load state\n");
		return (EXIT_ERROR);
	}

	/*
	 * mi salvo lo stato attuale da ripristinare in caso
	 * di un file di salvataggio corrotto.
	 */
	timelineSnap(TLSAVESTATE);

	if (stateOperation(SSREAD, savestate.slot, fp)) {
		fprintf(stderr, "error on loading state, corrupted file.\n");
		timelineBack(TLSAVESTATE, 0);
	}

	fclose(fp);

	/* riavvio il timeline */
	timelineInit();

	return (EXIT_OK);
}
void savestatePreview(BYTE slot) {
	char file[512];
	FILE *fp;

	if (!savestate.previewStart) {
		memcpy(tl.snaps[TLSNAPFREE] + tl.preview, screen.data, screenSize());
		savestate.previewStart = TRUE;
	}

	if (!savestate.slotState[slot]) {
		memcpy(screen.data, tl.snaps[TLSNAPFREE] + tl.preview, screenSize());
		gfxDrawScreen(TRUE);
		return;
	}

	if (nameStateFile(file, slot)) {
		memcpy(screen.data, tl.snaps[TLSNAPFREE] + tl.preview, screenSize());
		gfxDrawScreen(TRUE);
		return;
	}

	if ((fp = fopen(file, "rb")) == NULL) {
		memcpy(screen.data, tl.snaps[TLSNAPFREE] + tl.preview, screenSize());
		gfxDrawScreen(TRUE);
		fprintf(stderr, "error on load preview\n");
		return;
	}

	fseek(fp, savestate.preview[slot], SEEK_SET);

	{
		DBWORD bytes;

		bytes = fread(screen.data, screenSize(), 1, fp);

		if (bytes != 1) {
			memcpy(screen.data, tl.snaps[TLSNAPFREE] + tl.preview, screenSize());
		}
	}

	fclose(fp);
	gfxDrawScreen(TRUE);
}
void savestateCountLoad(void) {
	BYTE i;
	struct stat status;
	char file[512];

	for (i = 0; i < SSAVAILABLE; i++) {
		savestate.preview[i] = savestate.totSize[i] = 0;

		savestate.slotState[i] = FALSE;
		nameStateFile(file, i);

		if (!(access(file, 0))) {
			stat(file, &status);
			if (status.st_mode & S_IFREG) {
				FILE *fp;

				savestate.slotState[i] = TRUE;

				if ((fp = fopen(file, "rb")) == NULL) {
					continue;
				}

				stateOperation(SSCOUNT, i, fp);
				fclose(fp);
			}
		}
	}

	savestate.previewStart = FALSE;

	if (!savestate.slotState[savestate.slot]) {
		BYTE i;

		savestate.slot = 0;

		for (i = 0; i < SSAVAILABLE; i++) {
			if (savestate.slotState[i]) {
				savestate.slot = i;
			}
		}
	}

	guiSavestate(savestate.slot);
}
BYTE savestateElementStruct(BYTE mode, BYTE slot, uintptr_t *src, DBWORD size, FILE *fp,
        BYTE preview) {
	DBWORD bytes;

	switch (mode) {
		case SSSAVE:
			bytes = fwrite(src, size, 1, fp);
			savestate.totSize[slot] += size;
			break;
		case SSREAD:
			bytes = fread(src, size, 1, fp);
			if ((bytes != 1) && !preview) {
				return (EXIT_ERROR);
			}
			break;
		case SSCOUNT:
			savestate.totSize[slot] += size;
			break;
	}
	return (EXIT_OK);
}

BYTE stateOperation(BYTE mode, BYTE slot, FILE *fp) {
	uint32_t tmp = 0;
	WORD i = 0;

	savestate.version = SAVEVERSION;

	if (mode == SSCOUNT) {
		savestate.totSize[slot] = 0;
		/*
		 * forzo la lettura perche' devo sapere la
		 * versione del file di salvataggio.
		 */
		savestateInt(SSREAD, slot, savestate.version)
	}

	if (mode == SSREAD) {
		savestateInt(mode, slot, savestate.version)
		i += sizeof(info.romFile);
		i += sizeof(info.sha1sum);
		i += sizeof(info.sha1sumString);
		fseek(fp, i, SEEK_CUR);
	} else {
		savestateInt(mode, slot, savestate.version)
		savestateEle(mode, slot, info.romFile)
		savestateEle(mode, slot, info.sha1sum)
		savestateEle(mode, slot, info.sha1sumString)
	}

	/* cpu */
	savestateEle(mode, slot, cpu.PC)
	savestateEle(mode, slot, cpu.SP)
	savestateEle(mode, slot, cpu.AR)
	savestateEle(mode, slot, cpu.XR)
	savestateEle(mode, slot, cpu.YR)
	savestateEle(mode, slot, cpu.SR)
	savestateEle(mode, slot, cpu.cf)
	savestateEle(mode, slot, cpu.zf)
	savestateEle(mode, slot, cpu.im)
	savestateEle(mode, slot, cpu.df)
	savestateEle(mode, slot, cpu.bf)
	savestateEle(mode, slot, cpu.of)
	savestateEle(mode, slot, cpu.sf)
	savestateEle(mode, slot, cpu.codeop)
	savestateEle(mode, slot, cpu.codeopPC)
	savestateEle(mode, slot, cpu.oddCycle)
	savestateEle(mode, slot, cpu.openbus)
	savestateEle(mode, slot, cpu.cycles)
	savestateEle(mode, slot, cpu.opCycle)
	savestateEle(mode, slot, cpu.dblRd)
	savestateEle(mode, slot, cpu.dblWr)
	savestateEle(mode, slot, cpu.prgRamRdActive)
	savestateEle(mode, slot, cpu.prgRamWrActive)
	/* irq */
	savestateEle(mode, slot, irq.high)
	savestateEle(mode, slot, irq.delay)
	savestateEle(mode, slot, irq.before)
	savestateEle(mode, slot, irq.inhibit)
	/* nmi */
	savestateEle(mode, slot, nmi.high)
	savestateEle(mode, slot, nmi.delay)
	savestateEle(mode, slot, nmi.before)
	savestateEle(mode, slot, nmi.inhibit)
	savestateEle(mode, slot, nmi.frameX)

	/* ppu */
	savestateEle(mode, slot, ppu.frameX)
	savestateEle(mode, slot, ppu.frameY)
	savestateEle(mode, slot, ppu.fineX)
	savestateEle(mode, slot, ppu.screenY)
	savestateEle(mode, slot, ppu.pixelTile)
	savestateEle(mode, slot, ppu.slineCycles)
	savestateEle(mode, slot, ppu.tmpVRAM)
	savestateEle(mode, slot, ppu.sprAdr)
	savestateEle(mode, slot, ppu.bckAdr)
	savestateEle(mode, slot, ppu.openbus)
	savestateEle(mode, slot, ppu.oddFrame)
	savestateEle(mode, slot, ppu.cycles)
	savestateEle(mode, slot, ppu.frames)
	/* ppuOpenbus */
	savestateEle(mode, slot, ppuOpenbus.bit0)
	savestateEle(mode, slot, ppuOpenbus.bit1)
	savestateEle(mode, slot, ppuOpenbus.bit2)
	savestateEle(mode, slot, ppuOpenbus.bit3)
	savestateEle(mode, slot, ppuOpenbus.bit4)
	savestateEle(mode, slot, ppuOpenbus.bit5)
	savestateEle(mode, slot, ppuOpenbus.bit6)
	savestateEle(mode, slot, ppuOpenbus.bit7)
	/* r2000 */
	savestateEle(mode, slot, r2000.value)
	savestateEle(mode, slot, r2000.NMIenable)
	savestateEle(mode, slot, r2000.sizeSPR)
	savestateEle(mode, slot, r2000.r2006Inc)
	savestateEle(mode, slot, r2000.SPTadr)
	savestateEle(mode, slot, r2000.BPTadr)
	/* r2001 */
	savestateEle(mode, slot, r2001.value)
	savestateEle(mode, slot, r2001.emphasis)
	savestateEle(mode, slot, r2001.visible)
	savestateEle(mode, slot, r2001.bckVisible)
	savestateEle(mode, slot, r2001.sprVisible)
	savestateEle(mode, slot, r2001.bckClipping)
	savestateEle(mode, slot, r2001.sprClipping)
	savestateEle(mode, slot, r2001.colorMode)
	/* r2002 */
	savestateEle(mode, slot, r2002.vblank)
	savestateEle(mode, slot, r2002.sprite0Hit)
	savestateEle(mode, slot, r2002.spriteOverflow)
	savestateEle(mode, slot, r2002.toggle)
	/* r2003 */
	savestateEle(mode, slot, r2003.value)
	/* r2004 */
	savestateEle(mode, slot, r2004.value)
	/* r2006 */
	savestateEle(mode, slot, r2006.value)
	savestateEle(mode, slot, r2006.changedFromOP)
	/* r2007 */
	savestateEle(mode, slot, r2007.value)
	/* sprEv */
	savestateEle(mode, slot, sprEv.range)
	savestateEle(mode, slot, sprEv.count)
	savestateEle(mode, slot, sprEv.countPlus)
	savestateEle(mode, slot, sprEv.tmpSprPlus)
	savestateEle(mode, slot, sprEv.evaluate)
	savestateEle(mode, slot, sprEv.byteOAM)
	savestateEle(mode, slot, sprEv.indexPlus)
	savestateEle(mode, slot, sprEv.index)
	savestateEle(mode, slot, sprEv.timing)
	savestateEle(mode, slot, sprEv.phase)
	/* sprite */
	for (i = 0; i < LENGTH(sprite); i++) {
		savestateEle(mode, slot, sprite[i].yC)
		savestateEle(mode, slot, sprite[i].tile)
		savestateEle(mode, slot, sprite[i].attrib)
		savestateEle(mode, slot, sprite[i].xC)
		savestateEle(mode, slot, sprite[i].number)
		savestateEle(mode, slot, sprite[i].flipV)
		savestateEle(mode, slot, sprite[i].lByte)
		savestateEle(mode, slot, sprite[i].hByte)
	}
	/* spritePlus */
	for (i = 0; i < LENGTH(spritePlus); i++) {
		savestateEle(mode, slot, spritePlus[i].yC)
		savestateEle(mode, slot, spritePlus[i].tile)
		savestateEle(mode, slot, spritePlus[i].attrib)
		savestateEle(mode, slot, spritePlus[i].xC)
		savestateEle(mode, slot, spritePlus[i].number)
		savestateEle(mode, slot, spritePlus[i].flipV)
		savestateEle(mode, slot, spritePlus[i].lByte)
		savestateEle(mode, slot, spritePlus[i].hByte)
	}
	/* tileRender */
	savestateEle(mode, slot, tileRender.attrib)
	savestateEle(mode, slot, tileRender.lByte)
	savestateEle(mode, slot, tileRender.hByte)
	/* tileFetch */
	savestateEle(mode, slot, tileFetch.attrib)
	savestateEle(mode, slot, tileFetch.lByte)
	savestateEle(mode, slot, tileFetch.hByte)

	/* apu */
	savestateEle(mode, slot, apu.mode)
	savestateEle(mode, slot, apu.type)
	savestateEle(mode, slot, apu.step)
	savestateEle(mode, slot, apu.length_clocked)
	savestateEle(mode, slot, apu.DMC)
	savestateEle(mode, slot, apu.cycles)
	/* r4015 */
	savestateEle(mode, slot, r4015.value)
	/* r4017 */
	savestateEle(mode, slot, r4017.value)
	savestateEle(mode, slot, r4017.jitter)
	savestateEle(mode, slot, r4017.delay)
	/* S1 */
	savestateSquare(S1, slot)
	/* S2 */
	savestateSquare(S2, slot)
	/* TR */
	savestateEle(mode, slot, TR.timer)
	savestateEle(mode, slot, TR.frequency)
	savestateEle(mode, slot, TR.linear.value)
	savestateEle(mode, slot, TR.linear.reload)
	savestateEle(mode, slot, TR.linear.halt)
	savestateEle(mode, slot, TR.length.value)
	savestateEle(mode, slot, TR.length.enabled)
	savestateEle(mode, slot, TR.length.halt)
	savestateEle(mode, slot, TR.sequencer)
	savestateEle(mode, slot, TR.output)
	/* NS */
	savestateEle(mode, slot, NS.timer)
	savestateEle(mode, slot, NS.frequency)
	savestateEle(mode, slot, NS.envelope.enabled)
	savestateEle(mode, slot, NS.envelope.divider)
	savestateEle(mode, slot, NS.envelope.counter)
	savestateEle(mode, slot, NS.envelope.constant_volume)
	savestateEle(mode, slot, NS.envelope.delay)
	savestateEle(mode, slot, NS.mode)
	savestateEle(mode, slot, NS.volume)
	/*
	 * ho portato da DBWORD a WORD NS.shift e per mantenere
	 * la compatibilita' con i vecchi salvataggi faccio questa
	 * conversione.
	 */
	if (savestate.version < 7) {
		if (mode == SSREAD) {
			DBWORD old_nsshift;

			savestateEle(mode, slot, old_nsshift)

			NS.shift = old_nsshift;
		} else if (mode == SSCOUNT) {
			savestate.totSize[slot] += sizeof(DBWORD);
		}
	} else {
		savestateEle(mode, slot, NS.shift)
	}
	savestateEle(mode, slot, NS.length.value)
	savestateEle(mode, slot, NS.length.enabled)
	savestateEle(mode, slot, NS.length.halt)
	savestateEle(mode, slot, NS.sequencer)
	savestateEle(mode, slot, NS.output)
	/* DMC */
	savestateEle(mode, slot, DMC.frequency)
	savestateEle(mode, slot, DMC.remain)
	savestateEle(mode, slot, DMC.irq_enabled)
	savestateEle(mode, slot, DMC.loop)
	savestateEle(mode, slot, DMC.rate_index)
	savestateEle(mode, slot, DMC.address_start)
	savestateEle(mode, slot, DMC.address)
	savestateEle(mode, slot, DMC.length)
	savestateEle(mode, slot, DMC.counter)
	savestateEle(mode, slot, DMC.empty)
	savestateEle(mode, slot, DMC.buffer)
	savestateEle(mode, slot, DMC.dma_cycle)
	savestateEle(mode, slot, DMC.silence)
	savestateEle(mode, slot, DMC.shift)
	savestateEle(mode, slot, DMC.counter_out)
	savestateEle(mode, slot, DMC.output)
	savestateEle(mode, slot, DMC.tick_type)

	/* mem map */
	savestateEle(mode, slot, mmcpu.ram)
	if (info.mapper == FDS_MAPPER) {
		savestateMem(mode, slot, prg.ram, 0x8000, FALSE)
	} else {
		savestateMem(mode, slot, prg.ram, 0x2000, FALSE)
	}
	if (mode == SSREAD) {
		savestateInt(mode, slot, tmp)
		if (tmp) {
			savestateMem(mode, slot, prg.ramPlus, prgRamPlusSize(), FALSE)
			savestatePos(mode, slot, prg.ramPlus, prg.ramPlus8k)
			savestateInt(mode, slot, tmp)
			if (tmp) {
				savestatePos(mode, slot, prg.ramPlus, prg.ramBattery)
			}
		}
	} else {
		if (prg.ramPlus) {
			tmp = TRUE;
			savestateInt(mode, slot, tmp)
			savestateMem(mode, slot, prg.ramPlus, prgRamPlusSize(), FALSE)
			savestatePos(mode, slot, prg.ramPlus, prg.ramPlus8k)
			if (prg.ramBattery) {
				tmp = TRUE;
				savestateInt(mode, slot, tmp)
				savestatePos(mode, slot, prg.ramPlus, prg.ramBattery)
			} else {
				tmp = FALSE;
				savestateInt(mode, slot, tmp)
			}
		} else {
			tmp = FALSE;
			savestateInt(mode, slot, tmp)
		}
	}
	for (i = 0; i < LENGTH(prg.rom8k); i++) {
		if (mode == SSSAVE) {
			uint32_t bank = mapper.romMapTo[i] << 13;
			savestateInt(mode, slot, bank)
		} else {
			savestatePos(mode, slot, prg.rom, prg.rom8k[i])
		}
	}
	savestateInt(mode, slot, mapper.writeVRAM)
	if (mapper.writeVRAM) {
		savestateMem(mode, slot, chr.data, chrRamSize(), FALSE)
	}
	for (i = 0; i < LENGTH(chr.bank1k); i++) {
		savestatePos(mode, slot, chr.data, chr.bank1k[i])
	}
	savestateEle(mode, slot, ntbl.data)
	for (i = 0; i < LENGTH(ntbl.bank1k); i++) {
		if (mode == SSSAVE) {
			uint32_t diff = ntbl.bank1k[i] - ntbl.data;
			if (diff > 0x1000) {
				tmp = 0;
				savestateInt(mode, slot, tmp)
			} else {
				savestatePos(mode, slot, ntbl.data, ntbl.bank1k[i])
			}
		} else {
			savestatePos(mode, slot, ntbl.data, ntbl.bank1k[i])
		}
	}
	savestateEle(mode, slot, palette.color)
	savestateEle(mode, slot, oam.data)
	savestateEle(mode, slot, oam.plus)
	for (i = 0; i < LENGTH(oam.elementPlus); i++) {
		savestatePos(mode, slot, oam.plus, oam.elementPlus[i])
	}

	/* mapper */
	savestateEle(mode, slot, mapper.mirroring)
	/*
	 * ho portato da BYTE a WORD mapper.romMapTo e per mantenere
	 * la compatibilita' con i vecchi salvataggi faccio questa
	 * conversione.
	 */
	if (savestate.version < 2) {
		if (mode == SSREAD) {
			BYTE old_romMapTo[4], i;

			savestateEle(mode, slot, old_romMapTo)

			for (i = 0; i < 4; i++) {
				mapper.romMapTo[i] = old_romMapTo[i];
			}
		} else if (mode == SSCOUNT) {
			savestate.totSize[slot] += sizeof(BYTE) * 4;
		}
	} else {
		savestateEle(mode, slot, mapper.romMapTo)
	}

	if (mapper.intStruct[0]) {
		extcl_save_mapper(mode, slot, fp);
	}

	/* irqA12 */
	if (irqA12.present) {
		savestateEle(mode, slot, irqA12.present)
		savestateEle(mode, slot, irqA12.delay)
		savestateEle(mode, slot, irqA12.counter)
		savestateEle(mode, slot, irqA12.latch)
		savestateEle(mode, slot, irqA12.reload)
		savestateEle(mode, slot, irqA12.enable)
		savestateEle(mode, slot, irqA12.saveCounter)
		savestateEle(mode, slot, irqA12.a12BS)
		savestateEle(mode, slot, irqA12.a12SB)
		savestateEle(mode, slot, irqA12.bAdrOld)
		savestateEle(mode, slot, irqA12.sAdrOld)
	}

	/* irql2f */
	if (irql2f.present) {
		savestateEle(mode, slot, irql2f.present)
		savestateEle(mode, slot, irql2f.enable)
		savestateEle(mode, slot, irql2f.counter)
		savestateEle(mode, slot, irql2f.scanline)
		savestateEle(mode, slot, irql2f.frameX)
		savestateEle(mode, slot, irql2f.delay)
		savestateEle(mode, slot, irql2f.inFrame)
		savestateEle(mode, slot, irql2f.pending)
	}

	if (fds.info.enabled) {
		/* libero la zona di memoria gia' occupata */
		BYTE old_side_inserted = fds.drive.side_inserted;

		/* salvo, leggo o conto quello che serve */
		savestateEle(mode, slot, fds.drive.disk_position)
		savestateEle(mode, slot, fds.drive.delay)
		savestateEle(mode, slot, fds.drive.disk_ejected)
		savestateEle(mode, slot, fds.drive.side_inserted)
		savestateEle(mode, slot, fds.drive.gap_ended)
		savestateEle(mode, slot, fds.drive.scan)
		savestateEle(mode, slot, fds.drive.crc_char)
		savestateEle(mode, slot, fds.drive.enabled_dsk_reg)
		savestateEle(mode, slot, fds.drive.enabled_snd_reg)
		savestateEle(mode, slot, fds.drive.data_readed)
		savestateEle(mode, slot, fds.drive.data_to_write)
		savestateEle(mode, slot, fds.drive.transfer_flag)
		savestateEle(mode, slot, fds.drive.motor_on)
		savestateEle(mode, slot, fds.drive.transfer_reset)
		savestateEle(mode, slot, fds.drive.read_mode)
		savestateEle(mode, slot, fds.drive.mirroring)
		savestateEle(mode, slot, fds.drive.crc_control)
		savestateEle(mode, slot, fds.drive.unknow)
		savestateEle(mode, slot, fds.drive.drive_ready)
		savestateEle(mode, slot, fds.drive.irq_disk_enabled)
		savestateEle(mode, slot, fds.drive.irq_disk_high)
		savestateEle(mode, slot, fds.drive.irq_timer_enabled)
		savestateEle(mode, slot, fds.drive.irq_timer_reload_enabled)
		savestateEle(mode, slot, fds.drive.irq_timer_high)
		savestateEle(mode, slot, fds.drive.irq_timer_reload)
		savestateEle(mode, slot, fds.drive.irq_timer_counter)
		savestateEle(mode, slot, fds.drive.irq_timer_delay)
		savestateEle(mode, slot, fds.drive.data_external_connector)
		savestateEle(mode, slot, fds.drive.filler)

		/*
		 * l'fds drive l'ho aggiunto nella versione 3, mentre il
		 * sound dalla 4 in poi.
		 */
		if (savestate.version >= 4) {

			savestateEle(mode, slot, fds.snd.wave.data)
			savestateEle(mode, slot, fds.snd.wave.writable)
			savestateEle(mode, slot, fds.snd.wave.volume)
			savestateEle(mode, slot, fds.snd.wave.index)
			savestateEle(mode, slot, fds.snd.wave.counter)

			savestateEle(mode, slot, fds.snd.envelope.speed)
			savestateEle(mode, slot, fds.snd.envelope.disabled)

			savestateEle(mode, slot, fds.snd.main.silence)
			savestateEle(mode, slot, fds.snd.main.frequency)
			savestateEle(mode, slot, fds.snd.main.output)

			savestateEle(mode, slot, fds.snd.volume.speed)
			savestateEle(mode, slot, fds.snd.volume.mode)
			savestateEle(mode, slot, fds.snd.volume.increase)
			savestateEle(mode, slot, fds.snd.volume.gain)
			savestateEle(mode, slot, fds.snd.volume.counter)

			savestateEle(mode, slot, fds.snd.sweep.bias)
			savestateEle(mode, slot, fds.snd.sweep.mode)
			savestateEle(mode, slot, fds.snd.sweep.increase)
			savestateEle(mode, slot, fds.snd.sweep.speed)
			savestateEle(mode, slot, fds.snd.sweep.gain)
			savestateEle(mode, slot, fds.snd.sweep.counter)

			savestateEle(mode, slot, fds.snd.modulation.data)
			savestateEle(mode, slot, fds.snd.modulation.frequency)
			savestateEle(mode, slot, fds.snd.modulation.disabled)
			savestateEle(mode, slot, fds.snd.modulation.index)
			savestateEle(mode, slot, fds.snd.modulation.counter)
			savestateEle(mode, slot, fds.snd.modulation.mod)
		}

		/*
		 * in caso di ripristino di una salvataggio, se era caricato
		 * un'altro side del disco, devo ricaricarlo.
		 */
		if ((mode == SSREAD)  && (old_side_inserted != fds.drive.side_inserted)) {
			fds_disk_op(FDS_DISK_TIMELINE_SELECT, fds.drive.side_inserted);
			guiUpdate();
		}
	}

	if ((mode == SSCOUNT) || (mode == SSSAVE)) {
		savestate.preview[slot] = savestate.totSize[slot];
	}

	savestateMem(mode, slot, screen.data, screenSize(), TRUE)

	return (EXIT_OK);
}
BYTE nameStateFile(char *file, BYTE slot) {
	char ext[10], *lastDot, *fl;

	/* game genie */
	if (info.mapper == GAMEGENIE_MAPPER) {
		fl = info.loadRomFile;
	} else {
		fl = info.romFile;
	}

	if (!fl[0]) {
		return (EXIT_ERROR);
	}

	sprintf(file, "%s" SAVEFOLDER "/%s", info.baseFolder, basename(fl));
	sprintf(ext, ".p%02d", slot);

	/* rintraccio l'ultimo '.' nel nome */
	lastDot = strrchr(file, '.');
	/* elimino l'estensione */
	*lastDot = 0x00;
	/* aggiungo l'estensione */
	strcat(file, ext);

	return (EXIT_OK);
}
