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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "ines.h"
#include "rom_mem.h"
#include "mappers.h"
#include "conf.h"
#include "cheat.h"
#include "info.h"
#include "gui.h"
#include "vs_system.h"
#include "patcher.h"
#include "sha1.h"
#include "database.h"
#include "../../c++/crc/crc.h"
#include "nes20db.h"

void calculate_checksums_from_rom(void *rom_mem);
void search_in_database(void);
BYTE ines10_search_in_database(void);

_ines ines;

BYTE ines_load_rom(void) {
	BYTE cpu_timing = 0;
	_rom_mem rom;

	{
		static const uTCHAR rom_ext[2][10] = { uL(".nes\0"), uL(".NES\0") };
		unsigned int i = 0;
		BYTE found = TRUE;
		FILE *fp = NULL;

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

	if ((rom.data[rom.position++] == 'N') &&
		(rom.data[rom.position++] == 'E') &&
		(rom.data[rom.position++] == 'S') &&
		(rom.data[rom.position++] == '\32')) {

		info.number_of_nes = 1;
		info.mapper.prgrom_banks_16k = rom.data[rom.position++];
		info.mapper.chrrom_banks_8k = rom.data[rom.position++];

		wram_set_ram_size(0);
		wram_set_nvram_size(0);

		for (int nesidx = 0; nesidx < NES_CHIPS_MAX; nesidx++) {
			vram_set_ram_size(nesidx, 0);
			vram_set_nvram_size(nesidx, 0);
		}

		if (rom_mem_ctrl_memcpy(&ines.flags[0], &rom, TOTAL_FL) == EXIT_ERROR) {
			free(rom.data);
			return (EXIT_ERROR);
		}

		if ((ines.flags[FL7] & 0x0C) == 0x08) {
			// NES 2.0
			info.format = NES_2_0;

			// visto che con il NES_2_0 non eseguo la ricerca nel
			// database inizializzo queste variabili.
			info.mirroring_db = DEFAULT;
			info.mapper.expansion = 0;

			info.mapper.id = ((ines.flags[FL8] & 0x0F) << 8) | (ines.flags[FL7] & 0xF0) | (ines.flags[FL6] >> 4);
			info.mapper.submapper_nes20 = (ines.flags[FL8] & 0xF0) >> 4;
			info.mapper.submapper = info.mapper.submapper_nes20;

			// PRGROM
			info.mapper.prgrom_banks_16k |= ((ines.flags[FL9] & 0x0F) << 8);
			info.mapper.prgrom_size = nes20_prg_chr_size(&info.mapper.prgrom_banks_16k, S16K);

			// CHROM
			info.mapper.chrrom_banks_8k |= ((ines.flags[FL9] & 0xF0) << 4);
			info.mapper.chrrom_size = nes20_prg_chr_size(&info.mapper.chrrom_banks_8k, S8K);

			// WRAM
			if (ines.flags[FL10] & 0x0F) {
				wram_set_ram_size(64 << (ines.flags[FL10] & 0x0F));
			}
			if (ines.flags[FL10] & 0xF0) {
				wram_set_nvram_size(64 << ((ines.flags[FL10] & 0xF0) >> 4));
			}

			// VRAM
			if (ines.flags[FL11] & 0x0F) {
				vram_set_ram_size(0, 64 << (ines.flags[FL11] & 0x0F));
			}
			if (ines.flags[FL11] & 0xF0) {
				vram_set_nvram_size(0, 64 << ((ines.flags[FL11] & 0xF0) >> 4));
			}

			info.mapper.battery = (ines.flags[FL6] & 0x02) >> 1;

			cpu_timing = ines.flags[FL12] & 0x03;

			vs_system.ppu = 0;
			vs_system.special_mode.type = 0;
			info.decimal_mode = FALSE;

			if ((ines.flags[FL7] & 0x03) == 0x01) {
				info.mapper.ext_console_type = 1;
				vs_system.ppu = ines.flags[FL13] & 0x0F;
				vs_system.special_mode.type = ines.flags[FL13] >> 4;
				info.decimal_mode = FALSE;
			} else if ((ines.flags[FL7] & 0x03) == 0x03) {
				info.mapper.ext_console_type = ines.flags[FL13] & 0x0F;

				switch (info.mapper.ext_console_type) {
					case 0:
					case 1:
					case 2:
						vs_system.special_mode.type = info.mapper.ext_console_type;
						break;
					case 3:
						// usato per esempio da :
						// Othello (rev0).nes
						info.decimal_mode = TRUE;
						break;
					default:
						break;
				}
			}

			miscrom.chips = ines.flags[FL14] & 0x03;
			info.mapper.expansion = ines.flags[FL15];
		} else {
			// iNES 1.0
			info.format = iNES_1_0;

			info.mapper.prgrom_size = (size_t)info.mapper.prgrom_banks_16k *  S16K;
			info.mapper.chrrom_size = (size_t)info.mapper.chrrom_banks_8k * S8K;

			// Older versions of the iNES emulator ignored bytes 7-15, and several ROM management tools
			// wrote messages in there. Commonly, these will be filled with "DiskDude!", which results
			// in 64 being added to the mapper number. A general rule of thumb: if the last 4 bytes are
			// not all zero, and the header is not marked for NES 2.0 format, an emulator should either
			// mask off the upper 4 bits of the mapper number or simply refuse to load the ROM.
			if ((ines.flags[FL12] >= ' ') | (ines.flags[FL13] >= ' ') |
				(ines.flags[FL14] >= ' ') | (ines.flags[FL15] >= ' ')) {
				info.mapper.id = ines.flags[FL6] >> 4;
				cpu_timing = 0;
			} else {
				info.mapper.id = (ines.flags[FL7] & 0xF0) | (ines.flags[FL6] >> 4);
				cpu_timing = ines.flags[FL9] & 0x01;
			}

			info.mapper.submapper = 0;
			info.mapper.battery = (ines.flags[FL6] & 0x02) >> 1;

			wram_set_ram_size(S8K);
			vram_set_ram_size(0, S8K);

			vs_system.ppu = 0;
			vs_system.special_mode.type = 0;
			info.mapper.expansion = 0;
			info.decimal_mode = FALSE;
		}

		miscrom.trainer.in_use = ines.flags[FL6] & 0x04;

		if (ines.flags[FL6] & 0x08) {
			info.mapper.mirroring = MIRRORING_FOURSCR;
		} else {
			if (ines.flags[FL6] & 0x01) {
				info.mapper.mirroring = MIRRORING_VERTICAL;
			} else {
				info.mapper.mirroring = MIRRORING_HORIZONTAL;
			}
		}

		switch (cpu_timing) {
			default:
			case 0:
			case 2:
				info.machine[HEADER] = NTSC;
				break;
			case 1:
				info.machine[HEADER] = PAL;
				break;
			case 3:
				info.machine[HEADER] = DENDY;
				break;
		}

		emu_save_header_info();
		calculate_checksums_from_rom(&rom);

		// inizializzo qui il writeVRAM per la mapper 96 perche'
		// e' l'unica mapper che utilizza 32k di CHR Ram e che
		// si permette anche il lusso di swappare. Quindi imposto
		// a FALSE qui in modo da poter cambiare impostazione nel
		// ines10_search_in_database().
		//mapper.write_vram = FALSE;

		if (info.format != NES_2_0)  {
			if (ines10_search_in_database()) {
				free(rom.data);
				return (EXIT_ERROR);
			}
			calculate_checksums_from_rom(&rom);
		}

		nes20db_search();

		// gestione Vs. System
		if ((info.mapper.id != 99) && !vs_system.ppu && !vs_system.special_mode.type) {
			vs_system.enabled = FALSE;
			vs_system.special_mode.r5e0x = NULL;
			vs_system.special_mode.index = 0;
			vs_system.rc2c05.enabled = FALSE;
		} else {
			vs_system.enabled = TRUE;

			switch (vs_system.ppu) {
				case RP2C03B:
				case RP2C03G:
				case RP2C04:
				case RP2C04_0002:
				case RP2C04_0003:
				case RP2C04_0004:
				case RC2C03B:
				case RC2C03C:
				default:
					vs_system.rc2c05.enabled = FALSE;
					vs_system.rc2c05.r2002 = 0x00;
					break;
				case RC2C05_01:
					vs_system.rc2c05.enabled = TRUE;
					vs_system.rc2c05.r2002 = 0x1B;
					break;
				case RC2C05_02:
					vs_system.rc2c05.enabled = TRUE;
					vs_system.rc2c05.r2002 = 0x3D;
					break;
				case RC2C05_03:
					vs_system.rc2c05.enabled = TRUE;
					vs_system.rc2c05.r2002 = 0x1C;
					break;
				case RC2C05_04:
					vs_system.rc2c05.enabled = TRUE;
					vs_system.rc2c05.r2002 = 0x1B;
					break;
				case RC2C05_05:
					vs_system.rc2c05.enabled = TRUE;
					vs_system.rc2c05.r2002 = 0x00;
					break;
			}

			switch (vs_system.special_mode.type) {
				default:
					vs_system.special_mode.type = VS_SM_Normal;
					vs_system.special_mode.r5e0x = NULL;
					break;
				case VS_SM_Normal:
					vs_system.special_mode.r5e0x = NULL;
					break;
				case VS_SM_RBI_Baseball:
					vs_system.special_mode.r5e0x = (BYTE *)&vs_protection_data[1][0];
					break;
				case VS_SM_TKO_Boxing:
					vs_system.special_mode.r5e0x = (BYTE *)&vs_protection_data[0][0];
					break;
				case VS_SM_Super_Xevious:
					vs_system.special_mode.r5e0x = NULL;
					break;
				case VS_DS_Normal:
				case VS_DS_Bungeling:
					info.number_of_nes = 2;
					break;
			}

			vs_system.special_mode.index = 0;

			if (wram_size() < S2K) {
				wram_set_ram_size(S2K);
			}
			for (int nesidx = 0; nesidx < info.number_of_nes; nesidx++) {
				memmap_wram_region_init(nesidx, S2K);
			}
		}

		for (int nesidx = 0; nesidx < info.number_of_nes; nesidx++) {
			ram_set_size(nesidx, S2K);
			nmt_set_size(nesidx, S4K);
		}
		ram_init();
		nmt_init();

		if (miscrom.trainer.in_use) {
			miscrom.chips = 0;
			miscrom_set_size(512);
			if ((miscrom_init() == EXIT_ERROR) ||
				(rom_mem_ctrl_memcpy(miscrom_pnt(), &rom, miscrom_size()) == EXIT_ERROR)) {
				free(rom.data);
				return (EXIT_ERROR);
			}
		}

		info.mapper.prgrom_banks_16k = !info.mapper.prgrom_banks_16k? 1 : info.mapper.prgrom_banks_16k;
		info.mapper.prgrom_size = !info.mapper.prgrom_size ||
			(info.mapper.prgrom_size > info.mapper.prgrom_banks_16k * S16K)
			? info.mapper.prgrom_banks_16k * S16K
			: info.mapper.prgrom_size;

		// alloco e carico la PRG Rom
		prgrom_set_size(info.mapper.prgrom_banks_16k * S16K);

		if (prgrom_init(0x00) == EXIT_ERROR) {
			free(rom.data);
			return (EXIT_ERROR);
		}

		if (rom_mem_ctrl_memcpy_truncated(prgrom_pnt(), &rom, info.mapper.prgrom_size) == EXIT_ERROR) {
			info.prg_truncated = TRUE;
		}

		// alloco e carico la CHR Rom
		chrrom_set_size(info.mapper.chrrom_banks_8k * S8K);

		if (chrrom_size()) {
			if (chrrom_init() == EXIT_ERROR) {
				free(rom.data);
				return (EXIT_ERROR);
			}
			if (rom_mem_ctrl_memcpy_truncated(chrrom_pnt(), &rom, info.mapper.chrrom_size) == EXIT_ERROR) {
				info.chr_truncated = TRUE;
			}
		}

		if (!miscrom.trainer.in_use && !miscrom.chips && ((rom.size - rom.position) > 0)) {
			miscrom.chips = 1;
		}
		if (miscrom.chips) {
			miscrom_set_size(rom.size - rom.position);
			if (miscrom_init() == EXIT_ERROR) {
				free(rom.data);
				return (EXIT_ERROR);
			}
			if (rom_mem_ctrl_memcpy_truncated(miscrom_pnt(), &rom, miscrom_size()) == EXIT_ERROR) {
				info.misc_truncated = TRUE;
			}
		}

		if (!wram_size() && info.mapper.battery) {
			wram_set_nvram_size(S8K);
		}
		if (info.mapper.battery && !wram_nvram_size()) {
			wram_set_nvram_size(wram_ram_size());
			wram_set_ram_size(0);
		}
		for (int nesidx = 0; nesidx < info.number_of_nes; nesidx++) {
			if (!chrrom_size() && !vram_size(nesidx)) {
				vram_set_ram_size(nesidx, S8K);
			}
		}

		free(rom.data);

		gui_dlgheadereditor_read_header();

		return (EXIT_OK);
	}

	free(rom.data);
	return (EXIT_ERROR);
}

size_t nes20_prg_chr_size(DBWORD *reg, double divider) {
	if (((*reg) & 0x0F00) == 0x0F00) {
		unsigned int exponent = ((*reg) & 0x00FC) >> 2;
		double len = pow(2, exponent) * ((((*reg) & 0x0003) * 2) + 1);

		(*reg) = (int)ceil(len / divider);
		return((int)len);
	}
	return((int)((*reg) * divider));
}

void calculate_checksums_from_rom(void *rom_mem) {
	_rom_mem *rom = (_rom_mem *)rom_mem;
	size_t position = 0x10, len = 0;
	size_t difference = 0;

	info.crc32.prg = 0;
	info.crc32.chr = 0;
	info.crc32.trainer = 0;
	info.crc32.misc = 0;
	info.crc32.total = 0;

	// punto oltre l'header
	if (miscrom.trainer.in_use) {
		len = 512;
		info.crc32.trainer = emu_crc32((void *)(rom->data + position), len);
		info.crc32.total = info.crc32.trainer;
		position += len;
	}

	// mapper 235
	// 260-in-1 [p1][b1].nes ha un numero di prg_rom_16k_count
	// pari a 256 (0x100) ed essendo un BYTE (0xFF) quello che l'INES
	// utilizza per indicare in numero di 16k, nell'INES header sara'
	// presente 0.
	// 150-in-1 [a1][p1][!].nes ha lo stesso chsum del 260-in-1 [p1][b1].nes
	// ma ha un numero di prg_rom_16k_count di 127.
	if (!info.mapper.prgrom_banks_16k && (info.mapper.id == 235)) {
		info.mapper.prgrom_banks_16k = 256;
	}

	{
		len = !info.mapper.prgrom_banks_16k ? S8K : (size_t)info.mapper.prgrom_banks_16k * S16K;
		difference = !info.mapper.prgrom_banks_16k ? S8K : 0;
		if (len > info.mapper.prgrom_size) {
			len = !info.mapper.prgrom_size ? S8K : info.mapper.prgrom_size;
			difference = !info.mapper.prgrom_size ? S8K : 0;
		}
		if ((position + len) > rom->size) {
			DBWORD banks = ((rom->size - position) / S16K) + ((rom->size - position) % S16K ? 1: 0);

			info.mapper.prgrom_banks_16k = banks <= 1 ? banks : emu_power_of_two(banks);
			len = rom->size - position;
			difference = len < S16K ? 0 : ((size_t)info.mapper.prgrom_banks_16k * S16K) - len;
		}
		// calcolo l'sha1 e il crc32 della PRG Rom
		sha1_csum(&rom->data[position], (int)len, info.sha1sum.prg.value, info.sha1sum.prg.string, LOWER);
		info.crc32.prg = emu_crc32((void *)&rom->data[position], len);
		info.crc32.prg = emu_crc32_zeroes(difference, info.crc32.prg);
		info.crc32.total = emu_crc32_continue((void *)&rom->data[position], len, info.crc32.total);
		position += len;
	}
	if (info.mapper.chrrom_banks_8k) {
		len = (size_t)info.mapper.chrrom_banks_8k * S8K;
		difference = 0;
		if ((position + len) > rom->size) {
			DBWORD banks = ((rom->size - position) / S8K) + ((rom->size - position) % S8K ? 1: 0) ;

			info.mapper.chrrom_banks_8k = banks <= 1 ? banks : emu_power_of_two(banks);
			len = rom->size - position;
			difference = len < S8K ? 0 : ((size_t)info.mapper.prgrom_banks_16k * S16K) - len;
		}
		// calcolo anche l'sha1 e il crc32 della CHR rom
		sha1_csum(&rom->data[position], (int)len, info.sha1sum.chr.value, info.sha1sum.chr.string, LOWER);
		info.crc32.chr = emu_crc32((void *)&rom->data[position], len);
		info.crc32.chr = emu_crc32_zeroes(difference, info.crc32.chr);
		info.crc32.total = emu_crc32_continue((void *)&rom->data[position], len, info.crc32.total);
		position += len;
	}

	if (position < rom->size) {
		len = rom->size - position;
		info.crc32.misc = emu_crc32((void *)(void *)&rom->data[position], len);
		info.crc32.total = emu_crc32_continue((void *)&rom->data[position], len, info.crc32.total);
		position += len;
	}
}
void search_in_database(void) {
	unsigned int i = 0;

	// Nesticle MMC3
	if ((info.mapper.id == 4) && miscrom.trainer.in_use) {
		info.mapper.id = 100;
	}

	// cerco nel database
	for (i = 0; i < LENGTH(dblist); i++) {
		if (!(memcmp(dblist[i].sha1sum, info.sha1sum.prg.string, 40))) {
			info.mapper.id = dblist[i].mapper;
			info.mapper.submapper = dblist[i].submapper == DEFAULT ? 0 : dblist[i].submapper;
			info.machine[DATABASE] = dblist[i].machine;
			info.mirroring_db = dblist[i].mirroring;
			vs_system.ppu = dblist[i].vs_ppu;
			vs_system.special_mode.type = dblist[i].vs_sm;
			info.mapper.expansion = dblist[i].extra;
			switch (info.mapper.id) {
				case 235:
					if (!info.mapper.prgrom_banks_16k) {
						info.mapper.prgrom_banks_16k = 256;
					}
					break;
				default:
					break;
			}
			if (info.mirroring_db == UNK_VERTICAL) {
				info.mapper.mirroring = MIRRORING_VERTICAL;
			}
			if (info.mirroring_db == UNK_HORIZONTAL) {
				info.mapper.mirroring = MIRRORING_HORIZONTAL;
			}
			break;
		}
	}
}
BYTE ines10_search_in_database(void) {
	// setto i default prima della ricerca
	info.machine[DATABASE] = info.mirroring_db = DEFAULT;
	info.mapper.submapper = 0;
	vs_system.ppu = vs_system.special_mode.type = DEFAULT;

	// cerco nel database
	search_in_database();

	if ((vs_system.ppu == DEFAULT) && (vs_system.special_mode.type == DEFAULT)) {
		vs_system.ppu = 0;
		vs_system.special_mode.type = 0;
	}

	return (EXIT_OK);
}
