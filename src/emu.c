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
#include "cpu.h"
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
#include "cfg_file.h"
#include "savestate.h"
#include "timeline.h"
#include "tas.h"
#include "ines.h"
#include "fds.h"
#include "gamegenie.h"

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
			info.execute_cpu = TRUE;

			while (info.execute_cpu) {
#ifdef DEBUG
				if (cpu.PC == PCBREAK) {
					BYTE pippo = 5;
					pippo = pippo + 1;
				}
#endif
				/* eseguo CPU, PPU e APU */
				cpu_exe_op();
			}

			if (gamegenie.phase == GG_LOAD_ROM) {
				emuReset(CHANGEROM);
				gamegenie.phase = GG_FINISH;
				gamegenie.print = FALSE;
			}

			if (tas.lag_frame) {
				tas.total_lag_frames++;
			}

			if (snd_end_frame) {
				snd_end_frame();
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
	map_quit();

	if (info.load_rom_file[0]) {
		strncpy(info.rom_file, info.load_rom_file, sizeof(info.rom_file));
		memset(info.load_rom_file, 0, sizeof(info.load_rom_file));
	}

	if (info.rom_file[0]) {
		sprintf(name_file, "%s", basename(info.rom_file));

		/* salvo l'estensione del file */
		strcpy(ext, strrchr(name_file, '.'));

		if (!(strcasecmp(ext, ".fds")) || !(strcasecmp(ext, ".FDS"))) {
			if (fds_load_rom()) {
				info.rom_file[0] = 0;
				goto elaborate_rom_file;
			}
		} else if (!(strcasecmp(ext, ".fm2")) || !(strcasecmp(ext, ".FM2"))) {
			tasFile(ext, info.rom_file);
			if (!info.rom_file[0]) {
				textAddLineInfo(1, "[red]error on loading rom");
				fprintf(stderr, "error on loading rom\n");
			}
			/* rielaboro il nome del file */
			goto elaborate_rom_file;
		} else {
			/* carico la rom in memoria */
			if (ines_load_rom()) {
				info.rom_file[0] = 0;
				textAddLineInfo(1, "[red]error on loading rom");
				fprintf(stderr, "error on loading rom\n");
				goto elaborate_rom_file;
			}
		}
	} else if (info.gui) {

		info.chr_rom_8k_count = info.prg_rom_16k_count = 1;

		info.prg_rom_8k_count = info.prg_rom_16k_count * 2;
		info.chr_rom_4k_count = info.chr_rom_8k_count * 2;
		info.chr_rom_1k_count = info.chr_rom_4k_count * 4;

		/* PRG Ram */
		if (!(prg.ram = malloc(0x2000))) {
			fprintf(stderr, "Out of memory\n");
			return (EXIT_ERROR);
		}

		/* PRG Rom */
		if ((prg.rom = malloc(info.prg_rom_16k_count * (16 * 1024)))) {
			memset(prg.rom, 0xEA, info.prg_rom_16k_count * (16 * 1024));
		} else {
			fprintf(stderr, "Out of memory\n");
			return (EXIT_ERROR);
		}

		info.no_rom = TRUE;
	}

	if (cfg->mode == AUTO) {
		if (info.machine_db == PAL) {
			machine = machinedb[PAL - 1];
		} else if (info.machine_db == DENDY) {
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
	info.machine_db = info.machine = 0;
	info.mapper_type = info.id = DEFAULT;

	/* posiziono il puntatore del file */
	if (info.trainer) {
		fseek(fp, (0x10 + 512), SEEK_SET);
	} else {
		fseek(fp, 0x10, SEEK_SET);
	}

	/* mi alloco una zona di memoria dove leggere la PRG Rom */
	sha1prg = malloc(info.prg_rom_16k_count * (16 * 1024));
	if (!sha1prg) {
		fprintf(stderr, "Out of memory\n");
		return (EXIT_ERROR);
	}
	/* leggo dal file la PRG Rom */
	if (fread(&sha1prg[0], (16 * 1024), info.prg_rom_16k_count, fp) < info.prg_rom_16k_count) {
		fprintf(stderr, "Error on read prg\n");
		free(sha1prg);
		return (EXIT_ERROR);
	}
	/* calcolo l'sha1 della PRG Rom */
	sha1_csum(sha1prg, info.prg_rom_16k_count * (16 * 1024), info.sha1sum, info.sha1sum_string, LOWER);
	/* libero la memoria */
	free(sha1prg);
	/* cerco nel database */
	for (i = 0; i < LENGTH(dblist); i++) {
		if (!(memcmp(dblist[i].sha1sum, info.sha1sum_string, 40))) {
			info.mapper = dblist[i].mapper;
			info.mapper_type = dblist[i].type;
			info.id = dblist[i].id;
			info.machine = dblist[i].machine;
			if (info.machine != DEFAULT) {
				info.machine_db = info.machine;
			}
			switch (info.mapper) {
				case 2:
					/*
					 * Fix per "Best of the Best - Championship Karate (E) [!].nes"
					 * che ha l'header INES non corretto.
					 */
					if (info.id == BADINESBOTBE) {
						info.prg_rom_16k_count = 16;
						info.chr_rom_8k_count = 0;
					}
					break;
				case 7:
					/*
					 * Fix per "WWF Wrestlemania (E) [!].nes"
					 * che ha l'header INES non corretto.
					 */
					if (info.id == BADINESWWFWE) {
						info.prg_rom_16k_count = 8;
						info.chr_rom_8k_count = 0;
					}
					break;
				case 10:
					/* Fix per Famicom Wars (J) [!] che ha l'header INES errato */
					if (info.id == BADINESFWJ) {
						info.chr_rom_8k_count = 8;
					}
					break;
				case 11:
					/* Fix per King Neptune's Adventure (Color Dreams) [!]
					 * che ha l'header INES errato */
					if (info.id == BADKINGNEPT) {
						info.prg_rom_16k_count = 4;
						info.chr_rom_8k_count = 4;
					}
					break;
				case 33:
					if (info.id == BADINEFLINJ) {
						info.chr_rom_8k_count = 32;
					}
					break;
				case 96:
					info.chr_rom_8k_count = 4;
					mapper.write_vram = TRUE;
					break;
				case 191:
					if (info.id == BADSUGOROQUEST) {
						info.chr_rom_8k_count = 16;
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
					if (!info.prg_rom_16k_count) {
						info.prg_rom_16k_count = 256;
					}
					break;
			}
			if (info.mapper_type == UNKVERTICAL) {
				mirroring_V();
			}
			if (info.mapper_type == UNKHORIZONTAL) {
				mirroring_H();
			}
			break;
		}
	}
	/* calcolo anche l'sha1 della CHR rom */
	if (info.chr_rom_8k_count) {
		BYTE *sha1chr;

		/* mi alloco una zona di memoria dove leggere la CHR Rom */
		sha1chr = malloc(info.chr_rom_8k_count * (8 * 1024));
		if (!sha1chr) {
			fprintf(stderr, "Out of memory\n");
			return (EXIT_ERROR);
		}
		/* leggo dal file la CHR Rom */
		if (fread(&sha1chr[0], (8 * 1024), info.chr_rom_8k_count, fp) < info.chr_rom_8k_count) {
			fprintf(stderr, "Error on read chr\n");
			free(sha1chr);
			return (EXIT_ERROR);
		}
		/* calcolo l'sha1 della CHR Rom */
		sha1_csum(sha1chr, info.chr_rom_8k_count * (8 * 1024), info.sha1sum_chr,
		        info.sha1sum_string_chr, LOWER);
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

	if (info.portable && (cfg->scale != X1)) {
		strcat(name, "_p");
	}

	if (cfg->scale == X1) {
		sprintf(title, "%s (%s", name, param_mode[machine.type].lname);
	} else if (cfg->filter == RGBNTSC) {
		sprintf(title, "%s (%s, %s, %s, %s", name, param_mode[machine.type].lname,
		        param_size[cfg->scale].lname, param_ntsc[cfg->ntsc_format].lname,
		        param_palette[cfg->palette].lname);
	} else {
		sprintf(title, "%s (%s, %s, %s, %s", name, param_mode[machine.type].lname,
		        param_size[cfg->scale].lname, param_filter[cfg->filter].lname,
		        param_palette[cfg->palette].lname);
	}

#ifndef RELEASE
	if (cfg->scale != X1) {
		char mapper_type[10];
		sprintf(mapper_type, ", %d", info.mapper);
		strcat(title, mapper_type);
	}
#endif

	if (cfg->scale != X1) {
		strcat(title, ", ");
		strcat(title, param_render[cfg->render].lname);
	}

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
	map_prg_rom_8k_reset();

	cfg_file_pgs_parse();

	/* APU */
	apu_turn_on();

	/* PPU */
	if (ppuTurnON()) {
		return (EXIT_ERROR);
	}

	/* CPU */
	cpu_turn_on();

	/*
	 * ...e inizializzazione della mapper (che
	 * deve necessariamente seguire quella della PPU.
	 */
	if (map_init(info.mapper)) {
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

	if (timelineInit()) {
		return (EXIT_ERROR);
	}

	savestateCountLoad();

	/* emulo i 9 cicli iniziali */
	{
		BYTE i;
		for (i = 0; i < 8; i++) {
			ppuTick(1);
			apu_tick(1, NULL);
			cpu.odd_cycle = !cpu.odd_cycle;
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

		cfg_file_pgs_parse();
		gfxSetScreen(NOCHANGE, NOCHANGE, NOCHANGE, NOCHANGE, TRUE);
	}

	chr_bank_1k_reset();

	if (info.reset >= HARD) {
		map_prg_rom_8k_reset();
	}

	/* APU */
	apu_turn_on();

	/* PPU */
	if (ppuTurnON()) {
		return (EXIT_ERROR);
	}

	/* CPU */
	cpu_turn_on();

	/* mapper */
	if (map_init(info.mapper)) {
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
			apu_tick(1, NULL);
			cpu.odd_cycle = !cpu.odd_cycle;
		}
	}

	info.reset = FALSE;

	emuPause(FALSE);

	return (EXIT_OK);
}
void emuQuit(BYTE exitCode) {
	if (cfg->save_on_exit) {
		cfg_file_save();
	}

	map_quit();

	fdsQuit();
	ppuQuit();
	gfxQuit();
	sndQuit();

	timelineQuit();

	jsQuit();

	exit(exitCode);
}
