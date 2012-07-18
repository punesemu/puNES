/*
 * emu.c
 * Copyright (C) Fabio Cavallo 2010 <fhorse@libero.it>
 * 
 * punes is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * punes is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <libgen.h>
#include "main.h"
#include "emu.h"
#include "gui.h"
#include "clock.h"
#include "cpu6502.h"
#include "memmap.h"
#include "mappers.h"
#include "fps.h"
#include "apu.h"
#include "ppu.h"
#include "sdlgfx.h"
#include "sdlsnd.h"
#include "sdltext.h"
#include "sha1.h"
#include "database.h"
#include "input.h"
#include "version.h"
#include "opengl.h"
#include "param.h"
#include "cfgfile.h"
#include "savestate.h"
#include "timeline.h"
#include "tas.h"
#include "ines.h"
#include "fds.h"
#include "gamegenie.h"
#include "audio_filter.h"

BYTE emuLoop(void) {
#ifdef DEBUG
	WORD PCBREAK = 0xDA5A;
	PCBREAK = 0xEC06; //PCBREAK = 0xDA7A;
	PCBREAK = 0xEBE5;
	PCBREAK = 0xEBF3;
	PCBREAK = 0x60C0;
	PCBREAK = 0xE1F8;
	PCBREAK = 0xE32A;
#endif

	/*
	 * ho notato che (sotto windows, per linux ho visto
	 * un lieve peggioramento) settandol'affinity di questo
	 * thread su un singolo core,le prestazioni migliorano
	 * notevolmente. In questo caso setto l'uso del core 0.
	 */
//#if defined MINGW32 || defined MINGW64
//	guiSetThreadAffinity(0);
//#endif

	fps.second_start = guiGetMs();

	fps.next_frame = guiGetMs() + machine.msFrame;

	for (;;) {
		tas.lag_frame = TRUE;

		/* controllo se ci sono eventi dalla tastiera */
		guiEvent();

		/* gestione uscita */
		if (info.stop) {
			emuQuit(EXIT_SUCCESS);
		}

		/* eseguo un frame dell'emulatore */
		if (!(info.no_rom | info.pause)) {

			/* riprendo a far correre la CPU */
			info.executeCPU = TRUE;

			while (info.executeCPU) {
#ifdef DEBUG
				if (cpu.PC == PCBREAK) {
					BYTE pippo = 5;
					pippo = pippo + 1;
				}
#endif
				/* eseguo CPU, PPU e APU */
				cpuExeOP();
			}

			if (gamegenie.phase == GG_LOAD_ROM) {
				emuReset(CHANGEROM);
				gamegenie.phase = GG_FINISH;
				gamegenie.print = FALSE;
			}

			if (tas.lag_frame) {
				tas.total_lag_frames++;
			}

#ifdef DEBUG
			gfxDrawScreen(TRUE);
#else
			gfxDrawScreen(FALSE);
#endif

			if (!tas.type && (++tl.frames == tl.framesSnap)) {
				timelineSnap(TLNORMAL);
			}

			r4011.frames++;
		} else {
			gfxDrawScreen(FALSE);
		}

		/* gestione frameskip e calcolo fps */
		fpsFrameskip();
	}
	return (EXIT_OK);
}
BYTE emuMakeDir(char *path) {
	struct stat status;

	if (!(access(path, 0))) {
		/* se esiste controllo che sia una directory */
		stat(path, &status);

		if (!(status.st_mode & S_IFDIR)) {
			/* non e' una directory */
			return (EXIT_ERROR);
		}
	} else {
#if defined MINGW32 || defined MINGW64
		if (mkdir(path)) {
			return (EXIT_ERROR);
		}
#else
		if (mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)) {
			return (EXIT_ERROR);
		}
#endif
	}
	return (EXIT_OK);
}
BYTE emuLoadRom(void) {
	char ext[10], name_file[255];

	elaborate_rom_file: info.no_rom = FALSE;

	fdsQuit();
	mapQuit();

	if (info.loadRomFile[0]) {
		strncpy(info.romFile, info.loadRomFile, sizeof(info.romFile));
		memset(info.loadRomFile, 0, sizeof(info.loadRomFile));
	}

	if (info.romFile[0]) {
		sprintf(name_file, "%s", basename(info.romFile));

		/* salvo l'estensione del file */
		strcpy(ext, strrchr(name_file, '.'));

		if (!(strcasecmp(ext, ".fds")) || !(strcasecmp(ext, ".FDS"))) {
			if (fds_load_rom()) {
				info.romFile[0] = 0;
				goto elaborate_rom_file;
			}
		} else if (!(strcasecmp(ext, ".fm2")) || !(strcasecmp(ext, ".FM2"))) {
			tasFile(ext, info.romFile);
			if (!info.romFile[0]) {
				textAddLineInfo(1, "[red]error on loading rom");
				fprintf(stderr, "error on loading rom\n");
			}
			/* rielaboro il nome del file */
			goto elaborate_rom_file;
		} else {
			/* carico la rom in memoria */
			if (ines_load_rom()) {
				info.romFile[0] = 0;
				textAddLineInfo(1, "[red]error on loading rom");
				fprintf(stderr, "error on loading rom\n");
				goto elaborate_rom_file;
			}
		}
	} else if (info.gui) {

		info.chrRom8kCount = info.prgRom16kCount = 1;

		info.prgRom8kCount = info.prgRom16kCount * 2;
		info.chrRom4kCount = info.chrRom8kCount * 2;
		info.chrRom1kCount = info.chrRom4kCount * 4;

		/* PRG Ram */
		if (!(prg.ram = malloc(0x2000))) {
			fprintf(stderr, "Out of memory\n");
			return (EXIT_ERROR);
		}

		/* PRG Rom */
		if ((prg.rom = malloc(info.prgRom16kCount * (16 * 1024)))) {
			memset(prg.rom, 0xEA, info.prgRom16kCount * (16 * 1024));
		} else {
			fprintf(stderr, "Out of memory\n");
			return (EXIT_ERROR);
		}

		info.no_rom = TRUE;
	}

	if (cfg->mode == AUTO) {
		if (info.machineDb == PAL) {
			machine = machinedb[PAL - 1];
		} else if (info.machineDb == DENDY) {
			machine = machinedb[DENDY - 1];
		} else {
			machine = machinedb[NTSC - 1];
		}
	} else if (cfg->mode == NTSC) {
		machine = machinedb[NTSC -1];
	} else if (cfg->mode == PAL) {
		machine = machinedb[PAL - 1];
	} else if (cfg->mode == DENDY) {
		machine = machinedb[DENDY - 1];
	}

	return (EXIT_OK);
}
BYTE emuSearchInDatabase(FILE *fp) {
	BYTE *sha1prg;
	WORD i;

	/* setto i default prima della ricerca */
	info.machineDb = info.machine = 0;
	info.mapperType = info.id = DEFAULT;

	/* posiziono il puntatore del file */
	if (info.trainer) {
		fseek(fp, (0x10 + 512), SEEK_SET);
	} else {
		fseek(fp, 0x10, SEEK_SET);
	}

	/* mi alloco una zona di memoria dove leggere la PRG Rom */
	sha1prg = malloc(info.prgRom16kCount * (16 * 1024));
	if (!sha1prg) {
		fprintf(stderr, "Out of memory\n");
		return (EXIT_ERROR);
	}
	/* leggo dal file la PRG Rom */
	if (fread(&sha1prg[0], (16 * 1024), info.prgRom16kCount, fp) < info.prgRom16kCount) {
		fprintf(stderr, "Error on read prg\n");
		free(sha1prg);
		return (EXIT_ERROR);
	}
	/* calcolo l'sha1 della PRG Rom */
	sha1_csum(sha1prg, info.prgRom16kCount * (16 * 1024), info.sha1sum, info.sha1sumString, LOWER);
	/* libero la memoria */
	free(sha1prg);
	/* cerco nel database */
	for (i = 0; i < LENGTH(dblist); i++) {
		if (!(memcmp(dblist[i].sha1sum, info.sha1sumString, 40))) {
			info.mapper = dblist[i].mapper;
			info.mapperType = dblist[i].type;
			info.id = dblist[i].id;
			info.machine = dblist[i].machine;
			if (info.machine != DEFAULT) {
				info.machineDb = info.machine;
			}
			switch (info.mapper) {
				case 2:
					/*
					 * Fix per "Best of the Best - Championship Karate (E) [!].nes"
					 * che ha l'header INES non corretto.
					 */
					if (info.id == BADINESBOTBE) {
						info.prgRom16kCount = 16;
						info.chrRom8kCount = 0;
					}
					break;
				case 7:
					/*
					 * Fix per "WWF Wrestlemania (E) [!].nes"
					 * che ha l'header INES non corretto.
					 */
					if (info.id == BADINESWWFWE) {
						info.prgRom16kCount = 8;
						info.chrRom8kCount = 0;
					}
					break;
				case 10:
					/* Fix per Famicom Wars (J) [!] che ha l'header INES errato */
					if (info.id == BADINESFWJ) {
						info.chrRom8kCount = 8;
					}
					break;
				case 11:
					/* Fix per King Neptune's Adventure (Color Dreams) [!]
					 * che ha l'header INES errato */
					if (info.id == BADKINGNEPT) {
						info.prgRom16kCount = 4;
						info.chrRom8kCount = 4;
					}
					break;
				case 33:
					if (info.id == BADINEFLINJ) {
						info.chrRom8kCount = 32;
					}
					break;
				case 96:
					info.chrRom8kCount = 4;
					mapper.writeVRAM = TRUE;
					break;
				case 191:
					if (info.id == BADSUGOROQUEST) {
						info.chrRom8kCount = 16;
					}
					break;
				case 235:
					/*
					 * 260-in-1 [p1][b1].nes ha un numero di prgRom16kCount
					 * pari a 256 (0x100) ed essendo un BYTE (0xFF) quello che l'INES
					 * utilizza per indicare in numero di 16k, nell'INES header sara'
					 * presente 0.
					 * 150-in-1 [a1][p1][!].nes ha lo stesso chsum del 260-in-1 [p1][b1].nes
					 * ma ha un numero di prgRom16kCount di 127.
					 */
					if (!info.prgRom16kCount) {
						info.prgRom16kCount = 256;
					}
					break;
			}
			if (info.mapperType == UNKVERTICAL) {
				mirroring_V();
			}
			if (info.mapperType == UNKHORIZONTAL) {
				mirroring_H();
			}
			break;
		}
	}
	/* calcolo anche l'sha1 della CHR rom */
	if (info.chrRom8kCount) {
		BYTE *sha1chr;

		/* mi alloco una zona di memoria dove leggere la CHR Rom */
		sha1chr = malloc(info.chrRom8kCount * (8 * 1024));
		if (!sha1chr) {
			fprintf(stderr, "Out of memory\n");
			return (EXIT_ERROR);
		}
		/* leggo dal file la CHR Rom */
		if (fread(&sha1chr[0], (8 * 1024), info.chrRom8kCount, fp) < info.chrRom8kCount) {
			fprintf(stderr, "Error on read chr\n");
			free(sha1chr);
			return (EXIT_ERROR);
		}
		/* calcolo l'sha1 della CHR Rom */
		sha1_csum(sha1chr, info.chrRom8kCount * (8 * 1024), info.sha1sumChr, info.sha1sumStringChr,
				LOWER);
		/* libero la memoria */
		free(sha1chr);
	}
	/* riposiziono il puntatore del file */
	if (info.trainer) {
		fseek(fp, (0x10 + 512), SEEK_SET);
	} else {
		fseek(fp, 0x10, SEEK_SET);
	}
	return (EXIT_OK);
}
void emuSetTitle(char *title) {
	char name[30];

	if (!info.gui) {
		sprintf(name, "%s v%s", NAME, VERSION);
	} else {
		sprintf(name, "%s", NAME);
	}

	if (info.portable && (gfx.scale != X1)) {
		strcat(name, "_p");
	}

	if (gfx.scale == X1) {
		sprintf(title, "%s (%s", name, pMode[machine.type].lname);
	} else if (gfx.filter == RGBNTSC) {
		sprintf(title, "%s (%s, %s, %s, %s", name, pMode[machine.type].lname,
				pSize[gfx.scale].lname, pNtsc[gfx.ntscFormat].lname, pPalette[gfx.palette].lname);
	} else {
		sprintf(title, "%s (%s, %s, %s, %s", name, pMode[machine.type].lname,
				pSize[gfx.scale].lname, pFilter[gfx.filter].lname, pPalette[gfx.palette].lname);
	}

#ifndef RELEASE
	if (gfx.scale != X1) {
		char mapperType[10];
		sprintf(mapperType, ", %d", info.mapper);
		strcat(title, mapperType);
	}
#endif

#ifdef OPENGL
	if (gfx.scale != X1) {
		strcat(title, ", ");
		strcat(title, pRendering[gfx.opengl + opengl.glsl.enabled].lname);
	}
#endif

	strcat(title, ")");
}
BYTE emuTurnON(void) {
	info.reset = POWERUP;

	info.first_illegal_opcode = FALSE;

	/*
	 * per produrre una serie di numeri pseudo-random
	 * ad ogni avvio del programma inizializzo il seed
	 * con l'orologio.
	 */
	srand(time(0));

	/* l'inizializzazione della memmap della cpu e della ppu */
	memset(&mmcpu, 0x00, sizeof(mmcpu));
	memset(&prg, 0x00, sizeof(prg));
	memset(&chr, 0x00, sizeof(chr));
	memset(&ntbl, 0x00, sizeof(ntbl));
	memset(&palette, 0x00, sizeof(palette));
	memset(&oam, 0x00, sizeof(oam));
	memset(&screen, 0x00, sizeof(screen));

	info.r4014_precise_timing_disabled = FALSE;
	info.r2002_race_condition_disabled = FALSE;
	info.r4016_dmc_double_read_disabled = FALSE;

	fdsInit();

	/* carico la rom in memoria */
	if (emuLoadRom()) {
		return (EXIT_ERROR);
	}

	/* ...nonche' dei puntatori alla PRG Rom... */
	mapPrgRom8kReset();

	cfgfilePgsParse();

	/* APU */
	apuTurnON();

	/* PPU */
	if (ppuTurnON()) {
		return (EXIT_ERROR);
	}

	/* CPU */
	cpuTurnON();

	/*
	 * ...e inizializzazione della mapper (che
	 * deve necessariamente seguire quella della PPU.
	 */
	if (mapInit(info.mapper)) {
		return (EXIT_ERROR);
	}

	INIT_PC

	/* controller */
	inputInit();

	/* joystick */
	jsInit();

	/* gestione grafica */
	if (gfxInit()) {
		return (EXIT_ERROR);
	}

	/* fps */
	fpsInit();

	/* gestione sonora */
	if (sndInit()) {
		return (EXIT_ERROR);
	}

	audio_filter(cfg->audio_filter);

	if (timelineInit()) {
		return (EXIT_ERROR);
	}

	savestateCountLoad();

	/* emulo i 9 cicli iniziali */
	{
		BYTE i;
		for (i = 0; i < 8; i++) {
			ppuTick(1);
			apuTick(1, NULL);
			cpu.oddCycle = !cpu.oddCycle;
		}
	}

	info.reset = FALSE;

	/* The End */
	return (EXIT_OK);
}
void emuPause(BYTE mode) {

	if (mode == TRUE) {
		info.pause = TRUE;
		return;
	}

	if (mode == FALSE) {
		info.pause = FALSE;
		fps.next_frame = guiGetMs();
		return;
	}
}
BYTE emuReset(BYTE type) {
	emuPause(TRUE);

	info.reset = type;

	info.first_illegal_opcode = FALSE;

	srand(time(0));

	if (info.reset == CHANGEROM) {
		info.r4014_precise_timing_disabled = FALSE;
		info.r2002_race_condition_disabled = FALSE;
		info.r4016_dmc_double_read_disabled = FALSE;

		/* se carico una rom durante un tas faccio un bel quit dal tas */
		if (tas.type != NOTAS) {
			tasQuit();
		}

		/* carico la rom in memoria */
		if (emuLoadRom()) {
			return (EXIT_ERROR);
		}

		cfgfilePgsParse();
		gfxSetScreen(NOCHANGE, NOCHANGE, NOCHANGE, NOCHANGE, TRUE);
	}

	chrBank1kReset();

	if (info.reset >= HARD) {
		mapPrgRom8kReset();
	}

	/* APU */
	apuTurnON();

	/* PPU */
	if (ppuTurnON()) {
		return (EXIT_ERROR);
	}

	/* CPU */
	cpuTurnON();

	/* mapper */
	if (mapInit(info.mapper)) {
		return (EXIT_ERROR);
	}

	INIT_PC

	if (info.no_rom) {
		info.reset = FALSE;

		emuPause(FALSE);

		return (EXIT_OK);
	}

	/* controller */
	inputInit();

	if (timelineInit()) {
		return (EXIT_ERROR);
	}

	if (info.reset == CHANGEROM) {
		savestateCountLoad();
	}


	fpsInit();

	if (info.reset >= CHANGEROM) {
		if (sndStart()) {
			return (EXIT_ERROR);
		}
	}

	/* ritardo della CPU */
	{
		BYTE i;
		for (i = 0; i < 8; i++) {
			ppuTick(1);
			apuTick(1, NULL);
			cpu.oddCycle = !cpu.oddCycle;
		}
	}

	info.reset = FALSE;

	emuPause(FALSE);

	return (EXIT_OK);
}
void emuQuit(BYTE exitCode) {
	if (cfg->saveOnExit) {
		cfgfileSave();
	}

	mapQuit();

	fdsQuit();
	ppuQuit();
	gfxQuit();
	sndQuit();

	timelineQuit();

	jsQuit();

	exit(exitCode);
}
