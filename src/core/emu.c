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
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include "main.h"
#include "debugger.h"
#include "emu.h"
#include "settings.h"
#include "clock.h"
#include "video/gfx_thread.h"
#include "mappers.h"
#include "vs_system.h"
#include "detach_barcode.h"
#include "version.h"
#include "conf.h"
#include "save_slot.h"
#include "tas.h"
#include "ines.h"
#include "unif.h"
#include "fds.h"
#include "nsfe.h"
#include "patcher.h"
#include "recent_roms.h"
#include "../../c++/crc/crc.h"
#include "gui.h"
#include "video/effects/pause.h"
#include "video/effects/tv_noise.h"
#if defined (FULLSCREEN_RESFREQ)
#include "video/gfx_monitor.h"
#include "nes20db.h"
#endif

#define RS_SCALE (1.0f / (1.0f + (float)RAND_MAX))

#if defined (DEBUG)
WORD PCBREAK = 0x7F92;
#endif

INLINE static void emu_frame_started(void);
INLINE static void emu_frame_finished(void);
INLINE static void emu_frame_sleep(void);

static BYTE emu_ctrl_if_rom_exist(void);
static uTCHAR *emu_ctrl_rom_ext(uTCHAR *file);
static void emu_recent_roms_add(BYTE *add, uTCHAR *file);

void emu_quit(void) {
	if (cfg->save_on_exit) {
		settings_save();
	}

#if defined (WITH_FFMPEG)
	recording_quit();
#endif

	map_quit();

	cheatslist_quit();
	nsf_quit();
	fds_quit();
	ppu_quit();
	snd_quit();
	gfx_quit();

	rewind_quit();

	js_quit(TRUE);

	gamegenie_quit();
	uncompress_quit();
	patcher_quit();

#if defined (FULLSCREEN_RESFREQ)
	if (gfx.type_of_fscreen_in_use == FULLSCR) {
		gfx_monitor_restore_res();
	}
	gfx_monitor_quit();
#endif

	memmap_quit();

	chinaersan2_quit();

	gui_quit();
}
void emu_frame(void) {
	// gestione uscita
	if (info.stop) {
		return;
	}

	info.start_frame_0 = FALSE;

	gui_control_visible_cursor();

	// eseguo un frame dell'emulatore
	if (info.no_rom) {
		tv_noise_effect(0);
		gfx_draw_screen(0);
		nes[0].p.ppu.frames++;
		fps_ppu_inc(0);
		emu_frame_sleep();
		return;
	} else if (nsf.state & (NSF_PAUSE | NSF_STOP)) {
		int i = 0;

		gui_decode_all_input_events();

		for (i = PORT1; i < PORT_MAX; i++) {
			if (port_funct[i].input_add_event) {
				port_funct[i].input_add_event(i);
			}
		}

		extcl_audio_samples_mod_nsf(NULL, 0);
		nsf_main_screen_event();
		nsf_effect();
		gfx_draw_screen(0);
		fps_ppu_inc(0);
		emu_frame_sleep();
		return;
	}

	// se nel debugger passo dal DBG_STEP al DBG_GO
	// arrivo qui che l'emu_frame_started e' stato gia' fatto
	// e non devo ripeterlo (soprattuto per il TAS). Lo capisco
	// dal info.frame_status che e' gia' impostato su FRAME_STARTED.
	if (info.frame_status == FRAME_FINISHED) {
		emu_frame_started();
	}

	// eseguo CPU, PPU e APU
	while (info.exec_cpu_op.w) {
#if defined (DEBUG)
		if (nes[0].c.cpu.PC.w == PCBREAK) {
			BYTE pippo = 5;
			pippo = pippo + 1;
		}
#endif
		if (info.exec_cpu_op.b[1]) cpu_exe_op(1);
		if (info.exec_cpu_op.b[0]) cpu_exe_op(0);
	}

	emu_frame_finished();
	emu_frame_sleep();
	emu_frame_input_and_rewind();
}
void emu_frame_debugger(void) {
	if (info.stop || (debugger.mode >= DBG_BREAKPOINT)) {
		return;
	}

	if (info.frame_status == FRAME_FINISHED) {
		if (debugger.mode == DBG_GO) {
			if (info.no_rom) {
				tv_noise_effect(0);
				gfx_draw_screen(0);
				nes[0].p.ppu.frames++;
				fps_ppu_inc(0);
				emu_frame_sleep();
				return;
			} else if (nsf.state & (NSF_PAUSE | NSF_STOP)) {
				int i = 0;

				gui_decode_all_input_events();

				for (i = PORT1; i < PORT_MAX; i++) {
					if (port_funct[i].input_add_event) {
						port_funct[i].input_add_event(i);
					}
				}

				extcl_audio_samples_mod_nsf(NULL, 0);
				nsf_main_screen_event();
				nsf_effect();
				gfx_draw_screen(0);
				fps_ppu_inc(0);
				emu_frame_sleep();
				return;
			}
		}

		if (info.start_frame_0) {
			info.start_frame_0 = FALSE;
			goto emu_frame_debugger_start_frame_0;
		}

		emu_frame_started();
	}

	// eseguo CPU, PPU e APU
//	if (debugger.mode == DBG_GO) {
//		// posso passare dal DBG_GO al DBG_STEP durante l'esecuzione di un frame intero
//		while ((info.frame_status == FRAME_STARTED) && (debugger.mode == DBG_GO)) {
//			if ((debugger.breakpoint == nes[nidx].c.cpu.PC.w) && !debugger.breakpoint_after_step) {
//				debugger.mode = DBG_BREAKPOINT;
//				//gui_dlgdebugger_click_step();
//				break;
//			} else {
//				debugger.breakpoint_after_step = FALSE;
//				info.CPU_PC_before = nes[nidx].c.cpu.PC.w;
//				cpu_exe_op();
//			}
//		}
//	} else if (debugger.mode == DBG_STEP) {
//		info.CPU_PC_before = nes[nidx].c.cpu.PC.w;
//		cpu_exe_op();
//	}

	if (info.frame_status == FRAME_FINISHED) {
		emu_frame_finished();

		if (debugger.mode == DBG_GO) {
			emu_frame_sleep();
		}

		emu_frame_input_and_rewind();

emu_frame_debugger_start_frame_0:
		if (debugger.breakframe) {
			debugger.mode = DBG_BREAKPOINT;
			debugger.breakframe = FALSE;
			//gui_dlgdebugger_click_step();
		}
	}
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
#if defined (_WIN32)
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
	FILE *fd = NULL;
	size_t len = 0;
	char *str = NULL;

	fd = ufopen(path, uL("r"));
	if (!fd) {
		log_error(uL("opengl;can't open file '" uPs("") "' for reading"), path);
		return (NULL);
	}

	fseek(fd, 0, SEEK_END);
	len = ftell(fd);
	fseek(fd, 0, SEEK_SET);

	str = (char *)malloc((len + 1) * sizeof(char));
	if (!str) {
		fclose(fd);
		log_error(uL("opengl;can't malloc space for '" uPs("")), path);
		return (NULL);
	}

	memset(str, 0x00, len + 1);

	if (fread(str, sizeof(char), len, fd) < len) {
		if(feof(fd)) {
			log_warning(uL("opengl;EOF intercepted before the end of the '" uPs("")), path);
		}
		if (ferror(fd)) {
			log_error(uL("opengl;error in reading from '" uPs("")), path);
			free(str);
			str = NULL;
		}
	}
	fclose(fd);

	return (str);
}
BYTE emu_load_rom(void) {
	BYTE recent_roms_permit_add = !info.block_recent_roms_update;

	gui_egds_stop_unnecessary();

	elaborate_rom_file:
	info.format = HEADER_UNKOWN;
	info.no_rom = FALSE;
	info.cpu_rw_extern = FALSE;
	info.prg_truncated = FALSE;
	info.chr_truncated = FALSE;
	info.misc_truncated = FALSE;

	nsf_quit();
	fds_quit();
	map_quit();

	ustrncpy(info.rom.file, info.rom.change_rom, usizeof(info.rom.file));
	info.rom.file[usizeof(info.rom.file) - 1] = 0x00;

	info.doublebuffer = TRUE;

	if (info.rom.file[0]) {
		uTCHAR *ext = emu_ctrl_rom_ext(info.rom.file);

		if (!ustrcasecmp(ext, uL(".fds"))) {
			if (fds_load_rom() == EXIT_ERROR) {
				info.rom.file[0] = 0;
				info.rom.change_rom[0] = 0;
				goto elaborate_rom_file;
			}
			emu_recent_roms_add(&recent_roms_permit_add, info.rom.file);
		} else if (!ustrcasecmp(ext, uL(".nsf"))) {
			if (nsf_load_rom() == EXIT_ERROR) {
				log_error(uL("error loading;" uPs("")), info.rom.file);
				info.rom.file[0] = 0;
				info.rom.change_rom[0] = 0;
				gui_overlay_info_append_msg_precompiled(5, NULL);
				goto elaborate_rom_file;
			}
			emu_recent_roms_add(&recent_roms_permit_add, info.rom.file);
		} else if (!ustrcasecmp(ext, uL(".nsfe"))) {
			if (nsfe_load_rom() == EXIT_ERROR) {
				log_error(uL("error loading;" uPs("")), info.rom.file);
				info.rom.file[0] = 0;
				info.rom.change_rom[0] = 0;
				gui_overlay_info_append_msg_precompiled(5, NULL);
				goto elaborate_rom_file;
			}
			emu_recent_roms_add(&recent_roms_permit_add, info.rom.file);
		} else if (!ustrcasecmp(ext, uL(".fm2"))) {
			tas_file(ext, info.rom.file);
			if (!info.rom.file[0]) {
				gui_overlay_info_append_msg_precompiled(5, NULL);
				log_error(uL("error loading rom"));
			}
			emu_recent_roms_add(&recent_roms_permit_add, tas.file);
			ustrncpy(info.rom.change_rom, info.rom.file, usizeof(info.rom.change_rom));
			// rielaboro il nome del file
			goto elaborate_rom_file;
		} else {
			// carico la rom in memoria
			if ((ines_load_rom() == EXIT_ERROR) && (unif_load_rom() == EXIT_ERROR)) {
				log_error(uL("unknow format;" uPs("")), info.rom.file);
				info.rom.file[0] = 0;
				info.rom.change_rom[0] = 0;
				gui_overlay_info_append_msg_precompiled(5, NULL);
				nes20db_reset();
				goto elaborate_rom_file;
			}
			emu_recent_roms_add(&recent_roms_permit_add, info.rom.file);
			info.turn_off = FALSE;
			info.block_recent_roms_update = FALSE;
		}
	} else if (info.gui) {
		// impostazione primaria
		info.mapper.prgrom_banks_16k = 1;
		info.mapper.chrrom_banks_8k = 1;
		// PRG Ram
		wram_set_ram_size(S8K);
		// PRG Rom
		prgrom_set_size(info.mapper.prgrom_banks_16k * S16K);

		if (prgrom_init(0xEA) == EXIT_ERROR) {
			return (EXIT_ERROR);
		}

		info.no_rom = TRUE;
	}

#if defined (FULLSCREEN_RESFREQ)
	// mi salvo la vecchia modalita'
	info.old_machine_type = machine.type;
#endif

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

	if (!nsf.enabled) {
		cheatslist_read_game_cheats();
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
		usnprintf(title, len, uL("" uPs("") " (" uPs("")), name, opt_mode[machine.type].lname);
	} else if (cfg->filter == NTSC_FILTER) {
		usnprintf(title, len,
			uL("" uPs("") " (" uPs("") ", " uPs("") ", " uPs("") ", "),
			name, opt_mode[machine.type].lname,
			opt_scale[cfg->scale - 1].sname, opt_ntsc[cfg->ntsc_format].lname);
	} else {
		usnprintf(title, len,
			uL("" uPs("") " (" uPs("") ", " uPs("") ", " uPs("") ", "),
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
	for (int nesidx = 0; nesidx < info.number_of_nes; nesidx++) {
		memset(&nes[nesidx].m.memmap_palette, 0x00, sizeof(nes[nesidx].m.memmap_palette));
		memset(&nes[nesidx].p.oam, 0x00, sizeof(_oam));
		memset(&nes[nesidx].p.ppu_screen, 0x00, sizeof(_ppu_screen));
	}
	memset(&vs_system, 0x00, sizeof(vs_system));
	memset(&detach_barcode, 0x00, sizeof(detach_barcode));

	info.lag_frame.next = TRUE;
	info.lag_frame.actual = TRUE;
	info.lag_frame.totals = 0;
	info.lag_frame.consecutive = 0;

	cfg->extra_vb_scanlines = cfg->extra_pr_scanlines = 0;

	vs_system.watchdog.next = vs_system_wd_next()

	info.r2002_jump_first_vblank = FALSE;
	info.r2002_race_condition_disabled = FALSE;
	info.r4014_precise_timing_disabled = FALSE;
	info.r4016_dmc_double_read_disabled = FALSE;

	cheatslist_init();

#if defined (WITH_OPENGL)
	gui_screen_info();
#endif

	if (gui_create() == EXIT_ERROR) {
		gui_critical(uL("GUI initialization failed."));
		return (EXIT_ERROR);
	}

	nsf_init();
	fds_init();

	if (gfx_palette_init()) {
		return (EXIT_ERROR);
	}

	// carico la rom in memoria
	{
		emu_ctrl_if_rom_exist();

		if (emu_load_rom()) {
			return (EXIT_ERROR);
		}
		emu_ctrl_doublebuffer();
	}

	overscan_set_mode(machine.type);

	// ...nonche' dei puntatori alla PRG Rom...
	prgrom_reset_chunks();

	settings_pgs_parse();

	// APU
	apu_turn_on();

	// PPU
	if (ppu_turn_on()) {
		return (EXIT_ERROR);
	}

	// gestione grafica
	if (gfx_init()) {
		return (EXIT_ERROR);
	}

	// ...e inizializzazione della mapper (che
	// deve necessariamente seguire quella della PPU.
	if (map_init()) {
		return (EXIT_ERROR);
	}

	// CPU
	for (int nesidx = 0; nesidx < info.number_of_nes; nesidx++) {
		cpu_turn_on(nesidx);
		// ritardo della CPU
		cpu_initial_cycles(nesidx);
		if (extcl_cpu_init_pc) {
			extcl_cpu_init_pc(nesidx);
		}
	}

	// trainer
	miscrom_trainer_init();

	// controller
	input_init(NO_SET_CURSOR);

	// joystick
	js_init(TRUE);

	// non viene eseguito nell'input_init() perche' la gui non e' ancora avviata quindi devo eseguirlo dopo il gfx_init().
	gui_nes_keyboard();

	// setto il cursore
	gfx_cursor_init();

	// fps
	fps_init();

	// setto l'fps del timer ext_gfx_draw_screen
	gui_egds_set_fps();

	// gestione sonora
	if (snd_init()) {
		return (EXIT_ERROR);
	}

	if (rewind_init()) {
		return (EXIT_ERROR);
	}

	save_slot_count_load();

	ext_win.vs_system = vs_system.enabled;
	ext_win.detach_barcode = detach_barcode.enabled;

	gui_external_control_windows_show();
	gui_wdgrewind_play();

	info.bat_ram_frames_snap = machine.fps * (60 * 3);

	info.reset = FALSE;
	info.start_frame_0 = TRUE;

	// The End
	return (EXIT_OK);
}
void emu_pause(BYTE mode) {
	if (mode) {
		if (++info.pause == 1) {
			pause_effect.frames = PAUSE_EFFECT_FRAMES;
			gui_egds_start_pause();
		}
	} else {
		if (--info.pause == 0) {
			gui_egds_stop_pause();
		} else if (info.pause < 0) {
			info.pause = 0;
		}
	}

	if (info.pause == 0) {
		if (nsf.enabled) {
			nsf_reset_timers();
		}
	}

	emu_ctrl_doublebuffer();
}
BYTE emu_reset(BYTE type) {
	if (info.turn_off && (type <= HARD)) {
		return (EXIT_OK);
	}

	gui_wdgrewind_play();

	if ((type == CHANGE_ROM) && (emu_ctrl_if_rom_exist() == EXIT_ERROR)) {
		return (EXIT_OK);
	}

	info.reset = type;

	info.first_illegal_opcode = FALSE;

	info.lag_frame.next = TRUE;
	info.lag_frame.actual = TRUE;
	info.lag_frame.totals = (type >= HARD) ? 0 : info.lag_frame.totals;
	info.lag_frame.consecutive = 0;

	if (info.reset == CHANGE_ROM) {
		BYTE vs_enab = vs_system.enabled;
		BYTE vs_ppu = vs_system.ppu;

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
		emu_ctrl_doublebuffer();

		ext_win.vs_system = vs_system.enabled;

		overscan_set_mode(machine.type);

		settings_pgs_parse();

		gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, TRUE,
			(vs_enab != vs_system.enabled) || (vs_ppu != vs_system.ppu));

		gui_update_gps_settings();
	}

	if ((info.reset == CHANGE_MODE) && overscan_set_mode(machine.type)) {
		gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, TRUE, FALSE);
	}

#if defined (FULLSCREEN_RESFREQ)
	if ((gfx.type_of_fscreen_in_use == FULLSCR) &&
		cfg->adaptive_rrate &&
		(info.old_machine_type != machine.type)) {
		gfx_monitor_set_res(cfg->fullscreen_res_w, cfg->fullscreen_res_h, cfg->adaptive_rrate, TRUE);
	}
#endif

	// APU
	apu_turn_on();

	// PPU
	if (ppu_turn_on()) {
		return (EXIT_ERROR);
	}

	// mapper
	if (map_init()) {
		return (EXIT_ERROR);
	}

	ext_win.detach_barcode = detach_barcode.enabled;

	// CPU
	for (int nesidx = 0; nesidx < info.number_of_nes; nesidx++) {
		cpu_turn_on(nesidx);
		// ritardo della CPU
		cpu_initial_cycles(nesidx);
		if (extcl_cpu_init_pc) {
			extcl_cpu_init_pc(nesidx);
		}
	}

	// trainer
	miscrom_trainer_init();

	if (info.no_rom) {
		info.reset = FALSE;
		if (info.pause_from_gui) {
			emu_pause((info.pause_from_gui = FALSE));
		}
		return (EXIT_OK);
	}

	// controller
	input_init(SET_CURSOR);

	if (rewind_init()) {
		return (EXIT_ERROR);
	}

	if (info.reset == CHANGE_ROM) {
		save_slot_count_load();
	}

	fps_init();

	if (info.reset >= CHANGE_ROM) {
		gui_egds_set_fps();

		if (snd_playback_start()) {
			return (EXIT_ERROR);
		}
	}

	memset(&vs_system.watchdog, 0x00, sizeof(vs_system.watchdog));
	memset(&vs_system.r4020, 0x00, sizeof(vs_system.r4020));
	memset(&vs_system.coins, 0x00, sizeof(vs_system.coins));
	vs_system.watchdog.next = vs_system_wd_next()

	gui_emit_et_external_control_windows_show();

	info.bat_ram_frames = 0;

	info.reset = FALSE;
	info.start_frame_0 = TRUE;

	if ((info.format == NSF_FORMAT) || (info.format == NSFE_FORMAT)) {
		nsf.draw_mask_frames = 2;
	}

	if (info.pause_from_gui) {
		emu_pause((info.pause_from_gui = FALSE));
	}

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
unsigned int emu_power_of_two(unsigned int base) {
	unsigned int pot = 1;

	while (pot < base) {
		pot <<= 1;
	}
	return (pot == 1 ? 2 : pot);
}
double emu_drand(void) {
	double d = 0.0;

	do {
		d = ((((double)rand() * RS_SCALE) + (double)rand()) * RS_SCALE + (double)rand()) * RS_SCALE;
	} while (d >= 1); // Round off
	return (d);
}
uTCHAR *emu_ustrncpy(uTCHAR *dst, uTCHAR *src) {
	uint32_t size = 0;

	if (dst) {
		free(dst);
		dst = NULL;
	}

	if (src == NULL) {
		return (dst);
	}

	size = ustrlen(src) + 1;
	dst = (uTCHAR *)malloc(sizeof(uTCHAR) * size);
	umemset(dst, 0x00, size);
	ustrcpy(dst, src);

	return (dst);
}
uTCHAR *emu_rand_str(void) {
	static uTCHAR dest[10];
	uTCHAR charset[] = uL("0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");
	unsigned int i = 0;

	for (i = 0; i < (usizeof(dest) - 1); i++) {
		size_t index = rand() / RAND_MAX * (usizeof(charset) - 1);

		dest[i] = charset[index];
	}
	dest[usizeof(dest) - 1] = '\0';

	return ((uTCHAR *)&dest);
}
void emu_ctrl_doublebuffer(void) {
	gfx_thread_pause();

	switch (info.format) {
		default:
		case iNES_1_0:
		case NES_2_0:
		case UNIF_FORMAT:
		case FDS_FORMAT:
			info.doublebuffer = TRUE;
			break;
		case NSF_FORMAT:
		case NSFE_FORMAT:
			info.doublebuffer = FALSE;
			break;
	}

	if (info.no_rom | info.turn_off) {
		info.doublebuffer = TRUE;
	} else if (info.pause) {
		info.doublebuffer = FALSE;
	}

	if (rwnd.active) {
		info.doublebuffer = FALSE;
	}

	switch (debugger.mode) {
		case DBG_STEP:
		case DBG_BREAKPOINT:
			info.doublebuffer = FALSE;
			break;
		case DBG_GO:
			if (debugger.breakframe) {
				info.doublebuffer = FALSE;
			}
			break;
	}

	gfx_thread_continue();
}
void emu_frame_input_and_rewind(void) {
	// controllo se ci sono eventi di input
	if (tas.type == NOTAS) {
		int i = 0;

		gui_decode_all_input_events();

		for (i = PORT1; i < PORT_MAX; i++) {
			if (port_funct[i].input_add_event) {
				port_funct[i].input_add_event(i);
			}
		}
	} else {
		tas_frame();
	}

	rewind_snapshoot();
}
void emu_info_rom(void) {
	BYTE changed = FALSE;
	BYTE at_least_one_change = FALSE;

	if (!info.rom.file[0]) {
		return;
	}

	{
		uTCHAR buffer[LENGTH_FILE_NAME_MID];

		log_info_open(uL("file;"));
		umemset(buffer, 0x00, usizeof(buffer));
		gui_utf_basename((uTCHAR *)info.rom.file, buffer, usizeof(buffer) - 1);
		log_close(uL("" uPs("")), buffer);

		log_info_box_open(uL("folder;"));
		umemset(buffer, 0x00, usizeof(buffer));
		gui_utf_dirname((uTCHAR *)info.rom.file, buffer, usizeof(buffer) - 1);
		log_close_box(uL("" uPs("")), buffer);

		if (patcher.patched == TRUE) {
			log_info_box(uL("patched;yes"));
		}
	}

	log_info_box_open(uL("format;"));
	if (info.header.format == NES_2_0) {
		log_close_box(uL("Nes 2.0"));
	} else if (info.header.format == iNES_1_0) {
		log_close_box(uL("iNES 1.0"));
	} else if (info.header.format == UNIF_FORMAT) {
		log_close_box(uL("UNIF"));
	} else if (info.header.format == NSF_FORMAT) {
		log_close_box(uL("NSF"));
		nsf_info();
		return;
	} else if (info.header.format == NSFE_FORMAT) {
		log_close_box(uL("NSFE"));
		nsfe_info();
		return;
	} else if (info.header.format == FDS_FORMAT) {
		log_close_box(uL("FDS"));
		fds_info();
		return;
	} else {
		log_close_box(uL("Unknow"));
		return;
	}

#define ischanged(a) changed = (a); at_least_one_change = changed ? TRUE : at_least_one_change
#define ifchanged() (changed ? " *" : "")

	log_info_box(uL("nes20db;%s"),
		info.mapper.nes20db.in_use ? "yes" : "no");

	{
		log_info_box_open(uL("console type;"));
		ischanged(info.header.ext_console_type != info.mapper.ext_console_type);
		switch (info.mapper.ext_console_type) {
			case REGULAR_NES:
				log_close_box(uL("Regular NES/Famicom/Dendy%s"), ifchanged());
				break;
			case VS_SYSTEM:
				log_close_box(uL("Nintendo Vs. System%s"), ifchanged());
				break;
			case PLAYCHOICE10:
				log_close_box(uL("Playchoice 10%s"), ifchanged());
				break;
			case FAMICLONE_DECIMAL_MODE:
				log_close_box(uL("Regular Famiclone, but with CPU that supports Decimal Mode%s"), ifchanged());
				break;
			case EPSM:
				log_close_box(uL("Regular NES/Famicom with EPSM module or plug-through cartridge [unsupported]%s"), ifchanged());
				break;
			case VT01:
				log_close_box(uL("V.R. Technology VT01 with red/cyan STN palette [unsupported]%s"), ifchanged());
				break;
			case VT02:
				log_close_box(uL("V.R. Technology VT02 [unsupported]%s"), ifchanged());
				break;
			case VT03:
				log_close_box(uL("V.R. Technology VT03 [unsupported]%s"), ifchanged());
				break;
			case VT09:
				log_close_box(uL("V.R. Technology VT09 [unsupported]%s"), ifchanged());
				break;
			case VT32:
				log_close_box(uL("V.R. Technology VT32 [unsupported]%s"), ifchanged());
				break;
			case VT369:
				log_close_box(uL("V.R. Technology VT369 [unsupported]%s"), ifchanged());
				break;
			case UMC_UM6578:
				log_close_box(uL("UMC UM6578 [unsupported]%s"), ifchanged());
				break;
			case FAMICOM_NETWORK_SYSTEM:
				log_close_box(uL("Famicom Network System [unsupported]%s"), ifchanged());
				break;
			case 13:
			case 14:
			case 15:
				log_close_box(uL("reserved%s"), ifchanged());
				break;
		}
	}

	{
		log_info_box_open(uL("CPU timing;"));
		ischanged(machine.type != info.header.cpu_timing);
		switch (machine.type) {
			default:
			case NTSC:
				log_close_box(uL("NTSC NES%s"), ifchanged());
				break;
			case PAL:
				log_close_box(uL("Licensed PAL NES%s"), ifchanged());
				break;
			case DENDY:
				log_close_box(uL("Dendy%s"), ifchanged());
				break;
		}
	}

	if (vs_system.enabled) {
		log_info_box_open(uL("vs ppu;"));
		ischanged(info.header.vs_ppu != vs_system.ppu);
		switch (vs_system.ppu) {
			case RP2C03B:
				log_close_box(uL("RP2C03B%s"), ifchanged());
				break;
			case RP2C03G:
				log_close_box(uL("RP2C03G%s"), ifchanged());
				break;
			case RP2C04:
				log_close_box(uL("RP2C04-0001%s"), ifchanged());
				break;
			case RP2C04_0002:
				log_close_box(uL("RP2C04-0002%s"), ifchanged());
				break;
			case RP2C04_0003:
				log_close_box(uL("RP2C04-0003%s"), ifchanged());
				break;
			case RP2C04_0004:
				log_close_box(uL("RP2C04-0004%s"), ifchanged());
				break;
			case RC2C03B:
				log_close_box(uL("RC2C03B%s"), ifchanged());
				break;
			case RC2C03C:
			default:
				log_close_box(uL("RC2C03C%s"), ifchanged());
				break;
			case RC2C05_01:
				log_close_box(uL("RC2C05-01%s"), ifchanged());
				break;
			case RC2C05_02:
				log_close_box(uL("RC2C05-02%s"), ifchanged());
				break;
			case RC2C05_03:
				log_close_box(uL("RC2C05-03%s"), ifchanged());
				break;
			case RC2C05_04:
				log_close_box(uL("RC2C05-04%s"), ifchanged());
				break;
			case RC2C05_05:
				log_close_box(uL("RC2C05-05%s"), ifchanged());
				break;
		}

		log_info_box_open(uL("vs system;"));
		ischanged(info.header.vs_hardware != vs_system.special_mode.type);
		switch (vs_system.special_mode.type) {
			case VS_SM_Normal:
			default:
				log_close_box(uL("Unisystem (normal)%s"), ifchanged());
				break;
			case VS_SM_RBI_Baseball:
				log_close_box(uL("Unisystem (RBI Baseball protection)%s"), ifchanged());
				break;
			case VS_SM_TKO_Boxing:
				log_close_box(uL("Unisystem (TKO Boxing protection)%s"), ifchanged());
				break;
			case VS_SM_Super_Xevious:
				log_close_box(uL("Unisystem (Super Xevious protection)%s"), ifchanged());
				break;
			case VS_SM_Ice_Climber:
				log_close_box(uL("Unisystem (Ice Climber Japan protection)%s"), ifchanged());
				break;
			case VS_DS_Normal:
				log_close_box(uL("DualSystem (normal)%s"), ifchanged());
				break;
			case VS_DS_Bungeling:
				log_close_box(uL("DualSystem (Raid on Bungeling Bay protection)%s"), ifchanged());
				break;
		}
	}

	{
		log_info_box_open(uL("expansion;"));
		ischanged(info.mapper.expansion != info.header.expansion);
		switch (info.mapper.expansion) {
			default:
			case 0x00:
				log_close_box(uL("Unspecified%s"), ifchanged());
				break;
			case 0x01:
				log_close_box(uL("Standard NES/Famicom controllers%s"), ifchanged());
				break;
			case 0x02:
				log_close_box(uL("NES Four Score/Satellite with two additional standard controllers%s"), ifchanged());
				break;
			case 0x03:
				log_close_box(uL("Famicom Four Players Adapter (two additional standard controllers, \"simple\" protocol)%s"), ifchanged());
				break;
			case 0x04:
				log_close_box(uL("Vs. System (1P via $4016)%s"), ifchanged());
				break;
			case 0x05:
				log_close_box(uL("Vs. System (1P via $4017)%s"), ifchanged());
				break;
			case 0x06:
				log_close_box(uL("Reserved%s"), ifchanged());
				break;
			case 0x07:
				log_close_box(uL("Vs. Zapper%s"), ifchanged());
				break;
			case 0x08:
				log_close_box(uL("Zapper ($4017)%s"), ifchanged());
				break;
			case 0x09:
				log_close_box(uL("Two Zappers%s"), ifchanged());
				break;
			case 0x0A:
				log_close_box(uL("Bandai Hyper Shot Lightgun%s"), ifchanged());
				break;
			case 0x0B:
				log_close_box(uL("Power Pad Side A%s"), ifchanged());
				break;
			case 0x0C:
				log_close_box(uL("Power Pad Side B%s"), ifchanged());
				break;
			case 0x0D:
				log_close_box(uL("Family Trainer Side A%s"), ifchanged());
				break;
			case 0x0E:
				log_close_box(uL("Family Trainer Side B%s"), ifchanged());
				break;
			case 0x0F:
				log_close_box(uL("Arkanoid Vaus Controller (NES)%s"), ifchanged());
				break;
			case 0x10:
				log_close_box(uL("Arkanoid Vaus Controller (Famicom)%s"), ifchanged());
				break;
			case 0x11:
				log_close_box(uL("Two Vaus Controllers plus Famicom Data Recorder%s"), ifchanged());
				break;
			case 0x12:
				log_close_box(uL("Konami Hyper Shot Controller%s"), ifchanged());
				break;
			case 0x13:
				log_close_box(uL("Coconuts Pachinko Controller%s"), ifchanged());
				break;
			case 0x14:
				log_close_box(uL("Exciting Boxing Punching Bag (Blowup Doll)%s"), ifchanged());
				break;
			case 0x15:
				log_close_box(uL("Jissen Mahjong Controller%s"), ifchanged());
				break;
			case 0x16:
				log_close_box(uL("Party Tap%s"), ifchanged());
				break;
			case 0x17:
				log_close_box(uL("Oeka Kids Tablet%s"), ifchanged());
				break;
			case 0x18:
				log_close_box(uL("Sunsoft Barcode Battler%s"), ifchanged());
				break;
			case 0x19:
				log_close_box(uL("Miracle Piano Keyboard%s"), ifchanged());
				break;
			case 0x1A:
				log_close_box(uL("Pokkun Moguraa (Whack-a-Mole Mat and Mallet)%s"), ifchanged());
				break;
			case 0x1B:
				log_close_box(uL("Top Rider (Inflatable Bicycle)%s"), ifchanged());
				break;
			case 0x1C:
				log_close_box(uL("Double-Fisted (Requires or allows use of two controllers by one player)%s"), ifchanged());
				break;
			case 0x1D:
				log_close_box(uL("Famicom 3D System%s"), ifchanged());
				break;
			case 0x1E:
				log_close_box(uL("Doremikko Keyboard%s"), ifchanged());
				break;
			case 0x1F:
				log_close_box(uL("R.O.B. Gyro Set%s"), ifchanged());
				break;
			case 0x20:
				log_close_box(uL("Famicom Data Recorder (\"silent\" keyboard)%s"), ifchanged());
				break;
			case 0x21:
				log_close_box(uL("ASCII Turbo File%s"), ifchanged());
				break;
			case 0x22:
				log_close_box(uL("IGS Storage Battle Box%s"), ifchanged());
				break;
			case 0x23:
				log_close_box(uL("Family BASIC Keyboard plus Famicom Data Recorder%s"), ifchanged());
				break;
			case 0x24:
				log_close_box(uL("Dongda PEC-586 Keyboard%s"), ifchanged());
				break;
			case 0x25:
				log_close_box(uL("Bit Corp. Bit-79 Keyboard%s"), ifchanged());
				break;
			case 0x26:
				log_close_box(uL("Subor Keyboard%s"), ifchanged());
				break;
			case 0x27:
				log_close_box(uL("Subor Keyboard plus mouse (3x8-bit protocol)%s"), ifchanged());
				break;
			case 0x28:
				log_close_box(uL("Subor Keyboard plus mouse (24-bit protocol via $4016)%s"), ifchanged());
				break;
			case 0x29:
				log_close_box(uL("SNES Mouse ($4017.d0)%s"), ifchanged());
				break;
			case 0x2A:
				log_close_box(uL("Multicart%s"), ifchanged());
				break;
			case 0x2B:
				log_close_box(uL("Two SNES controllers replacing the two standard NES controllers%s"), ifchanged());
				break;
			case 0x2C:
				log_close_box(uL("RacerMate Bicycle%s"), ifchanged());
				break;
			case 0x2D:
				log_close_box(uL("U-Force%s"), ifchanged());
				break;
			case 0x2E:
				log_close_box(uL("R.O.B. Stack-Up%s"), ifchanged());
				break;
			case 0x2F:
				log_close_box(uL("City Patrolman Lightgun%s"), ifchanged());
				break;
			case 0x30:
				log_close_box(uL("Sharp C1 Cassette Interface%s"), ifchanged());
				break;
			case 0x31:
				log_close_box(uL("Standard Controller with swapped Left-Right/Up-Down/B-A%s"), ifchanged());
				break;
			case 0x32:
				log_close_box(uL("Excalibor Sudoku Pad%s"), ifchanged());
				break;
			case 0x33:
				log_close_box(uL("ABL Pinball%s"), ifchanged());
				break;
			case 0x34:
				log_close_box(uL("Golden Nugget Casino extra buttons%s"), ifchanged());
				break;
			case 0x35:
				log_close_box(uL("Unknown famiclone keyboard used by the \"Golden Key\" educational cartridge%s"), ifchanged());
				break;
			case 0x36:
				log_close_box(uL("Subor Keyboard plus mouse (24-bit protocol via $4017)%s"), ifchanged());
				break;
			case 0x37:
				log_close_box(uL("Port test controller%s"), ifchanged());
				break;
			case 0x38:
				log_close_box(uL("Bandai Multi Game Player Gamepad buttons%s"), ifchanged());
				break;
			case 0x39:
				log_close_box(uL("Venom TV Dance Mat%s"), ifchanged());
				break;
			case 0x3A:
				log_close_box(uL("LG TV Remote Control%s"), ifchanged());
				break;
		}
	}

#define ifsupported() (info.mapper.supported ? "" : " [not supported]")

	if (info.header.format == UNIF_FORMAT) {
		char *trimmed = &unif.board[0];
		size_t i = 0;

		for (i = 0; i < strlen(unif.stripped_board); i++) {
			if ((*unif.stripped_board) != ' ') {
				break;
			}
			trimmed++;
		}
		log_info_box(uL("UNIF board;%s"), trimmed);

		if (strlen(unif.name) > 0) {
			log_info_box(uL("UNIF name;%s"), unif.name);
		}

		ischanged(info.header.mapper != info.mapper.id);
		log_info_box(uL("NES mapper;%u%s%s"), info.mapper.id, ifsupported(), ifchanged());
	} else {
		ischanged(info.header.mapper != info.mapper.id);
		log_info_box(uL("NES mapper;%u%s%s"), info.mapper.id, ifsupported(), ifchanged());
	}

#undef ifsupported

	{
		log_info_box_open(uL("submapper;"));
		ischanged(info.header.submapper != info.mapper.submapper);
		if ((info.header.format == NES_2_0) || info.mapper.nes20db.in_use) {
			info.mapper.submapper == info.mapper.submapper_nes20
				? log_close_box(uL("%u%s"), info.mapper.submapper_nes20, ifchanged())
				: log_close_box(uL("%u (%u)%s"), info.mapper.submapper_nes20, info.mapper.submapper, ifchanged());
		} else {
				log_close_box(uL("%u%s"), info.mapper.submapper, ifchanged());
		}
	}

	if (info.header.format == UNIF_FORMAT) {
		if (strlen(unif.dumped.by) > 0) {
			log_info_box_open(uL("dumped by;%s"), unif.dumped.by);

			if (strlen(unif.dumped.with) > 0) {
				log_append(uL(" with %s"), unif.dumped.with);
			}

			if (unif.dumped.month && unif.dumped.day && unif.dumped.year) {
				char *months[12] = {
					"January",   "February", "March",    "April",
					"May",       "June",     "July",     "August",
					"September", "October",  "November", "December"
				};

				log_append(uL(" on %s %d, %d"), months[(unif.dumped.month - 1) % 12], unif.dumped.day, unif.dumped.year);
			}
			log_close_box(uL(""));
		}

		if (chinaersan2.font.data) {
			log_info_box(uL("EXT font;%ld"), (long)chinaersan2.font.size);
		}
	}

	{
		log_info_box_open(uL("mirroring;"));
		if ((info.header.format == UNIF_FORMAT) && (info.mapper.mirroring == 5)) {
			log_close_box(uL("controlled by the mapper"));
		} else {
			ischanged(info.header.mirroring != info.mapper.mirroring);
			switch (info.mapper.mirroring) {
				default:
				case MIRRORING_HORIZONTAL:
					log_close_box(uL("horizontal%s"), ifchanged());
					break;
				case MIRRORING_VERTICAL:
					log_close_box(uL("vertical%s"), ifchanged());
					break;
				case MIRRORING_SINGLE_SCR0:
					log_close_box(uL("scr0%s"), ifchanged());
					break;
				case MIRRORING_SINGLE_SCR1:
					log_close_box(uL("scr1%s"), ifchanged());
					break;
				case MIRRORING_FOURSCR:
					log_close_box(uL("4 screen%s"), ifchanged());
					break;
			}
		}
	}

	if (dipswitch.used) {
		log_info_box(uL("dipswitch;%s"), "present");
	}

	ischanged(info.header.battery != info.mapper.battery);
	if (changed || info.mapper.battery) {
		log_info_box(uL("battery;%s%s"), (info.mapper.battery ? "present" : "not present"), ifchanged());
	}
	if (wram_ram_size()) {
		ischanged(info.header.prgram != wram_ram_size());
		log_info_box(uL("PRG RAM;%u%s"), wram_ram_size(), ifchanged());
	}
	if (wram_nvram_size()) {
		ischanged(info.header.prgnvram != wram_nvram_size());
		log_info_box(uL("PRG NVRAM;%u%s"), wram_nvram_size(), ifchanged());
	}
	if (vram_ram_size(0)) {
		ischanged(info.header.chrram != vram_ram_size(0));
		log_info_box(uL("CHR RAM;%u%s"), vram_ram_size(0), ifchanged());
	}
	if (vram_nvram_size(0)) {
		ischanged(info.header.chrnvram != vram_nvram_size(0));
		log_info_box(uL("CHR NVRAM;%u%s"), vram_nvram_size(0), ifchanged());
	}

	ischanged(info.header.prgrom_size != info.mapper.prgrom_size);
	log_info_box(uL("PRG 8k rom;%-4lu [ %08X %ld ]%s%s"),
		prgrom_banks(S8K),
		info.crc32.prg,
		info.mapper.prgrom_size,
		(info.prg_truncated ? " truncated" : ""),
		ifchanged());

	if (info.header.format == UNIF_FORMAT) {
		if (prgrom.chips.amount > 1) {
			int chip = 0;

			for (chip = 0; chip < prgrom.chips.amount; chip++) {
				log_info_box(uL(" 8k chip %d;%-4lu [ %08X %ld ]"),
					chip, prgrom_chip_size(chip) / 0x2000,
					emu_crc32((void *)prgrom_chip(chip), prgrom_chip_size(chip)),
					prgrom_chip_size(chip));
			}
		}
	}

	if (chrrom_size()) {
		ischanged(info.header.chrrom_size != info.mapper.chrrom_size);
		log_info_box(uL("CHR 4k vrom;%-4lu [ %08X %ld ]%s%s"),
			chrrom_banks(S4K),
			info.crc32.chr,
			info.mapper.chrrom_size,
			(info.chr_truncated ? " truncated" : ""),
			ifchanged());

		if (info.header.format == UNIF_FORMAT) {
			if (chrrom.chips.amount > 1) {
				int chip = 0;

				for (chip = 0; chip < chrrom.chips.amount; chip++) {
					log_info_box(uL(" 4k chip %d;%-4lu [ %08X %ld ]"),
						chip, chrrom_chip_size(chip) / 0x1000,
						emu_crc32((void *)chrrom_chip(chip), chrrom_chip_size(chip)),
						chrrom_chip_size(chip));
				}
			}
		}
	}

	if (miscrom.trainer.in_use) {
		log_info_box(uL("trainer;yes  [ %08X ]"), info.crc32.trainer);
	} else if (miscrom_size()) {
		log_info_box(uL("MISC chips;%-4lu [ %08X %ld ]%s"),
			(long)miscrom.chips,
			info.crc32.misc,
			miscrom_size(),
			(info.misc_truncated ? " truncated" : ""));
	}

	if (info.header.format == iNES_1_0) {
		log_info_box(uL("sha1prg;%40s"), info.sha1sum.prg.string);
		if (chrrom_size()) {
			log_info_box(uL("shachr;%40s"), info.sha1sum.chr.string);
		}
	}

#undef ischanged
#undef ifchanged

	log_info_box(uL("CRC32;%08X"), info.crc32.total);

	log_info_box(uL("CPU/PPU alig.;PPU %d/%d, CPU %d/%d"),
		ppu_alignment.ppu, (machine.ppu_divide - 1),
		ppu_alignment.cpu, (machine.cpu_divide - 1));

	if (at_least_one_change) {
		log_info_box(uL("Note;* different values than the header"));
	}
}
void emu_initial_ram(BYTE *ram, unsigned int length) {
	unsigned int i = 0;

	if (!ram || !length) {
		return;
	}
	for (i = 0; i < length; i++) {
		ram[i] = cfg->initial_ram_value == IRV_0X00
			? 0x00
			: cfg->initial_ram_value == IRV_0XFF
				? 0xFF
				: rand();
	}
}
void emu_save_header_info(void) {
	info.header.format = info.format;
	info.header.prgrom_size = info.mapper.prgrom_size;
	info.header.prgrom = info.mapper.prgrom_banks_16k;
	info.header.chrrom_size = info.mapper.chrrom_size;
	info.header.chrrom = info.mapper.chrrom_banks_8k;
	info.header.prgram = wram_ram_size();
	info.header.prgnvram = wram_nvram_size();
	info.header.chrram = vram_ram_size(0);
	info.header.chrnvram = vram_nvram_size(0);
	info.header.battery = info.mapper.battery;
	info.header.mapper = info.mapper.id;
	info.header.submapper = info.mapper.submapper;
	info.header.mirroring = info.mapper.mirroring;
	info.header.cpu_timing = info.machine[HEADER];
	info.header.ext_console_type = info.mapper.ext_console_type;
	info.header.expansion = info.mapper.expansion;
	info.header.vs_ppu = vs_system.ppu;
	info.header.vs_hardware = vs_system.special_mode.type;
}
BYTE emu_active_nidx(void) {
	return (vs_system.special_mode.type == VS_DS_Normal ? cfg->vs_monitor : 0);
}

INLINE static void emu_frame_started(void) {
	info.lag_frame.next = TRUE;

	// riprendo a far correre la CPU
	info.frame_status = FRAME_STARTED;

	for (int nesidx = 0; nesidx < info.number_of_nes; nesidx++) {
		info.exec_cpu_op.b[nesidx] = TRUE;
	}
}
INLINE static void emu_frame_finished(void) {
	info.frame_status = FRAME_FINISHED;

	if ((cfg->cheat_mode == GAMEGENIE_MODE) && (gamegenie.phase == GG_LOAD_ROM)) {
		gui_emit_et_gg_reset();
	}

	info.lag_frame.actual = info.lag_frame.next;

	if (info.lag_frame.actual) {
		info.lag_frame.consecutive++;
		info.lag_frame.totals++;
		gui_update_ppu_hacks_lag_frames();
	} else {
		info.lag_frame.consecutive = 0;
	}

	if (snd_end_frame) {
		snd_end_frame();
	}

	if (cfg->save_battery_ram_file && (++info.bat_ram_frames >= info.bat_ram_frames_snap)) {
		// faccio anche un refresh del file della battery ram
		info.bat_ram_frames = 0;
		nvram_save_file();
	}

	if (vs_system.enabled & vs_system.watchdog.reset) {
		gui_emit_et_vs_reset();
	}

	r4011.frames++;

	gui_nes_keyboard_frame_finished();
}
INLINE static void emu_frame_sleep(void) {
	double diff = 0.0, now = gui_get_ms();

	diff = fps.frame.expected_end - now;

	if (diff > 0) {
		gui_sleep(diff);
	} else {
		fps.info.emu_too_long++;
		fps.frame.expected_end = gui_get_ms();
	}
	fps.frame.expected_end += fps.frame.estimated_ms;
}

static BYTE emu_ctrl_if_rom_exist(void) {
	BYTE found = FALSE;

	if (info.rom.from_load_menu) {
		ustrncpy(info.rom.change_rom, info.rom.from_load_menu, usizeof(info.rom.change_rom));
		info.rom.change_rom[usizeof(info.rom.change_rom) - 1] = 0x00;
		free(info.rom.from_load_menu);
		info.rom.from_load_menu = NULL;
	} else if (gamegenie.rom) {
		ustrncpy(info.rom.change_rom, gamegenie.rom, usizeof(info.rom.change_rom));
		info.rom.change_rom[usizeof(info.rom.change_rom) - 1] = 0x00;
	} else {
		if (info.rom.file[0] && (emu_file_exist(info.rom.file) == EXIT_ERROR)) {
			log_error(uL("emu;'" uPs("") "' not found"), info.rom.file);
			info.rom.file[0] = 0x00;
			return (EXIT_ERROR);
		}
		ustrncpy(info.rom.change_rom, info.rom.file, usizeof(info.rom.change_rom));
		info.rom.change_rom[usizeof(info.rom.change_rom) - 1] = 0x00;
	}

	if (info.rom.change_rom[0]) {
		_uncompress_archive *archive = NULL;
		BYTE rc = 0;

		archive = uncompress_archive_alloc(info.rom.change_rom, &rc);

		if (rc == UNCOMPRESS_EXIT_OK) {
			BYTE is_rom = FALSE, is_patch = FALSE;
			uTCHAR *patch = NULL;

			if (archive->rom.count > 0) {
				is_rom = TRUE;
			}
			if (archive->patch.count > 0) {
				is_patch = TRUE;
			}
			if (is_patch && !is_rom && !info.rom.file[0]) {
				is_patch = FALSE;
			}
			if (is_rom) {
				switch ((rc = uncompress_archive_extract_file(archive, UNCOMPRESS_TYPE_ROM))) {
					case UNCOMPRESS_EXIT_OK:
						ustrncpy(info.rom.change_rom, uncompress_archive_extracted_file_name(archive, UNCOMPRESS_TYPE_ROM),
							usizeof(info.rom.change_rom) - 1);
						found = TRUE;
						break;
					case UNCOMPRESS_EXIT_ERROR_ON_UNCOMP:
						break;
					case UNCOMPRESS_EXIT_IS_COMP_BUT_NOT_SELECTED:
					case UNCOMPRESS_EXIT_IS_COMP_BUT_NO_ITEMS:
						ustrncpy(info.rom.change_rom, info.rom.file, usizeof(info.rom.change_rom));
						info.rom.change_rom[usizeof(info.rom.change_rom) - 1] = 0x00;
						break;
					default:
						break;
				}
			}
			if (is_patch) {
				switch ((rc = uncompress_archive_extract_file(archive, UNCOMPRESS_TYPE_PATCH))) {
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

	if (!found) {
		ustrncpy(info.rom.change_rom, info.rom.file, usizeof(info.rom.change_rom));
		info.rom.change_rom[usizeof(info.rom.change_rom) - 1] = 0x00;
		return (EXIT_ERROR);
	}

	if (patcher_ctrl_if_exist(NULL) == EXIT_OK) {
		uTCHAR buffer[LENGTH_FILE_NAME_MID];

		log_info_open(uL("patch file;"));
		umemset(buffer, 0x00, usizeof(buffer));
		gui_utf_basename((uTCHAR *)patcher.file, buffer, usizeof(buffer) - 1);
		log_close(uL("" uPs("")), buffer);

		log_info_box_open(uL("folder;"));
		umemset(buffer, 0x00, usizeof(buffer));
		gui_utf_dirname((uTCHAR *)patcher.file, buffer, usizeof(buffer) - 1);
		log_close_box(uL("" uPs("")), buffer);
	}

	return (EXIT_OK);
}
static uTCHAR *emu_ctrl_rom_ext(uTCHAR *file) {
	static uTCHAR ext[10];
	uTCHAR name_file[255], *last_dot = NULL;

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
	if ((*add)) {
		(*add) = FALSE;
		recent_roms_add(file);
	}
}
