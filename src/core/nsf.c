/*  Copyright (C) 2010-2023 Fabio Cavallo (aka FHorse)
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#define _NSF_STATIC_
#include "nsf.h"
#undef _NSF_STATIC_
#define _DOS_STATIC_
#include "draw_on_screen.h"
#undef _DOS_STATIC_
#include "nsfe.h"
#include "rom_mem.h"
#include "mem_map.h"
#include "mappers.h"
#include "video/gfx.h"
#include "info.h"
#include "ppu.h"
#include "cpu.h"
#include "conf.h"
#include "gui.h"
#include "audio/blipbuf.h"
#include "audio/snd.h"
#include "patcher.h"
#include "extra/kissfft/kiss_fft.h"

enum nsf_flags {
	VERSION,
	TOTAL_SONGS,
	STARTING_SONGS,
	LOAD_ADR_LO,
	LOAD_ADR_HI,
	INIT_ADR_LO,
	INIT_ADR_HI,
	PLAY_ADR_LO,
	PLAY_ADR_HI,
	TOTAL_FL
};
enum nsf_timining {
	NFS_TIME_CHANGE_SONG = 150,
	NFS_TIME_AUTO_NEXT_SONG = 5000,
	NSF_TIME_FADEOUT = 3000,
	NSF_TIME_EFFECT_UPDATE = 1000 / 25
};
enum nsf_text_curtain {
	NSF_TEXT_CURTAIN_INIT,
	NSF_TEXT_CURTAIN_TICK,
	NSF_TEXT_CURTAIN_QUIT
};
enum nsf_options {
	NSF_OPTION_PLAYLIST,
	NSF_OPTION_FADEOUT,
	NSF_OPTION_REVERSE_BITS_DPCM
};
enum nsf_gui {
	NSF_GUI_WC = 22,
	NSF_GUI_COMMANDS_PREV = 0,
	NSF_GUI_COMMANDS_PLAY,
	NSF_GUI_COMMANDS_PAUSE,
	NSF_GUI_COMMANDS_STOP,
	NSF_GUI_COMMANDS_NEXT,
	NSF_GUI_COMMANDS,
	NSF_GUI_COMMANDS_W = (NSF_GUI_WC * NSF_GUI_COMMANDS) + 2,
	NSF_GUI_COMMANDS_X = (SCR_COLUMNS - NSF_GUI_COMMANDS_W) >> 1,
	NSF_GUI_COMMANDS_Y = dospf(6) - 4,
	NSF_GUI_COMMANDS_H = dospf(2),

	NSF_GUI_INFO_SONG_LINES = 3,
	NSF_GUI_INFO_SONG_W = (SCR_COLUMNS - dospf(3) - dospf(3)) + 4,
	NSF_GUI_INFO_SONG_X = (SCR_COLUMNS - NSF_GUI_INFO_SONG_W) >> 1,
	NSF_GUI_INFO_SONG_Y = NSF_GUI_COMMANDS_Y + NSF_GUI_COMMANDS_H - 1,
	NSF_GUI_INFO_SONG_H = (dospf(NSF_GUI_INFO_SONG_LINES) * 2),
	NSF_GUI_INFO_SONG_TITLE_ROW = 26,

	NSF_GUI_EFFECT_BOOKMARK_W = 12,
	NSF_GUI_EFFECT_W = SCR_COLUMNS,
	NSF_GUI_EFFECT_H = 50,
	NSF_GUI_EFFECT_X = (SCR_COLUMNS - NSF_GUI_EFFECT_W) / 2,
	NSF_GUI_EFFECT_Y = SCR_ROWS - NSF_GUI_EFFECT_H,
	NSF_GUI_EFFECT_BARS = 60,
	NSF_GUI_EFFECT_BARS_W = SCR_COLUMNS - 16,
	NSF_GUI_EFFECT_BARS_H = NSF_GUI_EFFECT_H,
	NSF_GUI_EFFECT_BARS_X = (SCR_COLUMNS - NSF_GUI_EFFECT_BARS_W) / 2,
	NSF_GUI_EFFECT_BARS_Y = SCR_ROWS - NSF_GUI_EFFECT_BARS_H,

	NSF_GUI_OPTIONS_Y = NSF_GUI_EFFECT_Y - dospf(5),
	NSF_GUI_OPTIONS_BOX_SIDE = 10,
	NSF_GUI_OPTIONS_BOX_SPACE = 4,
	NSF_GUI_OPTIONS_PLAYLIST_X = 12,
	NSF_GUI_OPTIONS_FADEOUT_X = SCR_COLUMNS - NSF_GUI_OPTIONS_PLAYLIST_X,

	NSF_GUI_Y = NSF_GUI_EFFECT_Y - dospf(5),
};
enum nsf_header { NSF_HEADER_LENGTH = 128 };

typedef struct _nsf_option_data {
	int x, y;
	char txt[20];
	BYTE direction;
	int x1, x2;
} _nsf_option_data;

_nsf nsf;

static void nsf_effect_set_coords(_nsf_effect_coords *coords, int x, int y, int w, int h);
static void nsf_effect_raw(BYTE solid);
static void nsf_effect_hanning_window(BYTE solid);
static void nsf_effect_bars(void);
static void nsf_change_song(BYTE button, unsigned int mode);
static void nsf_change_current_song(unsigned int mode);
static void nsf_draw_controls(void);
static char *nsf_print_color(int color);
static char *nsf_print_number(unsigned int song, BYTE decimal, int color);
static char *nsf_print_time(double timer, BYTE mode, int color);
static void nsf_print_option(_nsf_option_data *option, BYTE active, BYTE mode);
static void nsf_text_scroll_set_buffer(_nsf_text_scroll *scroll, const char *fmt, ...);
static void nsf_text_scroll_tick(_nsf_text_scroll *scroll);
static void nsf_text_curtain(_nsf_text_curtain *curtain, BYTE mode);
static void nsf_text_curtain_add_line(_nsf_text_curtain *curtain, const char *fmt, ...);
static void nsf_reset_song_variables(void);

static _nsf_option_data nsf_options_data[] = {
	{ 4,               NSF_GUI_EFFECT_Y - dospf(6) - 4, "Playlist",          FALSE, -1, -1 },
	{ SCR_COLUMNS - 4, NSF_GUI_EFFECT_Y - dospf(6) - 4, "Fadeout",           TRUE,  -1, -1 },
	{ 4,               NSF_GUI_EFFECT_Y - dospf(5) + 1, "Reverse bits DPCM", FALSE, -1, -1 },
};

void nsf_init(void) {
	memset(&nsf, 0x00, sizeof(nsf));

	nsf_effect_set_coords(&nsf.effect_coords, NSF_GUI_EFFECT_X, NSF_GUI_EFFECT_Y, NSF_GUI_EFFECT_W, NSF_GUI_EFFECT_H);
	nsf_effect_set_coords(&nsf.effect_bars_coords, NSF_GUI_EFFECT_BARS_X, NSF_GUI_EFFECT_BARS_Y,
		NSF_GUI_EFFECT_BARS_W, NSF_GUI_EFFECT_BARS_H);
}
void nsf_quit(void) {
	if (nsf.info.track_label) {
		free(nsf.info.track_label);
	}
	if (nsf.info.auth) {
		free(nsf.info.auth);
	}
	if (nsf.playlist.data) {
		free(nsf.playlist.data);
	}
	if (nsf.text.data) {
		free(nsf.text.data);
	}
	if (nsf.info_song) {
		free(nsf.info_song);
	}
	nsf_text_curtain(&nsf.curtain_info, NSF_TEXT_CURTAIN_QUIT);
	nsf_text_curtain(&nsf.curtain_title_song, NSF_TEXT_CURTAIN_QUIT);
	nsf_init();
}
void nsf_reset(void) {
	nsf.timers.effect = 0;
}
void nsf_info(void) {
	log_info_box(uL("name;%s"), nsf.info.name);
	log_info_box(uL("artist;%s"), nsf.info.artist);
	log_info_box(uL("copyright;%s"), nsf.info.copyright);
	log_info_box(uL("ripper;%s"), nsf.info.ripper);
	log_info_box(uL("text;%s"), nsf.text.data);
}
BYTE nsf_load_rom(void) {
	_rom_mem rom;

	{
		static const uTCHAR rom_ext[2][10] = { uL(".nsf\0"), uL(".NSF\0") };
		BYTE found = TRUE;
		FILE *fp = NULL;

		fp = ufopen(info.rom.file, uL("rb"));

		if (!fp) {
			unsigned int i = 0;

			found = FALSE;

			for (i = 0; i < LENGTH(rom_ext); i++) {
				uTCHAR rom_file[LENGTH_FILE_NAME_LONG];

				umemset(rom_file, 0x00, usizeof(rom_file));
				umemcpy(rom_file, info.rom.file, usizeof(rom_file) - 10 - 1);
				ustrcat(rom_file, rom_ext[i]);

				fp = ufopen(rom_file, uL("rb"));

				if (fp) {
					ustrncpy(info.rom.file, rom_file, usizeof(info.rom.file));
					found = TRUE;
					break;
				}
			}
		}

		if (!found) {
			return (EXIT_ERROR);
		}

		fseek(fp, 0L, SEEK_END);
		rom.size = ftell(fp);
		fseek(fp, 0L, SEEK_SET);

		if ((rom.data = (BYTE *)malloc(rom.size)) == NULL) {
			fclose(fp);
			return (EXIT_ERROR);
		}

		if (fread(rom.data, 1, rom.size, fp) != rom.size) {
			fclose(fp);
			free(rom.data);
			return (EXIT_ERROR);
		}

		fclose(fp);
	}

	patcher_apply(&rom);

	rom.position = 0;

	if ((rom.data[rom.position++] == 'N') &&
		(rom.data[rom.position++] == 'E') &&
		(rom.data[rom.position++] == 'S') &&
		(rom.data[rom.position++] == 'M') &&
		(rom.data[rom.position++] == '\32')) {
		BYTE tmp = 0, flags[TOTAL_FL];
		size_t len = rom.size;

		info.format = NSF_FORMAT;

		info.machine[DATABASE] = DEFAULT;
		info.prg.ram.bat.banks = 0;
		info.prg.ram.banks_8k_plus = 0;

		nsf.info.name = &nsf_default_label[0];
		nsf.info.artist = &nsf_default_label[0];
		nsf.info.copyright = &nsf_default_label[0];
		nsf.info.ripper = &nsf_default_label[0];

		if (len < NSF_HEADER_LENGTH) {
			free(rom.data);
			return (EXIT_ERROR);
		}

		len -= NSF_HEADER_LENGTH;

		if (rom_mem_ctrl_memcpy(&flags[0], &rom, sizeof(flags)) == EXIT_ERROR) {
			free(rom.data);
			return (EXIT_ERROR);
		}

		nsf.version = flags[VERSION];
		nsf.songs.total = flags[TOTAL_SONGS];
		nsf.songs.starting = flags[STARTING_SONGS];
		nsf.adr.load = (flags[LOAD_ADR_HI] << 8) | flags[LOAD_ADR_LO];
		nsf.adr.init = (flags[INIT_ADR_HI] << 8) | flags[INIT_ADR_LO];
		nsf.adr.play = (flags[PLAY_ADR_HI] << 8) | flags[PLAY_ADR_LO];

		if ((nsf.songs.total == 0) || (nsf.adr.load < 0x6000) || (nsf.adr.init < 0x6000) || (nsf.adr.play < 0x6000)) {
			free(rom.data);
			return (EXIT_ERROR);
		}

		if (nsf.songs.starting > 0) {
			if (nsf.songs.starting > nsf.songs.total) {
				nsf.songs.starting = 1;
			}
		} else {
			nsf.songs.starting = 1;
		}

		nsf.songs.total--;

		{
			char buffer[33];
			char *auth = NULL, **dst = NULL;

			nsf.info.auth = (char *)malloc((size_t)((32 + 1) * 4));
			if (!nsf.info.auth) {
				log_error(uL("nsf;out of memory"));
				free(rom.data);
				return (EXIT_ERROR);
			}

			memset(nsf.info.auth, 0x00, (size_t)((32 + 1) * 4));

			auth = nsf.info.auth;

			for (tmp = 0; tmp < 3; tmp++) {
				switch (tmp) {
					default:
					case 0:
						dst = &nsf.info.name;
						break;
					case 1:
						dst = &nsf.info.artist;
						break;
					case 2:
						dst = &nsf.info.copyright;
						break;
				}

				memset(&buffer, 0x00, sizeof(buffer));

				if (rom_mem_ctrl_memcpy(&buffer, &rom, 32) == EXIT_ERROR) {
					free(rom.data);
					return (EXIT_ERROR);
				}

				(*dst) = auth;
				strncpy((*dst), buffer, (int)sizeof(buffer));
				if ((*dst[0]) == 0) {
					(*dst) = &nsf_default_label[0];
				}
				auth += (strlen(buffer) + 1);
			}
		}

		nsf.play_speed.ntsc = rom.data[rom.position++];
		nsf.play_speed.ntsc |= (rom.data[rom.position++] << 8);

		if (rom_mem_ctrl_memcpy(&nsf.bankswitch.banks, &rom, sizeof(nsf.bankswitch.banks)) == EXIT_ERROR) {
			free(rom.data);
			return (EXIT_ERROR);
		}

		if (nsf.bankswitch.banks[0] | nsf.bankswitch.banks[1] | nsf.bankswitch.banks[2] |
			nsf.bankswitch.banks[3] | nsf.bankswitch.banks[4] | nsf.bankswitch.banks[5] |
			nsf.bankswitch.banks[6] | nsf.bankswitch.banks[7]) {
			nsf.bankswitch.enabled = TRUE;
		}

		nsf.play_speed.pal = rom.data[rom.position++];
		nsf.play_speed.pal |= (rom.data[rom.position++] << 8);

		nsf.type = rom.data[rom.position++] & 0x03;

		tmp = rom.data[rom.position++];
		nsf.sound_chips.vrc6 = tmp & 0x01;
		nsf.sound_chips.vrc7 = tmp & 0x02;
		nsf.sound_chips.fds = tmp & 0x04;
		nsf.sound_chips.mmc5 = tmp & 0x08;
		nsf.sound_chips.namco163 = tmp & 0x10;
		nsf.sound_chips.sunsoft5b = tmp & 0x20;

		if (!nsf.sound_chips.fds && (nsf.adr.load < 0x8000)) {
			free(rom.data);
			return (EXIT_ERROR);
		}

		if ((rom.position + 4) > rom.size) {
			free(rom.data);
			return (EXIT_ERROR);
		}
		rom.position += 4;

		{
			switch (nsf.type & 0x03) {
				case NSF_NTSC_MODE:
				case NSF_DUAL_MODE:
					info.machine[DATABASE] = NTSC;
					break;
				case NSF_PAL_MODE:
					info.machine[DATABASE] = PAL;
					break;
				default:
					nsf.type = NSF_NTSC_MODE;
					info.machine[DATABASE] = NTSC;
					break;
			}

			if (!nsf.play_speed.ntsc) {
				nsf.play_speed.ntsc = 0x40FF;
			}

			if (!nsf.play_speed.pal) {
				nsf.play_speed.pal = 0x4E1D;
			}
		}

		{
			WORD ram = 0x2000;

			if (nsf.sound_chips.fds) {
				ram = 0xA000;
			}

			if (map_prg_ram_malloc(ram) != EXIT_OK) {
				free(rom.data);
				return (EXIT_ERROR);
			}
		}

		{
			int padding = nsf.adr.load & 0x0FFF;

			nsf.prg.banks_4k = ((len + padding) / 0x1000);

			if (((len + padding) % 0x1000)) {
				nsf.prg.banks_4k++;
			}

			if (map_prg_malloc((size_t)(nsf.prg.banks_4k * 0x1000), 0xF2, TRUE) == EXIT_ERROR) {
				free(rom.data);
				return (EXIT_ERROR);
			}

			if (rom_mem_ctrl_memcpy(prg_rom() + padding, &rom, len) == EXIT_ERROR) {
				free(rom.data);
				return (EXIT_ERROR);
			}

			nsf.prg.banks_4k--;
		}

		nsf.enabled = TRUE;

		nsf.songs.current = nsf.songs.starting - 1;

		memcpy(nsf.routine.prg, nsf_routine, 17);

		nsf.routine.prg[NSF_R_JMP_PLAY] = NSF_R_LOOP;

		info.mapper.id = NSF_MAPPER;
		info.cpu_rw_extern = TRUE;

		emu_save_header_info();

		free(rom.data);
	} else {
		free(rom.data);
		return (nsfe_load_rom());
	}

	nsf_after_load_rom();

	return (EXIT_OK);
}
void nsf_after_load_rom(void) {
	nsf.draw_mask_frames = 2;

	nsf.scroll_info_nsf.x = 0;
	nsf.scroll_info_nsf.y = dospf(2);
	nsf.scroll_info_nsf.rows = SCR_COLUMNS / dospf(1);
	nsf.scroll_info_nsf.reload = 150;
	nsf.scroll_info_nsf.velocity = 4;
	nsf_text_scroll_set_buffer(&nsf.scroll_info_nsf, "[cyan]%s[normal] - [yellow]%s[normal] - %s",
		nsf.info.name, nsf.info.artist, nsf.info.copyright);

	nsf.scroll_title_song.rows = NSF_GUI_INFO_SONG_TITLE_ROW;
	nsf.scroll_title_song.reload = 200;
	nsf.scroll_title_song.velocity = 5;

	nsf.curtain_info.reload.r1 = 50;
	nsf.curtain_info.reload.r2 = 3000;
	nsf.curtain_info.x = DOS_CENTER;
	nsf.curtain_info.y = NSF_GUI_EFFECT_Y - dospf(2);
	nsf.curtain_info.borders.left = -1;
	nsf.curtain_info.borders.right = -1;
	nsf.curtain_info.borders.bottom = 0;
	nsf.curtain_info.borders.top = -1;
	nsf.curtain_info.rows = SCR_COLUMNS / dospf(1);
	nsf_text_curtain(&nsf.curtain_info, NSF_TEXT_CURTAIN_INIT);

	nsf_text_curtain_add_line(&nsf.curtain_info, "-Pad commands-");
	nsf_text_curtain_add_line(&nsf.curtain_info, "[yellow][right][gray] : [green]Next Song");
	nsf_text_curtain_add_line(&nsf.curtain_info, "[yellow][left][gray] : [green]Restart[gray]/[green]Previous Song");
	nsf_text_curtain_add_line(&nsf.curtain_info, "[yellow][up][normal] or [yellow][down][gray] : [green]Change Effect");
	nsf_text_curtain_add_line(&nsf.curtain_info, "[yellow]Button [a][gray] : [green]Stop");
	nsf_text_curtain_add_line(&nsf.curtain_info, "[yellow][start1][start2][start3][gray] : [green]Play[gray]/[green]Pause");
	nsf_text_curtain_add_line(&nsf.curtain_info, "or use the mouse [lmouse]");
}
void nsf_init_tune(void) {
	WORD i = 0;

	nsf.made_tick = FALSE;

	cpu.SP = 0xFD;
	memset(mmcpu.ram, 0x00, sizeof(mmcpu.ram));

	memset(prg.ram.data, 0x00, prg.ram.size);

	if (nsf.sound_chips.vrc6) {
		init_NSF_VRC6(0x01, 0x02);
	}
	if (nsf.sound_chips.vrc7) {
		init_NSF_VRC7(0x10, 0x20);
	}
	if (nsf.sound_chips.fds) {
		map_init_NSF_FDS();
	}
	if (nsf.sound_chips.mmc5) {
		map_init_NSF_MMC5();
	}
	if (nsf.sound_chips.namco163) {
		map_init_NSF_N163();
	}
	if (nsf.sound_chips.sunsoft5b) {
		init_NSF_FME7();
	}

	for (i = 0x4000; i <= 0x4013; i++) {
		cpu_wr_mem(i, 0x00);
	}
	cpu_wr_mem(0x4015, 0x0F);
	cpu_wr_mem(0x4017, 0x40);

	if (nsf.sound_chips.fds) {
		cpu_wr_mem(0x4089, 0x80);
		cpu_wr_mem(0x408A, 0xE8);
	}

	nsf_reset_prg();

	nsf.routine.prg[NSF_R_SONG] = nsf.songs.current;
	nsf.routine.prg[NSF_R_TYPE] = nsf.type;

	nsf.routine.prg[NSF_R_INIT_LO] = nsf.adr.init & 0xFF;
	nsf.routine.prg[NSF_R_INIT_HI] = nsf.adr.init >> 8;

	nsf.routine.prg[NSF_R_PLAY_LO] = nsf.adr.play & 0xFF;
	nsf.routine.prg[NSF_R_PLAY_HI] = nsf.adr.play >> 8;

	nsf.made_tick = TRUE;
	nsf.state = NSF_PLAY;

	nsf.songs.started = FALSE;

	nsf_reset_song_variables();
	nsf_reset_timers();
}
void nsf_tick(void) {
	if (nsf.rate.count && (--nsf.rate.count == 0)) {
		nsf.frames++;
		nsf.rate.count = nsf.rate.reload;
		nsf.routine.prg[NSF_R_JMP_PLAY] = NSF_R_PLAY;
		if (nsf.routine.INT_NMI) {
			nmi.high = TRUE;
		}
		ppu.odd_frame = !ppu.odd_frame;
		info.frame_status = FRAME_FINISHED;

		if (!cfg->apu.channel[APU_MASTER]) {
			extcl_audio_samples_mod_nsf(NULL, 0);
		}

		nsf_main_screen_event();
		nsf_effect();
		gfx_draw_screen();
	}
}
void extcl_audio_samples_mod_nsf(SWORD *samples, int count) {
	double now = gui_get_ms();
	BYTE silence = TRUE;
	int i = 0;

	nsf.timers.diff = (now - nsf.timers.last_tick);

	//timer dell'effect
	nsf.timers.effect += nsf.timers.diff;

	if (!nsf.timers.update_only_diff) {
		// timer della canzone
		nsf.timers.song += nsf.timers.diff;

		// timer della rom
		nsf.timers.total_rom += nsf.timers.diff;

		// controllo se c'e' da applicare il fadeout
		if ((nsf.current_song.time > 0) && (nsf.current_song.fade > 0)) {
			if (nsf.timers.song > (nsf.current_song.time - nsf.current_song.fade)) {
				nsf.timers.fadeout += nsf.timers.diff;
				if (cfg->nsf_player_nsfe_fadeout) {
					double divider = 0;

					divider = 1.0f - ((1.0f / 100.f) *
						(100.0f - ((100.0f / (double)nsf.current_song.fade) *
						((double)nsf.current_song.fade - nsf.timers.fadeout))));
					for (i = 0; i < count; i++) {
						samples[i] = (SWORD)(samples[i] * divider);
					}
				}
			}
		} else {
			for (i = 0; i < count; i++) {
				if (samples[i]) {
					silence = FALSE;
					break;
				}
			}
			if (silence && nsf.songs.started) {
				nsf.timers.silence += nsf.timers.diff;
			} else {
				nsf.songs.started = TRUE;
				nsf.timers.silence = 0;
			}
		}
	}

	// timer dell'ultimo tick
	nsf.timers.last_tick = now;
}

void nsf_reset_prg(void) {
	DBWORD i = 0;

	if (nsf.bankswitch.enabled) {
		if (nsf.sound_chips.fds) {
			for (i = 0x5FF6; i <= 0x5FF7; i++) {
				cpu_wr_mem(i, nsf.bankswitch.banks[i & 0x07]);
			}
		}
		for (i = 0x5FF8; i <= 0x5FFF; i++) {
			cpu_wr_mem(i, nsf.bankswitch.banks[i & 0x07]);
		}
	} else {
		BYTE value = 0, bank = 0;

		nsf.bankswitch.enabled = TRUE;

		if (nsf.sound_chips.fds) {
			i = 0x6000;
		} else {
			i = 0x8000;
		}

		for (; i < 0x10000; i += 0x1000) {
			value = bank;
			control_bank(nsf.prg.banks_4k)
			if (i < (nsf.adr.load & 0xF000)) {
				cpu_wr_mem(0x5FF0 | (i >> 12), 0);
			} else {
				cpu_wr_mem(0x5FF0 | (i >> 12), value);
				if (bank < nsf.prg.banks_4k) {
					bank++;
				}
			}
		}
		nsf.bankswitch.enabled = FALSE;
	}
}
void nsf_reset_timers(void) {
	nsf.timers.last_tick = gui_get_ms();
}
void nsf_reset_song_title(void) {
	if (nsf.info_song) {
		memcpy(&nsf.current_song, &nsf.info_song[nsf.songs.current], sizeof(_nsf_info_song));

		nsf_text_scroll_set_buffer(&nsf.scroll_title_song, "%s", nsf.current_song.track_label);

		nsf_text_curtain(&nsf.curtain_title_song, NSF_TEXT_CURTAIN_INIT);
		nsf_text_curtain_add_line(&nsf.curtain_title_song, "%s", nsf.current_song.track_label);
	} else {
		nsf.current_song.track_label = nsf_default_label;
	}
}

void nsf_main_screen(void) {
	nsf_draw_controls();
	nsf_main_screen_event();
}
void nsf_main_screen_event(void) {
	int reset = FALSE;

	if (!(nsf.state & NSF_CHANGE_SONG) &&
		((nsf.timers.silence > NFS_TIME_AUTO_NEXT_SONG) ||
		((nsf.current_song.time > 0) && (nsf.timers.song >= nsf.current_song.time)))) {
		nsf.songs.started = FALSE;
		nsf_change_current_song(NSF_NEXT);
		nsf.routine.INT_NMI = 2;
		nsf.state |= NSF_CHANGE_SONG;
	}

	// gestione eventi
	if (port[PORT1].data[START] == PRESSED) {
		switch (nsf.state & 0x07) {
			case NSF_STOP:
				nsf.state = NSF_PLAY | NSF_CHANGE_SONG;
				nsf.timers.update_only_diff = FALSE;
				nsf.routine.INT_NMI = 2;
				reset = TRUE;
				break;
			case NSF_PAUSE:
				nsf.state = (nsf.state & NSF_CHANGE_SONG) | NSF_PLAY;
				nsf.timers.update_only_diff = FALSE;
				nsf.timers.last_tick = gui_get_ms();
				break;
			case NSF_PLAY:
				nsf.state = NSF_PAUSE;
				nsf.timers.update_only_diff = TRUE;
				break;
		}
		port[PORT1].data[START] = RELEASED;
	} else if (port[PORT1].data[BUT_A] == PRESSED) {
		nsf.state = NSF_STOP;
		nsf.timers.update_only_diff = TRUE;
		nsf.timers.song = 0;
		port[PORT1].data[BUT_A] = RELEASED;
		reset = TRUE;
	} else if (port[PORT1].data[UP] == PRESSED) {
		cfg->nsf_player_effect++;
		if (cfg->nsf_player_effect >= NSF_EFFECTS) {
			cfg->nsf_player_effect = 0;
		}
		port[PORT1].data[UP] = RELEASED;
	} else if (port[PORT1].data[DOWN] == PRESSED) {
		cfg->nsf_player_effect--;
		if (cfg->nsf_player_effect > NSF_EFFECTS) {
			cfg->nsf_player_effect = NSF_EFFECTS - 1;
		}
		port[PORT1].data[DOWN] = RELEASED;
	} else if (port[PORT1].data[LEFT] == PRESSED) {
		if (nsf.state & NSF_PAUSE) {
			nsf.state = NSF_STOP;
			nsf_change_song(LEFT, NSF_PREV);
		} else if (nsf.timers.song < 1000) {
			nsf_change_song(LEFT, NSF_PREV);
		} else {
			nsf_change_song(LEFT, NSF_RESTART_SONG);
		}
		reset = TRUE;
	} else if (port[PORT1].data[RIGHT] == PRESSED) {
		if (nsf.state & NSF_PAUSE) {
			nsf.state = NSF_STOP;
		}
		nsf_change_song(RIGHT, NSF_NEXT);
		reset = TRUE;
	}

	if (reset) {
		snd_reset_buffers();
		nsf_reset();
	}

	// ridisegno i controlli
	nsf_draw_controls();
}

void nsf_controls_mouse_in_gui(int x_mouse, int y_mouse) {
	int x = 0, y = 0, w = 0, h = 0, wc = 0;

#define nsf_c_x1(ctr) (x + 2 + (wc * (ctr)))
#define nsf_c_x2(ctr) (nsf_c_x1(ctr) + (wc - 1))
#define nsf_c_y1(ctr) (y + 2)
#define nsf_c_y2(ctr) (nsf_c_y1(ctr) + (h - 2 - 1))

	x = NSF_GUI_COMMANDS_X;
	y = NSF_GUI_COMMANDS_Y;
	h = NSF_GUI_COMMANDS_H;
	wc = NSF_GUI_WC;

	if (((x_mouse >= nsf_c_x1(NSF_GUI_COMMANDS_PREV)) && (x_mouse <= nsf_c_x2(NSF_GUI_COMMANDS_PREV))) &&
		((y_mouse >= nsf_c_y1(NSF_GUI_COMMANDS_PREV)) && (y_mouse <= nsf_c_y2(NSF_GUI_COMMANDS_PREV)))) {
		port->data[LEFT] = PRESSED;
		nsf.timers.button[LEFT] = 0;
		return;
	}
	if (((x_mouse >= nsf_c_x1(NSF_GUI_COMMANDS_PLAY)) && (x_mouse <= nsf_c_x2(NSF_GUI_COMMANDS_PLAY))) &&
		((y_mouse >= nsf_c_y1(NSF_GUI_COMMANDS_PLAY)) && (y_mouse <= nsf_c_y2(NSF_GUI_COMMANDS_PLAY)))) {
		if (!(nsf.state & NSF_PLAY)) {
			port->data[START] = PRESSED;
			nsf.timers.button[START] = 0;
		}
		return;
	}
	if (((x_mouse >= nsf_c_x1(NSF_GUI_COMMANDS_PAUSE)) && (x_mouse <= nsf_c_x2(NSF_GUI_COMMANDS_PAUSE))) &&
		((y_mouse >= nsf_c_y1(NSF_GUI_COMMANDS_PAUSE)) && (y_mouse <= nsf_c_y2(NSF_GUI_COMMANDS_PAUSE)))) {
		if (nsf.state & NSF_PLAY) {
			port->data[START] = PRESSED;
			nsf.timers.button[START] = 0;
		}
		return;
	}
	if (((x_mouse >= nsf_c_x1(NSF_GUI_COMMANDS_STOP)) && (x_mouse <= nsf_c_x2(NSF_GUI_COMMANDS_STOP))) &&
		((y_mouse >= nsf_c_y1(NSF_GUI_COMMANDS_STOP)) && (y_mouse <= nsf_c_y2(NSF_GUI_COMMANDS_STOP)))) {
		port->data[BUT_A] = PRESSED;
		nsf.timers.button[BUT_A] = 0;
		return;
	}
	if (((x_mouse >= nsf_c_x1(NSF_GUI_COMMANDS_NEXT)) && (x_mouse <= nsf_c_x2(NSF_GUI_COMMANDS_NEXT))) &&
		((y_mouse >= nsf_c_y1(NSF_GUI_COMMANDS_NEXT)) && (y_mouse <= nsf_c_y2(NSF_GUI_COMMANDS_NEXT)))) {
		port->data[RIGHT] = PRESSED;
		nsf.timers.button[RIGHT] = 0;
		return;
	}

#undef nsf_c_x1
#undef nsf_c_x2
#undef nsf_c_y1
#undef nsf_c_y2

#define nsf_c_x1() (nsf.effect_coords.x2 / 2)
#define nsf_c_x2() (nsf.effect_coords.x2)
#define nsf_c_y1() (nsf.effect_coords.y1)
#define nsf_c_y2() (nsf.effect_coords.y2)

	if (((x_mouse >= nsf_c_x1()) && (x_mouse <= nsf_c_x2())) && ((y_mouse >= nsf_c_y1()) && (y_mouse <= nsf_c_y2()))) {
		port->data[UP] = PRESSED;
		nsf.timers.button[UP] = 0;
		return;
	}

#undef nsf_c_x1
#undef nsf_c_x2
#undef nsf_c_y1
#undef nsf_c_y2

#define nsf_c_x1() (nsf.effect_coords.x1)
#define nsf_c_x2() (nsf.effect_coords.x2 / 2)
#define nsf_c_y1() (nsf.effect_coords.y1)
#define nsf_c_y2() (nsf.effect_coords.y2)

	if (((x_mouse >= nsf_c_x1()) && (x_mouse <= nsf_c_x2())) && ((y_mouse >= nsf_c_y1()) && (y_mouse <= nsf_c_y2()))) {
		port->data[DOWN] = PRESSED;
		nsf.timers.button[DOWN] = 0;
		return;
	}

#undef nsf_c_x1
#undef nsf_c_x2
#undef nsf_c_y1
#undef nsf_c_y2

#define nsf_c_x1() (x + (w / 2) + 1)
#define nsf_c_x2() (nsf_c_x1() + (w / 2) - 2)
#define nsf_c_y1() (y + ((h / NSF_GUI_INFO_SONG_LINES) * 1) + 1)
#define nsf_c_y2() (nsf_c_y1() + (dospf(1) * 2) - 1)

	if (info.format == NSFE_FORMAT) {
		x = NSF_GUI_INFO_SONG_X;
		y = NSF_GUI_INFO_SONG_Y;
		w = NSF_GUI_INFO_SONG_W;
		h = NSF_GUI_INFO_SONG_H;

		if (((x_mouse >= nsf_c_x1()) && (x_mouse <= nsf_c_x2())) && ((y_mouse >= nsf_c_y1()) && (y_mouse <= nsf_c_y2()))) {
			nsf.options.visual_duration = !nsf.options.visual_duration;
			return;
		}
	}

#undef nsf_c_x1
#undef nsf_c_x2
#undef nsf_c_y1
#undef nsf_c_y2

#define nsf_c_x1() (x)
#define nsf_c_x2() (nsf_c_x1() + w)
#define nsf_c_y1() (y)
#define nsf_c_y2() (nsf_c_y1() + h)

	if (info.format == NSFE_FORMAT) {
		x = nsf_options_data[NSF_OPTION_PLAYLIST].x1;
		y = nsf_options_data[NSF_OPTION_PLAYLIST].y;
		w = nsf_options_data[NSF_OPTION_PLAYLIST].x2 - nsf_options_data[NSF_OPTION_PLAYLIST].x1;
		h = NSF_GUI_OPTIONS_BOX_SIDE;

		if (((x_mouse >= nsf_c_x1()) && (x_mouse <= nsf_c_x2())) && ((y_mouse >= nsf_c_y1()) && (y_mouse <= nsf_c_y2()))) {
			BYTE current = nsf.songs.current;

			cfg->nsf_player_nsfe_playlist = !cfg->nsf_player_nsfe_playlist;
			nsf_change_current_song(NSF_RESTART_SONG);
			if (nsf.songs.current != current) {
				nsf.routine.INT_NMI = 2;
				nsf.state |= NSF_CHANGE_SONG;
			}
			return;
		}

		x = nsf_options_data[NSF_OPTION_FADEOUT].x1;
		y = nsf_options_data[NSF_OPTION_FADEOUT].y;
		w = nsf_options_data[NSF_OPTION_FADEOUT].x2 - nsf_options_data[NSF_OPTION_FADEOUT].x1;
		h = NSF_GUI_OPTIONS_BOX_SIDE;

		if (((x_mouse >= nsf_c_x1()) && (x_mouse <= nsf_c_x2())) && ((y_mouse >= nsf_c_y1()) && (y_mouse <= nsf_c_y2()))) {
			cfg->nsf_player_nsfe_fadeout = !cfg->nsf_player_nsfe_fadeout;
			return;
		}
	}

	x = nsf_options_data[NSF_OPTION_REVERSE_BITS_DPCM].x1;
	y = nsf_options_data[NSF_OPTION_REVERSE_BITS_DPCM].y;
	w = nsf_options_data[NSF_OPTION_REVERSE_BITS_DPCM].x2 - nsf_options_data[NSF_OPTION_REVERSE_BITS_DPCM].x1;
	h = NSF_GUI_OPTIONS_BOX_SIDE;

	if (((x_mouse >= nsf_c_x1()) && (x_mouse <= nsf_c_x2())) && ((y_mouse >= nsf_c_y1()) && (y_mouse <= nsf_c_y2()))) {
		cfg->reverse_bits_dpcm = !cfg->reverse_bits_dpcm;
		gui_update_apu_channels_widgets();
		return;
	}

#undef nsf_c_x1
#undef nsf_c_x2
#undef nsf_c_y1
#undef nsf_c_y2
}

void nsf_effect(void) {
	switch (cfg->nsf_player_effect) {
		case NSF_EFFECT_BARS:
		case NSF_EFFECT_BARS_MIXED:
			nsf_effect_bars();
			break;
		case NSF_EFFECT_RAW:
			nsf_effect_raw(FALSE);
			break;
		case NSF_EFFECT_RAW_FULL:
			nsf_effect_raw(TRUE);
			break;
		case NSF_EFFECT_HANNING:
			nsf_effect_hanning_window(FALSE);
			break;
		case NSF_EFFECT_HANNING_FULL:
			nsf_effect_hanning_window(TRUE);
			break;
	}
}

static void nsf_effect_set_coords(_nsf_effect_coords *coords, int x, int y, int w, int h) {
	coords->x1 = x;
	coords->x2 = (x + w) - 1;
	coords->y1 = y;
	coords->y2 = (y + h) - 1;
	if (coords->x1 < 0) {
		coords->x1 = 0;
	}
	if (coords->x2 > SCR_COLUMNS) {
		coords->x2 = SCR_COLUMNS - 1;
	}
	if (coords->y1 < 0) {
		coords->y1 = 0;
	}
	if (coords->y2 > SCR_ROWS) {
		coords->y2 = SCR_ROWS - 1;
	}
	coords->w = (coords->x2 - coords->x1) + 1;
	coords->h = (coords->y2 - coords->y1) + 1;
	coords->y_center = coords->y1 + (coords->h / 2);
}
static void nsf_effect_raw(BYTE solid) {
	int x = 0, y = 0, count = 0, len = 0, y_last = nsf.effect_coords.y_center;
	SWORD *buffer = NULL;

	if (nsf.timers.effect <= NSF_TIME_EFFECT_UPDATE) {
		return;
	}

	nsf.timers.effect = 0;

	if (nsf.state & NSF_STOP) {
		len = 0;
	} else {
		len = audio_buffer_blipbuf(&buffer);
	}

	for (y = nsf.effect_coords.y1; y <= nsf.effect_coords.y2; y++) {
		for (x = nsf.effect_coords.x1; x <= nsf.effect_coords.x2; x++) {
			ppu_screen.wr->line[y][x] = doscolor(DOS_BLACK);
		}
	}

	for (x = nsf.effect_coords.x1, count = 0; x <= nsf.effect_coords.x2; x++, count++) {
		int a = 0, b = 0;

		if (!buffer || !len) {
			y = nsf.effect_coords.y_center;
		} else {
			y = nsf.effect_coords.y_center + (int)(((((buffer[(int)((count * len) / nsf.effect_coords.w)]))) /
				(16384.0 / nsf.effect_coords.h)) * (cfg->apu.channel[APU_MASTER] * cfg->apu.volume[APU_MASTER]));
		}

		if ((y >= nsf.effect_coords.y1) && (y < nsf.effect_coords.y2)) {
			ppu_screen.wr->line[y][x] = doscolor(DOS_GREEN);
		}

		if (y_last > y) {
			a = y;
			b = y_last;
		} else if (y_last < y) {
			a = y_last;
			b = y;
		} else {
			a = y;
			b = y;
		}

		for (; a < b; a++) {
			if (((a >= nsf.effect_coords.y1) && (a <= nsf.effect_coords.y2)) && (ppu_screen.wr->line[a][x] != doscolor(DOS_GREEN))) {
				ppu_screen.wr->line[a][x] = doscolor(DOS_YELLOW);
			}
		}

		if (!solid) {
			y_last = y;
		}
	}
}
static void nsf_effect_hanning_window(BYTE solid) {
	int x = 0, y = 0, count = 0, len = 0, y_last = nsf.effect_coords.y_center;
	SWORD *buffer = NULL;
	double multiplier;

	if (nsf.timers.effect <= NSF_TIME_EFFECT_UPDATE) {
		return;
	}

	nsf.timers.effect = 0;

	if (nsf.state & NSF_STOP) {
		len = 0;
	} else {
		len = audio_buffer_blipbuf(&buffer);
	}

	for (y = nsf.effect_coords.y1; y <= nsf.effect_coords.y2; y++) {
		for (x = nsf.effect_coords.x1; x <= nsf.effect_coords.x2; x++) {
			ppu_screen.wr->line[y][x] = doscolor(DOS_BLACK);
		}
	}

	for (x = nsf.effect_coords.x1, count = 0; x <= nsf.effect_coords.x2; x++, count++) {
		int a = 0, b = 0;

		if (!buffer || !len) {
			y = nsf.effect_coords.y_center;
		} else {
			multiplier = 0.5 * (1 - cos(2 * M_PI * count / nsf.effect_coords.w));
			y = nsf.effect_coords.y_center + (int)(((multiplier *
				((buffer[(int)((count * len) / nsf.effect_coords.w)]))) / (16384.0 / nsf.effect_coords.h)) *
				(cfg->apu.channel[APU_MASTER] * cfg->apu.volume[APU_MASTER]));
		}

		if ((y >= nsf.effect_coords.y1) && (y <= nsf.effect_coords.y2)) {
			ppu_screen.wr->line[y][x] = doscolor(DOS_GREEN);
		}

		if (y_last > y) {
			a = y;
			b = y_last;
		} else if (y_last < y) {
			a = y_last;
			b = y;
		} else {
			a = y;
			b = y;
		}

		for (; a < b; a++) {
			if (((a >= nsf.effect_coords.y1) && (a <= nsf.effect_coords.y2)) && (ppu_screen.wr->line[a][x] != doscolor(DOS_GREEN))) {
				ppu_screen.wr->line[a][x] = doscolor(DOS_YELLOW);
			}
		}

		if (!solid) {
			y_last = y;
		}
	}
}
static void nsf_effect_bars(void) {
	int x = 0, y = 0, count = 0, len = 0, y_last = nsf.effect_bars_coords.y2 - 5;
	double bars = cfg->nsf_player_effect == NSF_EFFECT_BARS ? NSF_GUI_EFFECT_BARS : NSF_GUI_EFFECT_BARS * 2.0f;
	double bar[(int)bars];
	SWORD *buffer = NULL;

	if (nsf.timers.effect <= NSF_TIME_EFFECT_UPDATE) {
		return;
	}

	nsf.timers.effect = 0;

	if (nsf.state & NSF_STOP) {
		len = 0;
	} else {
		len = audio_buffer_blipbuf(&buffer);
	}

	for (y = nsf.effect_coords.y1; y <= nsf.effect_coords.y2; y++) {
		for (x = nsf.effect_coords.x1; x <= nsf.effect_coords.x2; x++) {
			ppu_screen.wr->line[y][x] = doscolor(DOS_BLACK);
		}
	}

	if (!buffer || !len) {
		for (count = 0; count < (int)bars; count++) {
			bar[count] = 0;
		}
	} else {
		kiss_fft_cpx fft_in[len], fft_out[len];
		kiss_fft_cfg fft_cfg = NULL;

		for (count = 0; count < len; count++) {
			if (cfg->nsf_player_effect == NSF_EFFECT_BARS_MIXED) {
				fft_in[count].r = 6.5f * ((float)buffer[count] / 16384.0f);
			} else {
				fft_in[count].r = (float)(6.5f * (1.0f - cos((2.0f * M_PI * count) / (len - 1))) * ((float)buffer[count] / 16384.0f));
			}
			fft_in[count].i = 0;
		}

		for (count = 0; count < (int)bars; count++) {
			bar[count] = 0;
		}

		fft_cfg = kiss_fft_alloc(len, 0, NULL, NULL);
		if (fft_cfg) {
			int index = (int)(bars / 2.0f), last_index = 999;

			kiss_fft(fft_cfg, fft_in, fft_out);
			kiss_fft_free(fft_cfg);

			for (count = 0; count <= (len / 2); count++) {
				const double band_width = ((float)len / 2.0f) / bars, multiplier = 0.17f;
				double magnitude = 0, amplitude = 0;
				int tmp = 0;

				tmp = (int)((double)count / band_width);

				if (cfg->nsf_player_effect == NSF_EFFECT_BARS_MIXED) {
					if (last_index != tmp) {
						if (tmp & 0x1) {
							index += tmp;
						} else {
							index -= tmp;
						}
						last_index = tmp;
					}
				} else {
					index = last_index = tmp;
				}

				magnitude = sqrt((double)(fft_out[count].r * fft_out[count].r + fft_out[count].i * fft_out[count].i));

				amplitude = multiplier * log(magnitude);
				if (amplitude < 0.0f) {
					amplitude = 0.0f;
				}

				if (amplitude > 0.70f) {
					amplitude /= 1.50f;
				}

				if (amplitude > bar[index]) {
					bar[index] = amplitude;
				}
			}
		} else {
			log_error(uL("nsf;not enough memory?"));
		}
	}

	for (count = 0; count < bars; count++) {
		int a = 0, b = 0, x1 = 0, x2 = 0;
		double value = (bar[count] * 16383.0f) / (16384.0f / (double)(nsf.effect_bars_coords.h - 5));

		y = (BYTE)((nsf.effect_bars_coords.y2 - 5) - (value * (cfg->apu.channel[APU_MASTER] * cfg->apu.volume[APU_MASTER])));

		x1 = (int)(nsf.effect_bars_coords.x1 + ((nsf.effect_bars_coords.w / bars) * count));
		x2 = (int)(x1 + (nsf.effect_bars_coords.w / bars));

		for (x = x1; x < x2; x++) {
			if ((y >= nsf.effect_bars_coords.y1) && (y <= nsf.effect_bars_coords.y2)) {
				ppu_screen.wr->line[y][x] = doscolor(DOS_GREEN);
			}
		}

		if ((x - 1) <= nsf.effect_bars_coords.x1) {
			a = y;
			b = y;
		} else if (y_last > y) {
			a = y;
			b = y_last;
		} else if (y_last < y) {
			a = y_last;
			b = y;
		} else {
			a = y;
			b = y;
		}

		for (; a <= b; a++) {
			if ((a >= nsf.effect_bars_coords.y1) && (a <= nsf.effect_bars_coords.y2)) {
				if (ppu_screen.wr->line[a][x1] != doscolor(DOS_GREEN)) {
					ppu_screen.wr->line[a][x1] = doscolor(DOS_RED);
				}
				if (ppu_screen.wr->line[a][x - 1] != doscolor(DOS_GREEN)) {
					ppu_screen.wr->line[a][x - 1] = doscolor(DOS_GRAY);
				}
			}
		}
	}
}
static void nsf_change_song(BYTE button, unsigned int mode) {
	if (nsf.timers.button[button] <= 0) {
		nsf_change_current_song(mode);
		nsf.routine.INT_NMI = 2;
		nsf.state |= NSF_CHANGE_SONG;
		nsf.timers.button[button] = NFS_TIME_CHANGE_SONG;
	} else {
		nsf.timers.button[button] -= nsf.timers.diff;
	}
}
static void nsf_change_current_song(unsigned int mode) {
	if (cfg->nsf_player_nsfe_playlist && (nsf.playlist.count > 0)) {
		nsf_change_current_song_next:
		switch (mode) {
			case NSF_NEXT:
				nsf.playlist.index++;
				break;
			case NSF_PREV:
				nsf.playlist.index--;
				break;
			default:
			case NSF_RESTART_SONG:
				break;
		}

		if (nsf.playlist.index >= nsf.playlist.count) {
			if (mode == NSF_NEXT) {
				nsf.playlist.index = 0;
			} else {
				nsf.playlist.index = nsf.playlist.count - 1;
			}
		}
		nsf.songs.current = nsf.playlist.data[nsf.playlist.index];

		if (nsf.songs.current > nsf.songs.total) {
			goto nsf_change_current_song_next;
		}
	} else {
		switch (mode) {
			case NSF_NEXT:
				nsf.songs.current++;
				break;
			case NSF_PREV:
				nsf.songs.current--;
				break;
			default:
			case NSF_RESTART_SONG:
				break;
		}

		if (nsf.songs.current > nsf.songs.total) {
			if (mode == NSF_NEXT) {
				nsf.songs.current = 0;
			} else {
				nsf.songs.current = nsf.songs.total;
			}
		}
		if (nsf.playlist.count > 0) {
			unsigned int i = 0;

			for (i = 0; i < nsf.playlist.count; i++) {
				if (nsf.playlist.data[i] == nsf.songs.current) {
					nsf.playlist.index = i;
					break;
				}
			}
		}
	}

	if ((nsf.state & NSF_STOP) || (nsf.state & NSF_PAUSE)) {
		nsf_reset_song_variables();
	}
}
static void nsf_draw_controls(void) {
	int i = 0, x = 0, y = 0, w = 0, h = 0, wc = 0;

	// titolo nsf - artista - copyright
	{
		if (nsf.draw_mask_frames) {
			dos_hline(0, dospf(1), SCR_COLUMNS, doscolor(DOS_BROWN));
			if (info.format == NSFE_FORMAT) {
				dos_text(DOS_CENTER, dospf(1) - 4, " [yellow]p[red]u[green]N[cyan]E[brown]S[normal]"
					" [bck][yellow][blue]NSFe[normal][bck][black] Player ")
			} else {
				dos_text(DOS_CENTER, dospf(1) - 4, " [yellow]p[red]u[green]N[cyan]E[brown]S[normal]"
					" [bck][yellow][blue]NSF[normal][bck][black] Player ")
			}
		}
		nsf_text_scroll_tick(&nsf.scroll_info_nsf);
	}

	// comandi
	{
		x = NSF_GUI_COMMANDS_X;
		y = NSF_GUI_COMMANDS_Y;
		w = NSF_GUI_COMMANDS_W;
		h = NSF_GUI_COMMANDS_H;
		wc = NSF_GUI_WC;

		if (nsf.draw_mask_frames) {
			dos_hline(0, dospf(4), SCR_COLUMNS, doscolor(DOS_BROWN));
			dos_text(DOS_CENTER, dospf(4) - 4, " [red]Controls ")

			dos_box(x, y, w, h, doscolor(DOS_NORMAL), doscolor(DOS_NORMAL), doscolor(DOS_GRAY));
			dos_hline(x + 1, y + 1, w - 2, 0x002D);
			for (i = 0; i < NSF_GUI_COMMANDS; i++) {
				dos_vline(x + 1 + (wc * i), y + 1, h - 2, 0x002D);
			}
		}

#define nsf_dc_box_ctrl(ctr, color, str)\
	dos_box(x + 2 + (wc * (ctr)), y + 2, wc - 1, h - 2 -1,\
		doscolor(color), doscolor(color), doscolor(color));\
	dos_text(x + 1 + (wc * (ctr)) + ((wc - 8) / 2), y + ((h - 8) / 2), str)

		if (port[PORT1].data[LEFT] == PRESSED) {
			nsf_dc_box_ctrl(NSF_GUI_COMMANDS_PREV, DOS_GRAY, "[bck][gray][brown][prev]")
		} else {
			nsf_dc_box_ctrl(NSF_GUI_COMMANDS_PREV, DOS_BLACK, "[bck][black][prev]")
		}
		if (nsf.state & NSF_PLAY) {
			nsf_dc_box_ctrl(NSF_GUI_COMMANDS_PLAY, DOS_GRAY, "[bck][gray][green][play]")
		} else {
			nsf_dc_box_ctrl(NSF_GUI_COMMANDS_PLAY, DOS_BLACK, "[bck][black][play]")
		}
		if (nsf.state & NSF_PAUSE) {
			nsf_dc_box_ctrl(NSF_GUI_COMMANDS_PAUSE, DOS_GRAY, "[bck][gray][yellow][pause]")
		} else {
			nsf_dc_box_ctrl(NSF_GUI_COMMANDS_PAUSE, DOS_BLACK, "[bck][black][pause]")
		}
		if (nsf.state & NSF_STOP) {
			nsf_dc_box_ctrl(NSF_GUI_COMMANDS_STOP, DOS_GRAY, "[bck][gray][red][stop]")
		} else {
			nsf_dc_box_ctrl(NSF_GUI_COMMANDS_STOP, DOS_BLACK, "[bck][black][stop]")
		}
		if (port[PORT1].data[RIGHT] == PRESSED) {
			nsf_dc_box_ctrl(NSF_GUI_COMMANDS_NEXT, DOS_GRAY, "[bck][gray][brown][next]")
		} else {
			nsf_dc_box_ctrl(NSF_GUI_COMMANDS_NEXT, DOS_BLACK, "[bck][black][next]")
		}
#undef nsf_dc_box_ctrl
	}

	if (nsf.info_song) {
		x = NSF_GUI_INFO_SONG_X;
		w = NSF_GUI_INFO_SONG_W;
	} else {
		x = NSF_GUI_COMMANDS_X;
		w = NSF_GUI_COMMANDS_W;
	}
	h = NSF_GUI_INFO_SONG_H;
	y = NSF_GUI_INFO_SONG_Y;

	if (nsf.info_song) {
		if (nsf.draw_mask_frames) {
			dos_hline(x + 1, y + ((h / NSF_GUI_INFO_SONG_LINES) * 0) + 1, w - 2, 0x002D);
			for (i = 1; i < NSF_GUI_INFO_SONG_LINES + 1; i++) {
				dos_hline(x + 1, y + ((h / NSF_GUI_INFO_SONG_LINES) * i) + 0, w - 2, 0x002D);
			}
		}
	} else {
		if (nsf.draw_mask_frames) {
			dos_hline(x + 1, y + ((h / NSF_GUI_INFO_SONG_LINES) * 0) + 1, w - 2, 0x002D);
			dos_hline(x + 1, y + ((h / NSF_GUI_INFO_SONG_LINES) * 1) + 0, w - 2, 0x002D);
		}
	}

	// canzoni
	{
		static char buff[300];

		memset(buff, 0x00, sizeof(buff));

		if (nsf.info_song) {
			if (nsf.draw_mask_frames) {
				dos_vline(x + (w / 2), y + ((h / NSF_GUI_INFO_SONG_LINES) * 0) + 1, (h / NSF_GUI_INFO_SONG_LINES) - 1, 0x002D);
			}

			if (cfg->nsf_player_nsfe_playlist && (nsf.playlist.count > 0)) {
				dos_box(x + 1          , y + 2, (w / 2) - 1, (dospf(1) * 2) - 2,
					doscolor(DOS_BLACK), doscolor(DOS_BLACK), doscolor(DOS_BLACK));
				dos_box(x + (w / 2) + 1, y + 2, (w / 2) - 2, (dospf(1) * 2) - 2,
					doscolor(DOS_GRAY), doscolor(DOS_GRAY), doscolor(DOS_GRAY));
				dos_vline(x + 1, y + 2, (h / NSF_GUI_INFO_SONG_LINES) - 1, 0x002D);

				sprintf(buff, "[bck][black]");
				sprintf(buff + (strlen(buff)), "[gray]Song %s", nsf_print_number(nsf.songs.current + 1, 3, DOS_GRAY));
				sprintf(buff + (strlen(buff)), "/%s", nsf_print_number(nsf.songs.total + 1, 3, DOS_GRAY));
			} else {
				dos_box(x + 1          , y + 2, (w / 2) - 1, (dospf(1) * 2) - 2,
					doscolor(DOS_GRAY), doscolor(DOS_GRAY), doscolor(DOS_GRAY));
				dos_box(x + (w / 2) + 1, y + 2, (w / 2) - 2, (dospf(1) * 2) - 2,
					doscolor(DOS_BLACK), doscolor(DOS_BLACK), doscolor(DOS_BLACK));
				dos_vline(x + w - 2, y + 2, (h / NSF_GUI_INFO_SONG_LINES) - 1, 0x002D);

				sprintf(buff, "[bck][gray]");
				sprintf(buff + (strlen(buff)), "Song [gray]%s", nsf_print_number(nsf.songs.current + 1, 3, DOS_CYAN));
				sprintf(buff + (strlen(buff)), "[normal]/[gray]%s", nsf_print_number(nsf.songs.total + 1, 3, DOS_NORMAL));
			}

			dos_text(x + ((w - dospf(25)) / 2),
				y + ((h / NSF_GUI_INFO_SONG_LINES) * 0) + (((h / NSF_GUI_INFO_SONG_LINES) - 8) / 2), buff)

			memset(buff, 0x00, sizeof(buff));

			if (nsf.playlist.count > 0) {
				if (cfg->nsf_player_nsfe_playlist) {
					sprintf(buff, "[bck][gray][normal]Plst [gray]%s", nsf_print_number(nsf.playlist.index + 1, 3, DOS_CYAN));
					sprintf(buff + (strlen(buff)), "[normal]/[gray]%s[gray]", nsf_print_number(nsf.playlist.count, 3, DOS_NORMAL));
				} else {
					sprintf(buff, "[bck][black][gray]Plst %s", nsf_print_number(nsf.playlist.index + 1, 3, DOS_GRAY));
					sprintf(buff + (strlen(buff)), "/%s", nsf_print_number(nsf.playlist.count, 3, DOS_GRAY));
				}
			} else {
				sprintf(buff, "[bck][black][gray]Plst 000:000");
			}

			dos_text(x + ((w - dospf(25)) / 2) + dospf(13),
				y + ((h / NSF_GUI_INFO_SONG_LINES) * 0) + (((h / NSF_GUI_INFO_SONG_LINES) - 8) / 2), buff)
		} else {
			dos_box(x + 1    , y + 2, w - 2, (dospf(1) * 2) - 2,
				doscolor(DOS_GRAY), doscolor(DOS_GRAY), doscolor(DOS_GRAY));
			dos_box(x + w + 1, y + 2, w - 2, (dospf(1) * 2) - 2,
				doscolor(DOS_BLACK), doscolor(DOS_BLACK), doscolor(DOS_BLACK));

			sprintf(buff, "[bck][gray]");
			sprintf(buff + (strlen(buff)), "Song [gray]%s", nsf_print_number(nsf.songs.current + 1, 3, DOS_CYAN));
			sprintf(buff + (strlen(buff)), "[normal]/[gray]%s", nsf_print_number(nsf.songs.total + 1, 3, DOS_NORMAL));

			dos_text(x + ((w - dospf(12)) / 2),
				y + ((h / NSF_GUI_INFO_SONG_LINES) * 0) + (((h / NSF_GUI_INFO_SONG_LINES) - 8) / 2), buff)
		}
	}

	// timers della canzone
	if (nsf.state) {
		double timer = nsf.timers.song;

		if ((nsf.state & NSF_CHANGE_SONG) || (nsf.state & NSF_STOP)) {
			timer = 0;
		}

		if (nsf.info_song) {
			if (nsf.draw_mask_frames) {
				dos_vline(x + (w / 2), y + ((h / NSF_GUI_INFO_SONG_LINES) * 1) + 1, (h / NSF_GUI_INFO_SONG_LINES) - 1, 0x002D);
			}

			dos_box(x + 1, y + ((h / NSF_GUI_INFO_SONG_LINES) * 1) + 1, (w / 2) - 1,
				(dospf(1) * 2) - 1, doscolor(DOS_GRAY), doscolor(DOS_GRAY), doscolor(DOS_GRAY));

			if (nsf.current_song.time > 0) {
				dos_box(x + (w / 2) + 1, y + ((h / NSF_GUI_INFO_SONG_LINES) * 1) + 1, (w / 2) - 2,
					(dospf(1) * 2) - 1, doscolor(DOS_GRAY), doscolor(DOS_GRAY), doscolor(DOS_GRAY));
			} else {
				dos_box(x + (w / 2) + 1, y + ((h / NSF_GUI_INFO_SONG_LINES) * 1) + 1, (w / 2) - 2,
					(dospf(1) * 2) - 1, doscolor(DOS_BLACK), doscolor(DOS_BLACK), doscolor(DOS_BLACK));
				dos_vline(x + w - 2,   y + ((h / NSF_GUI_INFO_SONG_LINES) * 1) + 1, (h / NSF_GUI_INFO_SONG_LINES) - 1, 0x002D);
			}

			dos_text(x + ((w - dospf(24)) / 2),
				y + ((h / NSF_GUI_INFO_SONG_LINES) * 1) + (((h / NSF_GUI_INFO_SONG_LINES) - 8) / 2),
				"%s", nsf_print_time(timer, 0, DOS_GREEN))

			if (nsf.current_song.time > 0) {
				if (nsf.options.visual_duration) {
					timer = nsf.current_song.time;
				} else {
					timer = (nsf.current_song.time - nsf.timers.song);
				}
			} else {
				timer = 0;
			}

			if (nsf.options.visual_duration) {
				if (nsf.current_song.time < 0) {
					dos_box(x + w - 6, y + ((h / NSF_GUI_INFO_SONG_LINES) * 1) + 5, 2, 2, 0x002D, 0x002D, 0x002D);
					dos_box(x + w - 6,
						y + ((h / NSF_GUI_INFO_SONG_LINES) * 1) + (h / NSF_GUI_INFO_SONG_LINES) - 7,
						2, 2, doscolor(DOS_GRAY), doscolor(DOS_GRAY), doscolor(DOS_GRAY));
				} else {
					dos_box(x + w - 6, y + ((h / NSF_GUI_INFO_SONG_LINES) * 1) + 5, 2, 2,
						doscolor(DOS_BLACK), doscolor(DOS_BLACK), doscolor(DOS_BLACK));
					dos_box(x + w - 6,
						y + ((h / NSF_GUI_INFO_SONG_LINES) * 1) + (h / NSF_GUI_INFO_SONG_LINES) - 7,
						2, 2, doscolor(DOS_GREEN), doscolor(DOS_GREEN), doscolor(DOS_GREEN));
				}
				dos_text(x + ((w - dospf(24)) / 2) + dospf(13),
					y + ((h / NSF_GUI_INFO_SONG_LINES) * 1) + (((h / NSF_GUI_INFO_SONG_LINES) - 8) / 2), "%s",
					nsf_print_time(timer, 0, DOS_NORMAL))
			} else {
				if (nsf.current_song.time < 0) {
					dos_box(x + w - 6, y + ((h / NSF_GUI_INFO_SONG_LINES) * 1) + 5, 2, 2,
						doscolor(DOS_GRAY), doscolor(DOS_GRAY), doscolor(DOS_GRAY));
					dos_box(x + w - 6,
						y + ((h / NSF_GUI_INFO_SONG_LINES) * 1) + (h / NSF_GUI_INFO_SONG_LINES) - 7, 2, 2, 0x002D, 0x002D, 0x002D);
				} else {
					dos_box(x + w - 6, y + ((h / NSF_GUI_INFO_SONG_LINES) * 1) + 5, 2, 2,
						doscolor(DOS_GREEN), doscolor(DOS_GREEN), doscolor(DOS_GREEN));
					dos_box(x + w - 6,
						y + ((h / NSF_GUI_INFO_SONG_LINES) * 1) + (h / NSF_GUI_INFO_SONG_LINES) - 7,
						2, 2, doscolor(DOS_BLACK), doscolor(DOS_BLACK), doscolor(DOS_BLACK));
				}
				dos_text(x + ((w - dospf(24)) / 2) + dospf(13),
					y + ((h / NSF_GUI_INFO_SONG_LINES) * 1) + (((h / NSF_GUI_INFO_SONG_LINES) - 8) / 2), "%s",
					nsf_print_time(timer, 1, DOS_YELLOW))
			}
		} else {
			dos_box(x + 1, y + ((h / NSF_GUI_INFO_SONG_LINES) * 1) + 1, w - 2,
				(dospf(1) * 2) - 1, doscolor(DOS_GRAY), doscolor(DOS_GRAY), doscolor(DOS_GRAY));

			if (nsf.current_song.time > 0) {
				dos_box(x + w + 1, y + ((h / NSF_GUI_INFO_SONG_LINES) * 1) + 1, w - 2,
					(dospf(1) * 2) - 1, doscolor(DOS_GRAY), doscolor(DOS_GRAY), doscolor(DOS_GRAY));
			} else {
				dos_box(x + w + 1, y + ((h / NSF_GUI_INFO_SONG_LINES) * 1) + 1, w - 2,
					(dospf(1) * 2) - 1, doscolor(DOS_BLACK), doscolor(DOS_BLACK), doscolor(DOS_BLACK));
			}

			dos_text(x + ((w - dospf(11)) / 2),
				y + ((h / NSF_GUI_INFO_SONG_LINES) * 1) + (((h / NSF_GUI_INFO_SONG_LINES) - 8) / 2),
				"%s", nsf_print_time(timer, 0, DOS_GREEN))

			x = NSF_GUI_INFO_SONG_X;
			w = NSF_GUI_INFO_SONG_W;
		}
	}

	// titolo canzone
	if (nsf.info_song) {
		if (nsf.draw_mask_frames) {
			dos_box(x + 2, y + ((h / NSF_GUI_INFO_SONG_LINES) * 2) + 1, w - 4,
				(dospf(1) * 2) - 3, doscolor(DOS_BLACK), doscolor(DOS_BLACK), doscolor(DOS_BLACK));
			dos_vline(x + 1, y + ((h / NSF_GUI_INFO_SONG_LINES) * 2) + 1, (h / NSF_GUI_INFO_SONG_LINES) - 1, 0x002D);
			dos_vline(x + w - 2, y + ((h / NSF_GUI_INFO_SONG_LINES) * 2) + 1, (h / NSF_GUI_INFO_SONG_LINES) - 1, 0x002D);
		}

		if (nsf.current_song.track_label) {
			if (nsf.timers.song > 100) {
				if (strlen(nsf.current_song.track_label) > NSF_GUI_INFO_SONG_TITLE_ROW) {
					nsf.scroll_title_song.x = x + ((w - dospf(nsf.scroll_title_song.rows)) / 2);
					nsf.scroll_title_song.y = y + ((h / NSF_GUI_INFO_SONG_LINES) * 2) + (((h / NSF_GUI_INFO_SONG_LINES) - 8) / 2);
					nsf_text_scroll_tick(&nsf.scroll_title_song);
				} else {
					if (nsf.curtain_title_song.count > 0) {
						nsf_text_curtain(&nsf.curtain_title_song, NSF_TEXT_CURTAIN_TICK);
					}
				}
			} else {
				dos_box(x + 2, y + ((h / NSF_GUI_INFO_SONG_LINES) * 2) + 1, w - 4,
					(dospf(1) * 2) - 3, doscolor(DOS_BLACK), doscolor(DOS_BLACK), doscolor(DOS_BLACK));
			}
		}
	}

	// chips
	if (nsf.draw_mask_frames) {
		int d = ((w - (6 * dospf(2))) / 7);

		if (nsf.sound_chips.vrc6) {
			dos_text(x + 1 + (d * 1) + (dospf(2) * 0),
				y + ((h / NSF_GUI_INFO_SONG_LINES) * 3) + (((h / NSF_GUI_INFO_SONG_LINES) - 8) / 2),
				"[green][bck][gray][vrc6a][vrc6b]")
		} else {
			dos_text(x + 1 + (d * 1) + (dospf(2) * 0),
				y + ((h / NSF_GUI_INFO_SONG_LINES) * 3) + (((h / NSF_GUI_INFO_SONG_LINES) - 8) / 2),
				"[gray][bck][black][vrc6a][vrc6b]")
		}
		if (nsf.sound_chips.vrc7) {
			dos_text(x + 1 + (d * 2) + (dospf(2) * 1),
				y + ((h / NSF_GUI_INFO_SONG_LINES) * 3) + (((h / NSF_GUI_INFO_SONG_LINES) - 8) / 2),
				"[green][bck][gray][vrc7a][vrc7b]")
		} else {
			dos_text(x + 1 + (d * 2) + (dospf(2) * 1),
				y + ((h / NSF_GUI_INFO_SONG_LINES) * 3) + (((h / NSF_GUI_INFO_SONG_LINES) - 8) / 2),
				"[gray][bck][black][vrc7a][vrc7b]")
		}
		if (nsf.sound_chips.mmc5) {
			dos_text(x + 1 + (d * 3) + (dospf(2) * 2),
				y + ((h / NSF_GUI_INFO_SONG_LINES) * 3) + (((h / NSF_GUI_INFO_SONG_LINES) - 8) / 2),
				"[green][bck][gray][mmc5a][mmc5b]")
		} else {
			dos_text(x + 1 + (d * 3) + (dospf(2) * 2),
				y + ((h / NSF_GUI_INFO_SONG_LINES) * 3) + (((h / NSF_GUI_INFO_SONG_LINES) - 8) / 2),
				"[gray][bck][black][mmc5a][mmc5b]")
		}
		if (nsf.sound_chips.namco163) {
			dos_text(x + 1 + (d * 4) + (dospf(2) * 3),
				y + ((h / NSF_GUI_INFO_SONG_LINES) * 3) + (((h / NSF_GUI_INFO_SONG_LINES) - 8) / 2),
				"[green][bck][gray][n163a][n163b]")
		} else {
			dos_text(x + 1 + (d * 4) + (dospf(2) * 3),
				y + ((h / NSF_GUI_INFO_SONG_LINES) * 3) + (((h / NSF_GUI_INFO_SONG_LINES) - 8) / 2),
				"[gray][bck][black][n163a][n163b]")
		}
		if (nsf.sound_chips.sunsoft5b) {
			dos_text(x + 1 + (d * 5) + (dospf(2) * 4),
				y + ((h / NSF_GUI_INFO_SONG_LINES) * 3) + (((h / NSF_GUI_INFO_SONG_LINES) - 8) / 2),
				"[green][bck][gray][s5b1][s5b2]")
		} else {
			dos_text(x + 1 + (d * 5) + (dospf(2) * 4),
				y + ((h / NSF_GUI_INFO_SONG_LINES) * 3) + (((h / NSF_GUI_INFO_SONG_LINES) - 8) / 2),
				"[gray][bck][black][s5b1][s5b2]")
		}
		if (nsf.sound_chips.fds) {
			dos_text(x + 1 + (d * 6) + (dospf(2) * 5),
				y + ((h / NSF_GUI_INFO_SONG_LINES) * 3) + (((h / NSF_GUI_INFO_SONG_LINES) - 8) / 2),
				"[green][bck][black][fds1][fds2]")
		} else {
			dos_text(x + 1 + (d * 6) + (dospf(2) * 5),
				y + ((h / NSF_GUI_INFO_SONG_LINES) * 3) + (((h / NSF_GUI_INFO_SONG_LINES) - 8) / 2),
				"[gray][bck][black][fds1][fds2]")
		}
	}

	// opzioni
	{
		if (nsf.draw_mask_frames) {
			dos_hline(0, NSF_GUI_EFFECT_Y - dospf(7), SCR_COLUMNS, doscolor(DOS_BROWN));
			dos_text(DOS_CENTER, NSF_GUI_EFFECT_Y - dospf(7) - 4, " [red]Options ")
		}

		nsf_print_option(&nsf_options_data[NSF_OPTION_PLAYLIST], (info.format == NSFE_FORMAT), cfg->nsf_player_nsfe_playlist);
		nsf_print_option(&nsf_options_data[NSF_OPTION_FADEOUT], (info.format == NSFE_FORMAT), cfg->nsf_player_nsfe_fadeout);
		nsf_print_option(&nsf_options_data[NSF_OPTION_REVERSE_BITS_DPCM], TRUE, cfg->reverse_bits_dpcm);
	}

	// info
	{
		if (nsf.draw_mask_frames) {
			dos_hline(0, NSF_GUI_EFFECT_Y - dospf(3), SCR_COLUMNS, doscolor(DOS_BROWN));
			dos_text(DOS_CENTER, NSF_GUI_EFFECT_Y - dospf(3) - 4, " [red]Info ")
		}

		nsf_text_curtain(&nsf.curtain_info, NSF_TEXT_CURTAIN_TICK);
	}

	// indicatore effetto
	{
		w = (SCR_COLUMNS - (NSF_EFFECTS * (NSF_GUI_EFFECT_BOOKMARK_W + 2))) / 2;

		if (nsf.draw_mask_frames) {
			dos_hline(0, NSF_GUI_EFFECT_Y - 1, w - 1, doscolor(DOS_BROWN));
			dos_hline(w + (NSF_EFFECTS * (NSF_GUI_EFFECT_BOOKMARK_W + 2)) + 1, NSF_GUI_EFFECT_Y - 1, w, doscolor(DOS_BROWN));
		}

		for (i = 0; i < NSF_EFFECTS; i++) {
			if (i == cfg->nsf_player_effect) {
				dos_hline(w + (i * (NSF_GUI_EFFECT_BOOKMARK_W + 2)) + 1, NSF_GUI_EFFECT_Y - 1,
					NSF_GUI_EFFECT_BOOKMARK_W, doscolor(DOS_GREEN));
			} else {
				dos_hline(w + (i * (NSF_GUI_EFFECT_BOOKMARK_W + 2)) + 1, NSF_GUI_EFFECT_Y - 1,
					NSF_GUI_EFFECT_BOOKMARK_W, doscolor(DOS_GRAY));
			}
		}
	}

	if (nsf.draw_mask_frames) {
		nsf.draw_mask_frames--;
	}
}
static char *nsf_print_color(int color) {
	static char cbuff[10];

	switch (color) {
		default:
		case DOS_NORMAL:
			sprintf(cbuff, "[normal]");
			break;
		case DOS_RED:
			sprintf(cbuff, "[red]");
			break;
		case DOS_YELLOW:
			sprintf(cbuff, "[yellow]");
			break;
		case DOS_GREEN:
			sprintf(cbuff, "[green]");
			break;
		case DOS_CYAN:
			sprintf(cbuff, "[cyan]");
			break;
		case DOS_BROWN:
			sprintf(cbuff, "[brown]");
			break;
		case DOS_BLUE:
			sprintf(cbuff, "[blue]");
			break;
		case DOS_GRAY:
			sprintf(cbuff, "[gray]");
			break;
		case DOS_BLACK:
			sprintf(cbuff, "[black]");
			break;
	}

	return (cbuff);
}
static char *nsf_print_number(unsigned int song, BYTE decimal, int color) {
	static char cbuff[30];
	BYTE ibuff[3], i = 0, is_normal = FALSE;

	ibuff[0] =  song / 100;
	ibuff[1] = (song - (ibuff[0] * 100)) / 10;
	ibuff[2] =  song - (ibuff[0] * 100) - (ibuff[1] * 10);

	if (decimal > 3) {
		decimal = 3;
	}

	memset(cbuff, 0x00, sizeof(cbuff));

	for (i = 3 - decimal; i < 3; i++) {
		if (!is_normal && (ibuff[i] > 0)) {
			is_normal = TRUE;
			sprintf(cbuff + strlen(cbuff), "%s", nsf_print_color(color));
		}
		sprintf(cbuff + strlen(cbuff), "%d", ibuff[i]);
	}

	return (cbuff);
}
static char *nsf_print_time(double timer, BYTE mode, int color) {
	BYTE is_normal = FALSE;
	static char buff[300];
	unsigned int i = 0;
	int tmp = 0;

	for (i = 0; i < 4; i++) {
		BYTE is_last = FALSE;

		switch (i) {
			case 0:
				tmp = ((int)timer / (1000 * 60 * 60)) % 24;
				if (tmp > 0) {
					is_normal = TRUE;
				}
				sprintf(buff, "[bck][black][gray]%s[gray]:", nsf_print_number(tmp, 2, color));
				continue;
			case 1:
				tmp = ((int)timer / (1000 * 60)) % 60;
				break;
			case 2:
				tmp = (int)(timer / 1000) % 60 ;
				break;
			case 3:
				tmp = ((int)timer / 10) % 100;
				is_last = TRUE;
				break;
			default:
				continue;
		}
		if (is_normal) {
			sprintf(buff + strlen(buff), "%s%02d", nsf_print_color(color), tmp);
		} else {
			if (tmp > 0) {
				is_normal = TRUE;
			}
			sprintf(buff + strlen(buff), "[gray]%s", nsf_print_number(tmp, 2, color));
		}
		if (!is_last) {
			sprintf(buff + strlen(buff), "[gray]:");
		}
	}

	if (mode == 1) {
		static char negative[sizeof(buff)];
		char chtmp[sizeof(negative) + 1];
		unsigned int index_zero = 0;

		for (i = 0; i < strlen(buff); i++) {
			if ((buff[i] < '1') || (buff[i] > '9')) {
				if ((buff[i] == '0') || (buff[i] == ':')) {
					index_zero = i;
				}
				continue;
			}
			snprintf(chtmp, sizeof(chtmp), "%s-%s", nsf_print_color(color), &buff[index_zero + 1]);
			if ((index_zero + strlen(chtmp)) >= sizeof(negative)) {
				return (buff);
			}
			buff[index_zero] = 0;
			strncpy(negative, buff, sizeof(negative));
			strncpy(negative + index_zero, chtmp, sizeof(negative) - index_zero);
			return (negative);
		}
	}

	return (buff);
}
static void nsf_print_option(_nsf_option_data *option, BYTE active, BYTE mode) {
	int xbox = 0, xtext = 0;

	if (option->x1 < 0) {
		if (option->direction) {
			option->x1 = option->x - NSF_GUI_OPTIONS_BOX_SPACE - NSF_GUI_OPTIONS_BOX_SIDE - dospf(dos_strlen(option->txt));
			option->x2 = option->x;
		} else {
			option->x1 = option->x;
			option->x2 = option->x + NSF_GUI_OPTIONS_BOX_SIDE + NSF_GUI_OPTIONS_BOX_SPACE + dospf(dos_strlen(option->txt));
		}
	}

	if (option->direction) {
		xbox = dospf(dos_strlen(option->txt)) + NSF_GUI_OPTIONS_BOX_SPACE;
		xtext = 0;
	} else {
		xbox = 0;
		xtext = NSF_GUI_OPTIONS_BOX_SIDE + NSF_GUI_OPTIONS_BOX_SPACE;
	}

	dos_box(option->x1 + xbox, option->y, NSF_GUI_OPTIONS_BOX_SIDE, NSF_GUI_OPTIONS_BOX_SIDE,
		0x002D,  0x002D, doscolor(DOS_BLACK));

	if (active) {
		if (mode) {
			dos_text(option->x1 + xbox + 1, option->y + 1, "[bck][green] ")
		} else {
			dos_text(option->x1 + xbox + 1, option->y + 1, "[bck][black] ")
		}
		dos_text(option->x1 + xtext, option->y + 1, "[normal]%s", option->txt)
	} else {
		if (mode) {
			dos_text(option->x1 + xbox + 1, option->y + 1, "[bck][gray] ")
		} else {
			dos_text(option->x1 + xbox + 1, option->y + 1, "[bck][black] ")
		}
		dos_text(option->x1 + xtext, option->y + 1, "[gray]%s", option->txt)
	}
}
static void nsf_text_scroll_set_buffer(_nsf_text_scroll *scroll, const char *fmt, ...) {
	va_list ap;
	size_t len = 0;

	memset(scroll->buffer, 0x20, sizeof(scroll->buffer));
	scroll->buffer[sizeof(scroll->buffer) - 1] = 0x00;

	va_start(ap, fmt);
	vsnprintf(scroll->buffer + scroll->rows, sizeof(scroll->buffer) - scroll->rows, fmt, ap);
	va_end(ap);

	scroll->pixel_len = dos_strlen(scroll->buffer) * 8;
	len = strlen(scroll->buffer);
	scroll->buffer[len] = 0x20;
	scroll->buffer[len + scroll->rows - 1] = 0x00;
}
static void nsf_text_scroll_tick(_nsf_text_scroll *scroll) {
	if ((dos_strlen(scroll->buffer) > 0) && (scroll->timer <= 0)) {
		int margin = 0, start = 0, row = 0, tmp = 0, tag = 0;
		int count = 0, bindex = 0, sindex = 0;

		scroll->timer = scroll->reload;

		memset(scroll->string, 0x00, sizeof(scroll->string));
		scroll->pixel = scroll->pixel + scroll->velocity;

		if (scroll->pixel >= scroll->pixel_len) {
			scroll->pixel -= scroll->pixel_len;
		}

		start = scroll->pixel / 8;
		margin = scroll->pixel % 8;

		for (row = 0; row < (scroll->rows + (margin > 0));) {
			if ((scroll->buffer[bindex] == '[') && ((tmp = dos_is_tag(scroll->buffer + bindex, &tag)) > 0)) {
				strncat(scroll->string, scroll->buffer + bindex, tmp);
				bindex += tmp;
				sindex += tmp;
				continue;
			}
			if (++count <= start) {
				bindex++;
				continue;
			}
			scroll->string[sindex] = scroll->buffer[bindex];
			bindex++;
			sindex++;
			row++;
		}

		_dos_text(scroll->x, scroll->y, margin, (margin ? 8 - margin : -1), -1, -1, scroll->string);
	} else {
		scroll->timer -= nsf.timers.diff;
	}
}
static void nsf_text_curtain(_nsf_text_curtain *curtain, BYTE mode) {
	if (mode == NSF_TEXT_CURTAIN_INIT) {
		nsf_text_curtain(curtain, NSF_TEXT_CURTAIN_QUIT);
		curtain->count = 0;
		curtain->index = 0;
		curtain->pause = FALSE;
		curtain->timer = curtain->reload.r1;
		curtain->borders.bottom = 0;
	} else if (mode == NSF_TEXT_CURTAIN_TICK) {
		if (curtain->redraw.all) {
			curtain->redraw.all = FALSE;

			dos_box(curtain->x, curtain->y, curtain->rows * dospf(1), dospf(1),
				doscolor(DOS_BLACK), doscolor(DOS_BLACK), doscolor(DOS_BLACK));

			_dos_text(curtain->x, curtain->y, -1, -1, curtain->redraw.bottom, -1, curtain->line[curtain->index].text);
		}
		if (curtain->timer <= 0) {
			if (!curtain->pause) {
				dos_hline(curtain->x, curtain->y + curtain->borders.bottom, curtain->rows * dospf(1), doscolor(DOS_BLACK));

				_dos_text(curtain->x, curtain->y, -1, -1, curtain->borders.bottom, -1, curtain->line[curtain->index].text);

				curtain->borders.bottom++;

				if (curtain->borders.bottom == dospf(1)) {
					curtain->borders.bottom = 0;
					curtain->timer = curtain->reload.r2;
					curtain->pause = TRUE;
				} else {
					curtain->timer = curtain->reload.r1;
				}
			} else {
				curtain->pause = FALSE;
				curtain->timer = curtain->reload.r1;
				if (++curtain->index == curtain->count) {
					curtain->index = 0;
				}
			}
		} else {
			curtain->timer -= nsf.timers.diff;
		}
	} else if (mode == NSF_TEXT_CURTAIN_QUIT) {
		if (curtain->line) {
			int i = 0;

			for (i = 0; i < curtain->count; i++) {
				if (curtain->line[i].text) {
					free(curtain->line[i].text);
				}
			}
			free(curtain->line);
			curtain->line = NULL;
		}
	}
}
static void nsf_text_curtain_add_line(_nsf_text_curtain *curtain, const char *fmt, ...) {
	va_list ap;
	char buffer[1024], *start = NULL;
	int length = 0, count = 0, index = 0, tmp = 0, tag = 0;
	unsigned int i = 0;
	_nsf_text_curtain_line *line = NULL;

	va_start(ap, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, ap);
	va_end(ap);

	count = (dos_strlen(buffer) / curtain->rows) + 1;
	start = buffer;
	line = realloc(curtain->line, (curtain->count + count) * sizeof(_nsf_text_curtain_line));
	if (line) {
		curtain->line = line;

		for (index = curtain->count; index < (curtain->count + count); index++) {
			length = 0;
			curtain->line[index].length = 0;
			for (; i < strlen(buffer); i++) {
				if (length == curtain->rows) {
					break;
				}
				if ((buffer[i] == '[') && ((tmp = dos_is_tag(buffer + i, &tag)) > 0)) {
					i += (tmp - 1);
					curtain->line[index].length += tmp;
					if (tag > DOS_BACKGROUND_COLOR) {
						length++;
					}
					continue;
				}
				length++;
				curtain->line[index].length++;
			}
			curtain->line[index].text = malloc(curtain->line[index].length + 1);
			memset(curtain->line[index].text, 0x00, curtain->line[index].length + 1);
			strncpy(curtain->line[index].text, start, curtain->line[index].length);
			start += curtain->line[index].length;
		}
		curtain->count += count;
	}
}
static void nsf_reset_song_variables(void) {
	nsf.timers.song = 0;
	nsf.timers.silence = 0;
	nsf.timers.fadeout = 0;

	if (nsf.info_song) {
		int h = NSF_GUI_INFO_SONG_H;
		int y = NSF_GUI_INFO_SONG_Y;

		nsf.scroll_title_song.timer = 0;
		nsf.scroll_title_song.pixel = 0;

		nsf.curtain_title_song.reload.r1 = 50;
		nsf.curtain_title_song.reload.r2 = 3000;
		nsf.curtain_title_song.x = DOS_CENTER;
		nsf.curtain_title_song.y = y + ((h / NSF_GUI_INFO_SONG_LINES) * 2) + (((h / NSF_GUI_INFO_SONG_LINES) - 8) / 2);
		nsf.curtain_title_song.borders.left = -1;
		nsf.curtain_title_song.borders.right = -1;
		nsf.curtain_title_song.borders.bottom = 0;
		nsf.curtain_title_song.borders.top = -1;
		nsf.curtain_title_song.rows = NSF_GUI_INFO_SONG_TITLE_ROW;
	} else {
		nsf.current_song.time = -1;
		nsf.current_song.fade = -1;
	}

	nsf_reset_song_title();
}
