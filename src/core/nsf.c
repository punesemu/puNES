/*
 *  Copyright (C) 2010-2024 Fabio Cavallo (aka FHorse)
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
#include "nsfe.h"
#include "rom_mem.h"
#include "mappers.h"
#include "info.h"
#include "nes.h"
#include "conf.h"
#include "gui.h"
#include "audio/blipbuf.h"
#include "audio/snd.h"
#include "patcher.h"
#include "extra/kissfft/kiss_fft.h"
#include "input/standard_controller.h"
#include "draw_on_screen.h"

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
enum nsf_gui {
	NSF_GUI_PPUX = 35,
	NSF_GUI_PPUY = 49,

	NSF_GUI_TITLE_PPUX = NSF_GUI_PPUX + 2,
	NSF_GUI_TITLE_PPUY = NSF_GUI_PPUY + 2 + 2,
	NSF_GUI_TITLE_W = 182,
	NSF_GUI_TITLE_H = 15,

	NSF_GUI_TIMERS_1_PPUX = NSF_GUI_TITLE_PPUX,
	NSF_GUI_TIMERS_1_PPUY = NSF_GUI_TITLE_PPUY + NSF_GUI_TITLE_H + 1,
	NSF_GUI_TIMERS_1_W = 90,
	NSF_GUI_TIMERS_1_H = 10,
	NSF_GUI_TIMERS_2_PPUX = NSF_GUI_TITLE_PPUX + NSF_GUI_TIMERS_1_W + 1,
	NSF_GUI_TIMERS_2_PPUY = NSF_GUI_TIMERS_1_PPUY,
	NSF_GUI_TIMERS_2_W = 91,
	NSF_GUI_TIMERS_2_H = NSF_GUI_TIMERS_1_H,
	NSF_GUI_TIMERS_3_PPUX = NSF_GUI_TIMERS_2_PPUX + NSF_GUI_TIMERS_2_W - 5,
	NSF_GUI_TIMERS_3_PPUY = NSF_GUI_TIMERS_2_PPUY,

	NSF_GUI_SONGS_PPUX = NSF_GUI_TITLE_PPUX,
	NSF_GUI_SONGS_PPUY = NSF_GUI_TIMERS_1_PPUY + NSF_GUI_TIMERS_1_H + 1,
	NSF_GUI_SONGS_W = NSF_GUI_TIMERS_1_W,
	NSF_GUI_SONGS_H = NSF_GUI_TIMERS_1_H,
	NSF_GUI_PLIST_PPUX = NSF_GUI_TIMERS_2_PPUX,
	NSF_GUI_PLIST_PPUY = NSF_GUI_SONGS_PPUY,
	NSF_GUI_PLIST_W = NSF_GUI_TIMERS_2_W,
	NSF_GUI_PLIST_H = NSF_GUI_TIMERS_2_H,

	NSF_GUI_CHIPS_PPUX = NSF_GUI_TITLE_PPUX,
	NSF_GUI_CHIPS_PPUY = NSF_GUI_PLIST_PPUY + NSF_GUI_TIMERS_2_H + 1 + 1,

	NSF_GUI_INFO_PPUX = NSF_GUI_PPUX + 2,
	NSF_GUI_INFO_PPUY = NSF_GUI_CHIPS_PPUY + 12,
	NSF_GUI_INFO_W = NSF_GUI_TITLE_W,
	NSF_GUI_INFO_H = 11,

	NSF_GUI_BUTTONS_PPUX = NSF_GUI_PPUX + 1,
	NSF_GUI_BUTTONS_PPUY = NSF_GUI_INFO_PPUY + NSF_GUI_INFO_H + 2,

	NSF_GUI_OPTIONS_PPUX = 5,
	NSF_GUI_OPTIONS_PPUY = 135,
	NSF_GUI_OPTIONS_BOX_PPUY = NSF_GUI_OPTIONS_PPUY + 14,
	NSF_GUI_OPTIONS_BOX_EXT_W = 7,
	NSF_GUI_OPTIONS_BOX_EXT_H = 7,
	NSF_GUI_OPTIONS_BOX_INT_W = 3,
	NSF_GUI_OPTIONS_BOX_INT_H = 3,
	NSF_GUI_OPTIONS_1_PPUX = NSF_GUI_OPTIONS_PPUX,
	NSF_GUI_OPTIONS_2_PPUX = NSF_GUI_OPTIONS_1_PPUX + 125,
	NSF_GUI_OPTIONS_3_PPUX = NSF_GUI_OPTIONS_2_PPUX + 70,

	NSF_GUI_EFFECT_W = SCR_COLUMNS,
	NSF_GUI_EFFECT_H = 67,
	NSF_GUI_EFFECT_X = (SCR_COLUMNS - NSF_GUI_EFFECT_W) / 2,
	NSF_GUI_EFFECT_Y = SCR_ROWS - NSF_GUI_EFFECT_H,
	NSF_GUI_EFFECT_BARS = 60,
	NSF_GUI_EFFECT_BARS_W = SCR_COLUMNS - 16,
	NSF_GUI_EFFECT_BARS_H = NSF_GUI_EFFECT_H,
	NSF_GUI_EFFECT_BARS_X = (SCR_COLUMNS - NSF_GUI_EFFECT_BARS_W) / 2,
	NSF_GUI_EFFECT_BARS_Y = SCR_ROWS - NSF_GUI_EFFECT_BARS_H,
};
enum nsf_header { NSF_HEADER_LENGTH = 128 };

_nsf nsf;
_nsf2 nsf2;

static void nsf_effect_set_coords(_nsf_effect_coords *coords, int x, int y, int w, int h);
static void nsf_effect_raw(BYTE solid);
static void nsf_effect_hanning_window(BYTE solid);
static void nsf_effect_bars(void);
static void nsf_change_song(BYTE button, unsigned int mode);
static void nsf_change_current_song(unsigned int mode);
static void nsf_draw_controls(void);
static uTCHAR *nsf_print_number(unsigned int song, BYTE decimal, WORD color);
static uTCHAR *nsf_print_time(double timer, BYTE mode, WORD color);
static void nsf_print_option(int ppu_x, int ppu_y, uTCHAR *desc, BYTE active, BYTE mode);
static void nsf_reset_song_variables(void);
static void ustrncat(uTCHAR *buffer, size_t buffer_size, const uTCHAR *fmt, ...);

void nsf_init(void) {
	memset(&nsf, 0x00, sizeof(nsf));

	nsf_effect_set_coords(&nsf.effect_coords, NSF_GUI_EFFECT_X, NSF_GUI_EFFECT_Y,
		NSF_GUI_EFFECT_W, NSF_GUI_EFFECT_H);
	nsf_effect_set_coords(&nsf.effect_bars_coords, NSF_GUI_EFFECT_BARS_X, NSF_GUI_EFFECT_BARS_Y,
		NSF_GUI_EFFECT_BARS_W, NSF_GUI_EFFECT_BARS_H);
}
void nsf_quit(void) {
	if (nsf.info.name) {
		free(nsf.info.name);
	}
	if (nsf.info.artist) {
		free(nsf.info.artist);
	}
	if (nsf.info.copyright) {
		free(nsf.info.copyright);
	}
	if (nsf.info.ripper) {
		free(nsf.info.ripper);
	}
	if (nsf.info.text) {
		free(nsf.info.text);
	}
	if (nsf.playlist.data) {
		free(nsf.playlist.data);
	}
	if (nsf.info_song) {
		for (int i = 0; i < nsf.songs.total; i++) {
			if (nsf.info_song[i].track_label) {
				free(nsf.info_song[i].track_label);
			}
			if (nsf.info_song[i].author) {
				free(nsf.info_song[i].author);
			}
		}
		free(nsf.info_song);
	}
	if (nsf.scroll_info_nsf.pimage.data) {
		free(nsf.scroll_info_nsf.pimage.data);
	}
	if (nsf.scroll_title_song.pimage.data) {
		free(nsf.scroll_title_song.pimage.data);
	}

	dos_text_curtain(0, 0, 0, &nsf.curtain_info, DOS_TEXT_CURTAIN_INIT);
	dos_text_curtain(0, 0, 0, &nsf.curtain_title_song, DOS_TEXT_CURTAIN_INIT);

	if (nsf.sound_chips.vrc7) {
		extcl_mapper_quit_VRC7();
	}

	gui_nsf_author_note_close();

	nsf_init();
}
void nsf_reset(void) {
	nsf.timers.effect = 0;
}
void nsf_info(void) {
	log_info_box(uL("name;" uPs("") ""), nsf.info.name);
	log_info_box(uL("artist;" uPs("") ""), nsf.info.artist);
	log_info_box(uL("copyright;" uPs("") ""), nsf.info.copyright);
	log_info_box(uL("ripper;" uPs("") ""), nsf.info.ripper);
	log_info_box(uL("text;%s"), nsf.info.text ? "yes" : "no");
	log_info_box(uL("region;supported %s%s%s%s"),
		nsf.region.supported & 0x01 ? "NTSC" : "",
		nsf.region.supported & 0x02 ? nsf.region.supported & 0x01 ? "/PAL" : "PAL" : "",
		nsf.region.supported & 0x04 ? nsf.region.supported & 0x03 ? "/Dendy" : "Dendy" : "",
		(nsf.region.supported == 0x01) || (nsf.region.supported == 0x02) || (nsf.region.supported == 0x04) ? ""
			: nsf.region.preferred == NSF_NTSC_MODE ? ", preferred NTSC"
			: nsf.region.preferred == NSF_PAL_MODE ? ", preferred PAL"
			: nsf.region.preferred == NSF_DENDY_MODE ? ", preferred Dendy"
			: ", preferred unknown");
	log_info_box(uL("irq support;%s"), nsf2.features.irq_support ? "yes" : "no");
	log_info_box(uL("non-ret INIT;%s"), nsf2.features.non_returning_init ? "yes" : "no");
	log_info_box(uL("disable PLAY;%s"), nsf2.features.suppressed_PLAY ? "yes" : "no");
	if (nsf.playlist.count) {
		log_info_box_open(uL("playlist;"));
		for (uint32_t tmp = 0; tmp < nsf.playlist.count; tmp++) {
			if (tmp == 0) {
				log_append(uL("%d"), nsf.playlist.data[tmp]);
			} else {
				log_append(uL(", %d"), nsf.playlist.data[tmp]);
			}
		}
		log_close_box(uL(""));
	}
	if (nsf.info_song) {
		for (int tmp = 0; tmp < nsf.songs.total; tmp++) {
			_nsf_info_song *song = &nsf.info_song[tmp];

			if (song->track_label) {
				log_info_box(uL("%d;%-7d %-7d " uPs("") ""), tmp, song->time, song->fade, song->track_label);
			}
		}
	}
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

		rom.data = (BYTE *)malloc(rom.size);
		if (rom.data == NULL) {
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

		info.number_of_nes = 1;
		info.machine[DATABASE] = DEFAULT;

		memset(&nsf2, 0x00, sizeof(_nsf2));

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
			uTCHAR **dst = NULL;
			char buffer[33];
			size_t size = 0;

			for (tmp = 0; tmp < 4; tmp++) {
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
					case 3:
						dst = &nsf.info.ripper;
						break;
				}
				memset(&buffer, 0x00, sizeof(buffer));
				if (tmp < 3) {
					if (rom_mem_ctrl_memcpy(&buffer, &rom, 32) == EXIT_ERROR) {
						free(rom.data);
						return (EXIT_ERROR);
					}
				}
				size = gui_utf8_to_utchar(&buffer[0], dst, sizeof(buffer) - 1);
				if (!size) {
					size = gui_utf8_to_utchar(&nsf_default_label[0], dst, strlen(nsf_default_label));
				}
				if (!size) {
					free(rom.data);
					return (EXIT_ERROR);
				}
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

		nsf.play_speed.dendy = 0;

		tmp = rom.data[rom.position++] & 0x03;
		switch (tmp & 0x01) {
			default:
			case 0:
				nsf.region.preferred = NSF_NTSC_MODE;
				break;
			case 1:
				nsf.region.preferred = NSF_PAL_MODE;
				break;
		}
		nsf.region.supported = tmp & 0x02 ? 0x03 : nsf.region.preferred == NSF_NTSC_MODE ? 0x01 : 0x02;

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

		if ((nsf.version == 2) && !(rom.data[rom.position] & 0x03)) {
			info.format = NSF2_FORMAT;
			tmp = rom.data[rom.position++];
			// $07C 1 BYTE : NSF2 feature flags
			// bits 0-3: reserved, must be 0
			// bit 4: if set, this NSF may use the IRQ support features
			// bit 5: if set, the non-returning INIT playback feature will be used
			// bit 6: if set, the PLAY subroutine will not be used
			// bit 7: if set, the appended NSFe metadata may contain a mandatory chunk required for playback
			nsf2.features.irq_support = !!(tmp & 0x10);
			nsf2.features.non_returning_init = !!(tmp & 0x20);
			nsf2.features.suppressed_PLAY = !!(tmp & 0x40);
			nsf2.features.metadata = !!(tmp & 0x80);
			// $07D 3 BYTES : 24-bit length of the NSF program data, allowing metadata to follow the data
			nsf2.prg_size = rom.data[rom.position++];
			nsf2.prg_size |= (rom.data[rom.position++] << 8);
			nsf2.prg_size |= (rom.data[rom.position++] << 16);
		} else {
			rom.position += 4;
		}

		ram_set_size(0, S2K);
		ram_init();

		wram_set_ram_size(nsf.sound_chips.fds ? 0xA000 : S8K);

		{
			size_t prgrom_len = nsf2.prg_size ? nsf2.prg_size : len;
			int padding = nsf.adr.load & 0x0FFF;

			prgrom_set_size((((prgrom_len + padding) / S4K) + (((prgrom_len + padding) % S4K) ? 1 : 0)) * S4K);

			if (prgrom_init(0xF2) == EXIT_ERROR) {
				free(rom.data);
				return (EXIT_ERROR);
			}

			if (rom_mem_ctrl_memcpy(prgrom_pnt() + padding, &rom, prgrom_len) == EXIT_ERROR) {
				free(rom.data);
				return (EXIT_ERROR);
			}
		}

		// metadata
		if (info.format == NSF2_FORMAT) {
			while ((rom.position + sizeof(nsf.chunk)) <= rom.size) {
				nsf2.features.metadata = TRUE;

				rom_mem_memcpy(&nsf.chunk, &rom, sizeof(nsf.chunk));

				if (strncmp(nsf.chunk.id, "RATE", 4) == 0) {
					if (nsfe_RATE(&rom, NSFE_READ)) {
						free(rom.data);
						return (EXIT_ERROR);
					}
				} else if (strncmp(nsf.chunk.id, "plst", 4) == 0) {
					if (nsfe_plst(&rom, NSFE_READ) == EXIT_ERROR) {
						free(rom.data);
						return (EXIT_ERROR);
					}
				} else if (strncmp(nsf.chunk.id, "time", 4) == 0) {
					if (nsfe_time(&rom, NSFE_READ) == EXIT_ERROR) {
						free(rom.data);
						return (EXIT_ERROR);
					}
				} else if (strncmp(nsf.chunk.id, "fade", 4) == 0) {
					if (nsfe_fade(&rom, NSFE_READ) == EXIT_ERROR) {
						free(rom.data);
						return (EXIT_ERROR);
					}
				} else if (strncmp(nsf.chunk.id, "tlbl", 4) == 0) {
					if (nsfe_tlbl(&rom, NSFE_READ) == EXIT_ERROR) {
						free(rom.data);
						return (EXIT_ERROR);
					}
				} else if (strncmp(nsf.chunk.id, "taut", 4) == 0) {
					if (nsfe_taut(&rom, NSFE_READ) == EXIT_ERROR) {
						free(rom.data);
						return (EXIT_ERROR);
					}
				} else if (strncmp(nsf.chunk.id, "auth", 4) == 0) {
					if (nsfe_auth(&rom, NSFE_READ) == EXIT_ERROR) {
						free(rom.data);
						return (EXIT_ERROR);
					}
				} else if (strncmp(nsf.chunk.id, "text", 4) == 0) {
					if (nsfe_text(&rom, NSFE_READ) == EXIT_ERROR) {
						free(rom.data);
						return (EXIT_ERROR);
					}
				} else if (strncmp(nsf.chunk.id, "regn", 4) == 0) {
					if (nsfe_regn(&rom, NSFE_READ) == EXIT_ERROR) {
						free(rom.data);
						return (EXIT_ERROR);
					}
				} else {
					// ignoro il typo di chunk non riconosciuto
					if (nsfe_NONE(&rom, NSFE_READ) == EXIT_ERROR) {
						free(rom.data);
						return (EXIT_ERROR);
					}
				}
			}
		}

		if (!nsf.play_speed.ntsc) {
			nsf.play_speed.ntsc = 0x40FF;
		}
		if (!nsf.play_speed.pal) {
			nsf.play_speed.pal = 0x4E1D;
		}
		if (!nsf.play_speed.dendy) {
			nsf.play_speed.dendy = nsf.play_speed.pal;
		}

		switch (nsf.region.preferred & 0x03) {
			case NSF_NTSC_MODE:
				info.machine[DATABASE] = NTSC;
				break;
			case NSF_PAL_MODE:
				info.machine[DATABASE] = PAL;
				break;
			case NSF_DENDY_MODE:
				info.machine[DATABASE] = DENDY;
				break;
			default:
				nsf.region.preferred = NSF_NTSC_MODE;
				info.machine[DATABASE] = NTSC;
				break;
		}

		nsf.enabled = TRUE;
		nsf.songs.current = nsf.songs.starting - 1;
		nsf.routine = &nsf_routine[0];

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
	nsf.scroll_info_nsf.rect.x = 0;
	nsf.scroll_info_nsf.rect.y = 0;
	nsf.scroll_info_nsf.rect.w = SCR_COLUMNS;
	nsf.scroll_info_nsf.rect.h = 16;
	nsf.scroll_info_nsf.timer = 4.0f;
	nsf.scroll_info_nsf.reload = 4.0f;
	nsf.scroll_info_nsf.velocity = 6;

	nsf.scroll_title_song.rect.x = 0;
	nsf.scroll_title_song.rect.y = 0;
	nsf.scroll_title_song.rect.w = NSF_GUI_TITLE_W;
	nsf.scroll_title_song.rect.h = NSF_GUI_TITLE_H;
	nsf.scroll_title_song.timer = 4.0f;
	nsf.scroll_title_song.reload = 4.0f;
	nsf.scroll_title_song.velocity = 6;

	nsf.curtain_title_song.reload.r1 = 50;
	nsf.curtain_title_song.reload.r2 = 3000;
	nsf.curtain_title_song.image.x = DOS_ALIGNHCENTER;
	nsf.curtain_title_song.image.y = 0;
	nsf.curtain_title_song.image.w = NSF_GUI_TITLE_W;
	nsf.curtain_title_song.image.h = NSF_GUI_TITLE_H;

	nsf.curtain_info.reload.r1 = 50;
	nsf.curtain_info.reload.r2 = 3000;
	nsf.curtain_info.image.x = DOS_ALIGNHCENTER;
	nsf.curtain_info.image.y = 0;
	nsf.curtain_info.image.w = NSF_GUI_INFO_W;
	nsf.curtain_info.image.h = NSF_GUI_INFO_H;
	dos_text_curtain(0, 0, 0, &nsf.curtain_info, DOS_TEXT_CURTAIN_INIT);
	dos_text_curtain_add_line(&nsf.curtain_info, DOS_TL02, DOS_TL01, uL("lemon_10"), 10, uL(" "));
	dos_text_curtain_add_line(&nsf.curtain_info, DOS_TL02, DOS_TL01, uL("lemon_10"), 10,
		uL("[top][image]:/pics/pics/controller_start.png[endimage] Start : Play/Pause"));
	dos_text_curtain_add_line(&nsf.curtain_info, DOS_TL02, DOS_TL01, uL("lemon_10"), 10,
		uL("[top][image]:/pics/pics/controller_select.png[endimage] Start : Stop"));
	dos_text_curtain_add_line(&nsf.curtain_info, DOS_TL02, DOS_TL01, uL("lemon_10"), 10,
		uL("[top][image]:/pics/pics/controller_right.png[endimage] Right : Next Song"));
	dos_text_curtain_add_line(&nsf.curtain_info, DOS_TL02, DOS_TL01, uL("lemon_10"), 10,
		uL("[top][image]:/pics/pics/controller_left.png[endimage] Left : Previous Song"));
	dos_text_curtain_add_line(&nsf.curtain_info, DOS_TL02, DOS_TL01, uL("lemon_10"), 10,
		uL("[top][image]:/pics/pics/controller_button_b.png[endimage] B : Mute"));
	dos_text_curtain_add_line(&nsf.curtain_info, DOS_TL02, DOS_TL01, uL("lemon_10"), 10,
		uL("[top][image]:/pics/pics/controller_button_a.png[endimage] A : Repeat Song"));
	dos_text_curtain_add_line(&nsf.curtain_info, DOS_TL02, DOS_TL01, uL("lemon_10"), 10,
		uL("[top][image]:/pics/pics/controller_up.png[endimage] Up : Change Effect"));
	dos_text_curtain_add_line(&nsf.curtain_info, DOS_TL02, DOS_TL01, uL("lemon_10"), 10,
		uL("or use your [top][image]:/pics/pics/controller_mouse.png[endimage] "));
}
void nsf_init_tune(void) {
	BYTE reset = info.reset;

	info.reset = HARD;
	info.disable_tick_hw = TRUE;

	ram_memset();
	wram_memset();

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
		map_init_NSF_005();
	}
	if (nsf.sound_chips.namco163) {
		map_init_NSF_N163();
	}
	if (nsf.sound_chips.sunsoft5b) {
		init_NSF_FME7();
	}

	for (WORD i = 0x4000; i < 0x4014; i++) {
		cpu_wr_mem(0, i, 0x00);
	}
	cpu_wr_mem(0, 0x4015, 0x00);
	cpu_wr_mem(0, 0x4015, 0x0F);
	cpu_wr_mem(0, 0x4017, 0x40);

	if (nsf.sound_chips.fds) {
		cpu_wr_mem(0, 0x4089, 0x80);
		cpu_wr_mem(0, 0x408A, 0xE8);
	}

	nsf_reset_prg();

	info.disable_tick_hw = FALSE;
	info.reset = reset;

	nsf.state = NSF_PLAY;
	nsf.songs.started = FALSE;

	nsf_reset_song_variables();
	nsf_reset_timers();
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
				if (cfg->nsf_player_nsf_fadeout) {
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

	wram_reset_chunks();

	if (nsf.bankswitch.enabled) {
		if (nsf.sound_chips.fds) {
			for (i = 0x5FF6; i <= 0x5FF7; i++) {
				cpu_wr_mem(0, i, nsf.bankswitch.banks[i & 0x07]);
			}
		}
		for (i = 0x5FF8; i <= 0x5FFF; i++) {
			cpu_wr_mem(0, i, nsf.bankswitch.banks[i & 0x07]);
		}
	} else {
		BYTE value = 0, bank = 0;

		nsf.bankswitch.enabled = TRUE;
		i = nsf.sound_chips.fds ? 0x6000 : 0x8000;
		for (; i < 0x10000; i += 0x1000) {
			value = prgrom_control_bank(S4K, bank);
			if (i < (nsf.adr.load & 0xF000)) {
				cpu_wr_mem(0, 0x5FF0 | (i >> 12), 0);
			} else {
				cpu_wr_mem(0, 0x5FF0 | (i >> 12), value);
				if (bank < prgrom_banks(S4K)) {
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
	static uTCHAR no_title[] = uL("<No info available>");

	dos_text_curtain(0, 0, 0, &nsf.curtain_title_song, DOS_TEXT_CURTAIN_INIT);
	if (nsf.info_song) {
		memcpy(&nsf.current_song, &nsf.info_song[nsf.songs.current], sizeof(_nsf_info_song));
		dos_text_curtain_add_line(&nsf.curtain_title_song, nsf.info_song ? DOS_TL03 : DOS_TL02, DOS_TL01,
			uL("Ttyp0_11"), 11, uL("" uPs("") uPs("") uPs("") ""),
			nsf.current_song.track_label,
			nsf.current_song.author ? uL(" - ") : uL(""),
			nsf.current_song.author ? nsf.current_song.author : uL(""));
	} else {
		nsf.current_song.track_label = &no_title[0];
		dos_text_curtain_add_line(&nsf.curtain_title_song, nsf.info_song ? DOS_TL03 : DOS_TL02, DOS_TL01,
			uL("Ttyp0_11"), 11, nsf.current_song.track_label);
	}
}

void nsf_main_screen(void) {
	nsf_draw_controls();
	nsf_main_screen_event();
}
void nsf_main_screen_event(void) {
	_port *prt = &port[PORT1];
	int reset = FALSE;

	if (!(nsf.state & NSF_CHANGE_SONG) &&
		((nsf.timers.silence > NFS_TIME_AUTO_NEXT_SONG) ||
		((nsf.current_song.time > 0) && (nsf.timers.song >= nsf.current_song.time)))) {
		nsf.songs.started = FALSE;
		if (nsf.repeat_song) {
			nsf_change_current_song(NSF_RESTART_SONG);
		} else {
			nsf_change_current_song(NSF_NEXT);
		}
		nsf.state |= NSF_CHANGE_SONG;
	}

	if (nsf.timers.button[INPUT_DECODE_COUNTS - 1] && !(--nsf.timers.button[INPUT_DECODE_COUNTS - 1])) {
		nsf.authors_note = !nsf.authors_note;
		if (nsf.authors_note) {
			gui_nsf_author_note_open(nsf.info.text);
			nsf.timers.button[INPUT_DECODE_COUNTS - 1] = 10;
		}
	}

	// gestione eventi
	if (prt->data.treated[START] == PRESSED) {
		switch (nsf.state & 0x07) {
			case NSF_STOP:
				nsf.state = NSF_PLAY | NSF_CHANGE_SONG;
				nsf.timers.update_only_diff = FALSE;
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
		input_data_set_standard_controller(START, RELEASED, prt);
	} else if (prt->data.treated[SELECT] == PRESSED) {
		nsf.state = NSF_STOP;
		nsf.timers.update_only_diff = TRUE;
		nsf.timers.song = 0;
		input_data_set_standard_controller(SELECT, RELEASED, prt);
		reset = TRUE;
	} else if (prt->data.treated[UP] == PRESSED) {
		cfg->nsf_player_effect++;
		if (cfg->nsf_player_effect >= NSF_EFFECTS) {
			cfg->nsf_player_effect = 0;
		}
		input_data_set_standard_controller(UP, RELEASED, prt);
	} else if (prt->data.treated[DOWN] == PRESSED) {
		cfg->nsf_player_effect--;
		if (cfg->nsf_player_effect > NSF_EFFECTS) {
			cfg->nsf_player_effect = NSF_EFFECTS - 1;
		}
		input_data_set_standard_controller(DOWN, RELEASED, prt);
	} else if (prt->data.treated[LEFT] == PRESSED) {
		if (nsf.state & NSF_PAUSE) {
			nsf.state = NSF_STOP;
			nsf_change_song(LEFT, NSF_PREV);
		} else if (nsf.timers.song < 1000) {
			nsf_change_song(LEFT, NSF_PREV);
		} else {
			nsf_change_song(LEFT, NSF_RESTART_SONG);
		}
		reset = TRUE;
	} else if (prt->data.treated[RIGHT] == PRESSED) {
		if (nsf.state & NSF_PAUSE) {
			nsf.state = NSF_STOP;
		}
		nsf_change_song(RIGHT, NSF_NEXT);
		reset = TRUE;
	} else if (prt->data.treated[BUT_B] == PRESSED) {
		if (nsf.timers.button[BUT_B] && !(--nsf.timers.button[BUT_B])) {
			gui_toggle_audio();
		}
	} else if (prt->data.treated[BUT_A] == PRESSED) {
		if (nsf.timers.button[BUT_A] && !(--nsf.timers.button[BUT_A])) {
			nsf.repeat_song = !nsf.repeat_song;
		}
	}

	if (reset) {
		snd_reset_buffers();
		nsf_reset();
	}

	// ridisegno i controlli
	nsf_draw_controls();
}

void nsf_controls_mouse_in_gui(int x_mouse, int y_mouse) {
	int x = 0, y = 0, w = 0, h = 0;
	_port *prt = &port[PORT1];

	// pulsanti
	if (nsf.info.text) {
		x = NSF_GUI_PPUX + dos_resource_w(uL(":/pics/pics/nsf_player.png")) -
			dos_resource_w(uL(":/pics/pics/authors_note_press.png"));
		y = NSF_GUI_PPUY - dos_resource_h(uL(":/pics/pics/authors_note_press.png"));
		dos_resource_size(&w, &h, uL(":/pics/pics/authors_note_press.png"));
		if (((x_mouse >= x) && (x_mouse <= (x + (w - 1)))) && ((y_mouse >= y) && (y_mouse <= (y + h)))) {
			if (!nsf.timers.button[INPUT_DECODE_COUNTS - 1]) {
				nsf.timers.button[INPUT_DECODE_COUNTS - 1] = 1;
			}
			return;
		}
	}
	x = NSF_GUI_BUTTONS_PPUX; y = NSF_GUI_BUTTONS_PPUY;
	dos_resource_size(&w, &h, uL(":/pics/pics/nsf_play_press.png"));
	if (((x_mouse >= x) && (x_mouse <= (x + (w - 1)))) && ((y_mouse >= y) && (y_mouse <= (y + h)))) {
		if (!(nsf.state & NSF_PLAY)) {
			input_data_set_standard_controller(START, PRESSED, prt);
			nsf.timers.button[START] = 0;
		}
		return;
	}
	x += w; w = dos_resource_w(uL(":/pics/pics/nsf_pause_press.png"));
	if (((x_mouse >= x) && (x_mouse <= (x + (w - 1)))) && ((y_mouse >= y) && (y_mouse <= (y + h)))) {
		if (nsf.state & NSF_PLAY) {
			input_data_set_standard_controller(START, PRESSED, prt);
			nsf.timers.button[START] = 0;
		}
		return;
	}
	x += w; w = dos_resource_w(uL(":/pics/pics/nsf_stop_press.png"));
	if (((x_mouse >= x) && (x_mouse <= (x + (w - 1)))) && ((y_mouse >= y) && (y_mouse <= (y + h)))) {
		input_data_set_standard_controller(SELECT, PRESSED, prt);
		nsf.timers.button[SELECT] = 0;
		return;
	}
	x += w; w = dos_resource_w(uL(":/pics/pics/nsf_prev_press.png"));
	if (((x_mouse >= x) && (x_mouse <= (x + (w - 1)))) && ((y_mouse >= y) && (y_mouse <= (y + h)))) {
		input_data_set_standard_controller(LEFT, PRESSED, prt);
		nsf.timers.button[LEFT] = 0;
		return;
	}
	x += w; w = dos_resource_w(uL(":/pics/pics/nsf_next_press.png"));
	if (((x_mouse >= x) && (x_mouse <= (x + w))) && ((y_mouse >= y) && (y_mouse <= (y + h)))) {
		input_data_set_standard_controller(RIGHT, PRESSED, prt);
		nsf.timers.button[RIGHT] = 0;
		return;
	}
	x += w; w = dos_resource_w(uL(":/pics/pics/nsf_mute_press.png"));
	if (((x_mouse >= x) && (x_mouse <= (x + w))) && ((y_mouse >= y) && (y_mouse <= (y + h)))) {
		input_data_set_standard_controller(BUT_B, PRESSED, prt);
		nsf.timers.button[BUT_B] = 1;
		return;
	}
	x += w; w = dos_resource_w(uL(":/pics/pics/nsf_repeat_song_press.png"));
	if (((x_mouse >= x) && (x_mouse <= (x + w))) && ((y_mouse >= y) && (y_mouse <= (y + h)))) {
		input_data_set_standard_controller(BUT_A, PRESSED, prt);
		nsf.timers.button[BUT_A] = 1;
		return;
	}
	// timer
	if (nsf.info_song && nsf.current_song.use_timer) {
		x = NSF_GUI_TIMERS_3_PPUX; y = NSF_GUI_TIMERS_3_PPUY; w = 2; h = NSF_GUI_TIMERS_2_H;
		if (((x_mouse >= x) && (x_mouse <= (x + w))) && ((y_mouse >= y) && (y_mouse <= (y + h)))) {
			nsf.options.visual_duration = !nsf.options.visual_duration;
			return;
		}
	}
	// opzioni
	x = NSF_GUI_OPTIONS_1_PPUX; y = NSF_GUI_OPTIONS_BOX_PPUY + 1;
	w = NSF_GUI_OPTIONS_BOX_EXT_W; h = NSF_GUI_OPTIONS_BOX_EXT_H;
	if (((x_mouse >= x) && (x_mouse <= (x + w))) && ((y_mouse >= y) && (y_mouse <= (y + h)))) {
		cfg->reverse_bits_dpcm = !cfg->reverse_bits_dpcm;
		gui_update_apu_channels_widgets();
		return;
	}
	if (nsf.playlist.data) {
		x = NSF_GUI_OPTIONS_2_PPUX;
		if (((x_mouse >= x) && (x_mouse <= (x + w))) && ((y_mouse >= y) && (y_mouse <= (y + h)))) {
			BYTE current = nsf.songs.current;

			cfg->nsf_player_playlist = !cfg->nsf_player_playlist;
			nsf_change_current_song(NSF_RESTART_SONG);
			if (nsf.songs.current != current) {
				nsf.state |= NSF_CHANGE_SONG;
			}
			return;
		}
		x = NSF_GUI_OPTIONS_3_PPUX;
		if (((x_mouse >= x) && (x_mouse <= (x + w))) && ((y_mouse >= y) && (y_mouse <= (y + h)))) {
			cfg->nsf_player_nsf_fadeout = !cfg->nsf_player_nsf_fadeout;
			return;
		}
	}
	// effetti
	x = 90; y = 159; w = 8; h = 11;
	if (((x_mouse >= x) && (x_mouse <= (x + w))) && ((y_mouse >= y) && (y_mouse <= (y + h)))) {
		input_data_set_standard_controller(DOWN, PRESSED, prt);
		nsf.timers.button[DOWN] = 0;
		return;
	}
	x = 160;
	if (((x_mouse >= x) && (x_mouse <= (x + w))) && ((y_mouse >= y) && (y_mouse <= (y + h)))) {
		input_data_set_standard_controller(UP, PRESSED, prt);
		nsf.timers.button[UP] = 0;
		return;
	}
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
			nes[0].p.ppu_screen.wr->line[y][x] = DOS_BLACK;
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
			nes[0].p.ppu_screen.wr->line[y][x] = DOS_GREEN;
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
			if (((a >= nsf.effect_coords.y1) && (a <= nsf.effect_coords.y2)) && (nes[0].p.ppu_screen.wr->line[a][x] != DOS_GREEN)) {
				nes[0].p.ppu_screen.wr->line[a][x] = DOS_YELLOW;
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
	double multiplier = 0;

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
			nes[0].p.ppu_screen.wr->line[y][x] = DOS_BLACK;
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
			nes[0].p.ppu_screen.wr->line[y][x] = DOS_GREEN;
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
			if (((a >= nsf.effect_coords.y1) && (a <= nsf.effect_coords.y2)) && (nes[0].p.ppu_screen.wr->line[a][x] != DOS_GREEN)) {
				nes[0].p.ppu_screen.wr->line[a][x] = DOS_YELLOW;
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
			nes[0].p.ppu_screen.wr->line[y][x] = DOS_BLACK;
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
				nes[0].p.ppu_screen.wr->line[y][x] = DOS_GREEN;
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
				if (nes[0].p.ppu_screen.wr->line[a][x1] != DOS_GREEN) {
					nes[0].p.ppu_screen.wr->line[a][x1] = DOS_RED;
				}
				if (nes[0].p.ppu_screen.wr->line[a][x - 1] != DOS_GREEN) {
					nes[0].p.ppu_screen.wr->line[a][x - 1] = DOS_GRAY;
				}
			}
		}
	}
}
static void nsf_change_song(BYTE button, unsigned int mode) {
	if (nsf.timers.button[button] <= 0) {
		nsf_change_current_song(mode);
		nsf.state |= NSF_CHANGE_SONG;
		nsf.timers.button[button] = NFS_TIME_CHANGE_SONG;
	} else {
		nsf.timers.button[button] -= nsf.timers.diff;
	}
}
static void nsf_change_current_song(unsigned int mode) {
	if (cfg->nsf_player_playlist && (nsf.playlist.count > 0)) {
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
	BYTE force_draw = nes[0].p.ppu.frames < 2;
	_port *prt = &port[PORT1];
	uTCHAR buff[300];
	int y = 0;

	// titolo nsf - artista - copyright
	{
		if (force_draw) {
			y = 8;
			dos_hline(0, 0, y, SCR_COLUMNS, DOS_BROWN);
			dos_text(0, DOS_ALIGNHCENTER, y - 5, 0, 0, -1, -1,
				DOS_WHITE, DOS_BLACK, uL("Commodore 64 Pixelized"), 10,
				uL(" [yellow]p[red]u[green]N[cyan]E[brown]S[white] "
				"[bck][yellow][blue]%s[white][bck][black] Player "),
				info.format == NSFE_FORMAT ? "NSFe" : info.format == NSF2_FORMAT ? "NSF2" : "NSF");
		}
		dos_text_scroll_tick(0, 0, 14, DOS_WHITE, DOS_BLACK, uL("Ttyp0_13"), 13, &nsf.scroll_info_nsf,
			uL("[cyan]" uPs("") "[white] - [yellow]" uPs("") "[white] - " uPs("") ""),
			nsf.info.name, nsf.info.artist, nsf.info.copyright);
	}
	// player
	{
		if (force_draw) {
			y = 33;
			dos_hline(0, 0, y, SCR_COLUMNS, DOS_BROWN);
			dos_text(0, DOS_ALIGNHCENTER, y - 4, 0, 0, -1, -1, DOS_RED, DOS_BLACK, uL("pixelmix"), 8, uL(" Player "));
			dos_image(0, NSF_GUI_PPUX, NSF_GUI_PPUY, 0, 0, -1, -1, uL(":/pics/pics/nsf_player.png"), NULL, 0);
		}
		// note dell'autore
		if (nsf.info.text) {
			static int sauthnote = -1;

			if (force_draw || (sauthnote != (nsf.authors_note == PRESSED))) {
				dos_image(0, (NSF_GUI_PPUX + dos_resource_w(uL(":/pics/pics/nsf_player.png"))) - dos_resource_w(uL(":/pics/pics/authors_note_press.png")),
					NSF_GUI_PPUY - dos_resource_h(uL(":/pics/pics/authors_note_press.png")), 0, 0, -1, -1, nsf.authors_note == PRESSED
					? uL(":/pics/pics/authors_note_press.png") : uL(":/pics/pics/authors_note_no_press.png"), NULL, 0);
				sauthnote = nsf.authors_note == PRESSED;
			}
		}
		// titolo canzone
		if (nsf.current_song.track_label) {
			if (dos_text_pixels_w(uL("Ttyp0_11"), 11, nsf.current_song.track_label) > NSF_GUI_TITLE_W) {
				dos_text_scroll_tick(0, NSF_GUI_TITLE_PPUX, NSF_GUI_TITLE_PPUY, DOS_TL03, DOS_TL01,
					uL("Ttyp0_11"), 11, &nsf.scroll_title_song, nsf.current_song.track_label);
			} else if (nsf.curtain_title_song.count > 0) {
				dos_text_curtain(0, NSF_GUI_TITLE_PPUX, NSF_GUI_TITLE_PPUY, &nsf.curtain_title_song, DOS_TEXT_CURTAIN_TICK);
			}
		}
		// timers
		if (nsf.state) {
			double timer = nsf.timers.song;
			WORD fg_ok = DOS_TL03;
			WORD fg_no = DOS_TL01;
			WORD bg = DOS_TL02;
			static double ttimer1 = -1, ttimer2 = -1;

			if ((nsf.state & NSF_CHANGE_SONG) || (nsf.state & NSF_STOP)) {
				timer = 0;
			}
			if (ttimer1 != timer) {
				dos_text(0, NSF_GUI_TIMERS_1_PPUX, NSF_GUI_TIMERS_1_PPUY,
					DOS_ALIGNHCENTER, 0, NSF_GUI_TIMERS_1_W, NSF_GUI_TIMERS_1_H, fg_ok, fg_no, uL("lemon_10"), 10,
					uL("" uPs("") ""), nsf_print_time(timer, 0, fg_ok));
				ttimer1 = timer;
			}
			timer = nsf.info_song && nsf.current_song.use_timer
				? nsf.options.visual_duration ? nsf.current_song.time : (nsf.current_song.time - timer) : 0;
			if (ttimer2 != timer) {
				dos_text(0, NSF_GUI_TIMERS_2_PPUX, NSF_GUI_TIMERS_2_PPUY,
					DOS_ALIGNHCENTER, 0, NSF_GUI_TIMERS_2_W, NSF_GUI_TIMERS_2_H, fg_ok, fg_no, uL("lemon_10"), 10,
					uL("" uPs("") ""),
					nsf_print_time(timer, nsf.info_song && nsf.current_song.use_timer && !nsf.options.visual_duration, fg_ok));
				ttimer2 = timer;
				bg = !nsf.info_song || !nsf.current_song.use_timer || nsf.options.visual_duration ? DOS_TL02 : DOS_TL03;
				dos_box(0, NSF_GUI_TIMERS_3_PPUX, NSF_GUI_TIMERS_3_PPUY + 2, 2, 2, bg, bg, bg);
				bg = !nsf.info_song || !nsf.current_song.use_timer || !nsf.options.visual_duration ? DOS_TL02 : DOS_TL03;
				dos_box(0, NSF_GUI_TIMERS_3_PPUX, NSF_GUI_TIMERS_3_PPUY + 6, 2, 2, bg, bg, bg);
			}
		}
		// canzoni
		{
			BYTE condition = cfg->nsf_player_playlist && (nsf.playlist.count > 0);
			WORD bg = condition ? DOS_TL01 : DOS_GR02;
			WORD fg = condition ? DOS_TL02 : DOS_TL03;
			static int scurrent = -1, spindex = -1, scfgplaylist = -1;
			BYTE force = force_draw || (scfgplaylist != cfg->nsf_player_playlist);

			if (force || (scurrent != nsf.songs.current)) {
				umemset(buff, 0x00, usizeof(buff));
				ustrncat(buff, usizeof(buff), uL("Song "));
				ustrncat(buff, usizeof(buff), uL("" uPs("") "/"), nsf_print_number(nsf.songs.current + 1, 3, fg));
				ustrncat(buff, usizeof(buff), uL("" uPs("") ""), nsf_print_number(nsf.songs.total + 1, 3, fg));
				dos_text(0, NSF_GUI_SONGS_PPUX, NSF_GUI_SONGS_PPUY,
					DOS_ALIGNHCENTER, 0, NSF_GUI_SONGS_W, NSF_GUI_SONGS_H, fg, bg, uL("lemon_10"), 10, buff);
				scurrent = nsf.songs.current;
			}
			if (nsf.playlist.count > 0) {
				if (force || (spindex != nsf.playlist.index)) {
					condition = !cfg->nsf_player_playlist;
					bg = condition ? DOS_TL01 : DOS_GR02;
					fg = condition ? DOS_TL02 : DOS_TL03;
					umemset(buff, 0x00, usizeof(buff));
					usnprintf(buff, usizeof(buff), uL("Playlist "));
					ustrncat(buff, usizeof(buff), uL("" uPs("") "/"), nsf_print_number(nsf.playlist.index + 1, 3, fg));
					ustrncat(buff, usizeof(buff), uL("" uPs("") ""), nsf_print_number(nsf.playlist.count, 3, fg));
					dos_text(0, NSF_GUI_PLIST_PPUX, NSF_GUI_PLIST_PPUY,
						DOS_ALIGNHCENTER, 0, NSF_GUI_PLIST_W, NSF_GUI_PLIST_H, fg, bg, uL("lemon_10"), 10, buff);
					spindex = nsf.playlist.index;
					scfgplaylist = cfg->nsf_player_playlist;
				}
			}
		}
		// chips
		if (force_draw) {
			dos_text(0, NSF_GUI_CHIPS_PPUX + 5, NSF_GUI_CHIPS_PPUY, 0, 0, -1, -1,
				nsf.sound_chips.vrc6 ? DOS_TL03 : DOS_TL02, DOS_TL01, uL("Pixelated"), 8, uL("VRC6"));
			dos_text(0, NSF_GUI_CHIPS_PPUX + 30, NSF_GUI_CHIPS_PPUY, 0, 0, -1, -1,
				nsf.sound_chips.vrc7 ? DOS_TL03 : DOS_TL02, DOS_TL01, uL("Pixelated"), 8, uL("VRC7"));
			dos_text(0, NSF_GUI_CHIPS_PPUX + 55, NSF_GUI_CHIPS_PPUY, 0, 0, -1, -1,
				nsf.sound_chips.mmc5 ? DOS_TL03 : DOS_TL02, DOS_TL01, uL("Pixelated"), 8, uL("MMC5"));
			dos_text(0, NSF_GUI_CHIPS_PPUX + 84, NSF_GUI_CHIPS_PPUY, 0, 0, -1, -1,
				nsf.sound_chips.namco163 ? DOS_TL03 : DOS_TL02, DOS_TL01, uL("Pixelated"), 8, uL("N163"));
			dos_text(0, NSF_GUI_CHIPS_PPUX + 110, NSF_GUI_CHIPS_PPUY, 0, 0, -1, -1,
				nsf.sound_chips.sunsoft5b ? DOS_TL03 : DOS_TL02, DOS_TL01, uL("Pixelated"), 8, uL("S5B"));
			dos_text(0, NSF_GUI_CHIPS_PPUX + 131, NSF_GUI_CHIPS_PPUY, 0, 0, -1, -1,
				nsf.sound_chips.fds ? DOS_TL03 : DOS_TL02, DOS_TL01, uL("Pixelated"), 8, uL("FDS"));
			dos_text(0, NSF_GUI_CHIPS_PPUX + 150, NSF_GUI_CHIPS_PPUY, DOS_ALIGNHCENTER, 0, 32, 9,
				DOS_TL03, DOS_TL01, uL("Pixelated"), 8,
				nsf.type == NSF_NTSC_MODE ? uL("NTSC") : nsf.type == NSF_PAL_MODE ? uL("PAL") : uL("DENDY"));
		}
		// info
		dos_text_curtain(0, NSF_GUI_INFO_PPUX, NSF_GUI_INFO_PPUY, &nsf.curtain_info, DOS_TEXT_CURTAIN_TICK);
		// pulsanti
		{
			static int wplay = 0, wpause = 0, wstop = 0, wprev = 0, wnext = 0, wmute = 0;
			static int splay = -1, spause = -1, sstop = -1, sprev = -1, snext = -1, smute = -1, srepeat = -1;
			int x = NSF_GUI_BUTTONS_PPUX;

			if (!wplay) {
				wplay = dos_resource_w(uL(":/pics/pics/nsf_play_press.png"));
			}
			if (!wpause) {
				wpause = dos_resource_w(uL(":/pics/pics/nsf_pause_press.png"));
			}
			if (!wstop) {
				wstop = dos_resource_w(uL(":/pics/pics/nsf_stop_press.png"));
			}
			if (!wprev) {
				wprev = dos_resource_w(uL(":/pics/pics/nsf_prev_press.png"));
			}
			if (!wnext) {
				wnext = dos_resource_w(uL(":/pics/pics/nsf_next_press.png"));
			}
			if (!wmute) {
				wmute = dos_resource_w(uL(":/pics/pics/nsf_mute_press.png"));
			}

			if (force_draw || (splay != (nsf.state & NSF_PLAY))) {
				dos_image(0, x, NSF_GUI_BUTTONS_PPUY, 0, 0, -1, -1, nsf.state & NSF_PLAY
					? uL(":/pics/pics/nsf_play_press.png") : uL(":/pics/pics/nsf_play_no_press.png"), NULL, 0);
				splay = (nsf.state & NSF_PLAY);
			}
			x += wplay;
			if (force_draw || (spause != (nsf.state & NSF_PAUSE))) {
				dos_image(0, x, NSF_GUI_BUTTONS_PPUY, 0, 0, -1, -1, nsf.state & NSF_PAUSE
				 ? uL(":/pics/pics/nsf_pause_press.png") : uL(":/pics/pics/nsf_pause_no_press.png"), NULL, 0);
				spause = (nsf.state & NSF_PAUSE);
			}
			x += wpause;
			if (force_draw || (sstop != (nsf.state & NSF_STOP))) {
				dos_image(0, x, NSF_GUI_BUTTONS_PPUY, 0, 0, -1, -1, nsf.state & NSF_STOP
					? uL(":/pics/pics/nsf_stop_press.png") : uL(":/pics/pics/nsf_stop_no_press.png"), NULL, 0);
				sstop = (nsf.state & NSF_STOP);
			}
			x += wstop;
			if (force_draw || (sprev != (prt->data.treated[LEFT] == PRESSED))) {
				dos_image(0, x, NSF_GUI_BUTTONS_PPUY, 0, 0, -1, -1, prt->data.treated[LEFT] == PRESSED
					? uL(":/pics/pics/nsf_prev_press.png") : uL(":/pics/pics/nsf_prev_no_press.png"), NULL, 0);
				sprev = (prt->data.treated[LEFT] == PRESSED);
			}
			x += wprev;
			if (force_draw || (snext != (prt->data.treated[RIGHT] == PRESSED))) {
				dos_image(0, x, NSF_GUI_BUTTONS_PPUY, 0, 0, -1, -1, prt->data.treated[RIGHT] == PRESSED
					? uL(":/pics/pics/nsf_next_press.png") : uL(":/pics/pics/nsf_next_no_press.png"), NULL, 0);
				snext = (prt->data.treated[RIGHT] == PRESSED);
			}
			x += wnext;
			if (force_draw || (smute != !cfg->apu.channel[APU_MASTER])) {
				dos_image(0, x, NSF_GUI_BUTTONS_PPUY, 0, 0, -1, -1, !cfg->apu.channel[APU_MASTER]
					? uL(":/pics/pics/nsf_mute_press.png") : uL(":/pics/pics/nsf_mute_no_press.png"), NULL, 0);
				smute = !cfg->apu.channel[APU_MASTER];
			}
			x += wmute;
			if (force_draw || (srepeat != (nsf.repeat_song == PRESSED))) {
				dos_image(0, x, NSF_GUI_BUTTONS_PPUY, 0, 0, -1, -1, nsf.repeat_song == PRESSED
					? uL(":/pics/pics/nsf_repeat_song_press.png") : uL(":/pics/pics/nsf_repeat_song_no_press.png"), NULL, 0);
				srepeat = nsf.repeat_song == PRESSED;
			}
		}
	}
	// opzioni
	{
		static int sreverse = -1, splaylist = -1, sfadeout = -1;

		if (force_draw) {
			y = NSF_GUI_OPTIONS_PPUY + 7;
			dos_hline(0, 0, y, SCR_COLUMNS, DOS_BROWN);
			dos_text(0, DOS_ALIGNHCENTER, y - 5, 0, 0, -1, -1, DOS_RED, DOS_BLACK, uL("pixelmix"), 8, uL(" Options "));
		}
		if (force_draw || (sreverse != cfg->reverse_bits_dpcm)) {
			nsf_print_option(NSF_GUI_OPTIONS_1_PPUX, NSF_GUI_OPTIONS_BOX_PPUY,
				uL("Reverse bits DPCM"), TRUE, cfg->reverse_bits_dpcm);
			sreverse = cfg->reverse_bits_dpcm;
		}
		if (force_draw || (splaylist != cfg->nsf_player_playlist)) {
			nsf_print_option(NSF_GUI_OPTIONS_2_PPUX, NSF_GUI_OPTIONS_BOX_PPUY,
				uL("Playlist"), nsf.playlist.count > 0, cfg->nsf_player_playlist);
			splaylist = cfg->nsf_player_playlist;
		}
		if (force_draw || (sfadeout != cfg->nsf_player_nsf_fadeout)) {
			nsf_print_option(NSF_GUI_OPTIONS_3_PPUX, NSF_GUI_OPTIONS_BOX_PPUY,
				uL("Fadeout"), nsf.info_song && nsf.current_song.fade, cfg->nsf_player_nsf_fadeout);
			sfadeout = cfg->nsf_player_nsf_fadeout;
		}
	}
	// indicatore effetto
	{
		static int seffect = -1;

		if (force_draw || (seffect != cfg->nsf_player_effect)) {
			umemset(buff, 0x00, usizeof(buff));
			usnprintf(buff, usizeof(buff), uL(" [image]:/pics/pics/nsf_arrow_left.png[endimage] "));
			for (int i = 0; i < NSF_EFFECTS; i++) {
				ustrncat(buff, usizeof(buff), uL("" uPs("") "%d "),
					i == cfg->nsf_player_effect ? uL("[red]") : uL("[gy01]"), i + 1);
			}
			ustrncat(buff, usizeof(buff), uL("[image]:/pics/pics/nsf_arrow_right.png[endimage] "));
			y = 166;
			dos_hline(0, 0, y, SCR_COLUMNS, DOS_BROWN);
			dos_text(0, DOS_ALIGNHCENTER, y - 4, 0, 0, -1, -1, DOS_GRAY, DOS_BLACK, uL("lemon_10"), 10, buff);
			seffect = cfg->nsf_player_effect;
		}
	}
}
static uTCHAR *nsf_print_number(unsigned int song, BYTE decimal, WORD color) {
	static uTCHAR cbuff[30];
	BYTE ibuff[3], i = 0, is_normal = FALSE;

	ibuff[0] =  song / 100;
	ibuff[1] = (song - (ibuff[0] * 100)) / 10;
	ibuff[2] =  song - (ibuff[0] * 100) - (ibuff[1] * 10);

	if (decimal > 3) {
		decimal = 3;
	}

	umemset(cbuff, 0x00, usizeof(cbuff));

	for (i = 3 - decimal; i < 3; i++) {
		if (!is_normal && (ibuff[i] > 0)) {
			is_normal = TRUE;
			ustrncat(cbuff, usizeof(cbuff), uL("" uPs("") ""), dclr(color));
		}
		ustrncat(cbuff, usizeof(cbuff), uL("%d"), ibuff[i]);
	}

	return (cbuff);
}
static uTCHAR *nsf_print_time(double timer, BYTE mode, WORD color) {
	BYTE is_normal = FALSE;
	static uTCHAR buff[100];
	unsigned int i = 0;
	int tmp = 0;

	umemset(&buff[0], 0x00, usizeof(buff));

	if (timer < 0) {
		timer = 0;
	}

	for (i = 0; i < 4; i++) {
		BYTE is_last = FALSE;

		switch (i) {
			case 0:
				tmp = ((int)timer / (1000 * 60 * 60)) % 24;
				if (tmp > 0) {
					is_normal = TRUE;
				}
				usnprintf(buff, usizeof(buff), uL("[bck][tl01][tl02]"));
				ustrncat(buff, usizeof(buff), uL("" uPs("") "[tl02]:"), nsf_print_number(tmp, 2, color));
				continue;
			case 1:
				tmp = ((int)timer / (1000 * 60)) % 60;
				break;
			case 2:
				tmp = (int)(timer / 1000) % 60 ;
				break;
			case 3: {
				static int var = 0;

				tmp = ((int)timer + (timer > 0 ? (var++ % 10) : 0)) % 1000;
				is_last = TRUE;
				break;
			}
			default:
				continue;
		}
		if (is_normal) {
			if (is_last) {
				ustrncat(buff, usizeof(buff), uL("" uPs("") "%03d"), dclr(color), tmp);
			} else {
				ustrncat(buff, usizeof(buff), uL("" uPs("") "%02d"), dclr(color), tmp);
			}
		} else {
			if (tmp > 0) {
				is_normal = TRUE;
			}
			ustrncat(buff, usizeof(buff), uL("[tl02]" uPs("")), nsf_print_number(tmp, is_last ? 3 : 2, color));
		}
		if (!is_last) {
			ustrncat(buff, usizeof(buff), uL("[tl02]:"));
		}
	}

	ustrncat(buff, usizeof(buff), uL("" uPs("") "-"), (mode == 1) && (timer > 0) ? dclr(color) : uL("[tl02]"));

	return (buff);
}
static void nsf_print_option(int ppu_x, int ppu_y, uTCHAR *desc, BYTE active, BYTE mode) {
	int fg = active ? DOS_WHITE : DOS_GRAY;
	int bg = DOS_BLACK;
	int fg_box_int = active
		? mode ? DOS_GREEN : DOS_BLACK
		: mode ? DOS_GRAY : DOS_BLACK;

	dos_box(0, ppu_x, ppu_y + 1, NSF_GUI_OPTIONS_BOX_EXT_W, NSF_GUI_OPTIONS_BOX_EXT_H, fg, fg, bg);
	dos_box(0, ppu_x + 2, ppu_y + 1 + 2, NSF_GUI_OPTIONS_BOX_INT_W, NSF_GUI_OPTIONS_BOX_INT_H,
		fg_box_int, fg_box_int, fg_box_int);
	dos_text(0, ppu_x + 10, ppu_y, 0, 0, -1, -1, fg, bg, uL("pixelmix"), 8, desc);
}
static void nsf_reset_song_variables(void) {
	nsf.timers.song = 0;
	nsf.timers.silence = 0;
	nsf.timers.fadeout = 0;
	if (nsf.info_song) {
		nsf.scroll_title_song.timer = 0;
	} else {
		nsf.current_song.time = -1;
		nsf.current_song.fade = -1;
	}
	if (nsf.scroll_title_song.pimage.data) {
		free(nsf.scroll_title_song.pimage.data);
		nsf.scroll_title_song.pimage.data = NULL;
	}
	nsf_reset_song_title();
}
static void ustrncat(uTCHAR *buffer, size_t buffer_size, const uTCHAR *fmt, ...) {
	size_t strlen_buffer = 0, strlen_fmt = 0, available = 0;
	uTCHAR text[1024];
	va_list ap;

	umemset(text, 0x00, usizeof(text));

	va_start(ap, fmt);
	uvsnprintf(text, usizeof(text), fmt, ap);
	va_end(ap);

	strlen_buffer = ustrlen(buffer);
	strlen_fmt = ustrlen(text);
	available = (strlen_buffer + strlen_fmt) >= buffer_size
		? strlen_fmt - (buffer_size - strlen_buffer)
		: buffer_size - strlen_buffer;
	usnprintf(buffer + strlen_buffer, available, uL("" uPs("") ""), text);
}
