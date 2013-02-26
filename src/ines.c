/*
 * ines.c
 *
 *  Created on: 25/mar/2012
 *      Author: fhorse
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ines.h"
#include "fds.h"
#include "mem_map.h"
#include "mappers.h"
#include "emu.h"
#include "cfg_file.h"
#include "clock.h"
#include "gamegenie.h"

BYTE ines_load_rom(void) {
	BYTE tmp, flags6, flags7;
	FILE *fp;

	{
		BYTE i, found = TRUE;
		char rom_ext[2][10] = { ".nes\0", ".NES\0" };

		fp = fopen(info.rom_file, "rb");

		if (!fp) {
			found = FALSE;

			for (i = 0; i < LENGTH(rom_ext); i++) {
				char rom_file[1024];

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

	if ((fgetc(fp) == 'N') && (fgetc(fp) == 'E') && (fgetc(fp) == 'S') && (fgetc(fp) == '\32')) {
		info.prg_rom_16k_count = fgetc(fp);
		info.chr_rom_8k_count = fgetc(fp);

		flags6 = fgetc(fp);
		flags7 = fgetc(fp);
		info.mapper = (flags7 & 0xF0) | (flags6 >> 4);
		info.prg_ram_bat_banks = (flags6 & 0x02) >> 1;
		info.trainer = flags6 & 0x04;
		if (flags6 & 0x08) {
			mirroring_FSCR();
		} else {
			if (flags6 & 0x01) {
				mirroring_V();
			} else {
				mirroring_H();
			}
		}

		/* il byte 8 non mi interessa */
		fgetc(fp);

		tmp = fgetc(fp);
		tmp &= 0x01;
		tmp = (~tmp & 0x01);

		/*
		 * inizializzo qui il writeVRAM per la mapper 96 perche'
		 * e' l'unica mapper che utilizza 32k di CHR Ram e che
		 * si permette anche il lusso di swappare. Quindi imposto
		 * a FALSE qui in modo da poter cambiare impostazione nel
		 * emu_search_in_database.
		 */
		mapper.write_vram = FALSE;

		if (emu_search_in_database(fp)) {
			return (EXIT_ERROR);
		}

		if (info.machine == DEFAULT) {
			info.machine = tmp;
		}

#ifndef RELEASE
		fprintf(stderr, "mapper %u\n8k rom = %u\n4k vrom = %u\n", info.mapper,
				info.prg_rom_16k_count * 2, info.chr_rom_8k_count * 2);
		fprintf(stderr, "sha1prg = %40s\n", info.sha1sum_string);
		fprintf(stderr, "sha1chr = %40s\n", info.sha1sum_string_chr);
#endif

		if (!info.chr_rom_8k_count) {
			mapper.write_vram = TRUE;
			info.chr_rom_8k_count = 1;
		}

		info.prg_rom_8k_count = info.prg_rom_16k_count * 2;
		info.chr_rom_4k_count = info.chr_rom_8k_count * 2;
		info.chr_rom_1k_count = info.chr_rom_4k_count * 4;

		if (info.prg_ram_bat_banks) {
			info.prg_ram_plus_8k_count = 1;
		}

		/* alloco la PRG Ram */
		if (!(prg.ram = malloc(0x2000))) {
			fprintf(stderr, "Out of memory\n");
			return (EXIT_ERROR);
		}

		/* alloco e carico la PRG Rom */
		if ((prg.rom = malloc(info.prg_rom_16k_count * (16 * 1024)))) {
			tmp = fread(&prg.rom[0], 16384, info.prg_rom_16k_count, fp);
		} else {
			fprintf(stderr, "Out of memory\n");
			return (EXIT_ERROR);
		}

		/*
		 * se e' settato mapper.write_vram, vuol dire
		 * che la rom non ha CHR Rom e che quindi la CHR Ram
		 * la trattero' nell'inizializzazione della mapper
		 * (perche' alcune mapper ne hanno 16k, altre 8k).
		 */
		if (!mapper.write_vram) {
			/* alloco la CHR Rom */
			if ((chr.data = malloc(info.chr_rom_8k_count * (8 * 1024)))) {
				tmp = fread(&chr.data[0], 8192, info.chr_rom_8k_count, fp);
				chr_bank_1k_reset();
			} else {
				fprintf(stderr, "Out of memory\n");
				return (EXIT_ERROR);
			}
		}
	} else {
		fprintf(stderr, "Formato non riconosciuto.\n");
		fclose(fp);
		return (EXIT_ERROR);
	}
	fclose(fp);

	return (EXIT_OK);
}
