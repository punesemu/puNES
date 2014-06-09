/*
 * unif.c
 *
 *  Created on: 03/mag/2014
 *      Author: fhorse
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "unif.h"
//#include "fds.h"
#include "mem_map.h"
#include "mappers.h"
#include "emu.h"
#include "cfg_file.h"
//#include "clock.h"
#include "gamegenie.h"

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
	char board[15];
	WORD ines_mapper;
	WORD unif_mapper;
} _unif_board;

static const _unif_board unif_boards[] = {
	{"NROM", 0 , NO_UNIF},
	{"NROM-128", 0, NO_UNIF},
	{"NROM-256", 0, NO_UNIF},
	{"Sachen-74LS374N", 150, NO_UNIF},
	{"A65AS", NO_INES , 0},
	{"UOROM", 2 , NO_UNIF},
	{"TC-U01-1.5M", 147, NO_UNIF},
	{"SA-NROM", 143, NO_UNIF},
	{"SLROM", 1, NO_UNIF},
	{"22211", 132, NO_UNIF},
	{"TLROM", 4, NO_UNIF},
	{"TBROM", 4, NO_UNIF},
	{"TKROM", 4, NO_UNIF},
	{"Sachen-8259C", 139, NO_UNIF},
	{"SA-016-1M", 146, NO_UNIF},
	{"Sachen-8259D", 137, NO_UNIF},
	{"ANROM", 7, NO_UNIF},
	//{"NTBROM", 68, NO_UNIF},
};

BYTE unif_load_rom(void) {
	BYTE tmp, phase;
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

	if (cfg->gamegenie) {
		fp = gamegenie_load_rom(fp);
	}

	phase = tmp = 0;
	memset(&unif, 0x00, sizeof(unif));

	tmp = fread(&unif.header, sizeof(unif.header), 1, fp);

	phase = UNIF_COUNT;

	/* setto dei default */
	mirroring_H();
	info.machine[HEADER] = NTSC;
	info.prg.ram.bat.banks = 0;

	if (strncmp(unif.header.identification, "UNIF", 4) == 0) {
		long position = ftell(fp);

		for (phase = UNIF_COUNT; phase <= UNIF_READ; phase++) {
			fseek(fp, position, SEEK_SET);

			if (phase == UNIF_READ) {
				if (unif.prg.size == 0) {
					fclose(fp);
					return (EXIT_ERROR);
				}

				info.prg.rom.banks_16k = unif.prg.size / (16 * 1024);
				info.chr.rom.banks_8k = unif.chr.size / (8 * 1024);








				// TODO : questo e' tutto da rifare
				/*
				 * inizializzo qui il writeVRAM per la mapper 96 perche'
				 * e' l'unica mapper che utilizza 32k di CHR Ram e che
				 * si permette anche il lusso di swappare. Quindi imposto
				 * a FALSE qui in modo da poter cambiare impostazione nel
				 * emu_search_in_database.
				 */
				//mapper.write_vram = FALSE;
				//if (emu_search_in_database(fp)) {
				//	fclose(fp);
				//	return (EXIT_ERROR);
				//}









#if !defined (RELEASE)
				fprintf(stderr, "unif format\n");
				fprintf(stderr, "mapper %u\n8k rom = %u\n4k vrom = %u\n", info.mapper.id,
				        info.prg.rom.banks_16k * 2, info.chr.rom.banks_8k * 2);
				fprintf(stderr, "sha1prg = %40s\n", info.sha1sum.prg.string);
				fprintf(stderr, "sha1chr = %40s\n", info.sha1sum.chr.string);
#endif

				if (!info.chr.rom.banks_8k) {
					mapper.write_vram = TRUE;
					info.chr.rom.banks_8k = 1;
				}
				info.prg.rom.banks_8k = info.prg.rom.banks_16k * 2;
				info.chr.rom.banks_4k = info.chr.rom.banks_8k * 2;
				info.chr.rom.banks_1k = info.chr.rom.banks_4k * 4;

				map_set_banks_max_prg_and_chr();

				if (info.prg.ram.bat.banks) {
					info.prg.ram.banks_8k_plus = 1;
				}

				/* alloco la PRG Ram */
				if (!(prg.ram = (BYTE *) malloc(0x2000))) {
					fprintf(stderr, "Out of memory\n");
					fclose(fp);
					return (EXIT_ERROR);
				}

				/* alloco e carico la PRG Rom */
				if ((prg.rom = (BYTE *) malloc(info.prg.rom.banks_16k * (16 * 1024)))) {
					unif.prg.pnt = prg.rom;
				} else {
					fprintf(stderr, "Out of memory\n");
					fclose(fp);
					return (EXIT_ERROR);
				}

				/*
				 * se e' settato mapper.write_vram, vuol dire
				 * che la rom non ha CHR Rom e che quindi la CHR Ram
				 * la trattero' nell'inizializzazione della mapper
				 * (perche' alcune mapper ne hanno 16k, altre 8k).
				 */
				if (mapper.write_vram == FALSE) {
					/* alloco la CHR Rom */
					if ((chr.data = (BYTE *) malloc(info.chr.rom.banks_8k * (8 * 1024)))) {
						unif.chr.pnt = chr.data;
						chr_bank_1k_reset();
					} else {
						fprintf(stderr, "Out of memory\n");
						fclose(fp);
						return (EXIT_ERROR);
					}
				}
			}

			while(fread(&unif.chunk, sizeof(unif.chunk), 1, fp)) {
				if (strncmp(unif.chunk.id, "MAPR", 4) == 0) {
					unif_MAPR(fp, phase);
				} else if (strncmp(unif.chunk.id, "PRG", 3) == 0) {
					unif_PRG(fp, phase);
				} else if (strncmp(unif.chunk.id, "CHR", 3) == 0) {
					unif_CHR(fp, phase);
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
		fread(&unif.board[0], unif.chunk.length, 1, fp);
	} else {
		fread(&unif.board[0], (sizeof(unif.board) - 1), 1, fp);
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
		fread(&unif.name[0], unif.chunk.length, 1, fp);
	} else {
		fread(&unif.name[0], (sizeof(unif.name) - 1), 1, fp);
		fseek(fp, unif.chunk.length - (sizeof(unif.name) - 1), SEEK_CUR);
	}

	printf("name : %s\n", unif.name);

	return (EXIT_OK);
}
BYTE unif_PRG(FILE *fp, BYTE phase) {
	if (phase == UNIF_COUNT) {
		if (strncmp(unif.chunk.id, "PRG0", 4) == 0) {
			unif.prg.size = unif.chunk.length;
		}
		//unif.prg.size += unif.chunk.length;
		fseek(fp, unif.chunk.length, SEEK_CUR);
	} else {
		fread(unif.prg.pnt, unif.chunk.length, 1, fp);
		unif.prg.pnt += unif.chunk.length;
	}

	return (EXIT_OK);
}
BYTE unif_CHR(FILE *fp, BYTE phase) {
	if (phase == UNIF_COUNT) {
		unif.chr.size += unif.chunk.length;
		fseek(fp, unif.chunk.length, SEEK_CUR);
	} else {
		fread(unif.chr.pnt, unif.chunk.length, 1, fp);
		//unif.chr.pnt += unif.chunk.length;
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

	fread(&tv, unif.chunk.length, 1, fp);

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
	fread(&batr, unif.chunk.length, 1, fp);

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
	fread(&mirr, unif.chunk.length, 1, fp);

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
