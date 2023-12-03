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
#include <string.h>
#include "fds.h"
#include "rom_mem.h"
#include "mappers.h"
#include "info.h"
#include "gui.h"
#include "patcher.h"
#include "conf.h"

#define BIOSFILE "disksys.rom"
#define DIFFVERSION 1

typedef struct _fds_sinfo {
	struct _fds_side_block1 {
		uint32_t position;
		BYTE name[3 + 1];
		WORD gversion;
		WORD snumber;
		WORD dnumber;
	} block1;
	struct _fds_side_block2 {
		uint32_t position;
		WORD files;
	} block2;
	struct _fds_side_file {
		struct _fds_side_file_block3 {
			uint32_t position;
			uint32_t length;
			BYTE name[8 + 1];
			WORD type;
		} block3;
		struct _fds_side_file_block4 {
			uint32_t position;
		} block4;
	} file[0xFF];
	uint32_t counted_files;
} _fds_sinfo;
typedef struct _fds_diff_ele {
	BYTE side;
	WORD value;
	uint32_t position;
} _fds_diff_ele;

void fds_to_image(void);
void fds_from_image(void);
void fds_diff_file_name(uTCHAR *dst, size_t lenght);
void fds_image_sinfo(BYTE side, _fds_sinfo *sinfo);
void fds_image_memset(WORD *dst, WORD value, uint32_t lenght);
void fds_image_memcpy(const BYTE *src, WORD *dst, uint32_t lenght);
void fds_image_memcpy_ASCII(const WORD *src, BYTE *dst, size_t lenght);
WORD fds_block_crc(const WORD *src, uint32_t lenght);

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
	if (fds.info.image) {
		fds_from_image();
		free(fds.info.image);
	}
	if (fds.info.data) {
		free(fds.info.data);
	}
	if (fds.info.diff) {
		fclose(fds.info.diff);
	}
	fds_init();
}
BYTE fds_load_rom(BYTE type) {
	_rom_mem rom;
	unsigned int i = 0;

	{
		BYTE found = TRUE;
		uTCHAR rom_ext[2][10] = { uL(".fds\0"), uL(".FDS\0") };
		FILE *fp = NULL;

		if (type == QD_FORMAT) {
			ustrncpy(&rom_ext[0][0], uL(".qd\0"), usizeof(rom_ext[0]));
			ustrncpy(&rom_ext[0][0], uL(".QD\0"), usizeof(rom_ext[0]));
		}

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
		fds.info.total_sides = fds.info.total_size / fds_disk_side_size();
		// mi riposiziono all'inizio
		rom.position = 0;
	}

	info.format = type;
	info.number_of_nes = 1;
	fds.info.expcted_side = fds.info.total_sides;

	// converto nel mio formato immagine
	fds_to_image();

	// inserisco il primo
	fds.info.frame_insert = nes[0].p.ppu.frames;
	fds.info.bios_first_run = !cfg->fds_disk1sideA_at_reset;
	fds_disk_op(cfg->fds_disk1sideA_at_reset ? FDS_DISK_SELECT_AND_INSERT : FDS_DISK_SELECT, 0, FALSE);

	info.cpu_rw_extern = TRUE;
	fds.info.enabled = TRUE;

	// WRAM
	wram_set_ram_size(S32K);
	// VRAM
	vram_set_ram_size(0, S8K);
	// RAM
	ram_set_size(0, S2K);
	ram_init();
	// NMT
	nmt_set_size(0, S4K);
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
	log_info_box_open(uL("sides;"));
	if (fds.info.expcted_side != fds.info.total_sides) {
		log_close_box(uL("expected %d, finded %d"),  fds.info.expcted_side, fds.info.total_sides);
	} else {
		log_close_box(uL("%d"), fds.info.total_sides);
	}
	for (int side = 0; side < fds.info.total_sides; side++) {
		_fds_sinfo sinfo = { 0 };

		fds_image_sinfo(side, &sinfo);
		log_info_box(uL("FDS side %d;vrt size %d, files %d"), side, fds.info.sides[side].size, sinfo.counted_files);
	}
}
void fds_info_side(BYTE side) {
	_fds_sinfo sinfo = { 0 };

	fds_image_sinfo(side, &sinfo);
	log_info(uL("FDS side %d;disk %d, side %X, name %3s, version %d"), side,
		sinfo.block1.dnumber,
		sinfo.block1.snumber + 0x0A,
		sinfo.block1.name,
		sinfo.block1.gversion);
	log_info_box(uL("vrt disk size;%d"), fds.info.sides[side].size);
	log_info_box(uL("block 1;pos %5d"), sinfo.block1.position);
	log_info_box(uL("block 2;pos %5d, files %d, counted %d"),
		sinfo.block2.position,
		sinfo.block2.files,
		sinfo.counted_files);
	for (uint32_t i = 0; i < sinfo.counted_files; i++) {
		log_info_box(uL("file %d;name %8s, size %5d, 0x%04X (b3 : %5d) (b4 : %5d)"), i,
			sinfo.file[i].block3.name,
			sinfo.file[i].block3.length,
			sinfo.file[i].block3.length,
			sinfo.file[i].block3.position,
			sinfo.file[i].block4.position);
	}
}
void fds_disk_op(WORD type, BYTE side_to_insert, BYTE quiet) {
	if (side_to_insert >= fds.info.total_sides) {
		return;
	}

fds_disk_op_start:
	switch (type) {
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
			fds.side.info = &fds.info.sides[side_to_insert];
			break;
		default:
			break;
	}

	if ((info.reset != CHANGE_ROM) && (type >= FDS_DISK_SELECT)) {
		if (fds.info.frame_insert != nes[0].p.ppu.frames) {
			fds.info.frame_insert = nes[0].p.ppu.frames;
			fds_info_side(side_to_insert);
		}
	}

	switch (type) {
		case FDS_DISK_SELECT_AND_INSERT:
			type = FDS_DISK_INSERT;
			fds.auto_insert.rE445.in_run = FALSE;
			fds.side.change.new_side = 0xFF;
			fds.drive.side_inserted = side_to_insert;
			goto fds_disk_op_start;
		case FDS_DISK_SELECT:
			fds.auto_insert.rE445.in_run = FALSE;
			fds.side.change.new_side = 0xFF;
			fds.drive.side_inserted = side_to_insert;
			gui_overlay_info_append_msg_precompiled(10, NULL);
			break;
		default:
			break;
	}
}
void fds_diff_op(BYTE side, BYTE mode, uint32_t position, WORD value) {
	if (!fds.info.diff) {
		uTCHAR file[LENGTH_FILE_NAME_LONG];

		fds_diff_file_name(&file[0], usizeof(file));
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
	fds.info.writings_occurred = TRUE;

	if (mode == FDS_OP_WRITE) {
		_fds_diff_ele in = { 0 }, out = { 0 };
		uint32_t version = DIFFVERSION;

		// salvo la versione
		if (fwrite(&version, sizeof(uint32_t), 1, fds.info.diff) < 1) {
			fprintf(stderr, "error on write version fds diff file\n");
		}
		// senza questo in windows non funziona correttamente
		fflush(fds.info.diff);

		out.side = side;
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
		WORD *dst = &fds.info.image[side * fds_image_side_size()];
		_fds_diff_ele ele = { 0 };
		uint32_t version = 0;

		// leggo la versione del file
		if (fread(&version, sizeof(uint32_t), 1, fds.info.diff) < 1) {
			log_error(uL("FDS;error on reading version diff file"));
		}

		while (fread(&ele, sizeof(_fds_diff_ele), 1, fds.info.diff)) {
			if (ele.side == side) {
				dst[ele.position] = ele.value;
			}
		}

		fclose(fds.info.diff);
		fds.info.diff = NULL;
	}
}
uint32_t fds_disk_side_size(void) {
	return (info.format == QD_FORMAT ? DISK_QD_SIDE_SIZE : DISK_FDS_SIDE_SIZE);
}
uint32_t fds_image_side_size(void) {
	return (fds_disk_side_size() + FDS_GAP_END);
}
uint32_t fds_image_side_bytes(void) {
	return (fds_image_side_size() * sizeof(WORD));
}

void fds_to_image(void) {
	uint32_t flength = 0;

	if (fds.info.image) {
		free(fds.info.image);
		fds.info.image = NULL;
	}

	for (BYTE side = 0; side < fds.info.total_sides; side++) {
		uint32_t position = 0, size = 0;
		_fds_info_side *is = NULL;
		const BYTE *src = NULL;
		WORD *dst = NULL;

		if (fds.info.type == FDS_FORMAT_FDS) {
			position = (side * fds_disk_side_size()) + 16;
		} else {
			position = side * fds_disk_side_size();
		}

		if ((position + fds_disk_side_size()) > fds.info.total_size) {
			fds.info.total_sides = side;
			break;
		}

		fds.info.image = realloc((void * )fds.info.image, (side + 1) * fds_image_side_bytes());
		src = &fds.info.data[position];
		dst = &fds.info.image[side * fds_image_side_size()];
		memset(dst, 0x00, fds_image_side_bytes());
		is = &fds.info.sides[side];
		position = 0;

		fds_image_memset(&dst[size], FDS_DISK_GAP, FDS_GAP_START);
		size += FDS_GAP_START;

		for (position = 0; position < fds_disk_side_size();) {
			BYTE block = src[position], stop = FALSE;
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
					flength = (src[position + 14] << 8) | src[position + 13];
					blength = 16;
					break;
				case BL_FILE_DATA:
					// il contenuto del file
					blength = flength + 1;
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
				fds_image_memset(&dst[size], FDS_DISK_BLOCK_MARK, 1);
				size += 1;
				// copio i dati
				fds_image_memcpy(&src[position], &dst[size], blength);
				size += blength;
				// dummy CRC
				fds_image_memset(&dst[size], FDS_DISK_CRC_CHAR1, 1);
				size += 1;
				fds_image_memset(&dst[size], FDS_DISK_CRC_CHAR2, 1);
				size += 1;
				// 1016 bit di gap alla fine di ogni blocco.
				// Note : con 976 funziona correttamente la read del disco ma non e'
				// sufficiente per la write.
				fds_image_memset(&dst[size], FDS_DISK_GAP, FDS_GAP_BLOCK);
				size += FDS_GAP_BLOCK;
			}
			position += (blength + (info.format == QD_FORMAT ? 2 : 0));
		}

		is->side = side;
		is->last_position = size;

		if (size < fds_disk_side_size()) {
			fds_image_memset(&dst[size], 0x0000, fds_disk_side_size() - size);
			size += (fds_disk_side_size() - size);
		}

		fds_image_memset(&dst[size], FDS_DISK_GAP, FDS_GAP_END);
		size += FDS_GAP_END;

		is->size = size;
		fds_diff_op(side, FDS_OP_READ, 0, 0);
	}
	// ultimo passaggio
	for (BYTE side = 0; side < fds.info.total_sides; side++) {
		fds.info.sides[side].data = &fds.info.image[side * fds_image_side_size()];
	}
}
void fds_from_image(void) {
	uint32_t flength = 0;
	uTCHAR *file = info.rom.file;
	FILE *fp = NULL;

	if ((cfg->fds_write_mode != FDS_WR_ORIGINAL_FILE) || !fds.info.writings_occurred) {
		return;
	}

	// creo il file
	fp = ufopen(file, uL("a+b"));
	if (fp) {
		// lo chiudo
		fclose(fp);
		// lo riapro in modalita' rb+
		fp = ufopen(file, uL("w+b"));
	} else {
		// ERRORE
		return;
	}

	if (fds.info.type == FDS_FORMAT_FDS) {
		fwrite("FDS", 3, 1, fp);
		fputc(0x1A, fp);
		fputc(fds.info.total_sides, fp);
		for (int i = 0; i < 11; i++) {
			fputc(0, fp);
		}
	}

	for (BYTE side = 0; side < fds.info.total_sides; side++) {
		_fds_info_side *is = &fds.info.sides[side];
		const WORD *src = is->data;
		uint32_t position = 0, size = 0;
		WORD crc = 0;

		for (position = 0; position < fds_disk_side_size();) {
			WORD block = src[position];
			uint32_t blength = 0;
			BYTE stop = FALSE;

			switch (block) {
				case BL_DISK_INFO:
					// le info sul disco
					blength = 56;
					crc = fds_block_crc(&src[position], blength);
					break;
				case BL_FILE_AMOUNT:
					// il numero dei file immagazzinati nel disco
					blength = 2;
					crc = fds_block_crc(&src[position], blength);
					break;
				case BL_FILE_HEADER:
					// l'header del file
					flength = (src[position + 14] << 8) | src[position + 13];
					blength = 16;
					crc = fds_block_crc(&src[position], blength);
					break;
				case BL_FILE_DATA:
					// il contenuto del file
					blength = flength + 1;
					crc = fds_block_crc(&src[position], blength);
					break;
				case FDS_DISK_GAP:
				case FDS_DISK_BLOCK_MARK:
				case FDS_DISK_CRC_CHAR1:
				case FDS_DISK_CRC_CHAR2:
					position++;
					continue;
				default:
					stop = TRUE;
					break;
			}

			if (stop) {
				break;
			}

			if (block) {
				for (unsigned int i = 0; i < blength; i++) {
					fputc(src[position] & 0xFF, fp);
					position++;
					size++;
				}
				if (info.format == QD_FORMAT) {
					fputc(((crc & 0x00FF) >> 0), fp);
					fputc(((crc & 0xFF00) >> 8), fp);
					size += 2;
				}
			}
		}
		while (size < fds_disk_side_size()) {
			fputc(0x00, fp);
			size++;
		}
	}
	fclose(fp);
	{
		uTCHAR diff[LENGTH_FILE_NAME_LONG];

		if (fds.info.diff) {
			fclose(fds.info.diff);
			fds.info.diff = NULL;
		}
		fds_diff_file_name(&diff[0], usizeof(diff));
		uremove(diff);
	}
}
void fds_diff_file_name(uTCHAR *dst, size_t lenght) {
	uTCHAR ext[10], basename[255], *last_dot = NULL;

	umemset(dst, 0x00, lenght);
	gui_utf_basename(info.rom.file, basename, usizeof(basename));
	usnprintf(dst, lenght, uL("" uPs("") DIFF_FOLDER "/" uPs("")), gui_data_folder(), basename);
	usnprintf(ext, usizeof(ext), uL("" uPs("")), (info.format == QD_FORMAT ? uL(".diq") : uL(".dif")));

	// rintraccio l'ultimo '.' nel nome
	last_dot = ustrrchr(dst, uL('.'));
	if (last_dot) {
		// elimino l'estensione
		(*last_dot) = 0x00;
	}
	// aggiungo l'estensione
	ustrcat(dst, ext);
}
void fds_image_sinfo(BYTE side, _fds_sinfo *sinfo) {
	_fds_info_side *is = &fds.info.sides[side];
	uint32_t size = 0, position = 0, flength = 0;
	const WORD *src = is->data;

	memset(sinfo, 0x00, sizeof(_fds_sinfo));

	for (position = 0; position < fds_disk_side_size();) {
		WORD block = src[position];
		uint32_t blength = 1;
		BYTE stop = FALSE;

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
				flength = (src[position + 14] << 8) | src[position + 13];
				blength = 16;
				break;
			case BL_FILE_DATA:
				// il contenuto del file
				blength = flength + 1;
				break;
			case FDS_DISK_GAP:
			case FDS_DISK_BLOCK_MARK:
			case FDS_DISK_CRC_CHAR1:
			case FDS_DISK_CRC_CHAR2:
				size++;
				position++;
				continue;
			default:
				if (is->last_position < position) {
					is->last_position = position;
				}
				stop = TRUE;
				break;
		}

		if (stop) {
			break;
		}

		if (block) {
			const uint32_t file = sinfo->counted_files;

			switch (block) {
				case 1:
					sinfo->block1.position = size;
					// name
					fds_image_memcpy_ASCII(&src[position + 0x10], &sinfo->block1.name[0],
						sizeof(sinfo->block1.name));
					// game version
					sinfo->block1.gversion = src[position + 0x14];
					// side number
					sinfo->block1.snumber = src[position + 0x15];
					// disk number
					sinfo->block1.dnumber = src[position + 0x16];
					break;
				case 2:
					sinfo->block2.position = size;
					sinfo->block2.files = src[position + 1];
					sinfo->counted_files = 0;
					break;
				case 3:
					sinfo->file[file].block3.position = size;
					sinfo->file[file].block3.length = flength;
					// name
					fds_image_memcpy_ASCII(&src[position + 0x03], &sinfo->file[file].block3.name[0],
						sizeof(sinfo->file[file].block3.name));
					// type
					sinfo->file[file].block3.type = src[position + 0x0F];
					break;
				case 4:
					sinfo->file[file].block4.position = size;
					sinfo->counted_files++;
					break;
				default:
					break;
			}
			size += blength;
			position += blength;
		}
	}
}
void fds_image_memset(WORD *dst, WORD value, uint32_t lenght) {
	for (uint32_t i = 0; i < lenght; i++) {
		(*dst++) = value;
	}
}
void fds_image_memcpy(const BYTE *src, WORD *dst, uint32_t lenght) {
	for (uint32_t i = 0; i < lenght; i++) {
		(*dst++) = (*src++);
	}
}
void fds_image_memcpy_ASCII(const WORD *src, BYTE *dst, size_t lenght) {
	memset(dst, 0x00, lenght);
	for (size_t i = 0; i < (lenght - 1); i++) {
		BYTE ch = src[i] & 0xFF;

		dst[i] = ((ch >= 0x20) && (ch <= 0x7E)) ? ch : 0x20;
	}
}
WORD fds_block_crc(const WORD *src, uint32_t lenght) {
	// Do not include any existing checksum, not even the blank checksums 00 00 or FF FF.
	// The formula will automatically count 2 0x00 bytes without the programmer adding them manually.
	// Also, do not include the gap terminator (0x80) in the data.
	// If you wish to do so, change sum to 0x0000.
	WORD sum = 0x8000;

	for (uint32_t byte_index = 0; byte_index < (lenght + 2); byte_index++) {
		BYTE byte = byte_index < lenght ? src[byte_index] & 0x00FF: 0x00;

		for(unsigned bit_index = 0; bit_index < 8; bit_index++) {
			BYTE bit = (byte >> bit_index) & 0x01;
			BYTE carry = sum & 0x01;

			sum = (sum >> 1) | (bit << 15);
			if (carry) {
				sum ^= 0x8408;
			}
		}
	}
	return (sum);
}
