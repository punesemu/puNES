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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libgen.h>
#include "fds.h"
#include "cpu.h"
#include "mappers.h"
#include "mem_map.h"
#include "text.h"
#include "emu.h"
#include "clock.h"
#include "info.h"

#define BIOSFILE "disksys.rom"
#define DIFFVERSION 1

typedef struct {
	BYTE side;
	WORD value;
	uint32_t position;
} _fds_diff_ele;

void fds_init(void) {
	memset(&fds, 0x00, sizeof(fds));

	fds.drive.disk_ejected = TRUE;
	fds.drive.motor_on = TRUE;
	fds.drive.enabled_dsk_reg = 0x01;
	fds.drive.enabled_snd_reg = 0x02;
}
void fds_quit(void) {
	if (fds.side.data) {
		free(fds.side.data);
	}
	if (fds.info.fp) {
		fclose(fds.info.fp);
	}
	if (fds.info.diff) {
		fclose(fds.info.diff);
	}
	fds_init();
}
BYTE fds_load_rom(void) {
	BYTE i;

	{
		BYTE found = TRUE;
		char rom_ext[2][10] = { ".fds\0", ".FDS\0" };

		fds.info.fp = fopen(info.rom_file, "rb");

		if (!fds.info.fp) {
			found = FALSE;

			for (i = 0; i < LENGTH(rom_ext); i++) {
				char rom_file[LENGTH_FILE_NAME_MID];

				strncpy(rom_file, info.rom_file, sizeof(rom_file));
				strcat(rom_file, rom_ext[i]);

				fds.info.fp = fopen(rom_file, "rb");

				if (fds.info.fp) {
					strncpy(info.rom_file, rom_file, sizeof(info.rom_file));
					found = TRUE;
					break;
				}
			}
		}

		if (!found) {
			text_add_line_info(1, "[red]error loading rom");
			fprintf(stderr, "error loading rom\n");
			return (EXIT_ERROR);
		}
	}

	if (fds_load_bios()) {
		return (EXIT_ERROR);
	}

	/* misuro la dimensione del file */
	fseek(fds.info.fp, 0, SEEK_END);
	fds.info.total_size = ftell(fds.info.fp);

	/* riposiziono il puntatore all'inizio del file */
	rewind(fds.info.fp);

	if ((fgetc(fds.info.fp) == 'F') && (fgetc(fds.info.fp) == 'D') && (fgetc(fds.info.fp) == 'S')
			&& (fgetc(fds.info.fp) == '\32')) {
		fds.info.type = FDS_FORMAT_FDS;
		/* il numero di disk sides */
		fds.info.total_sides = fgetc(fds.info.fp);
	} else {
		fds.info.type = FDS_FORMAT_RAW;
		/* il numero di disk sides */
		fds.info.total_sides = fds.info.total_size / DISK_SIDE_SIZE;
		/* mi riposiziono all'inizio del file */
		rewind(fds.info.fp);
	}

	info.format = FDS_FORMAT;

	/* conto le dimensioni dei vari sides */
	for (i = 0; i < fds.info.total_sides; i++) {
		fds_disk_op(FDS_DISK_COUNT, i);
	}

	/* inserisco il primo */
	fds_disk_op(FDS_DISK_SELECT_AND_INSERT, 0);

	fds.info.enabled = TRUE;

	/* Prg Ram */
	if (map_prg_ram_malloc(0x8000) != EXIT_OK) {
		return (EXIT_ERROR);
	}

	info.mapper.id = FDS_MAPPER;
	info.machine[HEADER] = NTSC;

	return (EXIT_OK);
}
BYTE fds_load_bios(void) {
	char bios_file[LENGTH_FILE_NAME_MID], *lastSlash;
	FILE *bios = NULL;

	/*
	 * ordine di ricerca:
	 * 1) directory di lavoro
	 * 2) directory contenente il file fds
	 * 3) directory puNES/bios
	 */
	if ((bios = fopen(BIOSFILE, "rb"))) {
		goto fds_load_bios_founded;
	}

	/* copio il nome del file nella variabile */
	strcpy(bios_file, info.rom_file);
	/* rintraccio l'ultimo '.' nel nome */
#if defined (__WIN32__)
	if ((lastSlash = strrchr(bios_file, '\\'))) {
		(*(lastSlash + 1)) = 0x00;
	}
#else
	if ((lastSlash = strrchr(bios_file, '/'))) {
		(*(lastSlash + 1)) = 0x00;
	}
#endif
	/* aggiungo il nome del file */
	strcat(bios_file, BIOSFILE);

	if ((bios = fopen(bios_file, "rb"))) {
		goto fds_load_bios_founded;
	}

	sprintf(bios_file, "%s" BIOS_FOLDER "/%s", info.base_folder, BIOSFILE);

	if ((bios = fopen(bios_file, "rb"))) {
		goto fds_load_bios_founded;
	}

	text_add_line_info(1, "[red]'bios/disksys.rom' not found");
	fprintf(stderr, "'bios/disksys.rom' not found\n");
	return (EXIT_ERROR);

	fds_load_bios_founded:
	if (map_prg_chip_malloc(0, 0x2000, 0x00) == EXIT_ERROR) {
		fclose(bios);
		return (EXIT_ERROR);
	}

	if (fread(prg_chip(0), 0x2000, 1, bios) < 1) {
		fprintf(stderr, "error on reading fds bios\n");
	}

	fclose(bios);

	return (EXIT_OK);
}
void fds_disk_op(WORD type, BYTE side_to_insert) {
	BYTE buffer[DISK_SIDE_SIZE];
	uint32_t position, size = 0, length = 0;

	if (side_to_insert >= fds.info.total_sides) {
		return;
	}

	fds_disk_op_start:
	switch (type) {
		case FDS_DISK_COUNT:
			fds.side.counted_files = 0xFFFF;
			break;
		case FDS_DISK_EJECT:
			fds.drive.disk_ejected = TRUE;
			text_add_line_info(1, "Disk [cyan]%d [normal]side [cyan]%c[normal]"
					" [yellow]ejected", (fds.drive.side_inserted / 2) + 1,
					(fds.drive.side_inserted & 0x01) + 'A');
			return;
		case FDS_DISK_INSERT:
			if (!fds.drive.disk_ejected) {
				text_add_line_info(1, "you must [yellow]eject[normal] disk first");
				return;
			}

			fds.drive.disk_position = 0;
			fds.drive.gap_ended = FALSE;

			fds.drive.disk_ejected = FALSE;

			text_add_line_info(1, "Disk [cyan]%d [normal]side [cyan]%c [green]inserted",
					(fds.drive.side_inserted / 2) + 1, (fds.drive.side_inserted & 0x01) + 'A');
			return;
		case FDS_DISK_SELECT:
		case FDS_DISK_SELECT_AND_INSERT:
		case FDS_DISK_TIMELINE_SELECT:
			if ((type == FDS_DISK_SELECT) && !fds.drive.disk_ejected) {
				text_add_line_info(1, "you must [yellow]eject[normal] disk first");
				return;
			}
			if (fds.side.data) {
				free(fds.side.data);
				fds.side.data = NULL;
			}

#if !defined (RELEASE)
			fprintf(stdout, "virtual disk size : %5d\n", fds.info.sides_size[side_to_insert]);
#endif

			fds.side.data = (WORD *) malloc(fds.info.sides_size[side_to_insert] * sizeof(WORD));

			fds.side.counted_files = 0xFFFF;

			break;
	}

	if (fds.info.type == FDS_FORMAT_FDS) {
		position = (side_to_insert * DISK_SIDE_SIZE) + 16;
	} else {
		position = side_to_insert * DISK_SIDE_SIZE;
	}

	fseek(fds.info.fp, position, SEEK_SET);
	if (fread(buffer, DISK_SIDE_SIZE, 1, fds.info.fp) < 1) {
		fprintf(stderr, "error in fds disk\n");
	}

	position = 0;

#define add_to_image(ty, md, vl, sz)\
	if (type >= FDS_DISK_SELECT) {\
		if (md == FDS_DISK_MEMSET) {\
			WORD *dst = fds.side.data + size;\
			uint32_t i;\
			for (i = 0; i < sz; i++) {\
				(*dst++) = vl;\
			}\
		} else if (md == FDS_DISK_MEMCPY) {\
			BYTE *src = buffer + position;\
			WORD *dst = fds.side.data + size;\
			uint32_t i;\
			for (i = 0; i < sz; i++) {\
				(*dst++) = (*src++);\
			}\
		}\
	}\
	size += sz

	add_to_image(type, FDS_DISK_MEMSET, FDS_DISK_GAP, 28300 / 8);

	for (position = 0; position < DISK_SIDE_SIZE;) {
		BYTE block = buffer[position], stop = FALSE;
		uint32_t blength = 1;

		switch (block) {
			case BL_DISK_INFO:
				/* le info sul disco */
				blength = 56;
				break;
			case BL_FILE_AMOUNT:
				/* il numero dei file immagazzinati nel disco */
				blength = 2;
				break;
			case BL_FILE_HEADER:
				/* l'header del file */
				length = buffer[position + 13] + (0x100 * buffer[position + 14]);
				blength = 16;
				break;
			case BL_FILE_DATA:
				/* il contenuto del file */
				blength = length + 1;
				break;
			default:
				/* nel caso il disco sia "sporco" */
				stop = TRUE;
				break;
		}

		/*
		 * in "Tobidase Daisakusen (1987)(Square)(J).fds" esiste un file nascosto
		 * esattamente dopo l'ultimo file "riconosciuto" dal file system.
		 * Il vecchio controllo che facevo per riconoscere i dischi "sporchi"
		 * si basava sul numero totale dei files che il file system si aspettava
		 * ci fossero (fds.side.block_2.tot_files), quindi il file nascosto non
		 * veniva mai letto, non permettendo l'avvio corretto dell'fds. Adesso il
		 * controllo lo eseguo direttamente sul byte del blocco. Se non e' tra i
		 * blocchi riconosciuti allora considero l'analisi del disco completa e
		 * tralascio tutto quello che sta dopo (in questo modo funziona anche
		 * "Akumajou Dracula v1.02 (1986)(Konami)(J).fds" il cui disco e' "sporco").
		 */
		if (stop == TRUE) {
			break;
		}

		if (block) {
			/* indico l'inizio del blocco */
			add_to_image(type, FDS_DISK_MEMSET, FDS_DISK_BLOCK_MARK, 1);

			if (type >= FDS_DISK_SELECT) {
				switch (block) {
					case 1:
						fds.side.block_1.position = size;
#if !defined (RELEASE)
						fprintf(stdout, "block 1 : (pos  : %5d)\n", fds.side.block_1.position);
#endif
						break;
					case 2:
						fds.side.block_2.position = size;
						fds.side.block_2.tot_files = buffer[position + 1];

						/* a questo punto fds.side.counted_files e' 0xFFFF */
						fds.side.counted_files = 0;

#if !defined (RELEASE)
						fprintf(stdout, "block 2 : (pos  : %5d) (fl : %5d)\n",
								fds.side.block_2.position,
								fds.side.block_2.tot_files);
#endif
						break;
					case 3:
						fds.side.file[fds.side.counted_files].block_3.position = size;
						fds.side.file[fds.side.counted_files].block_3.length = length;
						break;
					case 4:
						fds.side.file[fds.side.counted_files].block_4.position = size;
#if !defined (RELEASE)
						fprintf(stdout, "file %2d : (size : %5d - 0x%04X) (b3 : %5d) (b4 : %5d)\n",
								fds.side.counted_files,
								fds.side.file[fds.side.counted_files].block_3.length,
								fds.side.file[fds.side.counted_files].block_3.length,
								fds.side.file[fds.side.counted_files].block_3.position,
								fds.side.file[fds.side.counted_files].block_4.position);
#endif
						fds.side.counted_files++;
						break;
				}
			}

			add_to_image(type, FDS_DISK_MEMCPY, 0, blength);
			/* dummy CRC */
			add_to_image(type, FDS_DISK_MEMSET, FDS_DISK_CRC_CHAR1, 1);
			add_to_image(type, FDS_DISK_MEMSET, FDS_DISK_CRC_CHAR2, 1);
			/*
			 * 1016 bit di gap alla fine di ogni blocco.
			 * Note : con 976 funziona correttamente la read del disco ma non e'
			 * sufficiente per la write.
			 */
			add_to_image(type, FDS_DISK_MEMSET, FDS_DISK_GAP, 1016 / 8);
		}
		position += blength;
	}

	if (size < DISK_SIDE_SIZE) {
		add_to_image(type, FDS_DISK_MEMSET, 0x0000, DISK_SIDE_SIZE - size);
	}

	add_to_image(type, FDS_DISK_MEMSET, FDS_DISK_GAP, 2000);

#undef add_to_image

	switch (type) {
		case FDS_DISK_COUNT:
			fds.info.sides_size[side_to_insert] = size;
			break;
		case FDS_DISK_SELECT_AND_INSERT:
			type = FDS_DISK_INSERT;
			fds.drive.side_inserted = side_to_insert;
			fds_diff_op(FDS_OP_READ, 0, 0);
			goto fds_disk_op_start;
		case FDS_DISK_SELECT:
			fds.drive.side_inserted = side_to_insert;
			text_add_line_info(1, "Disk [cyan]%d [normal]side [cyan]%c [brown]selected",
					(fds.drive.side_inserted / 2) + 1, (fds.drive.side_inserted & 0x01) + 'A');
			fds_diff_op(FDS_OP_READ, 0, 0);
			break;
	}
}
void fds_diff_op(BYTE mode, uint32_t position, WORD value) {
	if (!fds.info.diff) {
		char file[LENGTH_FILE_NAME_MID];
		char ext[10], *last_dot;

		sprintf(file, "%s" DIFF_FOLDER "/%s", info.base_folder, basename(info.rom_file));
		sprintf(ext, ".dif");

		/* rintraccio l'ultimo '.' nel nome */
		last_dot = strrchr(file, '.');
		/* elimino l'estensione */
		*last_dot = 0x00;
		/* aggiungo l'estensione */
		strcat(file, ext);

		fds.info.diff = fopen(file, "r+b");

		if ((mode == FDS_OP_WRITE) && !fds.info.diff) {
			/* creo il file */
			if ((fds.info.diff = fopen(file, "a+b"))) {
				/* lo chiudo */
				fclose(fds.info.diff);
				/* lo riapro in modalita' rb+ */
				fds.info.diff = fopen(file, "r+b");
			}
		}
	}

	if (!fds.info.diff) {
		return;
	}

	rewind(fds.info.diff);

	if (mode == FDS_OP_WRITE) {
		_fds_diff_ele in, out;
		uint32_t version = DIFFVERSION;

		/* salvo la versione */
		if (fwrite(&version, sizeof(uint32_t), 1, fds.info.diff) < 1) {
			fprintf(stderr, "error on write version fds diff file\n");
		}
		/* senza questo in windows non funziona correttamente */
		fflush(fds.info.diff);

		out.side = fds.drive.side_inserted;
		out.position = position;
		out.value = value;

		while (fread(&in, sizeof(_fds_diff_ele), 1, fds.info.diff)) {
			if ((in.position == out.position) && (in.side == out.side)) {
				fseek(fds.info.diff, ftell(fds.info.diff) - sizeof(_fds_diff_ele), SEEK_SET);
				break;
			}
		}

		if (fwrite(&out, sizeof(_fds_diff_ele), 1, fds.info.diff) < 1) {
			fprintf(stderr, "error on write fds diff file\n");
		}
		/* senza questo in windows non funziona correttamente */
		fflush(fds.info.diff);
	} else if (mode == FDS_OP_READ) {
		_fds_diff_ele ele;
		uint32_t version;

		/* leggo la versione del file */
		if (fread(&version,  sizeof(uint32_t), 1, fds.info.diff) < 1) {
			fprintf(stderr, "error on error version fds diff file\n");
		}

		while (fread(&ele, sizeof(_fds_diff_ele), 1, fds.info.diff)) {
			if (ele.side == fds.drive.side_inserted) {
				fds.side.data[ele.position] = ele.value;
			}
		}

		fclose(fds.info.diff);
		fds.info.diff = NULL;
	}
}
