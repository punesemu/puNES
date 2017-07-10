/*  Copyright (C) 2010-2017 Fabio Cavallo (aka FHorse)
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
#include "nsf.h"
#include "mem_map.h"
#include "mappers.h"
#include "mappers/mapper_VRC7_snd.h"
#include "emu.h"
#include "conf.h"
#include "clock.h"
#include "info.h"
#include "vs_system.h"
#include "ppu.h"
#include "apu.h"
#include "cpu.h"
#include "fds.h"
#include "draw_on_screen.h"
#include "conf.h"
#include "gui.h"
#include "audio/blipbuf.h"

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
enum nsf_mode {
	NSF_NTSC_MODE,
	NSF_PAL_MODE,
	NSF_DUAL_MODE
};
enum nsf_timining {
	NFS_TIME_CHANGE_SONG = 250,
	NFS_TIME_START_FOUT = 500,
	NFS_TIME_TICK_FOUT = 15,
	NFS_TIME_AUTO_NEXT_SONG = 5000,
};
enum nsf_controls {
	NSF_CTRL_X = dospf(1) + 2,
	NSF_CTRL_Y = dospf(11) + 8,
	NSF_CTRL_WC = 22,
	NSF_CTRL_W = (NSF_CTRL_WC * 5) + 2,
	NSF_CTRL_LINES = 4,
	NSF_CTRL_H = dospf(NSF_CTRL_LINES) * 2,
};
enum nsf_header { NSF_HEADER_LENGTH = 128 };

extern void cpu_wr_mem(WORD address, BYTE value);
static void INLINE nsf_draw_controls(void);

static const BYTE nsf_routine[17] = {
//	0     1
	0xA9, 0x00,       // 0x2500 : LDA [current song]
//	2     3
	0xA2, 0x00,       // 0x2502 : LDX [PAL or NTSC]
//	4     5     6
	0x20, 0x00, 0x00, // 0x2504 : JSR [address init routine]
//	7
	0x78,             // 0x2507 : SEI
//	8     9     10
	0x4C, 0x0E, 0x25, // 0x2508 : JMP 0x250E
//	11    12    13
	0x20, 0x00, 0x00, // 0x250B : JSR [address play routine]
//	14    15    16
	0x4C, 0x00, 0x25  // 0x250E : JMP [0x250B / 0x250E]
};

void nsf_init(void) {
	memset(&nsf, 0x00, sizeof(nsf));
}
void nsf_quit(void) {
	nsf_init();
}
BYTE nsf_load_rom(void) {
	FILE *fp;

	{
		BYTE i, found = TRUE;
		static const uTCHAR rom_ext[2][10] = { uL(".nsf\0"), uL(".NSF\0") };

		fp = ufopen(info.rom_file, uL("rb"));

		if (!fp) {
			found = FALSE;

			for (i = 0; i < LENGTH(rom_ext); i++) {
				uTCHAR rom_file[LENGTH_FILE_NAME_MID];

				ustrncpy(rom_file, info.rom_file, usizeof(rom_file));
				ustrcat(rom_file, rom_ext[i]);

				fp = ufopen(rom_file, uL("rb"));

				if (fp) {
					ustrncpy(info.rom_file, rom_file, usizeof(info.rom_file));
					found = TRUE;
					break;
				}
			}
		}

		if (!found) {
			return (EXIT_ERROR);
		}
	}

	if ((fgetc(fp) == 'N') && (fgetc(fp) == 'E') && (fgetc(fp) == 'S') && (fgetc(fp) == 'M')
			&& (fgetc(fp) == '\32')) {
		BYTE tmp, flags[TOTAL_FL];
		long len;

		info.machine[DATABASE] = DEFAULT;
		info.prg.ram.bat.banks = 0;
		info.prg.ram.banks_8k_plus = 0;

		fseek(fp, 0, SEEK_END);
		len = ftell(fp);

		if (len < NSF_HEADER_LENGTH) {
			fclose(fp);
			return (EXIT_ERROR);
		}

		len -= NSF_HEADER_LENGTH;

		fseek(fp, 5, SEEK_SET);

		if (fread(&flags[0], sizeof(flags), 1, fp)) {
			nsf.version = flags[VERSION];
			nsf.song.total = flags[TOTAL_SONGS];
			nsf.song.starting = flags[STARTING_SONGS];
			nsf.adr.load = (flags[LOAD_ADR_HI] << 8) | flags[LOAD_ADR_LO];
			nsf.adr.init = (flags[INIT_ADR_HI] << 8) | flags[INIT_ADR_LO];
			nsf.adr.play = (flags[PLAY_ADR_HI] << 8) | flags[PLAY_ADR_LO];
		}

		if ((nsf.song.total == 0) || (nsf.adr.load < 0x6000) || (nsf.adr.init < 0x6000)
				|| (nsf.adr.play < 0x6000)) {
			fclose(fp);
			return (EXIT_ERROR);
		}

		if (nsf.song.starting > 0) {
			if (nsf.song.starting > nsf.song.total) {
				nsf.song.starting = 1;
			}
		} else {
			nsf.song.starting = 1;
		}

		nsf.song.total--;

		if (fread(&nsf.info.name, sizeof(nsf.info.name), 1, fp)) {
			nsf.info.name[sizeof(nsf.info.name) - 1] = 0;
		}
		if (fread(&nsf.info.artist, sizeof(nsf.info.artist), 1, fp)) {
			nsf.info.artist[sizeof(nsf.info.artist) - 1] = 0;
		}
		if (fread(&nsf.info.copyright, sizeof(nsf.info.copyright), 1, fp)) {
			nsf.info.copyright[sizeof(nsf.info.copyright) - 1] = 0;
		}

		nsf.play_speed.ntsc = fgetc(fp) | (fgetc(fp) << 8);

		if (fread(&nsf.bankswitch.banks, sizeof(nsf.bankswitch.banks), 1, fp)) {
			if (nsf.bankswitch.banks[0] | nsf.bankswitch.banks[1] | nsf.bankswitch.banks[2] |
				nsf.bankswitch.banks[3] | nsf.bankswitch.banks[4] | nsf.bankswitch.banks[5] |
				nsf.bankswitch.banks[6] | nsf.bankswitch.banks[7]) {
				nsf.bankswitch.enabled = TRUE;
			}
		}

		nsf.play_speed.pal = fgetc(fp) | (fgetc(fp) << 8);

		nsf.type = fgetc(fp) & 0x03;

		tmp = fgetc(fp);
		nsf.sound_chips.vrc6 = tmp & 0x01;
		nsf.sound_chips.vrc7 = tmp & 0x02;
		nsf.sound_chips.fds = tmp & 0x04;
		nsf.sound_chips.mmc5 = tmp & 0x08;
		nsf.sound_chips.namco163 = tmp & 0x10;
		nsf.sound_chips.sunsoft5b = tmp & 0x20;

		if (!nsf.sound_chips.fds && (nsf.adr.load < 0x8000)) {
			fclose(fp);
			return (EXIT_ERROR);
		}

		fseek(fp, 4, SEEK_CUR);

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
				fclose(fp);
				return (EXIT_ERROR);
			}
		}

		{
			int padding = nsf.adr.load & 0x0FFF;
			int len_4k;

			nsf.prg.banks_4k = ((len + padding) / 0x1000);

			if (((len + padding) % 0x1000)) {
				nsf.prg.banks_4k++;
			}

			len_4k = nsf.prg.banks_4k * 0x1000;

			if (map_prg_chip_malloc(0, len_4k, 0x00) == EXIT_ERROR) {
				fclose(fp);
				return (EXIT_ERROR);
			}

			memset(prg_chip(0), 0xF2, len_4k);

			if (!(fread(prg_chip(0) + padding, len, 1, fp))) {
				fclose(fp);
				return (EXIT_ERROR);
			}

			nsf.prg.banks_4k--;
		}

		nsf.enabled = TRUE;

		nsf.song.current = nsf.song.starting - 1;

		memcpy(nsf.routine.prg, nsf_routine, 17);

		nsf.routine.prg[NSF_R_JMP_PLAY] = NSF_R_LOOP;

		info.mapper.id = NSF_MAPPER;
		info.cpu_rw_extern = TRUE;

		for (tmp = 0; tmp <  LENGTH(nsf.timers.buttons); tmp++) {
			nsf.timers.buttons[tmp] = gui_get_ms();
		}

		fclose(fp);
	}

	return (EXIT_OK);
}
void nsf_init_tune(void) {
	WORD i;

	nsf.made_tick = FALSE;

	cpu.SP = 0xFD;
	memset(mmcpu.ram, 0x00, sizeof(mmcpu.ram));

	memset(prg.ram.data, 0x00, prg.ram.size);

	if (nsf.sound_chips.vrc6) {
		map_init_NSF_VRC6(VRC6A);
	}
	if (nsf.sound_chips.vrc7) {
		map_init_NSF_VRC7(VRC7A);
	}
	if (nsf.sound_chips.fds) {
		map_init_NSF_FDS();
	}
	if (nsf.sound_chips.mmc5) {
		map_init_NSF_MMC5();
	}
	if (nsf.sound_chips.namco163) {
		map_init_NSF_Namco(N163);
	}
	if (nsf.sound_chips.sunsoft5b) {
		map_init_NSF_Sunsoft(FM7);
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

	nsf.routine.prg[NSF_R_SONG] = nsf.song.current;
	nsf.routine.prg[NSF_R_TYPE] = nsf.type;

	nsf.routine.prg[NSF_R_INIT_LO] = nsf.adr.init & 0xFF;
	nsf.routine.prg[NSF_R_INIT_HI] = nsf.adr.init >> 8;

	nsf.routine.prg[NSF_R_PLAY_LO] = nsf.adr.play & 0xFF;
	nsf.routine.prg[NSF_R_PLAY_HI] = nsf.adr.play >> 8;

	nsf.made_tick = TRUE;
	nsf.state = NSF_PLAY;

	nsf.song.started = FALSE;

	nsf.timers.song = 0;
	nsf.timers.silence = 0;

	nsf_reset_timers();
}
void nsf_tick(WORD cycles_cpu) {
	if (nsf.rate.count && (--nsf.rate.count == 0)) {
		nsf.frames++;
		nsf.rate.count = nsf.rate.reload;
		nsf.routine.prg[NSF_R_JMP_PLAY] = NSF_R_PLAY;
		if (nsf.routine.INT_NMI) {
			nmi.high = TRUE;
		}
		ppu.odd_frame = !ppu.odd_frame;
		info.execute_cpu = FALSE;

		{
			double now = gui_get_ms();
			SWORD *buffer = NULL;
			BYTE silence = TRUE;
			int i, len;

			// timer
			nsf.timers.song += (now - nsf.timers.last_tick);
			nsf.timers.last_tick = now;

			// timer del silenzio
			len = audio_buffer_blipbuf(&buffer);

			for (i = 0; i < len; i++) {
				if (buffer[i]) {
					silence = FALSE;
					break;
				}
			}
			if ((silence == TRUE) && (nsf.song.started == TRUE)) {
				nsf.timers.silence += (now - nsf.timers.last_silence);
			} else {
				nsf.song.started = TRUE;
				nsf.timers.silence = 0;
			}
			nsf.timers.last_silence = now;;
		}

		nsf_main_screen_event();
		nsf_effect();
	}
}
void nsf_reset_prg(void) {
	DBWORD i;

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
		BYTE value, bank = 0;

		nsf.bankswitch.enabled = TRUE;

		if (nsf.sound_chips.fds) {
			i = 0x6000;
		} else {
			i = 0x8000;
		}

		for (; i < 0x10000; i += 0x1000) {
			value = bank;
			control_bank(nsf.prg.banks_4k);
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
	nsf.timers.last_silence = nsf.timers.last_tick;
}

void nsf_main_screen(void) {
	WORD x, y;

	for (y = 0; y < SCR_LINES; y++) {
		for (x = 0; x < SCR_ROWS; x++) {
			screen.line[y][x] = doscolor(DOS_BLACK);
		}
	}

	dos_text(DOS_CENTER, dospf(1), "[yellow]p[red]u[green]N[cyan]E[brown]S[normal] [bck][yellow][blue]NSF[normal][bck][black] Player");

	dos_hline(dospf(0), dospf(3), SCR_ROWS, doscolor(DOS_BROWN));

	dos_text(DOS_CENTER, dospf(4)    , "[cyan]%s", nsf.info.name);
	dos_text(DOS_CENTER, dospf(6)    , "[yellow]%s", nsf.info.artist);
	dos_text(DOS_CENTER, dospf(7) + 3, "%s", nsf.info.copyright);

	dos_vline(dospf(16) + 4,  dospf(9), dospf(12)  , doscolor(DOS_GRAY));
	dos_hline(dospf(0), dospf(9), SCR_ROWS, doscolor(DOS_BROWN));
	dos_hline(dospf(0), dospf(21), SCR_ROWS, doscolor(DOS_GRAY));

	// box
	dos_text(32, dospf(10), "[red]Controls");
	nsf_draw_controls();

	dos_text(dospf(17), dospf(10), "    [red]Buttons");
	dos_text(dospf(17) + 4, dospf(12) +  0, "  [right]   :   [brown][next]");
	dos_text(dospf(17) + 4, dospf(13) +  4, "  [left]   :   [brown][prev]");
	dos_text(dospf(17) + 4, dospf(14) +  8, "  [a]   :   [red][stop]");
	dos_text(dospf(17) + 4, dospf(15) + 12, " [start1][start2][start3]  : [green][play][normal] / [yellow][pause]");
	dos_text(dospf(17) + 3, dospf(16) + 22, " or use the [lmouse]");

	nsf_main_screen_event();
}
void nsf_main_screen_event(void) {
	if (nsf.timers.silence > NFS_TIME_AUTO_NEXT_SONG) {
		nsf.song.started = FALSE;
		nsf.song.current++;
		if (nsf.song.current > nsf.song.total) {
			nsf.song.current = 0;
		}
		nsf.routine.INT_NMI = 2;
		nsf.state |= NSF_CHANGE_SONG;
	}

	// gestione eventi
	if (port[PORT1].data[START] == PRESSED) {
		switch (nsf.state & 0x07) {
			case NSF_STOP:
				nsf.state = NSF_PLAY | NSF_CHANGE_SONG;
				nsf.routine.INT_NMI = 2;
				break;
			case NSF_PAUSE:
				nsf.state = (nsf.state & NSF_CHANGE_SONG) | NSF_PLAY;
				nsf.timers.last_tick = gui_get_ms();
				break;
			case NSF_PLAY:
				nsf.state = NSF_PAUSE;
				break;
		}
		port[PORT1].data[START] = RELEASED;
	} else if (port[PORT1].data[BUT_A] == PRESSED) {
		nsf.state = NSF_STOP;
		port[PORT1].data[BUT_A] = RELEASED;
	} else if (port[PORT1].data[LEFT] == PRESSED) {
		if (nsf.timers.buttons[LEFT] == 0) {
			nsf.timers.buttons[LEFT] = gui_get_ms();
		} else if ((gui_get_ms() - nsf.timers.buttons[LEFT]) > NFS_TIME_CHANGE_SONG) {
			nsf.song.current--;
			if (nsf.song.current > nsf.song.total) {
				nsf.song.current = nsf.song.total;
			}
			nsf.routine.INT_NMI = 2;
			nsf.state |= NSF_CHANGE_SONG;
			nsf.timers.buttons[LEFT] = 0;
		}
	} else if (port[PORT1].data[RIGHT] == PRESSED) {
		if (nsf.timers.buttons[RIGHT] == 0) {
			nsf.timers.buttons[RIGHT] = gui_get_ms();
		} else if ((gui_get_ms() - nsf.timers.buttons[RIGHT]) > NFS_TIME_CHANGE_SONG) {
			nsf.song.current++;
			if (nsf.song.current > nsf.song.total) {
				nsf.song.current = 0;
			}
			nsf.routine.INT_NMI = 2;
			nsf.state |= NSF_CHANGE_SONG;
			nsf.timers.buttons[RIGHT] = 0;
		}
	}

	// ridisegno i controlli
	nsf_draw_controls();
}

int nsf_controls_mouse_in_buttons(int x_mouse, int y_mouse) {
	int x = NSF_CTRL_X, y = NSF_CTRL_Y;
	int wc = NSF_CTRL_WC, h = NSF_CTRL_H;

#define nsf_c_x1(ctr) (x + 2 + (wc * ctr))
#define nsf_c_x2(ctr) (nsf_c_x1(ctr) + (wc - 1))
#define nsf_c_y1(ctr) (y + 2)
#define nsf_c_y2(ctr) (nsf_c_y1(ctr) + ((h / NSF_CTRL_LINES) - 2 - 1))

	if (((x_mouse >= nsf_c_x1(0)) && (x_mouse <= nsf_c_x2(0))) &&
		((y_mouse >= nsf_c_y1(0)) && (y_mouse <= nsf_c_y2(0)))) {
		return (NSF_PREV);
	}
	if (((x_mouse >= nsf_c_x1(1)) && (x_mouse <= nsf_c_x2(1))) &&
		((y_mouse >= nsf_c_y1(1)) && (y_mouse <= nsf_c_y2(1)))) {
		return (NSF_PLAY);
	}
	if (((x_mouse >= nsf_c_x1(2)) && (x_mouse <= nsf_c_x2(2))) &&
		((y_mouse >= nsf_c_y1(2)) && (y_mouse <= nsf_c_y2(2)))) {
		return (NSF_PAUSE);
	}
	if (((x_mouse >= nsf_c_x1(3)) && (x_mouse <= nsf_c_x2(3))) &&
		((y_mouse >= nsf_c_y1(3)) && (y_mouse <= nsf_c_y2(3)))) {
		return (NSF_STOP);
	}
	if (((x_mouse >= nsf_c_x1(4)) && (x_mouse <= nsf_c_x2(4))) &&
		((y_mouse >= nsf_c_y1(4)) && (y_mouse <= nsf_c_y2(4)))) {
		return (NSF_NEXT);
	}

	return (0);

#undef nsf_c_x1
#undef nsf_c_x2
#undef nsf_c_y1
#undef nsf_c_y2
}

void nsf_effect(void) {
	int x, y, len, amp = (40 * cfg->apu.channel[APU_MASTER]) * cfg->apu.volume[APU_MASTER];
	SWORD *buffer = NULL;

	len = audio_buffer_blipbuf(&buffer);

	for (y = 169; y < SCR_LINES; y++) {
		for (x = 0; x < SCR_ROWS; x++) {
			screen.line[y][x] = doscolor(DOS_BLACK);
		}
	}

	for (x = 0; x < SCR_ROWS; x++) {
		if (!buffer || !len) {
			y = 205;
		} else {
			y = 205 + ((buffer[(x * len) / 256] * amp) / 16384);
		}

		if ((y < SCR_LINES) && (y > 169)) {
			screen.line[y][x] = doscolor(DOS_GREEN);
		}
	}
}

static void INLINE nsf_draw_controls(void) {
	int x = NSF_CTRL_X, y = NSF_CTRL_Y;
	int wc = NSF_CTRL_WC, w = NSF_CTRL_W, h = NSF_CTRL_H;

	dos_box(x, y, w, h, doscolor(DOS_NORMAL), doscolor(DOS_NORMAL), doscolor(DOS_GRAY));
	dos_hline(x + 1           , y + ((h / NSF_CTRL_LINES) * 1) - 1, w - 2, doscolor(DOS_NORMAL));
	dos_hline(x + 1           , y + ((h / NSF_CTRL_LINES) * 2) - 1, w - 2, doscolor(DOS_NORMAL));
	dos_hline(x + 1           , y + ((h / NSF_CTRL_LINES) * 3) - 1, w - 2, doscolor(DOS_NORMAL));
	dos_hline(x + 1           , y + 1          , w - 2      , 0x002D);
	dos_vline(x + 1           , y + 1          , (h / NSF_CTRL_LINES) - 2, 0x002D);
	dos_vline(x + 1 + (wc * 1), y + 1          , (h / NSF_CTRL_LINES) - 2, 0x002D);
	dos_vline(x + 1 + (wc * 2), y + 1          , (h / NSF_CTRL_LINES) - 2, 0x002D);
	dos_vline(x + 1 + (wc * 3), y + 1          , (h / NSF_CTRL_LINES) - 2, 0x002D);
	dos_vline(x + 1 + (wc * 4), y + 1          , (h / NSF_CTRL_LINES) - 2, 0x002D);
	dos_hline(x + 1           , y + ((h / NSF_CTRL_LINES) * 1) + 0, w - 2, 0x002D);
	dos_vline(x + 1           , y + ((h / NSF_CTRL_LINES) * 1) + 1, (h / NSF_CTRL_LINES) - 2, 0x002D);
	dos_hline(x + 1           , y + ((h / NSF_CTRL_LINES) * 2) + 0, w - 2, 0x002D);
	dos_vline(x + 1           , y + ((h / NSF_CTRL_LINES) * 2) + 1, (h / NSF_CTRL_LINES) - 2, 0x002D);
	dos_hline(x + 1           , y + ((h / NSF_CTRL_LINES) * 3) + 0, w - 2, 0x002D);
	dos_vline(x + 1           , y + ((h / NSF_CTRL_LINES) * 3) + 1, (h / NSF_CTRL_LINES) - 2, 0x002D);

#define nsf_dc_box_ctrl(ctr, color, str)\
	dos_box(x + 2 + (wc * ctr), y + 2, wc - 1, (h / NSF_CTRL_LINES) - 2 -1,\
		doscolor(color), doscolor(color), doscolor(color));\
	dos_text(x + 1 + (wc * ctr) + ((wc - 8) / 2), y + (((h / NSF_CTRL_LINES) - 8) / 2), str)

	if (port[PORT1].data[LEFT] == PRESSED) {
		nsf_dc_box_ctrl(0, DOS_GRAY, "[bck][gray][brown][prev]");
	} else {
		nsf_dc_box_ctrl(0, DOS_BLACK, "[bck][black][prev]");
	}
	if (nsf.state & NSF_PLAY) {
		nsf_dc_box_ctrl(1, DOS_GRAY, "[bck][gray][green][play]");
	} else {
		nsf_dc_box_ctrl(1, DOS_BLACK, "[bck][black][play]");
	}
	if (nsf.state & NSF_PAUSE) {
		nsf_dc_box_ctrl(2, DOS_GRAY, "[bck][gray][yellow][pause]");
	} else {
		nsf_dc_box_ctrl(2, DOS_BLACK, "[bck][black][pause]");
	}
	if (nsf.state & NSF_STOP) {
		nsf_dc_box_ctrl(3, DOS_GRAY, "[bck][gray][red][stop]");
	} else {
		nsf_dc_box_ctrl(3, DOS_BLACK, "[bck][black][stop]");
	}
	if (port[PORT1].data[RIGHT] == PRESSED) {
		nsf_dc_box_ctrl(4, DOS_GRAY, "[bck][gray][brown][next]");
	} else {
		nsf_dc_box_ctrl(4, DOS_BLACK, "[bck][black][next]");
	}

	dos_text(x + ((w - dospf(12)) / 2), y + ((h / NSF_CTRL_LINES) * 1) +
			(((h / NSF_CTRL_LINES) - 8) / 2),
			"[bck][gray]Song [cyan]%3d[normal]/%3d", nsf.song.current + 1, nsf.song.total + 1);

	if (nsf.state) {
		static char cbuff[3][50], mscolor[10];
		static char buff[300];
		int ibuff[4], a;
		double timer = nsf.timers.song;

		if ((nsf.state & NSF_CHANGE_SONG) || (nsf.state & NSF_STOP)) {
			timer = 0;
		}

		// millisecondi
		ibuff[3] = ((int) timer / 10) % 100;
		// secondi
		ibuff[0] = (int) (timer / 1000) % 60 ;
		// minuti
		ibuff[1] = ((int) timer / (1000 * 60)) % 60;
		// ore
		ibuff[2] = ((int) timer / (1000 * 60 * 60)) % 24;

		for (a = 0; a < 3; a++) {
			int i, max, itmp, index = 0;
			BYTE is_normal = FALSE, no_more = FALSE;

			memset(cbuff[a], 0x00, sizeof(cbuff[a]));

			for (i = a + 1; i < 3; i++) {
				if (ibuff[i] > 0) {
					is_normal = TRUE;
				}
			}

			if (a == 2) {
				if (is_normal == TRUE) {
					sprintf(&cbuff[a][index], "[normal]%03d", ibuff[a]);
					continue;
				}
				max = 100;
				i = 3;
			} else {
				if (is_normal == TRUE) {
					sprintf(&cbuff[a][index], "[normal]%02d", ibuff[a]);
					continue;
				}
				max = 10;
				i = 2;
			}

			for (;i > 0; i--) {
				itmp = ibuff[a] / max;

				if (no_more == FALSE) {
					if (itmp == 0) {
						if (index == 0) {
							sprintf(&cbuff[a][index], "[gray]");
							index += 6;
						}
						no_more = FALSE;
					} else {
						if (index > 0) {
							sprintf(&cbuff[a][index], "[normal]");
							index += 8;
						}
						no_more = TRUE;
					}
				}
				sprintf(&cbuff[a][index], "%d", itmp);
				index++;
				ibuff[a] -= (itmp * max);
				max /= 10;
			}
		}

		if (timer == 0) {
			sprintf(mscolor, "[gray]");
		} else {
			sprintf(mscolor, "[normal]");
		}

		snprintf(buff, sizeof(buff), "%s[gray]:[normal]%s[gray]:[normal]%s[gray]:%s%02d",
			cbuff[2], cbuff[1], cbuff[0], mscolor, ibuff[3]);

		dos_text(x + ((w - dospf(12)) / 2), y + ((h / NSF_CTRL_LINES) * 2) +
				(((h / NSF_CTRL_LINES) - 8) / 2), "%s", buff);
	}

	{
		int d = ((w - (6 * dospf(2))) / 6);

		if (nsf.sound_chips.vrc6) {
			dos_text(x + 1 + (d * 1) + (dospf(2) * 0), y + ((h / NSF_CTRL_LINES) * 3) +
					(((h / NSF_CTRL_LINES) - 8) / 2),
					"[green][bck][gray][vrc6a][vrc6b]");
		} else {
			dos_text(x + 1 + (d * 1) + (dospf(2) * 0), y + ((h / NSF_CTRL_LINES) * 3) +
					(((h / NSF_CTRL_LINES) - 8) / 2),
					"[gray][bck][black][vrc6a][vrc6b]");
		}

		if (nsf.sound_chips.vrc7) {
			dos_text(x + 1 + (d * 2) + (dospf(2) * 1), y + ((h / NSF_CTRL_LINES) * 3) +
					(((h / NSF_CTRL_LINES) - 8) / 2),
					"[green][bck][gray][vrc7a][vrc7b]");
		} else {
			dos_text(x + 1 + (d * 2) + (dospf(2) * 1), y + ((h / NSF_CTRL_LINES) * 3) +
					(((h / NSF_CTRL_LINES) - 8) / 2),
					"[gray][bck][black][vrc7a][vrc7b]");
		}

		if (nsf.sound_chips.mmc5) {
			dos_text(x + 1 + (d * 3) + (dospf(2) * 2), y + ((h / NSF_CTRL_LINES) * 3) +
					(((h / NSF_CTRL_LINES) - 8) / 2),
					"[green][bck][gray][mmc5a][mmc5b]");
		} else {
			dos_text(x + 1 + (d * 3) + (dospf(2) * 2), y + ((h / NSF_CTRL_LINES) * 3) +
					(((h / NSF_CTRL_LINES) - 8) / 2),
					"[gray][bck][black][mmc5a][mmc5b]");
		}

		if (nsf.sound_chips.namco163) {
			dos_text(x + 1 + (d * 4) + (dospf(2) * 3), y + ((h / NSF_CTRL_LINES) * 3) +
					(((h / NSF_CTRL_LINES) - 8) / 2),
					"[green][bck][gray][n163a][n163b]");
		} else {
			dos_text(x + 1 + (d * 4) + (dospf(2) * 3), y + ((h / NSF_CTRL_LINES) * 3) +
					(((h / NSF_CTRL_LINES) - 8) / 2),
					"[gray][bck][black][n163a][n163b]");
		}

		if (nsf.sound_chips.sunsoft5b) {
			dos_text(x + 1 + (d * 5) + (dospf(2) * 4), y + ((h / NSF_CTRL_LINES) * 3) +
					(((h / NSF_CTRL_LINES) - 8) / 2),
					"[green][bck][gray][s5b1][s5b2]");
		} else {
			dos_text(x + 1 + (d * 5) + (dospf(2) * 4), y + ((h / NSF_CTRL_LINES) * 3) +
					(((h / NSF_CTRL_LINES) - 8) / 2),
					"[gray][bck][black][s5b1][s5b2]");
		}

		if (nsf.sound_chips.fds) {
			dos_text(x + 1 + (d * 6) + (dospf(2) * 5), y + ((h / NSF_CTRL_LINES) * 3) +
					(((h / NSF_CTRL_LINES) - 8) / 2),
					"[green][bck][gray][fds1][fds2]");
		} else {
			dos_text(x + 1 + (d * 6) + (dospf(2) * 5), y + ((h / NSF_CTRL_LINES) * 3) +
					(((h / NSF_CTRL_LINES) - 8) / 2),
					"[gray][bck][black][fds1][fds2]");
		}
	}

#undef nsf_dc_box_ctrl
}
