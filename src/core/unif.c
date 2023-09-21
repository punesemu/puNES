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
#include "mappers.h"
#include "conf.h"
#include "cheat.h"
#include "vs_system.h"
#include "patcher.h"
#include "../../c++/crc/crc.h"
#include "emu.h"
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

void find_board(void);

_unif unif;

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
	info.number_of_cpu = 1;
	info.machine[HEADER] = info.machine[DATABASE] = NTSC;
	info.mapper.submapper = 0;
	info.mapper.mirroring = MIRRORING_HORIZONTAL;
	info.mapper.battery = FALSE;
	info.mirroring_db = DEFAULT;
	info.extra_from_db = 0;
	info.mapper.chrrom_banks_8k = 0;
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

				if (prgrom_chip_size(0) == 0) {
					free(rom.data);
					return (EXIT_ERROR);
				}

				// PRG
				{
					for (i = 0, size = 0; i < prgrom.chips.amount; i++) {
						size += prgrom_chip_size(i);
					}

					prgrom_set_size(size);

					if (prgrom_init(0x00) == EXIT_ERROR) {
						free(rom.data);
						return (EXIT_ERROR);
					}
					prgrom_chip(0) = prgrom_pnt();
					info.mapper.prgrom_banks_16k = prgrom_size() / S16K;
				}

				// CHR
				{
					for (i = 0, size = 0; i < chrrom.chips.amount; i++) {
						size += chrrom_chip_size(i);
					}

					chrrom_set_size(size);

					if (size > 0) {
						if (chrrom_init() == EXIT_ERROR) {
							free(rom.data);
							return (EXIT_ERROR);
						}
						chrrom_chip(0) = chrrom_pnt();
						info.mapper.chrrom_banks_8k = chrrom_size() / S8K;
					}
				}

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
			info.crc32.prg = emu_crc32((void *)prgrom_pnt(), prgrom_size());
			info.crc32.total = info.crc32.prg;

			if (chrrom_size()) {
				info.crc32.chr = emu_crc32((void *)chrrom_pnt(), chrrom_size());
				info.crc32.total = emu_crc32_continue((void *)chrrom_pnt(), chrrom_size(), info.crc32.prg);
			}
		}

		find_board();

		ram_set_size(0, S2K);
		ram_init();

		nmt_set_size(0, S4K);
		nmt_init();

		// nes20db non ha trovato la rom
		if (!wram_size() && info.mapper.battery) {
			wram_set_nvram_size(S8K);
		}
		if (info.mapper.battery && !wram_nvram_size()) {
			wram_set_nvram_size(wram_ram_size());
			wram_set_ram_size(0);
		}
		if (!vram_size(0)) {
			if ((info.mapper.id != 256) && (info.mapper.id != 270) && (info.mapper.id != 296)) {
				vram_set_ram_size(0, chrrom_size() ? 0 : S8K);
			}
		}

		emu_save_header_info();
		nes20db_search();

		switch (info.mapper.mirroring) {
			default:
			case MIRRORING_HORIZONTAL:
				mirroring_H(0);
				break;
			case MIRRORING_VERTICAL:
				mirroring_V(0);
				break;
			case MIRRORING_FOURSCR:
				mirroring_FSCR(0);
				break;
			case MIRRORING_SINGLE_SCR0:
				mirroring_SCR0(0);
				break;
			case MIRRORING_SINGLE_SCR1:
				mirroring_SCR1(0);
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
		prgrom.chips.amount = ++unif.chips.prg;
		prgrom_chip_size(chip) = unif.chunk.length;
		rom->position += unif.chunk.length;
		return (EXIT_OK);
	} else {
		int i = 0;

		unif.chips.prg++;
		for (i = 0, prgrom_chip(chip) = prgrom_pnt(); i < chip; i++) {
			prgrom_chip(chip) += prgrom_chip_size(i);
		}
		rom_mem_memcpy(prgrom_chip(chip), rom, prgrom_chip_size(chip));
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
		chrrom.chips.amount = ++unif.chips.chr;
		chrrom_chip_size(chip) = unif.chunk.length;
		rom->position += unif.chunk.length;
		return (EXIT_OK);
	} else {
		int i = 0;

		unif.chips.chr++;
		for (i = 0, chrrom_chip(chip) = chrrom_pnt(); i < chip; i++) {
			chrrom_chip(chip) += chrrom_chip_size(i);
		}
		rom_mem_memcpy(chrrom_chip(chip), rom, chrrom_chip_size(chip));
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
	info.mapper.battery = TRUE;

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

void find_board(void) {
	unif.finded = FALSE;
	info.mapper.id = DEFAULT;
	info.mapper.submapper = 0;
	info.extra_from_db = DEFAULT;

	if (!strncasecmp("NROM", unif.stripped_board, strlen(unif.stripped_board)) ||
		!strncasecmp("NROM-128", unif.stripped_board, strlen(unif.stripped_board)) ||
		!strncasecmp("NROM-256", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 0;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("Transformer", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 0;
		wram_set_ram_size(S8K);
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("SLROM", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 1;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("UOROM", unif.stripped_board, strlen(unif.stripped_board)) ||
		!strncasecmp("UNROM", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 2;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("CNROM", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 3;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("TLROM", unif.stripped_board, strlen(unif.stripped_board)) ||
		!strncasecmp("TBROM", unif.stripped_board, strlen(unif.stripped_board)) ||
		!strncasecmp("TFROM", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 4;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("TKROM", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 4;
		wram_set_ram_size(S8K);
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("ANROM", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 7;
		info.mapper.submapper = 1;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("AMROM", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 7;
		info.mapper.submapper = 2;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("AOROM", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 7;
		info.mapper.submapper = 0;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("SL1632", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 14;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("CC-21", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 27;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("AC08", unif.stripped_board, strlen(unif.stripped_board)) ||
		!strncasecmp("LH09", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 42;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("SuperHIK8in1", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 45;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("STREETFIGTER-GAME4IN1", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 49;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("A60AS", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 51;
		info.mapper.submapper = 1;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("Supervision16in1", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 53;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("MARIO1-MALEE2", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 55;
		wram_set_ram_size(S2K);
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("T3H53", unif.stripped_board, strlen(unif.stripped_board)) ||
		!strncasecmp("D1038", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 59;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("NTBROM", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 68;
		info.mapper.submapper = 1;
		wram_set_nvram_size(S8K);
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("SA-016-1M", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 79;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("VRC7", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 85;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("TEK90", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 90;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("BB", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 108;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("H2288", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 123;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("LH32", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 125;
		wram_set_ram_size(S8K);
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("22211", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 132;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("SA-72008", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 133;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("T4A54A", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 134;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("Sachen-8259D", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 137;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("Sachen-8259B", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 138;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("Sachen-8259C", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 139;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("Sachen-8259A", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 141;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("KS7032", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 142;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("SA-NROM", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 143;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("SA-72007", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 145;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("TC-U01-1.5M", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 147;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("SA-0037", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 148;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("SA-0036", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 149;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("Sachen-74LS374N", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 150;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("FS304", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 162;
		wram_set_nvram_size(S8K);
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("Super24in1SC03", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 176;
		vram_set_ram_size(0, S8K);
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("WAIXING-FS005", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 176;
		wram_set_ram_size(S16K);
		wram_set_nvram_size(S16K);
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("FK23C", unif.stripped_board, strlen(unif.stripped_board)) ||
		!strncasecmp("FK23CA", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 176;
		if ((prgrom_size() == S2M) || (prgrom_size() == S16M)) {
			vram_set_ram_size(0, S128K);
		} else if (prgrom_size() == S8M) {
			vram_set_ram_size(0, S256K);
		}
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("FC-28-5027", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 197;
		info.mapper.submapper = 1;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("NovelDiamond9999999in1", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 201;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("JC-016-2", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 205;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("8237", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 215;
		info.mapper.submapper = 0;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("8237A", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 215;
		info.mapper.submapper = 1;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("N625092", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 221;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("Ghostbusters63in1", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 226;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("WAIXING-FW01", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 227;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("42in1ResetSwitch", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 233;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("150in1A", unif.stripped_board, strlen(unif.stripped_board)) ||
		!strncasecmp("212-HONG-KONG", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 235;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("70in1", unif.stripped_board, strlen(unif.stripped_board)) ||
		!strncasecmp("70in1B", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 236;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("603-5052", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 238;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("43272", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 242;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("OneBus", unif.stripped_board, strlen(unif.stripped_board)) ||
		!strncasecmp("DANCE", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 256;
		wram_set_ram_size(S8K);
		info.mapper.ext_console_type = VT03;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("158B", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 258;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("F-15", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 259;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("HPXX", unif.stripped_board, strlen(unif.stripped_board)) ||
		!strncasecmp("HP2018-A", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 260;
		wram_set_ram_size(S8K);
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("810544-C-A1", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 261;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("SHERO", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 262;
		info.mapper.mirroring = MIRRORING_FOURSCR;
		vram_set_ram_size(0, S8K);
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("KOF97", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 263;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("YOKO", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 264;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("T-262", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 265;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("CITYFIGHT", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 266;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("COOLBOY", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 268;
		wram_set_ram_size(S8K);
		vram_set_ram_size(0, S256K);
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("MINDKIDS", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 224;
		info.mapper.submapper = 1;
		wram_set_ram_size(S8K);
		vram_set_ram_size(0, S256K);
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("22026", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 271;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("80013-B", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 274;
		vram_set_ram_size(0, S8K);
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("GS-2004", unif.stripped_board, strlen(unif.stripped_board)) ||
		!strncasecmp("GS-2013", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 283;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("DRIPGAME", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 284;
		wram_set_nvram_size(S8K);
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("A65AS", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 285;
		vram_set_ram_size(0, S8K);
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("BS-5", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 286;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("411120-C", unif.stripped_board, strlen(unif.stripped_board)) ||
		!strncasecmp("K-3088", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 287;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("GKCXIN1", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 288;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("60311C", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 289;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("NTD-03", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 290;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("DRAGONFIGHTER", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 292;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("13in1JY110", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 295;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("TF1201", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 298;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("11160", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 299;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("190in1", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 300;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("8157", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 301;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("KS7057", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 302;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("KS7017", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 303;
		wram_set_ram_size(S8K);
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("SMB2J", unif.stripped_board, strlen(unif.stripped_board))) {
		if (prgrom_size() < 81920) {
			info.mapper.id = 304;
		} else {
			info.mapper.id = 311;
		}
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("KS7031", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 305;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("KS7016", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 306;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("KS7037", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 307;
		wram_set_ram_size(S8K);
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("TH2131-1", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 308;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("LH51", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 309;
		wram_set_ram_size(S8K);
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("KS7013B", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 312;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("RESET-TXROM", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 313;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("64in1NoRepeat", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 314;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("830134C", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 315;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("HP898F", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 319;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("830425C-4391T", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 320;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("K-3033", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 322;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("FARID_SLROM_8-IN-1", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 323;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("FARID_UNROM_8-IN-1", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 324;
		vram_set_ram_size(0, S8K);
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("MALISB", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 325;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("10-24-C-A1", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 327;
		wram_set_ram_size(S8K);
		vram_set_ram_size(0, S8K);
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("RT-01", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 328;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("EDU2000", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 329;
		wram_set_ram_size(S32K);
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("12-IN-1", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 331;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("WS", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 332;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("8-IN-1", unif.stripped_board, strlen(unif.stripped_board)) ||
		!strncasecmp("NEWSTAR-GRM070-8IN1", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 333;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("CTC-09", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 335;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("K-3046", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 336;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("CTC-12IN1", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 337;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("SA005-A", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 338;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("K-3006", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 339;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("K-3036", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 340;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("TJ-03", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 341;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("COOLGIRL", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 342;
		wram_set_ram_size(S32K);
		vram_set_ram_size(0, S256K);
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("KS106C", unif.stripped_board, strlen(unif.stripped_board))) {
		if (info.crc32.prg == 0xEB2474F7) {
			info.mapper.id = 352;
		} else {
			info.mapper.id = 343;
			info.mapper.submapper = 1;
		}
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("RESETNROM-XIN1", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 343;
		info.mapper.submapper = 0;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("GN-26", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 344;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("L6IN1", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 345;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("KS7012", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 346;
		wram_set_ram_size(S8K);
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("KS7030", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 347;
		wram_set_ram_size(S8K);
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("830118C", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 348;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("G-146", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 349;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("891227", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 350;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("3D-BLOCK", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 355;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("SB-5013", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 359;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("SB-5013", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 359;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("N49C-300", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 369;
		wram_set_ram_size(S8K);
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("820561C", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 377;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("M2C52A", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 380;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("830752C", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 396;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("BS-400R", unif.stripped_board, strlen(unif.stripped_board)) ||
		!strncasecmp("BS-4040R", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 422;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("S-2009", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 434;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("K-3010", unif.stripped_board, strlen(unif.stripped_board)) ||
		!strncasecmp("K-3071", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 438;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("SA-9602B", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 513;
		vram_set_ram_size(0, S32K);
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("DANCE2000", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 518;
		wram_set_ram_size(S8K);
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("EH8813A", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 519;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("DREAMTECH01", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 521;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("LH10", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 522;
		wram_set_ram_size(S8K);
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("900218", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 524;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("KS7021A", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 525;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("BJ-56", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 526;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("AX-40G", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 527;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("831128C", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 528;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("T-230", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 529;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("AX5705", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 530;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("82112C", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 540;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("CHINA_ER_SAN2", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 532;
		wram_set_nvram_size(S8K);
		info.mapper.battery = TRUE;
		unif.finded = TRUE;
		return;
	}
	if (!strncasecmp("KONAMI-QTAI", unif.stripped_board, strlen(unif.stripped_board))) {
		info.mapper.id = 547;
		wram_set_ram_size(S8K);
		wram_set_nvram_size(S8K);
		vram_set_ram_size(0, S8K);
		unif.finded = TRUE;
		return;
	}
	unif.finded = FALSE;
}
