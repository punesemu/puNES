/*  Copyright (C) 2010-2021 Fabio Cavallo (aka FHorse)
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

#include <string.h>
#include <stdlib.h>
#include "nsfe.h"
#include "rom_mem.h"
#define _NSF_STATIC_
#include "nsf.h"
#undef _NSF_STATIC_
#include "info.h"
#include "mem_map.h"
#include "mappers.h"
#include "gui.h"
#include "audio/blipbuf.h"
#include "patcher.h"

enum nsfe_phase_type { NSFE_COUNT, NSFE_READ };
enum nsfe_flags {
	LOAD_ADR_LO,
	LOAD_ADR_HI,
	INIT_ADR_LO,
	INIT_ADR_HI,
	PLAY_ADR_LO,
	PLAY_ADR_HI,
	PAL_NTSC_BITS,
	EXTRA_SOUND_CHIPS,
	TOTAL_SONGS,
	STARTING_SONGS,
	TOTAL_FL
};

BYTE nsfe_NONE(_rom_mem *rom, BYTE phase);
BYTE nsfe_INFO(_rom_mem *rom, BYTE phase);
BYTE nsfe_DATA(_rom_mem *rom, BYTE phase);
BYTE nsfe_BANK(_rom_mem *rom, BYTE phase);
BYTE nsfe_plst(_rom_mem *rom, BYTE phase);
BYTE nsfe_time(_rom_mem *rom, BYTE phase);
BYTE nsfe_fade(_rom_mem *rom, BYTE phase);
BYTE nsfe_tlbl(_rom_mem *rom, BYTE phase);
BYTE nsfe_auth(_rom_mem *rom, BYTE phase);
BYTE nsfe_text(_rom_mem *rom, BYTE phase);

struct _nsfe {
	struct _nsfe_chunk {
		uint32_t length;
		char id[4];
	} chunk;
} nsfe;

void nsfe_init(void) {
}
void nsfe_quit(void) {
	extcl_audio_samples_mod = NULL;
}
BYTE nsfe_load_rom(void) {
	_rom_mem rom;
	BYTE phase;

	{
		static const uTCHAR rom_ext[2][10] = { uL(".nsfe\0"), uL(".NSFE\0") };
		BYTE i, found = TRUE;
		FILE *fp;

		fp = ufopen(info.rom.file, uL("rb"));

		if (!fp) {
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

	if (strncmp((char *)rom.data, "NSFE", 4) == 0) {
		info.format = NSFE_FORMAT;

		info.machine[DATABASE] = DEFAULT;
		info.prg.ram.bat.banks = 0;
		info.prg.ram.banks_8k_plus = 0;

		nsf.info.name = &nsf_default_label[0];
		nsf.info.artist = &nsf_default_label[0];
		nsf.info.copyright = &nsf_default_label[0];
		nsf.info.ripper = &nsf_default_label[0];

		for (phase = NSFE_COUNT; phase <= NSFE_READ; phase++) {
			rom.position = 4;

			while ((rom.position + sizeof(nsfe.chunk)) <= rom.size) {
				rom_mem_memcpy(&nsfe.chunk, &rom, sizeof(nsfe.chunk));

				if (strncmp(nsfe.chunk.id, "INFO", 4) == 0) {
					if (nsfe_INFO(&rom, phase) == EXIT_ERROR) {
						free(rom.data);
						return (EXIT_ERROR);
					}
				} else if (strncmp(nsfe.chunk.id, "DATA", 4) == 0) {
					if (nsfe_DATA(&rom, phase) == EXIT_ERROR) {
						free(rom.data);
						return (EXIT_ERROR);
					}
				} else if (strncmp(nsfe.chunk.id, "NEND", 4) == 0) {
					if (nsfe_NONE(&rom, phase) == EXIT_ERROR) {
						free(rom.data);
						return (EXIT_ERROR);
					}
				} else if (strncmp(nsfe.chunk.id, "BANK", 4) == 0) {
					if (nsfe_BANK(&rom, phase)) {
						free(rom.data);
						return (EXIT_ERROR);
					}
				} else if (strncmp(nsfe.chunk.id, "plst", 4) == 0) {
					if (nsfe_plst(&rom, phase) == EXIT_ERROR) {
						free(rom.data);
						return (EXIT_ERROR);
					}
				} else if (strncmp(nsfe.chunk.id, "time", 4) == 0) {
					if (nsfe_time(&rom, phase) == EXIT_ERROR) {
						free(rom.data);
						return (EXIT_ERROR);
					}
				} else if (strncmp(nsfe.chunk.id, "fade", 4) == 0) {
					if (nsfe_fade(&rom, phase) == EXIT_ERROR) {
						free(rom.data);
						return (EXIT_ERROR);
					}
				} else if (strncmp(nsfe.chunk.id, "tlbl", 4) == 0) {
					if (nsfe_tlbl(&rom, phase) == EXIT_ERROR) {
						free(rom.data);
						return (EXIT_ERROR);
					}
				} else if (strncmp(nsfe.chunk.id, "auth", 4) == 0) {
					if (nsfe_auth(&rom, phase) == EXIT_ERROR) {
						free(rom.data);
						return (EXIT_ERROR);
					}
				} else if (strncmp(nsfe.chunk.id, "text", 4) == 0) {
					if (nsfe_text(&rom, phase) == EXIT_ERROR) {
						free(rom.data);
						return (EXIT_ERROR);
					}
				} else {
					// ignoro il typo di chunk non riconosciuto
					if (nsfe_NONE(&rom, phase) == EXIT_ERROR) {
						free(rom.data);
						return (EXIT_ERROR);
					}
				}
			}
		}

#if !defined (RELEASE)
		{
			BYTE tmp;

			fprintf(stderr, "nam : %s\n", nsf.info.name);
			fprintf(stderr, "art : %s\n", nsf.info.artist);
			fprintf(stderr, "cop : %s\n", nsf.info.copyright);
			fprintf(stderr, "rip : %s\n", nsf.info.ripper);
			fprintf(stderr, "txt : %s\n", nsf.text.data);
			fprintf(stderr, "pls : ");
			for (tmp = 0; tmp < nsf.playlist.count; tmp++) {
				if (tmp == 0) {
					fprintf(stderr, "%3d", nsf.playlist.data[tmp]);
				} else {
					fprintf(stderr, ",%3d", nsf.playlist.data[tmp]);
				}
			}
			fprintf(stderr, "\n");
			for (tmp = 0; tmp < nsf.songs.total; tmp++) {
				_nsf_info_song *song = &nsf.info_song[tmp];

				fprintf(stderr, "%3d : %7d %7d %s\n", tmp, song->time, song->fade, song->track_label);
			}
		}
#endif

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

		if (!nsf.sound_chips.fds && (nsf.adr.load < 0x8000)) {
			free(rom.data);
			return (EXIT_ERROR);
		}

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

		nsf.enabled = TRUE;

		if (nsf.playlist.count > 0) {
			nsf.songs.current = nsf.playlist.starting;
		} else {
			nsf.songs.current = nsf.songs.starting - 1;
		}

		memcpy(nsf.routine.prg, nsf_routine, 17);

		nsf.routine.prg[NSF_R_JMP_PLAY] = NSF_R_LOOP;

		info.mapper.id = NSF_MAPPER;
		info.cpu_rw_extern = TRUE;
	} else {
		free(rom.data);
		return (EXIT_ERROR);
	}

	free(rom.data);

	nsf_after_load_rom();

	return (EXIT_OK);
}

BYTE nsfe_NONE(_rom_mem *rom, BYTE phase) {
	if (phase == NSFE_COUNT) {
		if ((rom->position + nsfe.chunk.length) > rom->size) {
			return (EXIT_ERROR);
		}
		rom->position += nsfe.chunk.length;
		return (EXIT_OK);
	}

	rom->position += nsfe.chunk.length;

	return (EXIT_OK);
}
BYTE nsfe_INFO(_rom_mem *rom, BYTE phase) {
	BYTE flags[TOTAL_FL];

	if (nsfe.chunk.length < TOTAL_FL) {
		return (EXIT_ERROR);
	}

	if (rom_mem_ctrl_memcpy(&flags[0], rom, sizeof(flags)) == EXIT_ERROR) {
		return (EXIT_ERROR);
	}

	nsf.songs.total = flags[TOTAL_SONGS];
	nsf.songs.starting = flags[STARTING_SONGS] + 1;
	nsf.adr.load = (flags[LOAD_ADR_HI] << 8) | flags[LOAD_ADR_LO];
	nsf.adr.init = (flags[INIT_ADR_HI] << 8) | flags[INIT_ADR_LO];
	nsf.adr.play = (flags[PLAY_ADR_HI] << 8) | flags[PLAY_ADR_LO];

	nsf.type = flags[PAL_NTSC_BITS] & 0x03;

	nsf.sound_chips.vrc6 = flags[EXTRA_SOUND_CHIPS] & 0x01;
	nsf.sound_chips.vrc7 = flags[EXTRA_SOUND_CHIPS] & 0x02;
	nsf.sound_chips.fds = flags[EXTRA_SOUND_CHIPS] & 0x04;
	nsf.sound_chips.mmc5 = flags[EXTRA_SOUND_CHIPS] & 0x08;
	nsf.sound_chips.namco163 = flags[EXTRA_SOUND_CHIPS] & 0x10;
	nsf.sound_chips.sunsoft5b = flags[EXTRA_SOUND_CHIPS] & 0x20;

	if (!nsf.sound_chips.fds && (nsf.adr.load < 0x8000)) {
		return (EXIT_ERROR);
	}

	if (phase == NSFE_READ) {
		if (!(nsf.info_song = (_nsf_info_song *)malloc(nsf.songs.total * sizeof(_nsf_info_song)))) {
			fprintf(stderr, "Out of memory\n");
			return (EXIT_ERROR);
		}
		memset(nsf.info_song, 0x00, nsf.songs.total * sizeof(_nsf_info_song));
	}

	return (EXIT_OK);
}
BYTE nsfe_DATA(_rom_mem *rom, BYTE phase) {
	int padding = nsf.adr.load & 0x0FFF;
	int len_4k;

	if (phase == NSFE_COUNT) {
		if ((rom->position + nsfe.chunk.length) > rom->size) {
			return (EXIT_ERROR);
		}
		rom->position += nsfe.chunk.length;
		return (EXIT_OK);
	}

	nsf.prg.banks_4k = ((nsfe.chunk.length + padding) / 0x1000);

	if (((nsfe.chunk.length + padding) % 0x1000)) {
		nsf.prg.banks_4k++;
	}

	len_4k = nsf.prg.banks_4k * 0x1000;

	if (map_prg_chip_malloc(0, len_4k, 0x00) == EXIT_ERROR) {
		return (EXIT_ERROR);
	}

	memset(prg_chip(0), 0xF2, len_4k);

	rom_mem_memcpy(prg_chip(0) + padding, rom, nsfe.chunk.length);

	nsf.prg.banks_4k--;

	return (EXIT_OK);
}
BYTE nsfe_BANK(_rom_mem *rom, BYTE phase) {
	if (nsfe.chunk.length > 8) {
		nsfe.chunk.length = 8;
	}

	if (phase == NSFE_COUNT) {
		if ((rom->position + nsfe.chunk.length) > rom->size) {
			return (EXIT_ERROR);
		}
		rom->position += nsfe.chunk.length;
		return (EXIT_OK);
	}

	memset(&nsf.bankswitch.banks[0], 0x00, sizeof(nsf.bankswitch.banks));

	rom_mem_memcpy(&nsf.bankswitch.banks, rom, nsfe.chunk.length);

	if (nsf.bankswitch.banks[0] | nsf.bankswitch.banks[1] | nsf.bankswitch.banks[2] |
		nsf.bankswitch.banks[3] | nsf.bankswitch.banks[4] | nsf.bankswitch.banks[5] |
		nsf.bankswitch.banks[6] | nsf.bankswitch.banks[7]) {
		nsf.bankswitch.enabled = TRUE;
	}

	return (EXIT_OK);
}
BYTE nsfe_plst(_rom_mem *rom, BYTE phase) {
	if (phase == NSFE_COUNT) {
		if ((rom->position + nsfe.chunk.length) > rom->size) {
			return (EXIT_ERROR);
		}
		rom->position += nsfe.chunk.length;
		return (EXIT_OK);
	}

	if (!(nsf.playlist.data = (BYTE *)malloc(nsfe.chunk.length))) {
		fprintf(stderr, "Out of memory\n");
		return (EXIT_ERROR);
	}

	rom_mem_memcpy(nsf.playlist.data, rom, nsfe.chunk.length);

	nsf.playlist.index = 0;
	nsf.playlist.count = nsfe.chunk.length;
	nsf.playlist.starting = nsf.playlist.data[0];

	return (EXIT_OK);
}
BYTE nsfe_time(_rom_mem *rom, BYTE phase) {
	int i, total;

	if (phase == NSFE_COUNT) {
		if ((rom->position + nsfe.chunk.length) > rom->size) {
			return (EXIT_ERROR);
		}
		rom->position += nsfe.chunk.length;
		return (EXIT_OK);
	}

	total = nsfe.chunk.length / 4;

	if (total > nsf.songs.total) {
		total = nsf.songs.total;
	}

	for (i = 0; i < total; i++) {
		_nsf_info_song *song = &nsf.info_song[i];

		rom_mem_memcpy(&song->time, rom, 4);
		nsfe.chunk.length -= 4;
	}

	rom->position += nsfe.chunk.length;

	return (EXIT_OK);
}
BYTE nsfe_fade(_rom_mem *rom, BYTE phase) {
	int i, total;

	if (phase == NSFE_COUNT) {
		if ((rom->position + nsfe.chunk.length) > rom->size) {
			return (EXIT_ERROR);
		}
		rom->position += nsfe.chunk.length;
		return (EXIT_OK);
	}

	total = nsfe.chunk.length / 4;

	if (total > nsf.songs.total) {
		total = nsf.songs.total;
	}

	for (i = 0; i < total; i++) {
		_nsf_info_song *song = &nsf.info_song[i];

		rom_mem_memcpy(&song->fade, rom, 4);
		nsfe.chunk.length -= 4;
	}

	rom->position += nsfe.chunk.length;

	return (EXIT_OK);
}
BYTE nsfe_tlbl(_rom_mem *rom, BYTE phase) {
	unsigned int i, count;
	char *src;

	if (phase == NSFE_COUNT) {
		if ((rom->position + nsfe.chunk.length) > rom->size) {
			return (EXIT_ERROR);
		}
		rom->position += nsfe.chunk.length;
		return (EXIT_OK);
	}

	if (!(nsf.info.track_label = (char *)malloc(nsfe.chunk.length))) {
		fprintf(stderr, "Out of memory\n");
		return (EXIT_ERROR);
	}

	memset(nsf.info.track_label, 0x00, nsfe.chunk.length);

	rom_mem_memcpy(nsf.info.track_label, rom, nsfe.chunk.length);

	i = 0;
	src = nsf.info.track_label;

	for (count = 0; count < nsfe.chunk.length; count++) {
		if (nsf.info.track_label[count] == 0) {
			_nsf_info_song *song = &nsf.info_song[i];

			song->track_label = src;
			if (song->track_label[0] == 0) {
				song->track_label = &nsf_default_label[0];
			}
			src = nsf.info.track_label + count + 1;
			if (++i >= nsf.songs.total) {
				break;
			}
		}
	}

	for (;i < nsf.songs.total; i++) {
		_nsf_info_song *song = &nsf.info_song[i];

		song->track_label = &nsf_default_label[0];
	}

	return (EXIT_OK);
}
BYTE nsfe_auth(_rom_mem *rom, BYTE phase) {
	unsigned int i, count;
	char *src = NULL, **dst = NULL;

	if (phase == NSFE_COUNT) {
		if ((rom->position + nsfe.chunk.length) > rom->size) {
			return (EXIT_ERROR);
		}
		rom->position += nsfe.chunk.length;
		return (EXIT_OK);
	}

	if (!(nsf.info.auth = (char *)malloc(nsfe.chunk.length))) {
		fprintf(stderr, "Out of memory\n");
		return (EXIT_ERROR);
	}

	rom_mem_memcpy(nsf.info.auth, rom, nsfe.chunk.length);

	i = 0;
	src = nsf.info.auth;

	for (count = 0; count < nsfe.chunk.length; count++) {
		if (nsf.info.auth[count] == 0) {
			switch (i) {
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
			(*dst) = src;
			if ((*dst[0]) == 0) {
				(*dst) = &nsf_default_label[0];
			}
			src = nsf.info.auth + count + 1;
			if (++i >= 4) {
				break;
			}
		}
	}

	return (EXIT_OK);
}
BYTE nsfe_text(_rom_mem *rom, BYTE phase) {
	if (phase == NSFE_COUNT) {
		if ((rom->position + nsfe.chunk.length) > rom->size) {
			return (EXIT_ERROR);
		}
		rom->position += nsfe.chunk.length;
		return (EXIT_OK);
	}

	if (!(nsf.text.data = (BYTE *)malloc(nsfe.chunk.length))) {
		fprintf(stderr, "Out of memory\n");
		return (EXIT_ERROR);
	}

	rom_mem_memcpy(nsf.text.data, rom, nsfe.chunk.length);

	nsf.text.index = 0;
	nsf.text.count = nsfe.chunk.length;

	return (EXIT_OK);
}
