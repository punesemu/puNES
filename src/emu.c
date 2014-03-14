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
#define __GUI_BASE__
#define __GUI_SND__
#include "gui.h"
#undef __GUI_SND__
#undef __GUI_BASE__
#include "clock.h"
#include "cpu.h"
#include "mem_map.h"
#include "mappers.h"
#include "fps.h"
#include "apu.h"
#include "ppu.h"
#include "gfx.h"
#include "text.h"
#include "sha1.h"
#include "database.h"
#include "input.h"
#include "version.h"
#include "param.h"
#include "cfg_file.h"
#include "save_slot.h"
#include "timeline.h"
#include "tas.h"
#include "ines.h"
#include "fds.h"
#include "gamegenie.h"
#include "recent_roms.h"
#if defined (SDL)
#include "opengl.h"
#endif
#include "uncompress.h"

#define recent_roms_add_wrap()\
	if (recent_roms_permit_add == TRUE) {\
		recent_roms_permit_add = FALSE;\
		recent_roms_add(save_rom_file);\
	}
#define save_rom_ext()\
	sprintf(name_file, "%s", basename(info.rom_file));\
	if (strrchr(name_file, '.') == NULL) {\
		strcpy(ext, ".nes");\
	} else {\
		/* salvo l'estensione del file */\
		strcpy(ext, strrchr(name_file, '.'));\
	}

BYTE emu_loop(void) {
#if defined (DEBUG)
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
//#if defined (MINGW32) || defined (MINGW64)
//	guiSetThreadAffinity(0);
//#endif

	fps.second_start = gui_get_ms();

	fps.next_frame = gui_get_ms() + machine.ms_frame;

	for (;;) {
		tas.lag_frame = TRUE;

		/* controllo se ci sono eventi dalla tastiera */
		gui_event();

		/* gestione uscita */
		if (info.stop == TRUE) {
			emu_quit(EXIT_SUCCESS);
		}

		/* eseguo un frame dell'emulatore */
		if (!(info.no_rom | info.pause)) {

			/* riprendo a far correre la CPU */
			info.execute_cpu = TRUE;

			while (info.execute_cpu == TRUE) {
#if defined (DEBUG)
				if (cpu.PC == PCBREAK) {
					BYTE pippo = 5;
					pippo = pippo + 1;
				}
#endif
				/* eseguo CPU, PPU e APU */
				cpu_exe_op();
			}

			if (gamegenie.phase == GG_LOAD_ROM) {
				emu_reset(CHANGE_ROM);
				gamegenie.phase = GG_FINISH;
			}

			if (tas.lag_frame) {
				tas.total_lag_frames++;
			}

			if (snd_end_frame) {
				snd_end_frame();
			}

#if defined (DEBUG)
			gfx_draw_screen(TRUE);
#else
			gfx_draw_screen(FALSE);
#endif

			if (!tas.type && (++tl.frames == tl.frames_snap)) {
				timeline_snap(TL_NORMAL);
			}

			r4011.frames++;
		} else {
			gfx_draw_screen(FALSE);
		}

		/* gestione frameskip e calcolo fps */
		fps_frameskip();
	}
	return (EXIT_OK);
}
BYTE emu_make_dir(const char *fmt, ...) {
	static char path[512];
	struct stat status;
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(path, sizeof(path), fmt, ap);
	va_end(ap);

	if (!(access(path, 0))) {
		/* se esiste controllo che sia una directory */
		stat(path, &status);

		if (!(status.st_mode & S_IFDIR)) {
			/* non e' una directory */
			return (EXIT_ERROR);
		}
	} else {
#if defined (MINGW32) || defined (MINGW64)
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
BYTE emu_load_rom(void) {
	char ext[10], name_file[255];
	BYTE recent_roms_permit_add = TRUE;

	elaborate_rom_file:
	info.no_rom = FALSE;

	fds_quit();
	map_quit();

	if (info.load_rom_file[0]) {
		strncpy(info.rom_file, info.load_rom_file, sizeof(info.rom_file));
		memset(info.load_rom_file, 0, sizeof(info.load_rom_file));
	}

	if (info.rom_file[0]) {
		char save_rom_file[1024];

		strncpy(save_rom_file, info.rom_file, 1024);

		save_rom_ext()

		if (uncomp_ctrl(ext) == EXIT_ERROR) {
			return (EXIT_ERROR);
		}

		save_rom_ext()

		if (!strcasecmp(ext, ".fds")) {
			if (fds_load_rom() == EXIT_ERROR) {
				info.rom_file[0] = 0;
				goto elaborate_rom_file;
			}
			recent_roms_add_wrap()
		} else if (!strcasecmp(ext, ".fm2")) {
			tas_file(ext, info.rom_file);
			if (!info.rom_file[0]) {
				text_add_line_info(1, "[red]error on loading rom");
				fprintf(stderr, "error on loading rom\n");
			}
			recent_roms_add_wrap()
			/* rielaboro il nome del file */
			goto elaborate_rom_file;
		} else {
			/* carico la rom in memoria */
			if (ines_load_rom() == EXIT_ERROR) {
				info.rom_file[0] = 0;
				text_add_line_info(1, "[red]error on loading rom");
				fprintf(stderr, "error on loading rom\n");
				goto elaborate_rom_file;
			}
			recent_roms_add_wrap()
		}
	} else if (info.gui) {
		/* impostazione primaria */
		info.chr.rom.banks_8k = info.prg.rom.banks_16k = 1;

		info.prg.rom.banks_8k = info.prg.rom.banks_16k * 2;
		info.chr.rom.banks_4k = info.chr.rom.banks_8k * 2;
		info.chr.rom.banks_1k = info.chr.rom.banks_4k * 4;

		/* PRG Ram */
		if (!(prg.ram = (BYTE *) malloc(0x2000))) {
			fprintf(stderr, "Out of memory\n");
			return (EXIT_ERROR);
		}

		/* PRG Rom */
		if ((prg.rom = (BYTE *) malloc(info.prg.rom.banks_16k * (16 * 1024)))) {
			memset(prg.rom, 0xEA, info.prg.rom.banks_16k * (16 * 1024));
		} else {
			fprintf(stderr, "Out of memory\n");
			return (EXIT_ERROR);
		}

		info.no_rom = TRUE;
	}

	/* setto il tipo di sistema */
	switch (cfg->mode) {
		case AUTO:
			switch (info.machine[DATABASE]) {
				case NTSC:
				case PAL:
				case DENDY:
					machine = machinedb[info.machine[DATABASE] - 1];
					break;
				case DEFAULT:
					if (info.machine[HEADER] == info.machine[DATABASE]) {
						/*
						 * posso essere nella condizione
						 * info.machine[DATABASE] == DEFAULT && info.machine[HEADER] == DEFAULT
						 * solo quando avvio senza caricare nessuna rom.
						 */
						machine = machinedb[NTSC - 1];
					} else {
						machine = machinedb[info.machine[HEADER] - 1];
					}
					break;
				default:
					machine = machinedb[NTSC - 1];
					break;
			}
			break;
		default:
			machine = machinedb[cfg->mode - 1];
			break;
	}

	return (EXIT_OK);
}
BYTE emu_search_in_database(FILE *fp) {
	BYTE *sha1prg;
	WORD i;

	/* setto i default prima della ricerca */
	info.machine[DATABASE] = info.mapper.from_db = info.id = DEFAULT;

	/* posiziono il puntatore del file */
	if (info.trainer) {
		fseek(fp, (0x10 + sizeof(trainer.data)), SEEK_SET);
	} else {
		fseek(fp, 0x10, SEEK_SET);
	}

	/* mi alloco una zona di memoria dove leggere la PRG Rom */
	sha1prg = (BYTE *) malloc(info.prg.rom.banks_16k * (16 * 1024));
	if (!sha1prg) {
		fprintf(stderr, "Out of memory\n");
		return (EXIT_ERROR);
	}
	/* leggo dal file la PRG Rom */
	if (fread(&sha1prg[0], (16 * 1024), info.prg.rom.banks_16k, fp) < info.prg.rom.banks_16k) {
		fprintf(stderr, "Error on read prg\n");
		free(sha1prg);
		return (EXIT_ERROR);
	}
	/* calcolo l'sha1 della PRG Rom */
	sha1_csum(sha1prg, info.prg.rom.banks_16k * (16 * 1024), info.sha1sum.prg.value,
	        info.sha1sum.prg.string, LOWER);
	/* libero la memoria */
	free(sha1prg);
	/* cerco nel database */
	for (i = 0; i < LENGTH(dblist); i++) {
		if (!(memcmp(dblist[i].sha1sum, info.sha1sum.prg.string, 40))) {
			info.mapper.id = dblist[i].mapper;
			info.mapper.from_db = dblist[i].type;
			info.id = dblist[i].id;
			info.machine[DATABASE] = dblist[i].machine;
			switch (info.mapper.id) {
				case 2:
					/*
					 * Fix per "Best of the Best - Championship Karate (E) [!].nes"
					 * che ha l'header INES non corretto.
					 */
					if (info.id == BAD_INES_BOTBE) {
						info.prg.rom.banks_16k = 16;
						info.chr.rom.banks_8k = 0;
					}
					break;
				case 7:
					/*
					 * Fix per "WWF Wrestlemania (E) [!].nes"
					 * che ha l'header INES non corretto.
					 */
					if (info.id == BAD_INES_WWFWE) {
						info.prg.rom.banks_16k = 8;
						info.chr.rom.banks_8k = 0;
					}
					break;
				case 10:
					/* Fix per Famicom Wars (J) [!] che ha l'header INES errato */
					if (info.id == BAD_INES_FWJ) {
						info.chr.rom.banks_8k = 8;
					}
					break;
				case 11:
					/* Fix per King Neptune's Adventure (Color Dreams) [!]
					 * che ha l'header INES errato */
					if (info.id == BAD_KING_NEPT) {
						info.prg.rom.banks_16k = 4;
						info.chr.rom.banks_8k = 4;
					}
					break;
				case 33:
					if (info.id == BAD_INES_FLINJ) {
						info.chr.rom.banks_8k = 32;
					}
					break;
				case 96:
					info.chr.rom.banks_8k = 4;
					mapper.write_vram = TRUE;
					break;
				case 191:
					if (info.id == BAD_SUGOROQUEST) {
						info.chr.rom.banks_8k = 16;
					}
					break;
				case 235:
					/*
					 * 260-in-1 [p1][b1].nes ha un numero di prg_rom_16k_count
					 * pari a 256 (0x100) ed essendo un BYTE (0xFF) quello che l'INES
					 * utilizza per indicare in numero di 16k, nell'INES header sara'
					 * presente 0.
					 * 150-in-1 [a1][p1][!].nes ha lo stesso chsum del 260-in-1 [p1][b1].nes
					 * ma ha un numero di prg_rom_16k_count di 127.
					 */
					if (!info.prg.rom.banks_16k) {
						info.prg.rom.banks_16k = 256;
					}
					break;
			}
			if (info.mapper.from_db == UNK_VERTICAL) {
				mirroring_V();
			}
			if (info.mapper.from_db == UNK_HORIZONTAL) {
				mirroring_H();
			}
			break;
		}
	}
	/* calcolo anche l'sha1 della CHR rom */
	if (info.chr.rom.banks_8k) {
		BYTE *sha1chr;

		/* mi alloco una zona di memoria dove leggere la CHR Rom */
		sha1chr = (BYTE *) malloc(info.chr.rom.banks_8k * (8 * 1024));
		if (!sha1chr) {
			fprintf(stderr, "Out of memory\n");
			return (EXIT_ERROR);
		}
		/* leggo dal file la CHR Rom */
		if (fread(&sha1chr[0], (8 * 1024), info.chr.rom.banks_8k, fp) < info.chr.rom.banks_8k) {
			fprintf(stderr, "Error on read chr\n");
			free(sha1chr);
			return (EXIT_ERROR);
		}
		/* calcolo l'sha1 della CHR Rom */
		sha1_csum(sha1chr, info.chr.rom.banks_8k * (8 * 1024), info.sha1sum.chr.value,
		        info.sha1sum.chr.string, LOWER);
		/* libero la memoria */
		free(sha1chr);
	}
	/* riposiziono il puntatore del file */
	fseek(fp, 0x10, SEEK_SET);

	return (EXIT_OK);
}
void emu_set_title(char *title) {
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
	} else if (cfg->filter == NTSC_FILTER) {
		sprintf(title, "%s (%s, %s, %s, %s", name, param_mode[machine.type].lname,
		        param_size[cfg->scale].lname, param_ntsc[cfg->ntsc_format].lname,
		        param_palette[cfg->palette].lname);
	} else {
		sprintf(title, "%s (%s, %s, %s, %s", name, param_mode[machine.type].lname,
		        param_size[cfg->scale].lname, param_filter[cfg->filter].lname,
		        param_palette[cfg->palette].lname);
	}

#if !defined (RELEASE)
	if (cfg->scale != X1) {
		char mapper_id[10];
		sprintf(mapper_id, ", %d", info.mapper.id);
		strcat(title, mapper_id);
	}
#endif

	if (cfg->scale != X1) {
		strcat(title, ", ");
		strcat(title, param_render[cfg->render].lname);
	}

	strcat(title, ")");
}
BYTE emu_turn_on(void) {
	info.reset = POWER_UP;

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

	fds_init();

	/* carico la rom in memoria */
	if (emu_load_rom()) {
		return (EXIT_ERROR);
	}

	/* ...nonche' dei puntatori alla PRG Rom... */
	map_prg_rom_8k_reset();

	cfg_file_pgs_parse();

	/* APU */
	apu_turn_on();

	/* PPU */
	if (ppu_turn_on()) {
		return (EXIT_ERROR);
	}

	/* CPU */
	cpu_turn_on();

	/*
	 * ...e inizializzazione della mapper (che
	 * deve necessariamente seguire quella della PPU.
	 */
	if (map_init()) {
		return (EXIT_ERROR);
	}

	init_PC()

	/* controller */
	input_init();

	/* joystick */
	js_init();

	/* gestione grafica */
	if (gfx_init()) {
		return (EXIT_ERROR);
	}

	/* fps */
	fps_init();

	/* gestione sonora */
	if (snd_init()) {
		return (EXIT_ERROR);
	}

	if (timeline_init()) {
		return (EXIT_ERROR);
	}

	save_slot_count_load();

	/* emulo i 9 cicli iniziali */
	{
		BYTE i;
		for (i = 0; i < 8; i++) {
			ppu_tick(1);
			apu_tick(1, NULL);
			cpu.odd_cycle = !cpu.odd_cycle;
		}
	}

	info.reset = FALSE;

	/* The End */
	return (EXIT_OK);
}
void emu_pause(BYTE mode) {
	if (mode == TRUE) {
		info.pause = TRUE;
		return;
	}

	if (mode == FALSE) {
		info.pause = FALSE;
		fps.next_frame = gui_get_ms();
		return;
	}
}
BYTE emu_reset(BYTE type) {
	emu_pause(TRUE);

	info.reset = type;

	info.first_illegal_opcode = FALSE;

	srand(time(0));

	if (info.reset == CHANGE_ROM) {
		info.r4014_precise_timing_disabled = FALSE;
		info.r2002_race_condition_disabled = FALSE;
		info.r4016_dmc_double_read_disabled = FALSE;

		/* se carico una rom durante un tas faccio un bel quit dal tas */
		if (tas.type != NOTAS) {
			tas_quit();
		}

		/* carico la rom in memoria */
		if (emu_load_rom()) {
			return (EXIT_ERROR);
		}

		cfg_file_pgs_parse();
		gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, TRUE);
	}

	chr_bank_1k_reset();

	if (info.reset >= HARD) {
		map_prg_rom_8k_reset();
	}

	/* APU */
	apu_turn_on();

	/* PPU */
	if (ppu_turn_on()) {
		return (EXIT_ERROR);
	}

	/* CPU */
	cpu_turn_on();

	/* mapper */
	if (map_init()) {
		return (EXIT_ERROR);
	}

	init_PC()

	if (info.no_rom) {
		info.reset = FALSE;

		emu_pause(FALSE);

		return (EXIT_OK);
	}

	/* controller */
	input_init();

	if (timeline_init()) {
		return (EXIT_ERROR);
	}

	if (info.reset == CHANGE_ROM) {
		save_slot_count_load();
	}

	fps_init();

	if (info.reset >= CHANGE_ROM) {
		if (snd_start()) {
			return (EXIT_ERROR);
		}
	}

	/* ritardo della CPU */
	{
		BYTE i;
		for (i = 0; i < 8; i++) {
			ppu_tick(1);
			apu_tick(1, NULL);
			cpu.odd_cycle = !cpu.odd_cycle;
		}
	}

	info.reset = FALSE;

	emu_pause(FALSE);

	return (EXIT_OK);
}
WORD emu_round_WORD(WORD number, WORD round) {
	WORD remainder = number % round;

	if (remainder < (round / 2)) {
		return (number - remainder);
	} else {
		return ((number - remainder) + round);
	}
}
void emu_quit(BYTE exit_code) {
	if (cfg->save_on_exit) {
		cfg_file_save();
	}

	map_quit();

	fds_quit();
	ppu_quit();
	snd_quit();
	gfx_quit();

	timeline_quit();

	js_quit();

	uncomp_quit();

	gui_quit();

	exit(exit_code);
}
