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

BYTE unif_MAPR(FILE *fp, BYTE phase);
BYTE unif_NAME(FILE *fp, BYTE phase);
BYTE unif_PRG(FILE *fp, BYTE phase);
BYTE unif_CHR(FILE *fp, BYTE phase);
BYTE unif_TVCI(FILE *fp, BYTE phase);
BYTE unif_BATR(FILE *fp, BYTE phase);
BYTE unif_MIRR(FILE *fp, BYTE phase);

enum {
	NO_INES = 65535,
	NO_UNIF = 65535,
};
typedef struct {
	char board[50];
	WORD ines_mapper;
	WORD unif_mapper;
	BYTE submapper;
	WORD id;
} _unif_board;

static const _unif_board unif_boards[] = {
	{"NROM", 0 , NO_UNIF, DEFAULT, DEFAULT},
	{"NROM-128", 0, NO_UNIF, DEFAULT, DEFAULT},
	{"NROM-256", 0, NO_UNIF, DEFAULT, DEFAULT},
	{"Sachen-74LS374N", 150, NO_UNIF, DEFAULT, DEFAULT},
	{"UOROM", 2 , NO_UNIF, DEFAULT, DEFAULT},
	{"TC-U01-1.5M", 147, NO_UNIF, DEFAULT, DEFAULT},
	{"SA-NROM", 143, NO_UNIF, DEFAULT, DEFAULT},
	{"SLROM", 1, NO_UNIF, DEFAULT, DEFAULT},
	{"22211", 132, NO_UNIF, DEFAULT, DEFAULT},
	{"TLROM", 4, NO_UNIF, DEFAULT, DEFAULT},
	{"TBROM", 4, NO_UNIF, DEFAULT, DEFAULT},
	{"TKROM", 4, NO_UNIF, DEFAULT, DEFAULT},
	{"Sachen-8259C", 139, NO_UNIF, DEFAULT, DEFAULT},
	{"SA-016-1M", 146, NO_UNIF, DEFAULT, DEFAULT},
	{"Sachen-8259D", 137, NO_UNIF, DEFAULT, DEFAULT},
	{"ANROM", 7, NO_UNIF, DEFAULT, DEFAULT},
	{"FK23C", 176, NO_UNIF, DEFAULT, DEFAULT},
	{"FK23CA", 176, NO_UNIF, DEFAULT, BMCFK23C_1 | BMCFK23CA},
	{"D1038", 60, NO_UNIF, MAP60_VT5201, DEFAULT},
	{"TEK90", 90, NO_UNIF, DEFAULT, DEFAULT},
	{"Sachen-8259A", 141, NO_UNIF, DEFAULT, DEFAULT},
	{"42in1ResetSwitch", 226, NO_UNIF, DEFAULT, DEFAULT},

	{"A65AS", NO_INES, 0, DEFAULT, DEFAULT},
	{"MARIO1-MALEE2", NO_INES, 1, DEFAULT, DEFAULT},
	{"TF1201", NO_INES, 2, DEFAULT, DEFAULT},
	{"EH8813A", NO_INES, 3, DEFAULT, DEFAULT},
	{"11160", NO_INES, 4, DEFAULT, DEFAULT},
	{"G-146", NO_INES, 5, DEFAULT, DEFAULT},
	{"12-IN-1", NO_INES, 6, DEFAULT, DEFAULT},
	{"411120-C", NO_INES, 7, DEFAULT, DEFAULT},
	{"T-262", NO_INES, 8, DEFAULT, DEFAULT},
	{"BS-5", NO_INES, 9, DEFAULT, DEFAULT},
	{"8157", NO_INES, 10, DEFAULT, DEFAULT},
	{"830118C", NO_INES, 11, DEFAULT, DEFAULT},
	{"8237", NO_INES, 12, DEFAULT, DEFAULT},
	{"8237A", NO_INES, 13, DEFAULT, DEFAULT},
	{"NTD-03", NO_INES, 14, DEFAULT, DEFAULT},

	//{"NTBROM", 68, NO_UNIF},
};

BYTE unif_load_rom(void) {
	BYTE phase;
	FILE *fp;

	{
		BYTE i, found = TRUE;
		static const char rom_ext[6][10] = {
			".nes\0",  ".NES\0",
			".unf\0",  ".UNF\0",
			".unif\0", ".UNIF\0"
		};

		fp = fopen(info.rom_file, "rb");

		if (!fp) {
			found = FALSE;

			for (i = 0; i < LENGTH(rom_ext); i++) {
				char rom_file[LENGTH_FILE_NAME_MID];

				strncpy(rom_file, info.rom_file, sizeof(rom_file));
				strcat(rom_file, rom_ext[i]);

				fp = fopen(rom_file, "rb");

				if (fp) {
					strncpy(info.rom_file, rom_file, sizeof(info.rom_file));
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
				fprintf(stderr, "mapper %u\n", info.mapper.id);
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
					fseek(fp, unif.chunk.length, SEEK_CUR);
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

		if (!info.chr.rom[0].banks_8k) {
			mapper.write_vram = TRUE;
			info.chr.rom[0].banks_8k = 1;
			info.chr.rom[0].banks_4k = info.chr.rom[0].banks_8k * 2;
			info.chr.rom[0].banks_1k = info.chr.rom[0].banks_4k * 4;
		}

		info.prg.max_chips = info.prg.chips - 1;
		info.chr.max_chips = info.chr.chips - 1;

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
			if (strncpy(unif.board, &strip[i][0], strlen(strip[i]))) {
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
				unif.internal_mapper = unif_boards[i].unif_mapper;
				unif.finded = TRUE;
				break;
			}
		}
	}

	return (EXIT_OK);
}
BYTE unif_NAME(FILE *fp, BYTE phase) {
	if (phase == UNIF_COUNT) {
		fseek(fp, unif.chunk.length, SEEK_CUR);
		return (EXIT_OK);
	}

	memset(&unif.name[0], 0x00, sizeof(unif.name));

	if (unif.chunk.length < sizeof(unif.name)) {
		if (!(fread(&unif.name[0], unif.chunk.length, 1, fp))) {
			;
		}
	} else {
		if (!(fread(&unif.name[0], (sizeof(unif.name) - 1), 1, fp))) {
			;
		}
		fseek(fp, unif.chunk.length - (sizeof(unif.name) - 1), SEEK_CUR);
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
		if (chip == 0) {
			map_chr_bank_1k_reset();
		}
		info.chr.rom[chip].banks_8k = chr_chip_size(chip) / (8 * 1024);
		info.chr.rom[chip].banks_4k = info.chr.rom[chip].banks_8k * 2;
		info.chr.rom[chip].banks_1k = info.chr.rom[chip].banks_4k * 4;
		map_set_banks_max_chr(chip);

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
