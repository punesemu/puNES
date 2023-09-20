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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "fds.h"
#include "rom_mem.h"
#include "mappers.h"
#include "info.h"
#include "gui.h"
#include "patcher.h"
#include "conf.h"
#include "ppu.h"

#define BIOSFILE "disksys.rom"
#define DIFFVERSION 1

typedef struct _fds_diff_ele {
	BYTE side;
	WORD value;
	uint32_t position;
} _fds_diff_ele;

_fds fds;

void fds_init(void) {
	memset(&fds, 0x00, sizeof(fds));
	fds.snd.modulation.counter = 0xFFFF;
	fds.snd.wave.counter = 0xFFFF;

	fds.side.change.new_side = 0xFF;

	fds.drive.disk_ejected = TRUE;
	fds.drive.motor_on = TRUE;
	fds.drive.enabled_dsk_reg = 0x01;
	fds.drive.enabled_snd_reg = 0x02;
}
void fds_quit(void) {
	if (fds.side.data) {
		free(fds.side.data);
	}
	if (fds.info.data) {
		free(fds.info.data);
	}
	if (fds.info.diff) {
		fclose(fds.info.diff);
	}
	fds_init();
}
BYTE fds_load_rom(void) {
	_rom_mem rom;
	unsigned int i = 0;

	{
		BYTE found = TRUE;
		uTCHAR rom_ext[2][10] = { uL(".fds\0"), uL(".FDS\0") };
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
			gui_overlay_info_append_msg_precompiled(5, NULL);
			log_error(uL("FDS;error loading rom"));
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

	patcher_apply(&rom);

	fds.info.data = rom.data;

	if (fds_load_bios()) {
		return (EXIT_ERROR);
	}

	// misuro la dimensione del file
	fds.info.total_size = rom.size;

	// riposiziono il puntatore
	rom.position = 0;

	if ((rom.data[rom.position++] == 'F') &&
		(rom.data[rom.position++] == 'D') &&
		(rom.data[rom.position++] == 'S') &&
		(rom.data[rom.position++] == '\32')) {
		fds.info.type = FDS_FORMAT_FDS;
		// il numero di disk sides
		fds.info.total_sides = rom.data[rom.position++];
	} else {
		fds.info.type = FDS_FORMAT_RAW;
		// il numero di disk sides
		fds.info.total_sides = fds.info.total_size / DISK_SIDE_SIZE;
		// mi riposiziono all'inizio
		rom.position = 0;
	}

	info.format = FDS_FORMAT;
	fds.info.expcted_side = fds.info.total_sides;

	// conto le dimensioni dei vari sides
	for (i = 0; i < fds.info.total_sides; i++) {
		fds_disk_op(FDS_DISK_COUNT, i, FALSE);
	}

	// inserisco il primo
	fds.info.frame_insert = ppudata.ppu.frames;
	fds.info.bios_first_run = !cfg->fds_disk1sideA_at_reset;
	fds_disk_op(cfg->fds_disk1sideA_at_reset ? FDS_DISK_SELECT_AND_INSERT : FDS_DISK_SELECT, 0, FALSE);

	info.cpu_rw_extern = TRUE;
	fds.info.enabled = TRUE;

	// WRAM
	wram_set_ram_size(S32K);
	// VRAM
	vram_set_ram_size(S8K);
	// RAM
	ram_set_size(S2K);
	ram_init();
	// NMT
	nmt_set_size(S4K);
	nmt_init();

	info.mapper.id = FDS_MAPPER;
	info.mapper.mirroring = MIRRORING_HORIZONTAL;
	info.machine[HEADER] = NTSC;

	emu_save_header_info();

	return (EXIT_OK);
}
BYTE fds_load_bios(void) {
	uTCHAR bios_file[LENGTH_FILE_NAME_LONG], *lastSlash = NULL;
	FILE *bios = NULL;

	// ordine di ricerca:

	// 1) file specificato dall'utente
	usnprintf(bios_file, usizeof(bios_file), uL("" uPs("")), cfg->fds_bios_file);
	bios = ufopen(bios_file, uL("rb"));
	if (bios) {
		goto fds_load_bios_founded;
	}

	// 2) directory di lavoro
	ustrncpy(bios_file, uL("" BIOSFILE), usizeof(bios_file));
	bios = ufopen(bios_file, uL("rb"));
	if (bios) {
		goto fds_load_bios_founded;
	}

	// 3) directory contenente il file fds
	ustrncpy(bios_file, info.rom.file, usizeof(bios_file));
	// rintraccio l'ultimo '.' nel nome
#if defined (_WIN32)
	lastSlash = ustrrchr(bios_file, uL('\\'));
	if (lastSlash) {
		(*(lastSlash + 1)) = 0x00;
	}
#else
	lastSlash = ustrrchr(bios_file, uL('/'));
	if (lastSlash) {
		(*(lastSlash + 1)) = 0x00;
	}
#endif
	// aggiungo il nome del file
	ustrcat(bios_file, uL("" BIOSFILE));
	bios = ufopen(bios_file, uL("rb"));
	if (bios) {
		goto fds_load_bios_founded;
	}

	// 4) directory puNES/bios
	usnprintf(bios_file, usizeof(bios_file), uL("" uPs("") BIOS_FOLDER "/" BIOSFILE), gui_data_folder());
	bios = ufopen(bios_file, uL("rb"));
	if (bios) {
		goto fds_load_bios_founded;
	}

	gui_overlay_info_append_msg_precompiled(6, NULL);
	log_error(uL("FDS;bios not found"));
	return (EXIT_ERROR);

	fds_load_bios_founded:
	prgrom_set_size(0x2000);

	if (prgrom_init(0x00) == EXIT_ERROR) {
		fclose(bios);
		return (EXIT_ERROR);
	}

	if (fread(prgrom_pnt(), prgrom_size(), 1, bios) < 1) {
		log_error(uL("FDS;error on reading bios"));
	}

	fclose(bios);

	return (EXIT_OK);
}
void fds_info(void) {
	int i = 0;

	log_info_box_open(uL("sides;"));
	if (fds.info.expcted_side != fds.info.total_sides) {
		log_close_box(uL("expected %d, finded %d"),  fds.info.expcted_side, fds.info.total_sides);
	} else {
		log_close_box(uL("%d"), fds.info.total_sides);
	}

	for (i = 0; i < fds.info.total_sides; i++) {
		log_info_box(uL("FDS side %d;vrt size %d, files %d"), i, fds.info.sides[i].size, fds.info.sides[i].files);
	}
}
void fds_info_side(BYTE side) {
	uint32_t i = 0;

	log_info(uL("FDS side;%d"), side);
	log_info_box(uL("vrt disk size;%5d"), fds.info.sides[side].size);
	log_info_box(uL("block 1;pos %5d"), fds.side.block_1.position);
	log_info_box(uL("block 2;pos %5d (fl : %5d)"), fds.side.block_2.position, fds.info.sides[side].files);
	for (i = 0; i < fds.side.counted_files; i++) {
		log_info_box(uL("file %d;size %5d - 0x%04X (b3 : %5d) (b4 : %5d)"), i,
			fds.side.file[i].block_3.length,
			fds.side.file[i].block_3.length,
			fds.side.file[i].block_3.position,
			fds.side.file[i].block_4.position);
	}
}
void fds_disk_op(WORD type, BYTE side_to_insert, BYTE quiet) {
	BYTE buffer[DISK_SIDE_SIZE];
	uint32_t position = 0, size = 0, length = 0;

	if (side_to_insert >= fds.info.total_sides) {
		return;
	}

fds_disk_op_start:
	if (fds.info.type == FDS_FORMAT_FDS) {
		position = (side_to_insert * DISK_SIDE_SIZE) + 16;
	} else {
		position = side_to_insert * DISK_SIDE_SIZE;
	}

	if ((position + DISK_SIDE_SIZE) > fds.info.total_size) {
		fds.info.total_sides = side_to_insert;
		return;
	}

	switch (type) {
		case FDS_DISK_COUNT:
			fds.side.counted_files = 0xFFFF;
			break;
		case FDS_DISK_EJECT:
			fds.drive.disk_ejected = TRUE;
			if (!quiet) {
				gui_overlay_info_append_msg_precompiled(7, NULL);
			}
			return;
		case FDS_DISK_INSERT:
			if (!fds.drive.disk_ejected) {
				if (!quiet) {
					gui_overlay_info_append_msg_precompiled(8, NULL);
				}
				return;
			}

			fds.info.bios_first_run = FALSE;
			fds.drive.disk_position = 0;
			fds.drive.gap_ended = FALSE;

			fds.drive.disk_ejected = FALSE;
			if (!quiet) {
				gui_overlay_info_append_msg_precompiled(9, NULL);
			}
			return;
		case FDS_DISK_SELECT:
		case FDS_DISK_SELECT_AND_INSERT:
		case FDS_DISK_SELECT_FROM_REWIND:
			if ((type == FDS_DISK_SELECT) && !fds.drive.disk_ejected) {
				gui_overlay_info_append_msg_precompiled(8, NULL);
				return;
			}
			if (fds.side.data) {
				free(fds.side.data);
				fds.side.data = NULL;
			}
			fds.side.data = (WORD *)malloc(fds.info.sides[side_to_insert].size * sizeof(WORD));
			fds.side.counted_files = 0xFFFF;
			break;
		default:
			break;
	}

	memcpy(buffer, fds.info.data + position, DISK_SIDE_SIZE);

	position = 0;

#define add_to_image(ty, md, vl, sz)\
	if ((ty) >= FDS_DISK_SELECT) {\
		if ((md) == FDS_DISK_MEMSET) {\
			WORD *dst = fds.side.data + size;\
			uint32_t i = 0;\
			for (i = 0; i < (sz); i++) {\
				(*dst++) = (vl);\
			}\
		} else if ((md) == FDS_DISK_MEMCPY) {\
			BYTE *src = buffer + position;\
			WORD *dst = fds.side.data + size;\
			uint32_t i = 0;\
			for (i = 0; i < (sz); i++) {\
				(*dst++) = (*src++);\
			}\
		}\
	}\
	size += (sz)

	add_to_image(type, FDS_DISK_MEMSET, FDS_DISK_GAP, 28300 / 8);

	for (position = 0; position < DISK_SIDE_SIZE;) {
		BYTE block = buffer[position], stop = FALSE;
		uint32_t blength = 1;

		switch (block) {
			case BL_DISK_INFO:
				// le info sul disco
				blength = 56;
				break;
			case BL_FILE_AMOUNT:
				// il numero dei file immagazzinati nel disco
				blength = 2;
				break;
			case BL_FILE_HEADER:
				// l'header del file
				length = buffer[position + 13] + (0x100 * buffer[position + 14]);
				blength = 16;
				break;
			case BL_FILE_DATA:
				// il contenuto del file
				blength = length + 1;
				break;
			default:
				// nel caso il disco sia "sporco"
				stop = TRUE;
				break;
		}

		// in "Tobidase Daisakusen (1987)(Square)(J).fds" esiste un file nascosto
		// esattamente dopo l'ultimo file "riconosciuto" dal file system.
		// Il vecchio controllo che facevo per riconoscere i dischi "sporchi"
		// si basava sul numero totale dei files che il file system si aspettava
		// ci fossero (fds.side.block_2.tot_files), quindi il file nascosto non
		// veniva mai letto, non permettendo l'avvio corretto dell'fds. Adesso il
		// controllo lo eseguo direttamente sul byte del blocco. Se non e' tra i
		// blocchi riconosciuti allora considero l'analisi del disco completa e
		// tralascio tutto quello che sta dopo (in questo modo funziona anche
		// "Akumajou Dracula v1.02 (1986)(Konami)(J).fds" il cui disco e' "sporco").
		if (stop) {
			break;
		}

		if (block) {
			// indico l'inizio del blocco
			add_to_image(type, FDS_DISK_MEMSET, FDS_DISK_BLOCK_MARK, 1);

			if (type == FDS_DISK_COUNT) {
				switch (block) {
					case 1:
						break;
					case 2:
						fds.info.sides[side_to_insert].files = 0;
						break;
					case 3:
						break;
					case 4:
						fds.info.sides[side_to_insert].files++;
						break;
					default:
						break;
				}
			} else if (type >= FDS_DISK_SELECT) {
				switch (block) {
					case 1:
						fds.side.block_1.position = size;
						break;
					case 2:
						fds.side.block_2.position = size;
						fds.side.block_2.tot_files = buffer[position + 1];
						// a questo punto fds.side.counted_files e' 0xFFFF
						fds.side.counted_files = 0;
						fds.info.sides[side_to_insert].files = 0;
						break;
					case 3:
						fds.side.file[fds.side.counted_files].block_3.position = size;
						fds.side.file[fds.side.counted_files].block_3.length = length;
						break;
					case 4:
						fds.side.file[fds.side.counted_files].block_4.position = size;
						fds.side.counted_files++;
						fds.info.sides[side_to_insert].files++;
						break;
					default:
						break;
				}
			}

			add_to_image(type, FDS_DISK_MEMCPY, 0, blength);
			// dummy CRC
			add_to_image(type, FDS_DISK_MEMSET, FDS_DISK_CRC_CHAR1, 1);
			add_to_image(type, FDS_DISK_MEMSET, FDS_DISK_CRC_CHAR2, 1);
			// 1016 bit di gap alla fine di ogni blocco.
			// Note : con 976 funziona correttamente la read del disco ma non e'
			// sufficiente per la write.
			add_to_image(type, FDS_DISK_MEMSET, FDS_DISK_GAP, 1016 / 8);
		}
		position += blength;
	}

	if ((info.reset != CHANGE_ROM) && (type >= FDS_DISK_SELECT)) {
		if (fds.info.frame_insert != ppudata.ppu.frames) {
			fds.info.frame_insert = ppudata.ppu.frames;
			fds_info_side(side_to_insert);
		}
	}

	if (size < DISK_SIDE_SIZE) {
		add_to_image(type, FDS_DISK_MEMSET, 0x0000, DISK_SIDE_SIZE - size);
	}

	add_to_image(type, FDS_DISK_MEMSET, FDS_DISK_GAP, 2000);

#undef add_to_image

	switch (type) {
		case FDS_DISK_COUNT:
			fds.info.sides[side_to_insert].size = size;
			break;
		case FDS_DISK_SELECT_AND_INSERT:
			type = FDS_DISK_INSERT;
			fds.auto_insert.rE445.in_run = FALSE;
			fds.side.change.new_side = 0xFF;
			fds.drive.side_inserted = side_to_insert;
			fds_diff_op(FDS_OP_READ, 0, 0);
			goto fds_disk_op_start;
		case FDS_DISK_SELECT:
			fds.auto_insert.rE445.in_run = FALSE;
			fds.side.change.new_side = 0xFF;
			fds.drive.side_inserted = side_to_insert;
			gui_overlay_info_append_msg_precompiled(10, NULL);
			fds_diff_op(FDS_OP_READ, 0, 0);
			break;
		default:
			break;
	}
}
void fds_diff_op(BYTE mode, uint32_t position, WORD value) {
	if (!fds.info.diff) {
		uTCHAR file[LENGTH_FILE_NAME_LONG];
		uTCHAR ext[10], basename[255], *last_dot = NULL;

		gui_utf_basename(info.rom.file, basename, usizeof(basename));
		usnprintf(file, usizeof(file), uL("" uPs("") DIFF_FOLDER "/" uPs("")), gui_data_folder(), basename);
		usnprintf(ext, usizeof(ext), uL(".dif"));

		// rintraccio l'ultimo '.' nel nome
		last_dot = ustrrchr(file, uL('.'));
		if (last_dot) {
			// elimino l'estensione
			(*last_dot) = 0x00;
		}
		// aggiungo l'estensione
		ustrcat(file, ext);

		fds.info.diff = ufopen(file, uL("r+b"));

		if ((mode == FDS_OP_WRITE) && !fds.info.diff) {
			// creo il file
			fds.info.diff = ufopen(file, uL("a+b"));
			if (fds.info.diff) {
				// lo chiudo
				fclose(fds.info.diff);
				// lo riapro in modalita' rb+
				fds.info.diff = ufopen(file, uL("r+b"));
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

		// salvo la versione
		if (fwrite(&version, sizeof(uint32_t), 1, fds.info.diff) < 1) {
			fprintf(stderr, "error on write version fds diff file\n");
		}
		// senza questo in windows non funziona correttamente
		fflush(fds.info.diff);

		out.side = fds.drive.side_inserted;
		out.position = position;
		out.value = value;

		while (fread(&in, sizeof(_fds_diff_ele), 1, fds.info.diff)) {
			if ((in.position == out.position) && (in.side == out.side)) {
				fseek(fds.info.diff, ftell(fds.info.diff) - (long)sizeof(_fds_diff_ele), SEEK_SET);
				break;
			}
		}

		if (fwrite(&out, sizeof(_fds_diff_ele), 1, fds.info.diff) < 1) {
			log_error(uL("FDS;error on writing diff file"));
		}
		// senza questo in windows non funziona correttamente
		fflush(fds.info.diff);
	} else if (mode == FDS_OP_READ) {
		_fds_diff_ele ele;
		uint32_t version = 0;

		// leggo la versione del file
		if (fread(&version, sizeof(uint32_t), 1, fds.info.diff) < 1) {
			log_error(uL("FDS;error on reading version diff file"));
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
