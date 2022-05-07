/*
 *  Copyright (C) 2010-2022 Fabio Cavallo (aka FHorse)
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
#include "unif.h"
#include "rom_mem.h"
#include "info.h"
#include "mem_map.h"
#include "mappers.h"
#include "emu.h"
#include "conf.h"
#include "cheat.h"
#include "vs_system.h"
#include "patcher.h"
#include "../../c++/crc/crc.h"

enum unif_phase_type { UNIF_COUNT, UNIF_READ };
enum unif_no_types { NO_INES = 65535, NO_UNIF = 65535 };

BYTE unif_NONE(_rom_mem *rom, BYTE phase);
BYTE unif_MAPR(_rom_mem *rom, BYTE phase);
BYTE unif_NAME(_rom_mem *rom, BYTE phase);
BYTE unif_PRG(_rom_mem *rom, BYTE phase);
BYTE unif_CHR(_rom_mem *rom, BYTE phase);
BYTE unif_TVCI(_rom_mem *rom, BYTE phase);
BYTE unif_BATR(_rom_mem *rom, BYTE phase);
BYTE unif_MIRR(_rom_mem *rom, BYTE phase);
BYTE unif_DINF(_rom_mem *rom, BYTE phase);

typedef struct _unif_board {
	char board[50];
	WORD ines_mapper;
	WORD unif_mapper;
	BYTE submapper;
	WORD id;
	WORD extra;
} _unif_board;

_unif unif;

static const _unif_board unif_boards[] = {
	{"NROM", 0 , NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"NROM-128", 0, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"NROM-256", 0, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"SLROM", 1, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"UOROM", 2 , NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"CNROM", 3, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"TLROM", 4, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"TBROM", 4, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"TKROM", 4, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"TFROM", 4, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"ANROM", 7, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"CHINA_ER_SAN2", 19, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"T-230", 23, NO_UNIF, VRC4T230, DEFAULT, NOEXTRA},
	{"SC-127", 35, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"SuperHIK8in1", 45, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"Supervision16in1", 53, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"D1038", 60, NO_UNIF, MAP60_VT5201, DEFAULT, NOEXTRA},
	{"NTBROM", 68, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"VRC7", 85, NO_UNIF, VRC7UNL, DEFAULT, NOEXTRA},
	{"TEK90", 90, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"H2288", 123, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"22211", 132, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"SA-72008", 133, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"Sachen-8259D", 137, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"Sachen-8259B", 138, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"Sachen-8259C", 139, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"Sachen-8259A", 141, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"SA-NROM", 143, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"SA-72007", 145, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"SA-016-1M", 146, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"TC-U01-1.5M", 147, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"SA-0037", 148, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"SA-0036", 149, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"Sachen-74LS374N", 150, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"FS304", 162, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"FK23C", 176, NO_UNIF, BMCFK23C, DEFAULT, NOEXTRA},
	{"FK23CA", 176, NO_UNIF, BMCFK23C, DEFAULT, NOEXTRA},
	{"Super24in1SC03", 176, NO_UNIF, LP8002KB, DEFAULT, NOEXTRA},
	{"WAIXING-FS005", 176, NO_UNIF, FS005, DEFAULT, NOEXTRA},
	{"JC-016-2", 205, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"N625092", 221, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"42in1ResetSwitch", 226, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"70in1", 236, NO_UNIF, BMC70IN1, DEFAULT, NOEXTRA},
	{"70in1B", 236, NO_UNIF, BMC70IN1B, DEFAULT, NOEXTRA},
	{"OneBus", 256, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"DANCE", 256, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"810544-C-A1", 261, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"CITYFIGHT", 266, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"COOLBOY", 268, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"GS-2004", 283, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"GS-2013", 283, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"BS-5", 286, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"411120-C", 287, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"K-3088", 287, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"DRAGONFIGHTER", 292, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"190in1", 300, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"SMB2J", 304, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"RESET-TXROM", 313, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"830425C-4391T", 320, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"K-3033", 322, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"FARID_SLROM_8-IN-1", 323, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"FARID_UNROM_8-IN-1", 324, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},	
	{"8-IN-1", 333, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"NEWSTAR-GRM070-8IN1", 333, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"G-146", 349, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"DREAMTECH01", 521, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},

	{"A65AS", NO_INES, 0, DEFAULT, DEFAULT, NOEXTRA},
	{"MARIO1-MALEE2", NO_INES, 1, DEFAULT, DEFAULT, NOEXTRA},
	{"TF1201", NO_INES, 2, DEFAULT, DEFAULT, NOEXTRA},
	{"EH8813A", NO_INES, 3, DEFAULT, DEFAULT, NOEXTRA},
	{"11160", NO_INES, 4, DEFAULT, DEFAULT, NOEXTRA},
	{"12-IN-1", NO_INES, 6, DEFAULT, DEFAULT, NOEXTRA},
	{"T-262", NO_INES, 8, DEFAULT, DEFAULT, NOEXTRA},
	{"8157", NO_INES, 10, DEFAULT, DEFAULT, NOEXTRA},
	{"830118C", NO_INES, 11, DEFAULT, DEFAULT, NOEXTRA},
	{"8237", NO_INES, 12, DEFAULT, DEFAULT, NOEXTRA},
	{"8237A", NO_INES, 13, DEFAULT, DEFAULT, NOEXTRA},
	{"NTD-03", NO_INES, 14, DEFAULT, DEFAULT, NOEXTRA},
	{"Ghostbusters63in1", NO_INES, 15, DEFAULT, DEFAULT, NOEXTRA},
	{"64in1NoRepeat", NO_INES, 16, DEFAULT, DEFAULT, NOEXTRA},
	{"KS7032", NO_INES, 19, DEFAULT, DEFAULT, NOEXTRA},
	{"KOF97", NO_INES, 20, DEFAULT, DEFAULT, NOEXTRA},
	{"603-5052", NO_INES, 21, DEFAULT, DEFAULT, NOEXTRA},
	{"BB", NO_INES, 23, DEFAULT, DEFAULT, NOEXTRA},
	{"43272", NO_INES, 24, DEFAULT, DEFAULT, NOEXTRA},
	{"AC08", NO_INES, 25, DEFAULT, DEFAULT, NOEXTRA},
	{"KS7013B", NO_INES, 26, DEFAULT, DEFAULT, NOEXTRA},
	{"MTECH01", NO_INES, 27, DEFAULT, DEFAULT, NOEXTRA},
	{"YOKO", NO_INES, 28, DEFAULT, DEFAULT, NOEXTRA},
	{"SA-9602B", NO_INES, 29, DEFAULT, DEFAULT, NOEXTRA},
	{"CC-21", NO_INES, 30, DEFAULT, DEFAULT, NOEXTRA},
	{"LH32", NO_INES, 31, DEFAULT, DEFAULT, NOEXTRA},
	{"NovelDiamond9999999in1", NO_INES, 32, DEFAULT, DEFAULT, NOEXTRA},
	{"SL1632", NO_INES, 33, DEFAULT, DEFAULT, NOEXTRA},
	{"SHERO", NO_INES, 34, DEFAULT, DEFAULT, NOEXTRA},
	{"AX5705", NO_INES, 36, DEFAULT, DEFAULT, NOEXTRA},
	{"KS7012", NO_INES, 39, DEFAULT, DEFAULT, NOEXTRA},
	{"KS7037", NO_INES, 40, DEFAULT, DEFAULT, NOEXTRA},
	{"KS7057", NO_INES, 41, DEFAULT, DEFAULT, NOEXTRA},
	{"KS7016", NO_INES, 42, DEFAULT, DEFAULT, NOEXTRA},
	{"KS7017", NO_INES, 43, DEFAULT, DEFAULT, NOEXTRA},
	{"LH10", NO_INES, 44, DEFAULT, DEFAULT, NOEXTRA},
	{"RT-01", NO_INES, 45, DEFAULT, DEFAULT, NOEXTRA},
	{"MALISB", NO_INES, 46, DEFAULT, DEFAULT, NOEXTRA},
	{"BOY", NO_INES, 47, DEFAULT, DEFAULT, NOEXTRA},
	{"HP898F", NO_INES, 49, DEFAULT, DEFAULT, NOEXTRA},
	{"158B", NO_INES, 50, DEFAULT, DEFAULT, NOEXTRA},
	{"KS7031", NO_INES, 52, DEFAULT, DEFAULT, NOEXTRA},
	{"EDU2000", NO_INES, 55, DEFAULT, DEFAULT, NOEXTRA},
};

BYTE unif_load_rom(void) {
	_rom_mem rom;
	BYTE phase;

	{
		FILE *fp;
		BYTE i, found = TRUE;
		static const uTCHAR rom_ext[6][10] = {
			uL(".nes\0"),  uL(".NES\0"),
			uL(".unf\0"),  uL(".UNF\0"),
			uL(".unif\0"), uL(".UNIF\0")
		};

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

	if (cfg->cheat_mode == GAMEGENIE_MODE) {
		gamegenie_load_rom(&rom);
	}

	patcher_apply(&rom);

	rom.position = 0;

	memset(&unif, 0x00, sizeof(unif));

	if (rom_mem_ctrl_memcpy(&unif.header, &rom, sizeof(unif.header)) == EXIT_ERROR) { 
		free(rom.data);
		return (EXIT_ERROR);
	}

	phase = UNIF_COUNT;

	// setto i defaults
	mirroring_H();
	info.machine[HEADER] = NTSC;
	info.prg.ram.bat.banks = 0;
	info.mapper.submapper = DEFAULT;
	info.mirroring_db = info.id = DEFAULT;
	info.extra_from_db = 0;
	info.chr.rom.banks_8k = 0;
	info.chr.ram.banks_8k_plus = 0;
	info.prg.chips = info.chr.chips = 0;
	vs_system.enabled = FALSE;

	if (strncmp(unif.header.identification, "UNIF", 4) == 0) {
		info.format = UNIF_FORMAT;

		for (phase = UNIF_COUNT; phase <= UNIF_READ; phase++) {
			rom.position = sizeof(unif.header);

			unif.chips.prg = 0;
			unif.chips.chr = 0;

			if (phase == UNIF_READ) {
				size_t size;
				BYTE i;

				if (prg_chip_size(0) == 0) {
					free(rom.data);
					return (EXIT_ERROR);
				}

				fprintf(stderr, "\n");
				fprintf(stderr, "format        : UNIF\n");

				// PRG
				{
					for (i = size = 0; i < info.prg.chips; i++) {
						size += prg_chip_size(i);
					}

					if ((size == 0) || (map_prg_malloc(size, 0x00, FALSE) == EXIT_ERROR)) {
						free(rom.data);
						return (EXIT_ERROR);
					}

					info.prg.rom.banks_16k = prg_size() / 0x4000;
					info.prg.rom.banks_8k = prg_size() / 0x2000;
					map_set_banks_max_prg();
				}

				// CHR
				{
					for (i = size = 0; i < info.chr.chips; i++) {
						size += chr_chip_size(i);
					}

					if (size > 0) {
						if (map_chr_malloc(size, 0x00, FALSE) == EXIT_ERROR) {
							free(rom.data);
							return (EXIT_ERROR);
						}
						info.chr.rom.banks_8k = chr_size() / 0x2000;
						info.chr.rom.banks_4k = chr_size() / 0x1000;
						info.chr.rom.banks_1k = chr_size() / 0x0400;
						map_set_banks_max_chr();

						map_chr_bank_1k_reset();
					}
				}

				if (info.prg.ram.bat.banks) {
					info.prg.ram.banks_8k_plus = 1;
				}

				// alloco la PRG Ram
				if (map_prg_ram_malloc(0x2000) != EXIT_OK) {
					free(rom.data);
					return (EXIT_ERROR);
				}

				// la CHR ram extra
				memset(&chr.extra, 0x00, sizeof(chr.extra));
			}

			while ((rom.position + sizeof(unif.chunk)) <= rom.size) {
				rom_mem_memcpy(&unif.chunk, &rom, sizeof(unif.chunk));

				if (strncasecmp(unif.chunk.id, "MAPR", 4) == 0) {
					if (unif_MAPR(&rom, phase) == EXIT_ERROR) {
						free(rom.data);
						return (EXIT_ERROR);
					}
				} else if (strncasecmp(unif.chunk.id, "PRG", 3) == 0) {
					if (unif_PRG(&rom, phase) == EXIT_ERROR) {
						free(rom.data);
						return (EXIT_ERROR);
					}
				} else if (strncasecmp(unif.chunk.id, "CHR", 3) == 0) {
					if (unif_CHR(&rom, phase) == EXIT_ERROR) {
						free(rom.data);
						return (EXIT_ERROR);
					}
				} else if (strncasecmp(unif.chunk.id, "PCK", 3) == 0) {
					if (unif_NONE(&rom, phase) == EXIT_ERROR) {
						free(rom.data);
						return (EXIT_ERROR);
					}
				} else if (strncasecmp(unif.chunk.id, "CCK", 3) == 0) {
					if (unif_NONE(&rom, phase) == EXIT_ERROR) {
						free(rom.data);
						return (EXIT_ERROR);
					}
				} else if (strncasecmp(unif.chunk.id, "NAME", 4) == 0) {
					if (unif_NAME(&rom, phase) == EXIT_ERROR) {
						free(rom.data);
						return (EXIT_ERROR);
					}
				} else if (strncasecmp(unif.chunk.id, "WRTR", 4) == 0) {
					if (unif_NONE(&rom, phase) == EXIT_ERROR) {
						free(rom.data);
						return (EXIT_ERROR);
					}
				} else if (strncasecmp(unif.chunk.id, "READ", 4) == 0) {
					if (unif_NONE(&rom, phase) == EXIT_ERROR) {
						free(rom.data);
						return (EXIT_ERROR);
					}
				} else if (strncasecmp(unif.chunk.id, "DINF", 4) == 0) {
					if (unif_DINF(&rom, phase) == EXIT_ERROR) {
						free(rom.data);
						return (EXIT_ERROR);
					}
				} else if (strncasecmp(unif.chunk.id, "TVCI", 4) == 0) {
					if (unif_TVCI(&rom, phase) == EXIT_ERROR) {
						free(rom.data);
						return (EXIT_ERROR);
					}
				} else if (strncasecmp(unif.chunk.id, "CTRL", 4) == 0) {
					if (unif_NONE(&rom, phase) == EXIT_ERROR) {
						free(rom.data);
						return (EXIT_ERROR);
					}
				} else if (strncasecmp(unif.chunk.id, "BATR", 4) == 0) {
					if (unif_BATR(&rom, phase) == EXIT_ERROR) {
						free(rom.data);
						return (EXIT_ERROR);
					}
				} else if (strncasecmp(unif.chunk.id, "VROR", 4) == 0) {
					if (unif_NONE(&rom, phase) == EXIT_ERROR) {
						free(rom.data);
						return (EXIT_ERROR);
					}
				} else if (strncasecmp(unif.chunk.id, "MIRR", 4) == 0) {
					if (unif_MIRR(&rom, phase) == EXIT_ERROR) {
						free(rom.data);
						return (EXIT_ERROR);
					}
				} else {
					// ignoro il tipo di chunk non riconosciuto
					if (unif_NONE(&rom, phase) == EXIT_ERROR) {
						free(rom.data);
						return (EXIT_ERROR);
					}
				}
			}
		}

		fprintf(stderr, "PRG 8k rom    : %-4lu [ %08X %ld ]\n",
			(long unsigned)prg_size() / 0x2000,
			emu_crc32((void *)prg_rom(), prg_size()),
			(long)prg_size());
		if (chr_size()) {
			fprintf(stderr, "CHR 4k vrom   : %-4lu [ %08X %ld ]\n",
				(long unsigned)chr_size() / 0x1000,
				emu_crc32((void *)chr_rom(), chr_size()),
				(long)chr_size());
		}

		fprintf(stderr, "\n");

		if (!info.chr.rom.banks_1k) {
			mapper.write_vram = TRUE;
		}

		if (unif.finded == FALSE) {
			free(rom.data);
			return (EXIT_ERROR);
		}
	} else {
		free(rom.data);
		return (EXIT_ERROR);
	}

	free(rom.data);
	return (EXIT_OK);
}

BYTE unif_NONE(_rom_mem *rom, BYTE phase) {
	if (phase == UNIF_COUNT) {
		if ((rom->position + unif.chunk.length) > rom->size) {
			return (EXIT_ERROR);
		}
		rom->position += unif.chunk.length;
		return (EXIT_OK);
	}

	rom->position += unif.chunk.length;

	return (EXIT_OK);
}
BYTE unif_MAPR(_rom_mem *rom, BYTE phase) {
	static const char strip[][5] = {
		"NES-", "UNL-", "HVC-", "BTL-", "BMC-"
	};

	if (phase == UNIF_COUNT) {
		if ((rom->position + unif.chunk.length) > rom->size) {
			return (EXIT_ERROR);
		}
		rom->position += unif.chunk.length;
		return (EXIT_OK);
	}

	memset(&unif.board[0], 0x00, sizeof(unif.board));

	if (unif.chunk.length < sizeof(unif.board)) {
		memcpy(&unif.board[0], rom->data + rom->position, unif.chunk.length);
	} else {
		memcpy(&unif.board[0], rom->data + rom->position, (sizeof(unif.board) - 1));
	}
	rom->position += unif.chunk.length;

	{
		static BYTE i;
		char *trimmed;

		unif.stripped_board = &unif.board[0];
		trimmed = &unif.board[0];

		for (i = 0; i < strlen(unif.stripped_board); i++) {
			if ((*unif.stripped_board) != ' ') {
				break;
			}
			unif.stripped_board++;
			trimmed++;
		}

		for (i = 0; i < LENGTH(strip); i++) {
			if (strncasecmp(unif.stripped_board, &strip[i][0], strlen(strip[i])) == 0) {
				unif.stripped_board += strlen(strip[i]);
				break;
			}
		}

		fprintf(stderr, "board         : %s\n", trimmed);
	}

	{
		static WORD i;

		unif.finded = FALSE;

		for (i = 0; i < LENGTH(unif_boards); i++) {
			if (strncasecmp(unif.stripped_board, unif_boards[i].board, strlen(unif.stripped_board)) == 0) {
				if (unif_boards[i].ines_mapper == NO_INES) {
					info.mapper.id = UNIF_MAPPER;
				} else {
					info.mapper.id = unif_boards[i].ines_mapper;
				}
				info.mapper.submapper = unif_boards[i].submapper;
				info.id = unif_boards[i].id;
				info.extra_from_db = unif_boards[i].extra;
				unif.internal_mapper = unif_boards[i].unif_mapper;
				unif.finded = TRUE;
				break;
			}
		}
	}

	if (unif.internal_mapper == NO_UNIF) {
		fprintf(stderr, "unif mapper   : UNDEF\n");
	} else {
		fprintf(stderr, "unif mapper   : %u\n", unif.internal_mapper);
	}
	if (info.mapper.id == UNIF_MAPPER) {
		fprintf(stderr, "nes mapper    : UNDEF\n");
	} else {
		fprintf(stderr, "nes mapper    : %u\n", info.mapper.id);
	}
	if (info.mapper.submapper == DEFAULT) {
		fprintf(stderr, "submapper     : DEFAULT\n");
	} else {
		fprintf(stderr, "submapper     : %u\n", info.mapper.submapper);
	}

	return (EXIT_OK);
}
BYTE unif_NAME(_rom_mem *rom, BYTE phase) {
	static size_t length;

	if (phase == UNIF_COUNT) {
		BYTE buf = 0;

		length = 0;

		while ((buf = rom->data[rom->position]) > 0) {
			if ((rom->position + 1) < rom->size) {
				rom->position++;
				length++;
				continue;
			}
			return (EXIT_ERROR);
		}
		rom->position++;
		length++;
		return (EXIT_OK);
	}

	memset(&unif.name[0], 0x00, sizeof(unif.name));

	if (length < sizeof(unif.name)) {
		memcpy(&unif.name[0], rom->data + rom->position, length);
	} else {
		memcpy(&unif.name[0], rom->data + rom->position, (sizeof(unif.name) - 1));
	}
	rom->position += length;

	fprintf(stderr, "name          : %s\n", unif.name);

	return (EXIT_OK);
}
BYTE unif_PRG(_rom_mem *rom, BYTE phase) {
	int real_chip = atoi(unif.chunk.id + 3);
	int chip = unif.chips.prg;

	if (chip >= MAX_CHIPS) {
		return (EXIT_ERROR);
	}

	if (phase == UNIF_COUNT) {
		if ((rom->position + unif.chunk.length) > rom->size) {
			return (EXIT_ERROR);
		}
		info.prg.chips = ++unif.chips.prg;
		prg_chip_size(chip) = unif.chunk.length;
		rom->position += unif.chunk.length;
		return (EXIT_OK);
	} else {
		BYTE i;

		unif.chips.prg++;
		for (i = 0, prg_chip_rom(chip) = prg_rom(); i < chip; i++) {
			prg_chip_rom(chip) += prg_chip_size(i);
		}
		rom_mem_memcpy(prg_chip_rom(chip), rom, prg_chip_size(chip));
		fprintf(stderr, "PRG %d 8k rom  : %-4lu [ %08X %ld ]\n",
			real_chip, (long unsigned)prg_chip_size(chip) / 0x2000,
			emu_crc32((void *)prg_chip_rom(chip), prg_chip_size(chip)),
			(long)prg_chip_size(chip));
	}

	return (EXIT_OK);
}
BYTE unif_CHR(_rom_mem *rom, BYTE phase) {
	int real_chip = atoi(unif.chunk.id + 3);
	int chip = unif.chips.chr;

	if (chip >= MAX_CHIPS) {
		return (EXIT_ERROR);
	}

	if (phase == UNIF_COUNT) {
		if ((rom->position + unif.chunk.length) > rom->size) {
			return (EXIT_ERROR);
		}
		info.chr.chips = ++unif.chips.chr;
		chr_chip_size(chip) = unif.chunk.length;
		rom->position += unif.chunk.length;
		return (EXIT_OK);
	} else {
		BYTE i;

		unif.chips.chr++;
		for (i = 0, chr_chip_rom(chip) = chr_rom(); i < chip; i++) {
			chr_chip_rom(chip) += chr_chip_size(i);
		}
		rom_mem_memcpy(chr_chip_rom(chip), rom, chr_chip_size(chip));
		fprintf(stderr, "CHR %d 4k vrom : %-4lu [ %08X %ld ]\n",
			real_chip, (long unsigned)chr_chip_size(chip) / 0x1000,
			emu_crc32((void *)chr_chip_rom(chip), chr_chip_size(chip)),
			(long)chr_chip_size(chip));
	}

	return (EXIT_OK);
}
BYTE unif_TVCI(_rom_mem *rom, BYTE phase) {
	BYTE tv;

	if (phase == UNIF_COUNT) {
		if ((rom->position + unif.chunk.length) > rom->size) {
			return (EXIT_ERROR);
		}
		rom->position += unif.chunk.length;
		return (EXIT_OK);
	}

	if (unif.chunk.length != 1) {
		return (EXIT_ERROR);
	}

	rom_mem_memcpy(&tv, rom, unif.chunk.length);

	switch (tv) {
		default:
		case 0:
			info.machine[HEADER] = NTSC;
			break;
		case 1:
			info.machine[HEADER] = PAL;
			break;
	}

	return (EXIT_OK);
}
BYTE unif_BATR(_rom_mem *rom, BYTE phase) {
	BYTE batr;

	if (phase == UNIF_COUNT) {
		if ((rom->position + unif.chunk.length) > rom->size) {
			return (EXIT_ERROR);
		}
		rom->position += unif.chunk.length;
		return (EXIT_OK);
	}

	batr = 0;

	rom_mem_memcpy(&batr, rom, unif.chunk.length);

	info.prg.ram.bat.banks = batr & 0x01;

	return (EXIT_OK);
}
BYTE unif_MIRR(_rom_mem *rom, BYTE phase) {
	BYTE mirr;

	if (phase == UNIF_COUNT) {
		if ((rom->position + unif.chunk.length) > rom->size) {
			return (EXIT_ERROR);
		}
		rom->position += unif.chunk.length;
		return (EXIT_OK);
	}

	mirr = 0;

	rom_mem_memcpy(&mirr, rom, unif.chunk.length);

	switch (mirr) {
		default:
		case 0:
			mirroring_H();
			fprintf(stderr, "mirroring     : horizontal\n");
			break;
		case 1:
			mirroring_V();
			fprintf(stderr, "mirroring     : vertical\n");
			break;
		case 2:
			mirroring_SCR0();
			fprintf(stderr, "mirroring     : scr0\n");
			break;
		case 3:
			mirroring_SCR1();
			fprintf(stderr, "mirroring     : scr1\n");
			break;
		case 4:
			mirroring_FSCR();
			fprintf(stderr, "mirroring     : 4 screen\n");
			break;
		case 5:
			mirroring_H();
			fprintf(stderr, "mirroring     : controlled by the mapper\n");
			break;
	}

	return (EXIT_OK);
}
BYTE unif_DINF(_rom_mem *rom, BYTE phase) {
	char *months[12] = {
		"January",   "February", "March",    "April",
		"May",       "June",     "July",     "August",
		"September", "October",  "November", "December"
	};

	if (phase == UNIF_COUNT) {
		if ((rom->position + 204) > rom->size) {
			return (EXIT_ERROR);
		}
		rom->position += 204;
		return (EXIT_OK);
	}

	rom_mem_memcpy(&unif.dumped, rom, 204);

	unif.dumped.by[99] = 0;
	unif.dumped.with[99] = 0;

	fprintf(stderr, "dumped by     : %s with %s on %s %d, %d\n", unif.dumped.by, unif.dumped.with,
		months[(unif.dumped.month - 1) % 12], unif.dumped.day, unif.dumped.year);

	return (EXIT_OK);
}
