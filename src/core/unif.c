/*
 *  Copyright (C) 2010-2016 Fabio Cavallo (aka FHorse)
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
#include "info.h"
#include "mem_map.h"
#include "mappers.h"
#include "emu.h"
#include "conf.h"
#include "cheat.h"

enum unif_phase_type { UNIF_COUNT, UNIF_READ };
enum unif_no_types { NO_INES = 65535, NO_UNIF = 65535 };

BYTE unif_MAPR(FILE *fp, BYTE phase);
BYTE unif_NAME(FILE *fp, BYTE phase);
BYTE unif_PRG(FILE *fp, BYTE phase);
BYTE unif_CHR(FILE *fp, BYTE phase);
BYTE unif_TVCI(FILE *fp, BYTE phase);
BYTE unif_BATR(FILE *fp, BYTE phase);
BYTE unif_MIRR(FILE *fp, BYTE phase);
BYTE unif_DINF(FILE *fp, BYTE phase);

typedef struct _unif_board {
	char board[50];
	WORD ines_mapper;
	WORD unif_mapper;
	BYTE submapper;
	WORD id;
	WORD extra;
} _unif_board;

static const _unif_board unif_boards[] = {
	{"NROM", 0 , NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"NROM-128", 0, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"NROM-256", 0, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"Sachen-74LS374N", 150, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"UOROM", 2 , NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"TC-U01-1.5M", 147, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"SA-NROM", 143, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"SLROM", 1, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"22211", 132, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"TLROM", 4, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"TBROM", 4, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"TKROM", 4, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"Sachen-8259C", 139, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"SA-016-1M", 146, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"Sachen-8259D", 137, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"ANROM", 7, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"FK23C", 176, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"FK23CA", 176, NO_UNIF, DEFAULT, BMCFK23C_1 | BMCFK23CA, NOEXTRA},
	{"D1038", 60, NO_UNIF, MAP60_VT5201, DEFAULT, NOEXTRA},
	{"TEK90", 90, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"Sachen-8259A", 141, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"42in1ResetSwitch", 226, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"CNROM", 3, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"TFROM", 4, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"T-230", 23, NO_UNIF, VRC4T230, DEFAULT, NOEXTRA},
	{"SA-72008", 133, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"FS304", 162, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"SA-0037", 148, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"NTBROM", 68, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"SA-72007", 145, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"Sachen-8259B", 138, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"SuperHIK8in1", 45, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"Supervision16in1", 53, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"SA-0036", 149, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"SC-127", 35, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"H2288", 123, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},

	{"A65AS", NO_INES, 0, DEFAULT, DEFAULT, NOEXTRA},
	{"MARIO1-MALEE2", NO_INES, 1, DEFAULT, DEFAULT, NOEXTRA},
	{"TF1201", NO_INES, 2, DEFAULT, DEFAULT, NOEXTRA},
	{"EH8813A", NO_INES, 3, DEFAULT, DEFAULT, NOEXTRA},
	{"11160", NO_INES, 4, DEFAULT, DEFAULT, NOEXTRA},
	{"G-146", NO_INES, 5, DEFAULT, DEFAULT, NOEXTRA},
	{"12-IN-1", NO_INES, 6, DEFAULT, DEFAULT, NOEXTRA},
	{"411120-C", NO_INES, 7, DEFAULT, DEFAULT, NOEXTRA},
	{"T-262", NO_INES, 8, DEFAULT, DEFAULT, NOEXTRA},
	{"BS-5", NO_INES, 9, DEFAULT, DEFAULT, NOEXTRA},
	{"8157", NO_INES, 10, DEFAULT, DEFAULT, NOEXTRA},
	{"830118C", NO_INES, 11, DEFAULT, DEFAULT, NOEXTRA},
	{"8237", NO_INES, 12, DEFAULT, DEFAULT, NOEXTRA},
	{"8237A", NO_INES, 13, DEFAULT, DEFAULT, NOEXTRA},
	{"NTD-03", NO_INES, 14, DEFAULT, DEFAULT, NOEXTRA},
	{"Ghostbusters63in1", NO_INES, 15, DEFAULT, DEFAULT, NOEXTRA},
	{"64in1NoRepeat", NO_INES, 16, DEFAULT, DEFAULT, NOEXTRA},
	{"70in1", NO_INES, 17, DEFAULT, DEFAULT, NOEXTRA},
	{"70in1B", NO_INES, 18, DEFAULT, DEFAULT, NOEXTRA},
	{"KS7032", NO_INES, 19, DEFAULT, DEFAULT, NOEXTRA},
	{"KOF97", NO_INES, 20, DEFAULT, DEFAULT, NOEXTRA},
	{"603-5052", NO_INES, 21, DEFAULT, DEFAULT, NOEXTRA},
	{"CITYFIGHT", NO_INES, 22, DEFAULT, DEFAULT, NOEXTRA},
	{"BB", NO_INES, 23, DEFAULT, DEFAULT, NOEXTRA},
	{"43272", NO_INES, 24, DEFAULT, DEFAULT, NOEXTRA},
	{"AC08", NO_INES, 25, DEFAULT, DEFAULT, NOEXTRA},
	{"KS7013B", NO_INES, 26, DEFAULT, DEFAULT, NOEXTRA},
	{"MTECH01", NO_INES, 27, DEFAULT, DEFAULT, NOEXTRA},
	{"YOKO", NO_INES, 28, DEFAULT, DEFAULT, NOEXTRA},
	{"SA-9602B", NO_INES, 29, DEFAULT, DEFAULT, CHRRAM32K},
	{"CC-21", NO_INES, 30, DEFAULT, DEFAULT, NOEXTRA},
	{"LH32", NO_INES, 31, DEFAULT, DEFAULT, NOEXTRA},
	{"NovelDiamond9999999in1", NO_INES, 32, DEFAULT, DEFAULT, NOEXTRA},
	{"SL1632", NO_INES, 33, DEFAULT, DEFAULT, NOEXTRA},
	{"SHERO", NO_INES, 34, DEFAULT, DEFAULT, NOEXTRA},
	{"SMB2J", NO_INES, 35, DEFAULT, DEFAULT, NOEXTRA},
	{"AX5705", NO_INES, 36, DEFAULT, DEFAULT, NOEXTRA},
	{"GS-2004", NO_INES, 37, DEFAULT, DEFAULT, NOEXTRA},
	{"GS-2013", NO_INES, 38, DEFAULT, DEFAULT, NOEXTRA},
	{"KS7012", NO_INES, 39, DEFAULT, DEFAULT, NOEXTRA},
	{"KS7037", NO_INES, 40, DEFAULT, DEFAULT, NOEXTRA},
	{"KS7057", NO_INES, 41, DEFAULT, DEFAULT, NOEXTRA},
	{"KS7016", NO_INES, 42, DEFAULT, DEFAULT, NOEXTRA},
	{"KS7017", NO_INES, 43, DEFAULT, DEFAULT, NOEXTRA},
	{"LH10", NO_INES, 44, DEFAULT, DEFAULT, NOEXTRA},
	{"RT-01", NO_INES, 45, DEFAULT, DEFAULT, NOEXTRA},
	{"MALISB", NO_INES, 46, DEFAULT, DEFAULT, NOEXTRA},
	{"BOY", NO_INES, 47, DEFAULT, DEFAULT, CHRRAM256K},
	{"8-IN-1", NO_INES, 48, DEFAULT, DEFAULT, CHRRAM256K},
	{"HP898F", NO_INES, 49, DEFAULT, DEFAULT, NOEXTRA},
	{"158B", NO_INES, 50, DEFAULT, DEFAULT, NOEXTRA},
	{"810544-C-A1", NO_INES, 51, DEFAULT, DEFAULT, NOEXTRA},
	{"KS7031", NO_INES, 52, DEFAULT, DEFAULT, NOEXTRA},
};

BYTE unif_load_rom(void) {
	BYTE phase;
	FILE *fp;

	{
		BYTE i, found = TRUE;
		static const uTCHAR rom_ext[6][10] = {
			uL(".nes\0"),  uL(".NES\0"),
			uL(".unf\0"),  uL(".UNF\0"),
			uL(".unif\0"), uL(".UNIF\0")
		};

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

	if (cfg->cheat_mode == GAMEGENIE_MODE) {
		fp = gamegenie_load_rom(fp);
	}

	phase = 0;
	memset(&unif, 0x00, sizeof(unif));

	if (!(fread(&unif.header, sizeof(unif.header), 1, fp))) {
		;
	}

	phase = UNIF_COUNT;

	/* setto dei default */
	mirroring_H();
	info.machine[HEADER] = NTSC;
	info.prg.ram.bat.banks = 0;
	info.mapper.submapper = DEFAULT;
	info.mirroring_db = info.id = DEFAULT;

	if (strncmp(unif.header.identification, "UNIF", 4) == 0) {
		long position = ftell(fp);

		info.format = UNIF_FORMAT;

		for (phase = UNIF_COUNT; phase <= UNIF_READ; phase++) {
			fseek(fp, position, SEEK_SET);

			if (phase == UNIF_READ) {
				if (prg_chip_size(0) == 0) {
					fclose(fp);
					return (EXIT_ERROR);
				}

#if !defined (RELEASE)
				fprintf(stderr, "unif format\n");
#endif

				info.chr.rom[0].banks_8k = 0;
				info.prg.chips = info.chr.chips = 0;

				if (info.prg.ram.bat.banks) {
					info.prg.ram.banks_8k_plus = 1;
				}

				/* alloco la PRG Ram */
				if (map_prg_ram_malloc(0x2000) != EXIT_OK) {
					fclose(fp);
					return (EXIT_ERROR);
				}

				/* la CHR ram extra */
				memset(&chr.extra, 0x00, sizeof(chr.extra));
			}

			while(fread(&unif.chunk, sizeof(unif.chunk), 1, fp)) {
				if (strncmp(unif.chunk.id, "MAPR", 4) == 0) {
					unif_MAPR(fp, phase);
				} else if (strncmp(unif.chunk.id, "PRG", 3) == 0) {
					if (unif_PRG(fp, phase) == EXIT_ERROR) {
						fclose(fp);
						return (EXIT_ERROR);
					}
				} else if (strncmp(unif.chunk.id, "CHR", 3) == 0) {
					if (unif_CHR(fp, phase) == EXIT_ERROR) {
						fclose(fp);
						return (EXIT_ERROR);
					}
				} else if (strncmp(unif.chunk.id, "PCK", 3) == 0) {
					fseek(fp, unif.chunk.length, SEEK_CUR);
				} else if (strncmp(unif.chunk.id, "CCK", 3) == 0) {
					fseek(fp, unif.chunk.length, SEEK_CUR);
				} else if (strncmp(unif.chunk.id, "NAME", 4) == 0) {
					unif_NAME(fp, phase);
				} else if (strncmp(unif.chunk.id, "WRTR", 4) == 0) {
					fseek(fp, unif.chunk.length, SEEK_CUR);
				} else if (strncmp(unif.chunk.id, "READ", 4) == 0) {
					fseek(fp, unif.chunk.length, SEEK_CUR);
				} else if (strncmp(unif.chunk.id, "DINF", 4) == 0) {
					if (unif_DINF(fp, phase) == EXIT_ERROR) {
						fclose(fp);
						return (EXIT_ERROR);
					}
				} else if (strncmp(unif.chunk.id, "TVCI", 4) == 0) {
					unif_TVCI(fp, phase);
				} else if (strncmp(unif.chunk.id, "CTRL", 4) == 0) {
					fseek(fp, unif.chunk.length, SEEK_CUR);
				} else if (strncmp(unif.chunk.id, "BATR", 4) == 0) {
					unif_BATR(fp, phase);
				} else if (strncmp(unif.chunk.id, "VROR", 4) == 0) {
					fseek(fp, unif.chunk.length, SEEK_CUR);
				} else if (strncmp(unif.chunk.id, "MIRR", 4) == 0) {
					unif_MIRR(fp, phase);
				}
			}
		}

		if (!info.chr.rom[0].banks_1k) {
			mapper.write_vram = TRUE;
			if (info.extra_from_db & CHRRAM32K) {
				info.chr.rom[0].banks_8k = 4;
			} else if (info.extra_from_db & CHRRAM256K) {
				info.chr.rom[0].banks_8k = 32;
			} else {
				info.chr.rom[0].banks_8k = 1;
			}
			info.chr.rom[0].banks_4k = info.chr.rom[0].banks_8k * 2;
			info.chr.rom[0].banks_1k = info.chr.rom[0].banks_4k * 4;
			map_set_banks_max_chr(0);
		}

		info.prg.max_chips = info.prg.chips - 1;
		if (info.chr.chips > 0) {
			info.chr.max_chips = info.chr.chips - 1;
		}

		if (unif.finded == FALSE) {
			fclose(fp);
			return (EXIT_ERROR);
		}
	} else {
		fclose(fp);
		return (EXIT_ERROR);
	}

	fclose(fp);
	return (EXIT_OK);
}

BYTE unif_MAPR(FILE *fp, BYTE phase) {
	static const char strip[][5] = {
		"NES-", "UNL-", "HVC-", "BTL-", "BMC-"
	};

	if (phase == UNIF_COUNT) {
		fseek(fp, unif.chunk.length, SEEK_CUR);
		return (EXIT_OK);
	}

	memset(&unif.board[0], 0x00, sizeof(unif.board));

	if (unif.chunk.length < sizeof(unif.board)) {
		if (!(fread(&unif.board[0], unif.chunk.length, 1, fp))) {
			;
		}
	} else {
		if (!(fread(&unif.board[0], (sizeof(unif.board) - 1), 1, fp))) {
			;
		}
		fseek(fp, unif.chunk.length - (sizeof(unif.board) - 1), SEEK_CUR);
	}

	{
		static BYTE i;

		unif.stripped_board = &unif.board[0];

		for (i = 0; i < LENGTH(strip); i++) {
			if (strncmp(unif.board, &strip[i][0], strlen(strip[i])) == 0) {
				unif.stripped_board += strlen(strip[i]);
				break;
			}
		}
	}

	printf("board : %s\n", unif.board);

	{
		static WORD i;

		unif.finded = FALSE;

		for (i = 0; i < LENGTH(unif_boards); i++) {
			if (strcmp(unif.stripped_board, unif_boards[i].board) == 0) {
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

#if !defined (RELEASE)
	fprintf(stderr, "internal unif mapper : %u\n", unif.internal_mapper);
#endif

	return (EXIT_OK);
}
BYTE unif_NAME(FILE *fp, BYTE phase) {
	static size_t length;

	if (phase == UNIF_COUNT) {
		char buf = 0;

		length = 0;
		while (fread(&buf, 1, 1, fp)) {
			if (buf == 0) {
				break;
			}
			length++;
		}
		length++;
		return (EXIT_OK);
	}

	memset(&unif.name[0], 0x00, sizeof(unif.name));

	if (length < sizeof(unif.name)) {
		if (!(fread(&unif.name[0], length, 1, fp))) {
			;
		}
	} else {
		if (!(fread(&unif.name[0], (sizeof(unif.name) - 1), 1, fp))) {
			;
		}
		fseek(fp, length - (sizeof(unif.name) - 1), SEEK_CUR);
	}
	printf("name : %s\n", unif.name);

	return (EXIT_OK);
}
BYTE unif_PRG(FILE *fp, BYTE phase) {
	int chip = atoi(unif.chunk.id + 3);

	if (chip >= MAX_CHIPS) {
		return (EXIT_ERROR);
	}

	if (phase == UNIF_COUNT) {
		prg_chip_size(chip) = unif.chunk.length;
		fseek(fp, unif.chunk.length, SEEK_CUR);
	} else {
		/* alloco e carico la PRG Rom */
		if (map_prg_chip_malloc(chip, prg_chip_size(chip), 0x00) == EXIT_ERROR) {
			return (EXIT_ERROR);
		}
		if (!(fread(prg_chip(chip), prg_chip_size(chip), 1, fp))) {
			;
		}
		info.prg.rom[chip].banks_16k = prg_chip_size(chip) / (16 * 1024);
		info.prg.rom[chip].banks_8k = info.prg.rom[chip].banks_16k * 2;
		map_set_banks_max_prg(chip);

#if !defined (RELEASE)
		fprintf(stderr, "PRG chip %d : 8k rom = %u\n", chip, info.prg.rom[chip].banks_16k * 2);
#endif
	}

	return (EXIT_OK);
}
BYTE unif_CHR(FILE *fp, BYTE phase) {
	int chip = atoi(unif.chunk.id + 3);

	if (chip >= MAX_CHIPS) {
		return (EXIT_ERROR);
	}
	if (phase == UNIF_COUNT) {
		chr_chip_size(chip) = unif.chunk.length;
		fseek(fp, unif.chunk.length, SEEK_CUR);
	} else {
		/* alloco e carico la PRG Rom */
		if (map_chr_chip_malloc(chip, chr_chip_size(chip), 0x00) == EXIT_ERROR) {
			return (EXIT_ERROR);
		}
		if (!(fread(chr_chip(chip), chr_chip_size(chip), 1, fp))) {
			;
		}

		info.chr.rom[chip].banks_8k = chr_chip_size(chip) / (8 * 1024);
		info.chr.rom[chip].banks_4k = chr_chip_size(chip) / (4 * 1024);
		info.chr.rom[chip].banks_1k = chr_chip_size(chip) / (1 * 1024);
		map_set_banks_max_chr(chip);

		if (chip == 0) {
			map_chr_bank_1k_reset();
		}
#if !defined (RELEASE)
		fprintf(stderr, "CHR chip %d : 4k vrom = %u\n", chip, info.chr.rom[chip].banks_4k);
#endif
	}

	return (EXIT_OK);
}
BYTE unif_TVCI(FILE *fp, BYTE phase) {
	BYTE tv;

	if (phase == UNIF_COUNT) {
		fseek(fp, unif.chunk.length, SEEK_CUR);
		return (EXIT_OK);
	}

	if (unif.chunk.length != 1) {
		return (EXIT_ERROR);
	}

	if (!(fread(&tv, unif.chunk.length, 1, fp))) {
		;
	}

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
BYTE unif_BATR(FILE *fp, BYTE phase) {
	BYTE batr;

	if (phase == UNIF_COUNT) {
		fseek(fp, unif.chunk.length, SEEK_CUR);
		return (EXIT_OK);
	}

	batr = 0;
	if (!(fread(&batr, unif.chunk.length, 1, fp))) {
		;
	}

	info.prg.ram.bat.banks = batr & 0x01;

	return (EXIT_OK);
}
BYTE unif_MIRR(FILE *fp, BYTE phase) {
	BYTE mirr;

	if (phase == UNIF_COUNT) {
		fseek(fp, unif.chunk.length, SEEK_CUR);
		return (EXIT_OK);
	}

	mirr = 0;
	if (!(fread(&mirr, unif.chunk.length, 1, fp))) {
		;
	}

	switch (mirr) {
		default:
		case 0:
			mirroring_H();
			break;
		case 1:
			mirroring_V();
			break;
		case 2:
			mirroring_SCR0();
			break;
		case 3:
			mirroring_SCR1();
			break;
		case 4:
			mirroring_FSCR();
			break;
	}

	return (EXIT_OK);
}
BYTE unif_DINF(FILE *fp, BYTE phase) {
	char *months[12] = {
		"January",   "February", "March",    "April",
		"May",       "June",     "July",     "August",
	    "September", "October",  "November", "December"
	};

	if (phase == UNIF_COUNT) {
		fseek(fp, 204, SEEK_CUR);
		return (EXIT_OK);
	}

	if (fread(&unif.dumped, 1, 204, fp) != 204) {
		return (EXIT_ERROR);
	}

	unif.dumped.by[99] = 0;
	unif.dumped.with[99] = 0;

	printf("dumped by %s with %s on %s %d, %d\n", unif.dumped.by, unif.dumped.with,
			months[(unif.dumped.month - 1) % 12], unif.dumped.day, unif.dumped.year);

	return (EXIT_OK);
}
