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

#include <string.h>
#include <stdlib.h>
#include "unif.h"
#include "rom_mem.h"
#include "info.h"
#include "mem_map.h"
#include "mappers.h"
#include "conf.h"
#include "cheat.h"
#include "vs_system.h"
#include "patcher.h"
#include "../../c++/crc/crc.h"
#include "emu.h"
#include "ines.h"
#include "nes20db.h"

enum unif_phase_type { UNIF_COUNT, UNIF_READ };

BYTE unif_NONE(_rom_mem *rom, BYTE phase);
BYTE unif_MAPR(_rom_mem *rom, BYTE phase);
BYTE unif_NAME(_rom_mem *rom, BYTE phase);
BYTE unif_PRG(_rom_mem *rom, BYTE phase);
BYTE unif_CHR(_rom_mem *rom, BYTE phase);
BYTE unif_TVCI(_rom_mem *rom, BYTE phase);
BYTE unif_BATR(_rom_mem *rom, BYTE phase);
BYTE unif_MIRR(_rom_mem *rom, BYTE phase);
BYTE unif_DINF(_rom_mem *rom, BYTE phase);
BYTE unif_FONT(_rom_mem *rom, BYTE phase);

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
	{"SL1632", 14, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"CC-21", 27, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"SC-127", 35, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"AC08", 42, NO_UNIF, 2, DEFAULT, NOEXTRA},
	{"SuperHIK8in1", 45, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"STREETFIGTER-GAME4IN1", 49, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"Supervision16in1", 53, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"MARIO1-MALEE2", 55, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"T3H53", 59, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"D1038", 59, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"NTBROM", 68, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"VRC7", 85, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"TEK90", 90, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"BB", 108, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"H2288", 123, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"LH32", 125, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"22211", 132, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"SA-72008", 133, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"T4A54A", 134, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"Sachen-8259D", 137, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"Sachen-8259B", 138, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"Sachen-8259C", 139, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"Sachen-8259A", 141, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"KS7032", 142, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
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
	{"NovelDiamond9999999in1", 201, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"JC-016-2", 205, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"8237", 215, NO_UNIF, U8237, DEFAULT, NOEXTRA},
	{"8237A", 215, NO_UNIF, U8237A, DEFAULT, NOEXTRA},
	{"N625092", 221, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"Ghostbusters63in1", 226, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"WAIXING-FW01", 227, NO_UNIF, WAIXING_FW01, DEFAULT, NOEXTRA},
	{"42in1ResetSwitch", 233, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"70in1", 236, NO_UNIF, BMC70IN1, DEFAULT, NOEXTRA},
	{"70in1B", 236, NO_UNIF, BMC70IN1B, DEFAULT, NOEXTRA},
	{"43272", 242, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"603-5052", 238, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"OneBus", 256, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"DANCE", 256, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"158B", 258, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"F-15", 259, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"HPXX", 260, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"HP2018-A", 260, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"810544-C-A1", 261, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"SHERO", 262, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"KOF97", 263, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"YOKO", 264, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"T-262", 265, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"CITYFIGHT", 266, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"COOLBOY", 268, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"MINDKIDS", 268, NO_UNIF, MINDKIDS, DEFAULT, NOEXTRA},
	{"22026", 271, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"80013-B", 274, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"GS-2004", 283, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"GS-2013", 283, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"A65AS", 285, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"DRIPGAME", 284, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"BS-5", 286, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"411120-C", 287, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"K-3088", 287, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"60311C", 289, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"NTD-03", 290, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"DRAGONFIGHTER", 292, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"TF1201", 298, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"11160", 299, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"190in1", 300, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"8157", 301, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"KS7057", 302, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"KS7017", 303, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"SMB2J", 304, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"KS7031", 305, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"KS7016", 306, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"KS7037", 307, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"TH2131-1", 308, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"LH51", 309, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"KS7013B", 312, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"RESET-TXROM", 313, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"64in1NoRepeat", 314, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"830134C", 315, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"HP898F", 319, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"830425C-4391T", 320, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"K-3033", 322, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"FARID_SLROM_8-IN-1", 323, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"FARID_UNROM_8-IN-1", 324, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"MALISB", 325, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"10-24-C-A1", 327, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"RT-01", 328, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"EDU2000", 329, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"12-IN-1", 331, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"WS", 332, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"8-IN-1", 333, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"NEWSTAR-GRM070-8IN1", 333, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"CTC-09", 335, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"K-3046", 336, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"CTC-12IN1", 337, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"SA005-A", 338, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"K-3006", 339, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"K-3036", 340, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"TJ-03", 341, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"COOLGIRL", 342, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"RESETNROM-XIN1", 343, NO_UNIF, 1, DEFAULT, NOEXTRA},
	{"GN-26", 344, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"L6IN1", 345, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"KS7012", 346, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"KS7030", 347, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"830118C", 348, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"G-146", 349, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"891227", 350, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"KS106C", 352, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"3D-BLOCK", 355, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"SB-5013", 359, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"N49C-300", 369, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"830752C", 396, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"BS-400R", 422, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"BS-4040R", 422, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"K-3010", 438, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"K-3071", 438, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"SA-9602B", 513, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"DANCE2000", 518, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"EH8813A", 519, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"DREAMTECH01", 521, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"LH10", 522, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"900218", 524, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"KS7021A", 525, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"BJ-56", 526, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"AX-40G", 527, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"831128C", 528, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"T-230", 529, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"AX5705", 530, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"CHINA_ER_SAN2", 532, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"82112C", 540, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},
	{"KONAMI-QTAI", 547, NO_UNIF, DEFAULT, DEFAULT, NOEXTRA},

	{"BOY", NO_INES, 1, DEFAULT, DEFAULT, NOEXTRA},
};

BYTE unif_load_rom(void) {
	_rom_mem rom;
	int phase = 0;

	{
		FILE *fp = NULL;
		BYTE found = TRUE;
		unsigned int i = 0;
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

	// setto i defaults
	info.machine[HEADER] = info.machine[DATABASE] = NTSC;
	info.prg.ram.bat.banks = 0;
	info.mapper.submapper = DEFAULT;
	info.mapper.mirroring = MIRRORING_HORIZONTAL;
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
				size_t size = 0;
				int i = 0;

				if (prg_chip_size(0) == 0) {
					free(rom.data);
					return (EXIT_ERROR);
				}

				// PRG
				{
					for (i = 0, size = 0; i < info.prg.chips; i++) {
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
					for (i = 0, size = 0; i < info.chr.chips; i++) {
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

				// alloco la PRG Ram
				if (map_prg_ram_malloc(0x2000) != EXIT_OK) {
					free(rom.data);
					return (EXIT_ERROR);
				}

				// la CHR ram extra
				memset(&chr.extra, 0x00, sizeof(chr.extra));

				if (chinaersan2_init() != EXIT_OK) {
					free(rom.data);
					return (EXIT_ERROR);
				}
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
				} else if (strncasecmp(unif.chunk.id, "FONT", 4) == 0) {
					if (unif_FONT(&rom, phase) == EXIT_ERROR) {
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

		{
			info.crc32.prg = info.crc32.total = emu_crc32((void *)prg_rom(), prg_size());

			if (chr_size()) {
				info.crc32.chr = emu_crc32((void *)chr_rom(), chr_size());
				info.crc32.total = emu_crc32_continue((void *)chr_rom(), chr_size(), info.crc32.prg);
			}
		}

		emu_save_header_info();
		nes20db_search();
		if (info.format == NES_2_0) {
			nes20_submapper();
		}

		if (info.prg.ram.bat.banks && !info.prg.ram.banks_8k_plus) {
			info.prg.ram.banks_8k_plus = info.prg.ram.bat.banks;
		}

		if ((info.format != NES_2_0) && !info.prg.ram.banks_8k_plus) {
			info.prg.ram.banks_8k_plus = 1;
		}

		if (!info.chr.rom.banks_1k) {
			mapper.write_vram = TRUE;
			if (info.chr.ram.banks_8k_plus) {
				info.chr.rom.banks_8k = info.chr.ram.banks_8k_plus;
				info.chr.ram.banks_8k_plus = 0;
			}
		}

		switch (info.mapper.mirroring) {
			default:
			case MIRRORING_HORIZONTAL:
				mirroring_H();
				break;
			case MIRRORING_VERTICAL:
				mirroring_V();
				break;
			case MIRRORING_FOURSCR:
				mirroring_FSCR();
				break;
			case MIRRORING_SINGLE_SCR0:
				mirroring_SCR0();
				break;
			case MIRRORING_SINGLE_SCR1:
				mirroring_SCR1();
				break;
		}

		if (!unif.finded) {
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
	unsigned int i = 0;
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
		unif.stripped_board = &unif.board[0];

		for (i = 0; i < strlen(unif.stripped_board); i++) {
			if ((*unif.stripped_board) != ' ') {
				break;
			}
			unif.stripped_board++;
		}

		for (i = 0; i < LENGTH(strip); i++) {
			if (strncasecmp(unif.stripped_board, &strip[i][0], strlen(strip[i])) == 0) {
				unif.stripped_board += strlen(strip[i]);
				break;
			}
		}
	}

	{
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

	return (EXIT_OK);
}
BYTE unif_NAME(_rom_mem *rom, BYTE phase) {
	static size_t length;

	if (phase == UNIF_COUNT) {
		length = 0;

		while (rom->data[rom->position] > 0) {
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

	return (EXIT_OK);
}
BYTE unif_PRG(_rom_mem *rom, BYTE phase) {
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
		int i = 0;

		unif.chips.prg++;
		for (i = 0, prg_chip_rom(chip) = prg_rom(); i < chip; i++) {
			prg_chip_rom(chip) += prg_chip_size(i);
		}
		rom_mem_memcpy(prg_chip_rom(chip), rom, prg_chip_size(chip));
	}

	return (EXIT_OK);
}
BYTE unif_CHR(_rom_mem *rom, BYTE phase) {
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
		int i = 0;

		unif.chips.chr++;
		for (i = 0, chr_chip_rom(chip) = chr_rom(); i < chip; i++) {
			chr_chip_rom(chip) += chr_chip_size(i);
		}
		rom_mem_memcpy(chr_chip_rom(chip), rom, chr_chip_size(chip));
	}

	return (EXIT_OK);
}
BYTE unif_TVCI(_rom_mem *rom, BYTE phase) {
	BYTE cpu_timing = 0;

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

	rom_mem_memcpy(&cpu_timing, rom, unif.chunk.length);

	switch (cpu_timing) {
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
	BYTE batr = 0;

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
	if (phase == UNIF_COUNT) {
		if ((rom->position + unif.chunk.length) > rom->size) {
			return (EXIT_ERROR);
		}
		rom->position += unif.chunk.length;
		return (EXIT_OK);
	}

	info.mapper.mirroring = 0;

	rom_mem_memcpy(&info.mapper.mirroring, rom, unif.chunk.length);

	switch (info.mapper.mirroring) {
		default:
		case 0:
			info.mapper.mirroring = MIRRORING_HORIZONTAL;
			break;
		case 1:
		case 5:
			info.mapper.mirroring = MIRRORING_VERTICAL;
			break;
		case 2:
			info.mapper.mirroring = MIRRORING_SINGLE_SCR0;
			break;
		case 3:
			info.mapper.mirroring = MIRRORING_SINGLE_SCR1;
			break;
		case 4:
			info.mapper.mirroring = MIRRORING_FOURSCR;
			break;
	}

	return (EXIT_OK);
}
BYTE unif_DINF(_rom_mem *rom, BYTE phase) {
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

	return (EXIT_OK);
}
BYTE unif_FONT(_rom_mem *rom, BYTE phase) {
	if (phase == UNIF_COUNT) {
		if ((rom->position + unif.chunk.length) > rom->size) {
			return (EXIT_ERROR);
		}
		chinaersan2.font.size = (size_t)(64 * 1024);
		rom->position += unif.chunk.length;
		return (EXIT_OK);
	}

	memset(chinaersan2.font.data, 0x00, chinaersan2.font.size);
	rom_mem_memcpy(chinaersan2.font.data, rom,  unif.chunk.length > chinaersan2.font.size ?
		chinaersan2.font.size : unif.chunk.length);

	return (EXIT_OK);
}
