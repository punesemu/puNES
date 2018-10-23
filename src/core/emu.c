/*
 *  Copyright (C) 2010-2019 Fabio Cavallo (aka FHorse)
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

#include <time.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <libgen.h>
#include "main.h"
#include "emu.h"
#include "rom_mem.h"
#include "info.h"
#include "settings.h"
#include "snd.h"
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
#include "conf.h"
#include "save_slot.h"
#include "timeline.h"
#include "tas.h"
#include "ines.h"
#include "unif.h"
#include "fds.h"
#include "nsf.h"
#include "nsfe.h"
#include "patcher.h"
#include "cheat.h"
#include "overscan.h"
#include "recent_roms.h"
#if defined (WITH_OPENGL)
#include "opengl.h"
#endif
#include "uncompress.h"
#include "gui.h"

#define RS_SCALE (1.0f / (1.0f + RAND_MAX))

#if defined (DEBUG)
	WORD PCBREAK = 0xC425;
#endif

static BYTE emu_ctrl_if_rom_exist(void);
static uTCHAR *emu_ctrl_rom_ext(uTCHAR *file);
static void emu_recent_roms_add(BYTE *add, uTCHAR *file);

BYTE emu_frame(void) {
	// gestione uscita
	if (info.stop == TRUE) {
		return (EXIT_OK);
	}

	gui_ef_lock();

	gui_control_visible_cursor();

	tas.lag_frame = TRUE;

	// eseguo un frame dell'emulatore
	if (!(info.no_rom | info.turn_off | info.pause)) {
		if (nsf.state & (NSF_PAUSE | NSF_STOP)) {
			BYTE i;

			for (i = PORT1; i < PORT_MAX; i++) {
				if (port_funct[i].input_add_event) {
					port_funct[i].input_add_event(i);
				}
			}

			extcl_audio_samples_mod_nsf(NULL, 0);
			nsf_main_screen_event();
			nsf_effect();

			gui_ef_unlock();

			gfx_draw_screen(TRUE);

			fps_frameskip();

			return (EXIT_OK);
		}

		// controllo se ci sono eventi di input
		if (tas.type) {
			tas_frame();
		} else {
			BYTE i;

			for (i = PORT1; i < PORT_MAX; i++) {
				if (port_funct[i].input_add_event) {
					port_funct[i].input_add_event(i);
				}
			}
		}

		// riprendo a far correre la CPU
		info.execute_cpu = TRUE;

		while (info.execute_cpu == TRUE) {
#if defined (DEBUG)
			if (cpu.PC == PCBREAK) {
				BYTE pippo = 5;
				pippo = pippo + 1;
			}
#endif
			// eseguo CPU, PPU e APU
			cpu_exe_op();
		}

		if ((cfg->cheat_mode == GAMEGENIE_MODE) && (gamegenie.phase == GG_LOAD_ROM)) {
			gui_ef_emit_gg_reset();
		}

		if (tas.lag_frame) {
			tas.total_lag_frames++;
			gui_ppu_hacks_widgets_update();
		}

		if (snd_end_frame) {
			snd_end_frame();
		}

		if (!tas.type && (++tl.frames == tl.frames_snap)) {
			timeline_snap(TL_NORMAL);
		}

		if (cfg->save_battery_ram_file && (++info.bat_ram_frames >= info.bat_ram_frames_snap)) {
			// faccio anche un refresh del file della battery ram
			info.bat_ram_frames = 0;
			map_prg_ram_battery_save();
		}

		if (vs_system.enabled & vs_system.watchdog.reset) {
			gui_ef_emit_vs_reset();
		}

		r4011.frames++;

		gui_ef_unlock();

#if defined (DEBUG)
		gfx_draw_screen(TRUE);
#else
		gfx_draw_screen(FALSE);
#endif
	} else {
		gui_ef_unlock();

		//gfx_draw_screen(TRUE);
		gfx_draw_screen(FALSE);
	}

	// gestione frameskip e calcolo fps
	fps_frameskip();
	return (EXIT_OK);
}
BYTE emu_make_dir(const uTCHAR *fmt, ...) {
	static uTCHAR path[LENGTH_FILE_NAME_MID];
	struct ustructstat status;
	va_list ap;

	va_start(ap, fmt);
	uvsnprintf(path, usizeof(path), fmt, ap);
	va_end(ap);

	if (!(uaccess(path, 0))) {
		// se esiste controllo che sia una directory
		ustat(path, &status);

		if (!(status.st_mode & S_IFDIR)) {
			// non e' una directory
			return (EXIT_ERROR);
		}
	} else {
#if defined (__WIN32__)
		if (_wmkdir(path)) {
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
BYTE emu_file_exist(const uTCHAR *file) {
	struct ustructstat status;

	if (!(uaccess(file, 0))) {
		ustat(file, &status);
		if (status.st_mode & S_IFREG) {
			return (EXIT_OK);
		}
	}
	return (EXIT_ERROR);
}
char *emu_file2string(const uTCHAR *path) {
	FILE *fd;
	long len, r;
	char *str;

	if (!(fd = ufopen(path, uL("r")))) {
		ufprintf(stderr, uL("OPENGL: Can't open file '" uPERCENTs "' for reading\n"), path);
		return (NULL);
	}

	fseek(fd, 0, SEEK_END);
	len = ftell(fd);
	fseek(fd, 0, SEEK_SET);

	if (!(str = (char *) malloc(len * sizeof(char)))) {
		fclose(fd);
		ufprintf(stderr, uL("OPENGL: Can't malloc space for '" uPERCENTs "'\n"), path);
		return (NULL);
	}

	r = fread(str, sizeof(char), len, fd);

	str[r - 1] = '\0';

	fclose(fd);

	return (str);
}
BYTE emu_load_rom(void) {
	BYTE recent_roms_permit_add = TRUE;

	elaborate_rom_file:
	info.no_rom = FALSE;
	info.cpu_rw_extern = FALSE;

	cheatslist_save_game_cheats();

	nsf_quit();
	fds_quit();
	map_quit();

	if (info.rom.file[0]) {
		uTCHAR *ext = emu_ctrl_rom_ext(info.rom.file);

		if (!ustrcasecmp(ext, uL(".fds"))) {
			if (fds_load_rom() == EXIT_ERROR) {
				info.rom.file[0] = 0;
				goto elaborate_rom_file;
			}
			emu_recent_roms_add(&recent_roms_permit_add, info.rom.file);
		} else if (!ustrcasecmp(ext, uL(".nsf"))) {
			if (nsf_load_rom() == EXIT_ERROR) {;
				info.rom.file[0] = 0;
				text_add_line_info(1, "[red]error loading rom");
				fprintf(stderr, "error loading rom\n");
				goto elaborate_rom_file;
			}
			emu_recent_roms_add(&recent_roms_permit_add, info.rom.file);
		} else if (!ustrcasecmp(ext, uL(".nsfe"))) {
			if (nsfe_load_rom() == EXIT_ERROR) {;
				info.rom.file[0] = 0;
				text_add_line_info(1, "[red]error loading rom");
				fprintf(stderr, "error loading rom\n");
				goto elaborate_rom_file;
			}
			emu_recent_roms_add(&recent_roms_permit_add, info.rom.file);
		} else if (!ustrcasecmp(ext, uL(".fm2"))) {
			tas_file(ext, info.rom.file);
			if (!info.rom.file[0]) {
				text_add_line_info(1, "[red]error loading rom");
				fprintf(stderr, "error loading rom\n");
			}
			emu_recent_roms_add(&recent_roms_permit_add, tas.file);
			// rielaboro il nome del file
			goto elaborate_rom_file;
		} else {
			// carico la rom in memoria
			if (ines_load_rom() == EXIT_OK) {
				;
			} else if (unif_load_rom() == EXIT_ERROR) {
				info.rom.file[0] = 0;
				text_add_line_info(1, "[red]error loading rom");
				fprintf(stderr, "error loading rom\n");
				goto elaborate_rom_file;
			}
			emu_recent_roms_add(&recent_roms_permit_add, info.rom.file);
			info.turn_off = FALSE;
		}
	} else if (info.gui) {
		// impostazione primaria
		info.prg.rom[0].banks_16k = info.chr.rom[0].banks_8k = 1;

		info.prg.rom[0].banks_8k = info.prg.rom[0].banks_16k * 2;
		info.chr.rom[0].banks_4k = info.chr.rom[0].banks_8k * 2;
		info.chr.rom[0].banks_1k = info.chr.rom[0].banks_4k * 4;

		// PRG Ram
		if (map_prg_ram_malloc(0x2000) != EXIT_OK) {
			return (EXIT_ERROR);
		}

		// PRG Rom
		if (map_prg_chip_malloc(0, info.prg.rom[0].banks_16k * (16 * 1024), 0xEA) == EXIT_ERROR) {
			return (EXIT_ERROR);
		}

		info.no_rom = TRUE;
	}

	// setto il tipo di sistema
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
						// posso essere nella condizione
						// info.machine[DATABASE] == DEFAULT && info.machine[HEADER] == DEFAULT
						// solo quando avvio senza caricare nessuna rom.
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

	if (nsf.enabled == FALSE) {
		cheatslist_read_game_cheats();
	}

	return (EXIT_OK);
}
BYTE emu_search_in_database(void *rom_mem) {
	_rom_mem *rom = (_rom_mem *)rom_mem;
	size_t position;
	WORD i;

	// setto i default prima della ricerca
	info.machine[DATABASE] = info.mapper.submapper = info.mirroring_db = info.id = DEFAULT;
	info.extra_from_db = 0;
	vs_system.ppu = vs_system.special_mode.type = DEFAULT;

	// punto oltre l'header
	if (info.trainer) {
		position = (0x10 + sizeof(trainer.data));
	} else {
		position = 0x10;
	}

	if ((position + (info.prg.rom[0].banks_16k * 0x4000)) > rom->size) {
		info.prg.rom[0].banks_16k = (rom->size - position) / 0x4000;
		fprintf(stderr, "truncated PRG ROM\n");
	}

	// calcolo l'sha1 della PRG Rom
	sha1_csum(rom->data + position, 0x4000 * info.prg.rom[0].banks_16k, info.sha1sum.prg.value,
		info.sha1sum.prg.string, LOWER);
	position += (info.prg.rom[0].banks_16k * 0x4000);

	if (info.chr.rom[0].banks_8k) {
		if ((position + (info.chr.rom[0].banks_8k * 0x2000)) > rom->size) {
			info.chr.rom[0].banks_8k = (rom->size - position) / 0x2000;
			fprintf(stderr, "truncated CHR ROM\n");
		}

		// calcolo anche l'sha1 della CHR rom
		sha1_csum(rom->data + position, 0x2000 * info.chr.rom[0].banks_8k, info.sha1sum.chr.value,
			info.sha1sum.chr.string, LOWER);
		position += (info.chr.rom[0].banks_8k * 0x2000);
	}

	// cerco nel database
	for (i = 0; i < LENGTH(dblist); i++) {
		if (!(memcmp(dblist[i].sha1sum, info.sha1sum.prg.string, 40))) {
			info.mapper.id = dblist[i].mapper;
			info.mapper.submapper = dblist[i].submapper;
			info.id = dblist[i].id;
			info.machine[DATABASE] = dblist[i].machine;
			info.mirroring_db = dblist[i].mirroring;
			vs_system.ppu = dblist[i].vs_ppu;
			vs_system.special_mode.type = dblist[i].vs_sm;
			info.default_dipswitches = dblist[i].dipswitches;
			info.extra_from_db = dblist[i].extra;
			switch (info.mapper.id) {
				case 1:
					// Fix per Famicom Wars (J) [!] che ha l'header INES errato
					if (info.id == BAD_YOSHI_U) {
						info.chr.rom[0].banks_8k = 4;
					} else if (info.id == MOWPC10) {
						info.chr.rom[0].banks_8k = 0;
					}
					break;
				case 2:
					// Fix per "Best of the Best - Championship Karate (E) [!].nes"
					// che ha l'header INES non corretto.
					if (info.id == BAD_INES_BOTBE) {
						info.prg.rom[0].banks_16k = 16;
						info.chr.rom[0].banks_8k = 0;
					}
					break;
				case 3:
					// Fix per "Tetris (Bulletproof) (Japan).nes"
					// che ha l'header INES non corretto.
					if (info.id == BAD_INES_TETRIS_BPS) {
						info.prg.rom[0].banks_16k = 2;
						info.chr.rom[0].banks_8k = 2;
					}
					break;
				case 7:
					// Fix per "WWF Wrestlemania (E) [!].nes"
					// che ha l'header INES non corretto.
					if (info.id == BAD_INES_WWFWE) {
						info.prg.rom[0].banks_16k = 8;
						info.chr.rom[0].banks_8k = 0;
					} else if (info.id == CSPC10) {
						info.chr.rom[0].banks_8k = 0;
					}
					break;
				case 10:
					// Fix per Famicom Wars (J) [!] che ha l'header INES errato
					if (info.id == BAD_INES_FWJ) {
						info.chr.rom[0].banks_8k = 8;
					}
					break;
				case 11:
					// Fix per King Neptune's Adventure (Color Dreams) [!]
					// che ha l'header INES errato
					if (info.id == BAD_KING_NEPT) {
						info.prg.rom[0].banks_16k = 4;
						info.chr.rom[0].banks_8k = 4;
					}
					break;
				case 33:
					if (info.id == BAD_INES_FLINJ) {
						info.chr.rom[0].banks_8k = 32;
					}
					break;
				case 113:
					if (info.id == BAD_INES_SWAUS) {
						info.prg.rom[0].banks_16k = 1;
						info.chr.rom[0].banks_8k = 2;
					}
					break;
				case 191:
					if (info.id == BAD_SUGOROQUEST) {
						info.chr.rom[0].banks_8k = 16;
					}
					break;
				case 235:
					// 260-in-1 [p1][b1].nes ha un numero di prg_rom_16k_count
					// pari a 256 (0x100) ed essendo un BYTE (0xFF) quello che l'INES
					// utilizza per indicare in numero di 16k, nell'INES header sara'
					// presente 0.
					// 150-in-1 [a1][p1][!].nes ha lo stesso chsum del 260-in-1 [p1][b1].nes
					// ma ha un numero di prg_rom_16k_count di 127.
					if (!info.prg.rom[0].banks_16k) {
						info.prg.rom[0].banks_16k = 256;
					}
					break;
				case UNIF_MAPPER:
					unif.internal_mapper = info.mapper.submapper;
					break;

			}
			if (info.mirroring_db == UNK_VERTICAL) {
				mirroring_V();
			}
			if (info.mirroring_db == UNK_HORIZONTAL) {
				mirroring_H();
			}
			break;
		}
	}

	if ((vs_system.ppu == DEFAULT) && (vs_system.special_mode.type == DEFAULT)) {
		vs_system.ppu = vs_system.special_mode.type = 0;
	}

	return (EXIT_OK);
}
void emu_set_title(uTCHAR *title, int len) {
	uTCHAR name[30];

	if (!info.gui) {
		usnprintf(name, usizeof(name), uL("" NAME " v" VERSION));
	} else {
		usnprintf(name, usizeof(name), uL("" NAME));
	}

	if (info.portable && (cfg->scale != X1)) {
		ustrcat(name, uL("_p"));
	}

	if (cfg->scale == X1) {
		usnprintf(title, len, uL("" uPERCENTs " (" uPERCENTs), name, opt_mode[machine.type].lname);
	} else if (cfg->filter == NTSC_FILTER) {
		usnprintf(title, len,
				uL("" uPERCENTs " (" uPERCENTs ", " uPERCENTs ", " uPERCENTs ", "),
				name, opt_mode[machine.type].lname,
				opt_scale[cfg->scale - 1].sname, opt_ntsc[cfg->ntsc_format].lname);
	} else {
		usnprintf(title, len,
				uL("" uPERCENTs " (" uPERCENTs ", " uPERCENTs ", " uPERCENTs ", "),
				name, opt_mode[machine.type].lname,
				opt_scale[cfg->scale - 1].sname, opt_filter[cfg->filter].lname);
	}

	if (cfg->scale != X1) {
		if ((cfg->palette == PALETTE_FILE) && ustrlen(cfg->palette_file) != 0) {
			ustrcat(title, uL("extern"));
		} else {
			ustrcat(title, opt_palette[cfg->palette].lname);
		}
	}

#if !defined (RELEASE)
	if (cfg->scale != X1) {
		uTCHAR mapper_id[10];
		usnprintf(mapper_id, usizeof(mapper_id), uL(", %d"), info.mapper.id);
		ustrcat(title, mapper_id);
	}
#endif

	ustrcat(title, uL(")"));
}
BYTE emu_turn_on(void) {
	info.reset = POWER_UP;

	info.first_illegal_opcode = FALSE;

	// per produrre una serie di numeri pseudo-random
	// ad ogni avvio del programma inizializzo il seed
	// con l'orologio.
	srand(time(0));

	// l'inizializzazione della memmap della cpu e della ppu
	memset(&mmcpu, 0x00, sizeof(mmcpu));
	memset(&prg, 0x00, sizeof(prg));
	memset(&chr, 0x00, sizeof(chr));
	memset(&ntbl, 0x00, sizeof(ntbl));
	memset(&palette, 0x00, sizeof(palette));
	memset(&oam, 0x00, sizeof(oam));
	memset(&screen, 0x00, sizeof(screen));
	memset(&vs_system, 0x00, sizeof(vs_system));

	cfg->extra_vb_scanlines = cfg->extra_pr_scanlines = 0;

	vs_system.watchdog.next = vs_system_wd_next();

	info.r4014_precise_timing_disabled = FALSE;
	info.r2002_race_condition_disabled = FALSE;
	info.r4016_dmc_double_read_disabled = FALSE;

	cheatslist_init();

	nsf_init();
	fds_init();

	// carico la rom in memoria
	{
		emu_ctrl_if_rom_exist();

		if (emu_load_rom()) {
			return (EXIT_ERROR);
		}
	}

	overscan_set_mode(machine.type);

	// ...nonche' dei puntatori alla PRG Rom...
	map_prg_rom_8k_reset();

	settings_pgs_parse();

	// APU
	apu_turn_on();

	// PPU
	if (ppu_turn_on()) {
		return (EXIT_ERROR);
	}

	// CPU
	cpu_turn_on();

	// ...e inizializzazione della mapper (che
	// deve necessariamente seguire quella della PPU.
	if (map_init()) {
		return (EXIT_ERROR);
	}

	cpu_init_PC();

	// controller
	input_init(NO_SET_CURSOR);

	// joystick
	js_init(TRUE);

	// gestione grafica
	if (gfx_init()) {
		return (EXIT_ERROR);
	}

	// setto il cursore
	gfx_cursor_init();

	// fps
	fps_init();

	// gestione sonora
	if (snd_init()) {
		return (EXIT_ERROR);
	}

	if (timeline_init()) {
		return (EXIT_ERROR);
	}

	save_slot_count_load();

	// emulo i 9 cicli iniziali
	{
		BYTE i;
		for (i = 0; i < 8; i++) {
			ppu_tick();
			apu_tick(NULL);
			cpu.odd_cycle = !cpu.odd_cycle;
		}
	}

	ext_win.vs_system = vs_system.enabled;
	if (vs_system.enabled == TRUE) {
		if ((cfg->dipswitch == 0xFF00) && (info.default_dipswitches != 0xFF00)) {
			cfg->dipswitch = info.default_dipswitches;
		}
	}

	gui_external_control_windows_show();

	info.bat_ram_frames_snap = machine.fps * (60 * 3);

	info.reset = FALSE;

	// The End
	return (EXIT_OK);
}
void emu_pause(BYTE mode) {
	if (mode == TRUE) {
		info.pause++;
	} else {
		if (--info.pause == 0xFFFF) {
			info.pause = 0;
		}
	}

	if (info.pause == 0) {
		fps.next_frame = gui_get_ms();

		if (nsf.enabled) {
			nsf_reset_timers();
		}
	}
}
BYTE emu_reset(BYTE type) {
	if (info.turn_off && (type <= HARD)) {
		return (EXIT_OK);
	}

	emu_pause(TRUE);

	if (type == CHANGE_ROM) {
		if (emu_ctrl_if_rom_exist() == EXIT_ERROR) {
			emu_pause(FALSE);
			return (EXIT_OK);
		}
	}

	info.reset = type;

	info.first_illegal_opcode = FALSE;

	srand(time(0));

	if (info.reset == CHANGE_ROM) {
		info.r4014_precise_timing_disabled = FALSE;
		info.r2002_race_condition_disabled = FALSE;
		info.r4016_dmc_double_read_disabled = FALSE;

		// se carico una rom durante un tas faccio un bel quit dal tas
		if (tas.type != NOTAS) {
			tas_quit();
		}

		// carico la rom in memoria
		if (emu_load_rom()) {
			return (EXIT_ERROR);
		}

		ext_win.vs_system = vs_system.enabled;

		overscan_set_mode(machine.type);

		settings_pgs_parse();

		gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, TRUE, TRUE);

		gui_update_gps_settings();
	}

	if ((info.reset == CHANGE_MODE) && (overscan_set_mode(machine.type) == TRUE)) {
		gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, TRUE, FALSE);
	}

	map_chr_bank_1k_reset();

	if (info.reset >= HARD) {
		map_prg_rom_8k_reset();
	}

	// APU
	apu_turn_on();

	// PPU
	if (ppu_turn_on()) {
		return (EXIT_ERROR);
	}

	// CPU
	cpu_turn_on();

	// mapper
	if (map_init()) {
		return (EXIT_ERROR);
	}

	cpu_init_PC();

	if (info.no_rom) {
		info.reset = FALSE;
		if (info.pause_from_gui == TRUE) {
			info.pause_from_gui = FALSE;
			emu_pause(FALSE);
		}

		emu_pause(FALSE);

		return (EXIT_OK);
	}

	// controller
	input_init(SET_CURSOR);

	// joystick
	js_quit(FALSE);
	js_init(FALSE);

	if (timeline_init()) {
		return (EXIT_ERROR);
	}

	if (info.reset == CHANGE_ROM) {
		save_slot_count_load();
	}

	fps_init();

	if (info.reset >= CHANGE_ROM) {
		if (snd_playback_start()) {
			return (EXIT_ERROR);
		}
	}

	// ritardo della CPU
	{
		BYTE i;
		for (i = 0; i < 8; i++) {
			ppu_tick();
			apu_tick(NULL);
			cpu.odd_cycle = !cpu.odd_cycle;
		}
	}

	if (vs_system.enabled == TRUE) {
		if (type >= HARD) {
			vs_system.shared_mem = 0;
		}
		if ((cfg->dipswitch == 0xFF00) && (info.default_dipswitches != 0xFF00)) {
			cfg->dipswitch = info.default_dipswitches;
		}
	}
	memset(&vs_system.watchdog, 0x00, sizeof(vs_system.watchdog));
	memset(&vs_system.r4020, 0x00, sizeof(vs_system.r4020));
	memset(&vs_system.coins, 0x00, sizeof(vs_system.coins));
	vs_system.watchdog.next = vs_system_wd_next();

	gui_ef_emit_external_control_windows_show();

	info.bat_ram_frames = 0;

	info.reset = FALSE;

	if (info.pause_from_gui == TRUE) {
		info.pause_from_gui = FALSE;
		emu_pause(FALSE);
	}

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
int emu_power_of_two(int base) {
	int pot = 1;

	while (pot < base) {
		pot <<= 1;
	}
	return (pot);
}
double emu_drand(void) {
	double d;

	do {
		d = (((rand() * RS_SCALE) + rand()) * RS_SCALE + rand()) * RS_SCALE;
	} while (d >= 1); // Round off
	return (d);
}
uTCHAR *emu_ustrncpy(uTCHAR *dst, uTCHAR *src) {
	uint32_t size;

	if (dst) {
		free (dst);
	}
	size = ustrlen(src) + 1;
	dst = (uTCHAR *)malloc(sizeof(uTCHAR) * size);
	umemset(dst, 0x00, size);
	ustrncpy(dst, src, size);

	return (dst);
}
void emu_quit(void) {
	if (cfg->save_on_exit) {
		settings_save();
	}

	map_quit();

	cheatslist_quit();
	nsf_quit();
	fds_quit();
	ppu_quit();
	snd_quit();
	gfx_quit();

	timeline_quit();

	js_quit(TRUE);

	gamegenie_quit();
	uncompress_quit();
	patcher_quit();

	gui_quit();
}

static BYTE emu_ctrl_if_rom_exist(void) {
	uTCHAR file[LENGTH_FILE_NAME_LONG];
	BYTE found = FALSE;

	umemset(file, 0x00, usizeof(file));

	if (info.rom.from_load_menu) {
		ustrncpy(file, info.rom.from_load_menu, usizeof(file));
		free (info.rom.from_load_menu);
		info.rom.from_load_menu = NULL;
	} else if (gamegenie.rom) {
		ustrncpy(file, gamegenie.rom, usizeof(file));
	} else {
		ustrncpy(file, info.rom.file, usizeof(file));
	}

	if (file[0]) {
		_uncompress_archive *archive;
		BYTE rc;

		archive = uncompress_archive_alloc(file, &rc);

		if (rc == UNCOMPRESS_EXIT_OK) {
			BYTE is_rom = FALSE, is_patch = FALSE;
			uTCHAR *rom = NULL, *patch = NULL;

			if (archive->rom.count > 0) {
				is_rom = TRUE;
			}
			if (archive->patch.count > 0) {
				is_patch = TRUE;
			}
			if ((is_patch == TRUE) && (is_rom == FALSE) && !info.rom.file[0]) {
				is_patch = FALSE;
			}
			if (is_rom) {
				switch ((rc = uncompress_archive_extract_file(archive,UNCOMPRESS_TYPE_ROM))) {
					case UNCOMPRESS_EXIT_OK:
						rom = uncompress_archive_extracted_file_name(archive, UNCOMPRESS_TYPE_ROM);
						found = TRUE;
						break;
					case UNCOMPRESS_EXIT_ERROR_ON_UNCOMP:
						break;
					case UNCOMPRESS_EXIT_IS_COMP_BUT_NOT_SELECTED:
					case UNCOMPRESS_EXIT_IS_COMP_BUT_NO_ITEMS:
						rom = info.rom.file;
						break;
					default:
						break;
				}
				if (rom) {
					ustrncpy(file, rom, usizeof(file));
				}
			}
			if (is_patch) {
				switch ((rc = uncompress_archive_extract_file(archive,UNCOMPRESS_TYPE_PATCH))) {
					case UNCOMPRESS_EXIT_OK:
						patch = uncompress_archive_extracted_file_name(archive, UNCOMPRESS_TYPE_PATCH);
						found = TRUE;
						break;
					case UNCOMPRESS_EXIT_ERROR_ON_UNCOMP:
						break;
					default:
						is_patch = FALSE;
						break;
				}
				if (patch) {
					patcher.file = emu_ustrncpy(patcher.file, patch);
				}
			}
			uncompress_archive_free(archive);
		} else if (rc == UNCOMPRESS_EXIT_IS_NOT_COMP) {
			found = TRUE;
		}
	}

	if (found == FALSE) {
		return (EXIT_ERROR);
	}

	umemset(info.rom.file, 0x00, usizeof(info.rom.file));
	ustrncpy(info.rom.file, file, usizeof(info.rom.file));

	if (patcher_ctrl_if_exist(NULL) == EXIT_OK) {
		ufprintf(stderr, uL("patch file : " uPERCENTs "\n"), patcher.file);
	}

	return (EXIT_OK);
}
static uTCHAR *emu_ctrl_rom_ext(uTCHAR *file) {
	static uTCHAR ext[10];
	uTCHAR name_file[255], *last_dot;

	gui_utf_basename(file, name_file, usizeof(name_file));

	last_dot = ustrrchr(name_file, uL('.'));

	if (last_dot == NULL) {
		ustrncpy((uTCHAR *)ext, uL(".nes"), usizeof(ext) - 1);
	} else {
		// salvo l'estensione del file
		ustrncpy((uTCHAR *)ext, last_dot, usizeof(ext) - 1);
	}

	return (ext);
}
static void emu_recent_roms_add(BYTE *add, uTCHAR *file) {
	if ((*add) == TRUE) {
		(*add) = FALSE;
		recent_roms_add(file);
	}
}
