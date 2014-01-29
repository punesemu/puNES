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

enum flags { FL6, FL7, FL8, FL9, FL10, FL11, FL12, FL13, FL14, FL15, TOTAL_FL };

BYTE ines_load_rom(void) {
	BYTE tmp, flags[TOTAL_FL];
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
		info.prg.rom.banks_16k = fgetc(fp);
		info.chr.rom.banks_8k = fgetc(fp);

		fread(&flags[0], TOTAL_FL, 1, fp);

		if ((flags[FL7] & 0x0C) == 0x08) {
			/* NES 2.0 */
			info.header = NES_2_0;

			info.mapper.id = ((flags[FL8] & 0x0F) << 8) | (flags[FL7] & 0xF0) | (flags[FL6] >> 4);
			info.mapper.submapper = (flags[FL8] & 0xF0) >> 4;

			info.prg.rom.banks_16k |= ((flags[FL9] & 0x0F) << 8);
			info.chr.rom.banks_8k |= ((flags[FL9] & 0xF0) << 4);
			tmp = flags[FL12] & 0x01;
		} else {
			/* iNES 1.0 */
			info.header = iNES_1_0;

			info.mapper.id = (flags[FL7] & 0xF0) | (flags[FL6] >> 4);
			info.prg.ram.bat.banks = (flags[FL6] & 0x02) >> 1;

			tmp = flags[FL9] & 0x01;
		}

		switch (tmp) {
			case 0:
				info.machine[HEADER] = NTSC;
				break;
			case 1:
				info.machine[HEADER] = PAL;
				break;
		}

		info.trainer = flags[FL6] & 0x04;

		if (flags[FL6] & 0x08) {
			mirroring_FSCR();
		} else {
			if (flags[FL6] & 0x01) {
				mirroring_V();
			} else {
				mirroring_H();
			}
		}

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

		if (info.trainer) {
			tmp = fread(&trainer.data, sizeof(trainer.data), 1, fp);
		} else {
			memset(&trainer.data, 0x00, sizeof(trainer.data));
		}

#ifndef RELEASE
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
			return (EXIT_ERROR);
		}

		/* alloco e carico la PRG Rom */
		if ((prg.rom = (BYTE *) malloc(info.prg.rom.banks_16k * (16 * 1024)))) {
			tmp = fread(&prg.rom[0], 16384, info.prg.rom.banks_16k, fp);
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
			if ((chr.data = (BYTE *) malloc(info.chr.rom.banks_8k * (8 * 1024)))) {
				tmp = fread(&chr.data[0], 8192, info.chr.rom.banks_8k, fp);
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
