/*
 *  Copyright (C) 2010-2024 Fabio Cavallo (aka FHorse)
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
#include "emu.h"
#include "rewind.h"
#include "save_slot.h"
#include "../../c++/crc/crc.h"

#define BIOSFILE "disksys.rom"

typedef struct _fds_sinfo {
	struct _fds_side_block1 {
		uint32_t position;
		BYTE name[3 + 1];
		WORD gversion;
		WORD snumber;
		WORD dnumber;
		BYTE manufacturing[3];
		BYTE rewritten[3];
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
			uint32_t crc32;
		} block4;
	} file[0xFF];
	uint32_t counted_files;
	uint32_t crc32prg;
} _fds_sinfo;
typedef struct _fds_diff_ele {
	BYTE side;
	BYTE value;
	uint32_t position;
} _fds_diff_ele;
typedef struct _fds_control_autoinsert {
	char name[4];
	uint32_t crc32prg[4];
	BYTE disable_autoinsert;
	BYTE disable_r4032;
	BYTE disable_end_of_head;
} _fds_control_autoinsert;
typedef struct _fds_info_block {
	uint32_t blength;
	WORD faddress;
	WORD flength;
	WORD flength_new;
	WORD flength_cpu;
	WORD flength_ppu;
	BYTE ftype;
	BYTE files;
	BYTE stop;
	int count2000;
	BYTE magic_card_trainer;
	BYTE quick_hunter;
	BYTE ouji;
	BYTE kgk;
} _fds_info_block;

BYTE fds_to_image(uTCHAR *file, _fds_info *finfo);
uTCHAR *fds_bcd_data(BYTE *bcd);
void fds_free_fds_info(void);
BYTE fds_examine_block(const BYTE *src, uint32_t position, _fds_info_block *fb);
void fds_control_autoinsert(_fds_sinfo *sinfo);
void fds_diff_file_name(uTCHAR *file, BYTE format, uTCHAR *dst, size_t len_buffer_dst);
void fds_image_sinfo(BYTE side, _fds_sinfo *sinfo);
void fds_image_memset(BYTE *dst, BYTE value, uint32_t lenght);
void fds_image_memcpy(const BYTE *src, BYTE *dst, uint32_t lenght);
void fds_image_memcpy_ASCII(const BYTE *src, BYTE *dst, size_t lenght);
WORD fds_crc_block(const BYTE *src, uint32_t lenght);
BYTE *fds_from_image_to_mem(BYTE format, BYTE type, size_t *size);
BYTE *fds_read_ips(BYTE *data, const size_t size, const uTCHAR *file);
BYTE fds_create_ips(const BYTE *d1, const size_t size1, const BYTE *d2, const size_t size2, const uTCHAR *file);

_fds fds;

void fds_init(void) {
	memset(&fds, 0x00, sizeof(fds));
	fds.snd.modulation.counter = 0xFFFF;
	fds.snd.wave.counter = 0xFFFF;

	fds.side.change.new_side = 0xFF;

	fds.drive.disk_ejected = TRUE;
	fds.drive.enabled_dsk_reg = 0x01;
	fds.drive.enabled_snd_reg = 0x02;
}
void fds_quit(void) {
	fds_free_fds_info();
	fds_init();
}
BYTE fds_load_rom(BYTE format) {
	_rom_mem rom;
	unsigned int i = 0;

	{
		BYTE found = TRUE;
		uTCHAR rom_ext[2][10] = { uL(".fds\0"), uL(".FDS\0") };
		FILE *fp = NULL;

		if (format == QD_FORMAT) {
			ustrncpy(&rom_ext[0][0], uL(".qd\0"), usizeof(rom_ext[0]));
			ustrncpy(&rom_ext[1][0], uL(".QD\0"), usizeof(rom_ext[0]));
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
		if (!rom.data) {
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
		(rom.data[rom.position++] == 0x1A)) {
		fds.info.type = FDS_TYPE_FDS;
		// il numero di disk sides
		fds.info.total_sides = rom.data[rom.position++];
	} else {
		fds.info.type = FDS_TYPE_RAW;
		// il numero di disk sides
		fds.info.total_sides = fds.info.total_size / fds_disk_side_size(format);
		if (!fds.info.total_sides) {
			fds.info.total_sides = 1;
		}
		// mi riposiziono all'inizio
		rom.position = 0;
	}

	info.format = fds.info.format = format;
	info.number_of_nes = 1;
	fds.info.expcted_sides = fds.info.total_sides;
	fds.info.write_protected = 0x00;
	fds.info.cycles_8bit_delay = emu_ms_to_cpu_cycles(FDS_8BIT_MS_DELAY);
	fds.info.cycles_insert_delay = emu_ms_to_cpu_cycles(FDS_INSERT_MS_DELAY);
	fds.info.cycles_dummy_delay = emu_ms_to_cpu_cycles(FDS_OP_SIDE_MS_DELAY);

	// converto nel mio formato immagine
	if (fds_to_image(&info.rom.file[0], &fds.info)) {
		return (EXIT_ERROR);
	}

	// inserisco il primo
	fds.info.frame_insert = nes[0].p.ppu.frames;
	fds.bios.first_run = !cfg->fds_disk1sideA_at_reset;
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
	uTCHAR *lastSlash = NULL;
	FILE *bios = NULL;

	// ordine di ricerca:

	// 1) file specificato dall'utente
	umemset(fds.bios.file, 0x00, usizeof(fds.bios.file));
	usnprintf(fds.bios.file, usizeof(fds.bios.file), uL("" uPs("")), cfg->fds_bios_file);
	bios = ufopen(fds.bios.file, uL("rb"));
	if (bios) {
		goto fds_load_bios_founded;
	}

	// 2) directory di lavoro
	umemset(fds.bios.file, 0x00, usizeof(fds.bios.file));
	ustrncpy(fds.bios.file, uL("" BIOSFILE), usizeof(fds.bios.file));
	bios = ufopen(fds.bios.file, uL("rb"));
	if (bios) {
		goto fds_load_bios_founded;
	}

	// 3) directory contenente il file fds
	umemset(fds.bios.file, 0x00, usizeof(fds.bios.file));
	ustrncpy(fds.bios.file, info.rom.file, usizeof(fds.bios.file));
	// rintraccio l'ultimo '.' nel nome
#if defined (_WIN32)
	lastSlash = ustrrchr(fds.bios.file, uL('\\'));
	if (lastSlash) {
		(*(lastSlash + 1)) = 0x00;
	}
#else
	lastSlash = ustrrchr(fds.bios.file, uL('/'));
	if (lastSlash) {
		(*(lastSlash + 1)) = 0x00;
	}
#endif
	// aggiungo il nome del file
	ustrcat(fds.bios.file, uL("" BIOSFILE));
	bios = ufopen(fds.bios.file, uL("rb"));
	if (bios) {
		goto fds_load_bios_founded;
	}

	// 4) directory puNES/bios
	umemset(fds.bios.file, 0x00, usizeof(fds.bios.file));
	usnprintf(fds.bios.file, usizeof(fds.bios.file), uL("" uPs("") BIOS_FOLDER "/" BIOSFILE), gui_data_folder());
	bios = ufopen(fds.bios.file, uL("rb"));
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

	fds.bios.crc32 = emu_crc32((void *)prgrom_pnt(), prgrom_size());

	return (EXIT_OK);
}
BYTE fds_create_empty_disk(uTCHAR *file, BYTE format, BYTE type, BYTE double_side) {
	BYTE total_sides = double_side ? 2 : 1, *mfds = NULL;
	size_t size = (type == FDS_TYPE_FDS ? 16 : 0) + (fds_disk_side_size(format) * total_sides);
	FILE *fp = NULL;
	BYTE rc = EXIT_OK;

	fp = ufopen(file, uL("w+b"));
	if (!fp) {
		return (EXIT_ERROR);
	}

	mfds = (BYTE *)malloc(size);
	if (!mfds) {
		fclose(fp);
		return (EXIT_ERROR);
	}
	memset(mfds, 0x00, size);

	if (type == FDS_TYPE_FDS) {
		memcpy(&mfds[0], "FDS", 3);
		mfds[3] = 0x1A;
		mfds[4] = 0x01;
	}

	if (fwrite(mfds, size, 1, fp) < 1) {
		rc = EXIT_ERROR;
	}
	fclose(fp);
	free(mfds);
	{
		uTCHAR diff[LENGTH_FILE_NAME_LONG];

		fds_diff_file_name(file, format, &diff[0], usizeof(diff));
		uremove(diff);
	}
	return (rc);
}
BYTE fds_change_disk(uTCHAR *file) {
	uTCHAR path[LENGTH_FILE_NAME_LONG], *ext = NULL;
	_uncompress_archive *archive = NULL;
	BYTE found = FALSE, rc = 0;
	size_t position = 0;
	_fds_info finfo;
	FILE *fp = NULL;

	fds_free_fds_info();

	memset(&finfo, 0x00, sizeof(_fds_info));
	umemset(&path[0], 0x00, usizeof(path));
	ustrncpy(path, file, usizeof(path) - 1);

	archive = uncompress_archive_alloc(path, &rc);

	if (rc == UNCOMPRESS_EXIT_OK) {
		if (archive->floppy_disk.count > 0) {
			switch ((rc = uncompress_archive_extract_file(archive, UNCOMPRESS_TYPE_FLOPPY_DISK))) {
				case UNCOMPRESS_EXIT_OK:
					umemset(&path[0], 0x00, usizeof(path));
					ustrncpy(path, uncompress_archive_extracted_file_name(archive, UNCOMPRESS_TYPE_FLOPPY_DISK), usizeof(path) - 1);
					found = TRUE;
					break;
				default:
				case UNCOMPRESS_EXIT_ERROR_ON_UNCOMP:
					break;
			}
		}
		uncompress_archive_free(archive);
	} else if (rc == UNCOMPRESS_EXIT_IS_NOT_COMP) {
		found = TRUE;
	}

	if (!found) {
		return (EXIT_ERROR);
	}

	ext = emu_ctrl_rom_ext(path);

	if (!ustrcasecmp(ext, uL(".fds"))) {
		finfo.format = FDS_FORMAT;
	} else if (!ustrcasecmp(ext, uL(".qd"))) {
		finfo.format = QD_FORMAT;
	} else {
		return (EXIT_ERROR);
	}

	fp = ufopen(path, uL("rb"));
	if (!fp) {
		return (EXIT_ERROR);
	}

	fseek(fp, 0L, SEEK_END);
	finfo.total_size = ftell(fp);
	fseek(fp, 0L, SEEK_SET);

	finfo.data = (BYTE *)malloc(finfo.total_size);
	if (!finfo.data) {
		fclose(fp);
		return (EXIT_ERROR);
	}
	if (fread(finfo.data, 1, finfo.total_size, fp) != finfo.total_size) {
		fclose(fp);
		free(finfo.data);
		return (EXIT_ERROR);
	}
	fclose(fp);

	if ((finfo.data[position++] == 'F') &&
		(finfo.data[position++] == 'D') &&
		(finfo.data[position++] == 'S') &&
		(finfo.data[position++] == 0x1A)) {
		finfo.type = FDS_TYPE_FDS;
		// il numero di disk sides
		finfo.total_sides = finfo.data[position++];
	} else {
		finfo.type = FDS_TYPE_RAW;
		// il numero di disk sides
		finfo.total_sides = finfo.total_size / fds_disk_side_size(finfo.format);
		if (!finfo.total_sides) {
			finfo.total_sides = 1;
		}
		// mi riposiziono all'inizio
		position = 0;
	}

	finfo.expcted_sides = finfo.total_sides;
	finfo.write_protected = 0x00;
	finfo.enabled = fds.info.enabled;
	finfo.writings_occurred = FALSE;
	finfo.cycles_8bit_delay = fds.info.cycles_8bit_delay;
	finfo.cycles_insert_delay = fds.info.cycles_insert_delay;
	finfo.cycles_dummy_delay = fds.info.cycles_dummy_delay;

	// converto nel mio formato immagine
	if (fds_to_image(file, &finfo)) {
		free(finfo.data);
		if (finfo.image) {
			free(finfo.image);
		}
		return (EXIT_ERROR);
	}

	finfo.frame_insert = nes[0].p.ppu.frames;

	info.format = finfo.format;
	ustrncpy(info.rom.file, path, usizeof(info.rom.file));

	memcpy(&fds.info, &finfo, sizeof(_fds_info));
	fds_info();

	if (rewind_init()) {
		return (EXIT_ERROR);
	}

	save_slot_count_load();

	fds.side.change.new_side = 0;
	fds.side.change.delay = fds.info.cycles_dummy_delay;
	fds_disk_op(FDS_DISK_EJECT, 0, FALSE);

	return (EXIT_OK);
}
void fds_info(void) {
	uTCHAR buffer[LENGTH_FILE_NAME_LONG];

	log_info_open(uL("bios;"));
	umemset(buffer, 0x00, usizeof(buffer));
	gui_utf_basename((uTCHAR *)fds.bios.file, buffer, usizeof(buffer) - 1);
	log_close(uL("" uPs("") ", crc : 0x%08X"), buffer, fds.bios.crc32);

	log_info_box_open(uL("folder;"));
	umemset(buffer, 0x00, usizeof(buffer));
	gui_utf_dirname((uTCHAR *)fds.bios.file, buffer, usizeof(buffer) - 1);
	log_close_box(uL("" uPs("")), buffer);

	log_info_open(uL("sides;"));
	if (fds.info.expcted_sides != fds.info.total_sides) {
		log_close(uL("expected %d, finded %d"), fds.info.expcted_sides, fds.info.total_sides);
	} else {
		log_close(uL("%d"), fds.info.total_sides);
	}
	for (int side = 0; side < fds.info.total_sides; side++) {
		_fds_sinfo sinfo = { 0 };

		fds_image_sinfo(side, &sinfo);
		log_info(uL("FDS side %d;disk %d, side %X, name %3s, version %d, files %d, counted %d"), side,
			sinfo.block1.dnumber,
			sinfo.block1.snumber + 0x0A,
			sinfo.block1.name,
			sinfo.block1.gversion,
			sinfo.block2.files,
			sinfo.counted_files);
		log_info_box(uL(";@ " uPs("") ", written " uPs("") ", crc 0x%08X"),
			fds_bcd_data(&sinfo.block1.manufacturing[0]),
			fds_bcd_data(&sinfo.block1.rewritten[0]),
			sinfo.crc32prg);
		// controllo se disabilitare l'autoinsert
		fds_control_autoinsert(&sinfo);
#if !defined (RELEASE)
		fds_info_side(side);
#endif
	}
}
void fds_info_side(BYTE side) {
	_fds_sinfo sinfo = { 0 };

	fds_image_sinfo(side, &sinfo);
	log_info(uL("FDS side %d;disk %d, side %X, name %3s, version %d, vsize %d"), side,
		sinfo.block1.dnumber,
		sinfo.block1.snumber + 0x0A,
		sinfo.block1.name,
		sinfo.block1.gversion,
		fds.info.sides[side].size);
	log_info_box(uL("info;@ " uPs("") ", written " uPs("") ", crc 0x%08X"),
		fds_bcd_data(&sinfo.block1.manufacturing[0]),
		fds_bcd_data(&sinfo.block1.rewritten[0]),
		sinfo.crc32prg);
	log_info_box(uL("block 1;pos %5d"), sinfo.block1.position);
	log_info_box(uL("block 2;pos %5d, files %d, counted %d"),
		sinfo.block2.position,
		sinfo.block2.files,
		sinfo.counted_files);
	for (uint32_t i = 0; i < sinfo.counted_files; i++) {
		log_info_box(uL("file %d;name %8s, size %5d, crc 0x%08X, 0x%04X (b3 : %5d) (b4 : %5d)"), i,
			sinfo.file[i].block3.name,
			sinfo.file[i].block3.length,
			sinfo.file[i].block4.crc32,
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
			fds.drive.scan = FALSE;
			fds.drive.scan_disabled = FALSE;
			fds.drive.end_of_head = 0x40;
			fds.drive.transfer_reset = 0x01;
			fds.drive.motor_stop = 0x02;
			fds.drive.motor_started = FALSE;
			fds.drive.delay_8bit = 0;
			fds.auto_insert.r4032.frames = 0;
			fds.auto_insert.r4032.checks = 0;
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
			fds.bios.first_run = FALSE;
			fds.drive.disk_position = 0;
			fds.drive.mark_finded = FALSE;
			fds.drive.disk_ejected = FALSE;
			fds.drive.scan = FALSE;
			fds.drive.scan_disabled = FALSE;
			fds.drive.end_of_head = 0x40;
			fds.drive.transfer_reset = 0x01;
			fds.drive.motor_stop = 0x02;
			fds.drive.motor_started = FALSE;
			fds.drive.delay_8bit = 0;
			fds.drive.delay_insert = fds.info.cycles_insert_delay;
			fds.auto_insert.r4032.frames = 0;
			fds.auto_insert.r4032.checks = 0;
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
			fds.auto_insert.r4032.frames = 0;
			fds.auto_insert.r4032.checks = 0;
			break;
		default:
			break;
	}

	if (((info.reset != CHANGE_ROM) || fds.info.total_sides == 1) && (type >= FDS_DISK_SELECT)) {
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
BYTE fds_from_image_to_file(uTCHAR *file, BYTE format, BYTE type) {
	size_t size = 0;
	BYTE *mfds = fds_from_image_to_mem(format, type, &size);
	FILE *fp = NULL;

	if (!mfds) {
		return (EXIT_ERROR);
	}
	// creo il file
	fp = ufopen(file, uL("a+b"));
	if (fp) {
		// lo chiudo
		fclose(fp);
		// lo riapro in modalita' w+b
		fp = ufopen(file, uL("w+b"));
	} else {
		free(mfds);
		return (EXIT_ERROR);
	}
	if (fwrite(mfds, size, 1, fp) < 1) {
		fclose(fp);
		free(mfds);
		return (EXIT_ERROR);
	};
	fclose(fp);
	free(mfds);
	{
		uTCHAR diff[LENGTH_FILE_NAME_LONG];

		fds_diff_file_name(&info.rom.file[0], fds.info.format, &diff[0], usizeof(diff));
		uremove(diff);
	}
	return (EXIT_OK);
}
BYTE fds_image_to_file(uTCHAR *file) {
	FILE *fp = NULL;

	// creo il file
	fp = ufopen(file, uL("a+b"));
	if (fp) {
		// lo chiudo
		fclose(fp);
		// lo riapro in modalita' rb+
		fp = ufopen(file, uL("w+b"));
	} else {
		return (EXIT_ERROR);
	}
	fwrite(fds.info.image, fds.info.total_sides * fds_image_side_size(), 1, fp);
	fclose(fp);
	return (EXIT_OK);
}
WORD fds_crc_byte(WORD base, BYTE data) {
	for (unsigned i = 0; i < 8; i++) {
		BYTE bit = (data >> i) & 0x01;
		BYTE carry = base & 0x01;

		base = (base >> 1) | (bit << 15);
		if (carry) {
			base ^= 0x8408;
		}
	}
	return (base);
}
uint32_t fds_disk_side_size(BYTE format) {
	return (format == QD_FORMAT ? DISK_QD_SIDE_SIZE : DISK_FDS_SIDE_SIZE);
}
uint32_t fds_image_side_size(void) {
	return (FDS_IMAGE_SIDE_SIZE);
}

BYTE fds_to_image(uTCHAR *file, _fds_info *finfo) {
	BYTE *mfds = NULL, *pointer = finfo->data;

	if (finfo->image) {
		free(finfo->image);
		finfo->image = NULL;
	}

	// applico la diff
	{
		uTCHAR diff[LENGTH_FILE_NAME_LONG];

		fds_diff_file_name(file, finfo->format, &diff[0], usizeof(diff));
		if (emu_file_exist(diff) == EXIT_OK) {
			mfds = malloc(finfo->total_size);
			if (mfds) {
				memcpy(mfds, finfo->data, finfo->total_size);
				mfds = fds_read_ips(mfds, finfo->total_size, diff);
			}
			if (!mfds) {
				log_error(uL("FDS;error on reading diff file"));
			} else {
				pointer = mfds;
				finfo->writings_occurred = TRUE;
			}
		}
	}

	for (BYTE side = 0; side < finfo->total_sides; side++) {
		uint32_t position = 0, size = 0;
		_fds_info_block fib = { 0 };
		_fds_info_side *is = NULL;
		const BYTE *src = NULL;
		BYTE *dst = NULL;
		uint32_t header = 0;

		if (finfo->type == FDS_TYPE_FDS) {
			header = 16;
		}

		finfo->protection.autodetect = TRUE;
		finfo->protection.magic_card_trainer = FALSE;
		finfo->protection.quick_hunter = FALSE;
		finfo->protection.ouji = FALSE;
		finfo->protection.kgk = FALSE;
		position = side * fds_disk_side_size(finfo->format) + header;

		// "Bishoujo Mahjong Club (Japan) (Unl).fds"
		// "Bodycon Quest I - Girls Exposed (Japan) (Unl) [T-En by DvD Translations Rev A] [n].fds"
		// "Dead Zone (Japan) [T-En by Stardust Crusaders v1.00].fds"
		// "Game no Tatsujin - Money Wars (Japan) (Unl).fds"
		// "Golf, The - Bishoujo Classic (Japan) (Unl).fds"
		// "Otocky (Japan) (Beta) (1986-04-15).fds"
		// la dimensione dell'immeagine finale e' superiore a 65500
		finfo->image = realloc((void *)finfo->image, (side + 1) * fds_image_side_size());
		src = &pointer[position];
		dst = &finfo->image[side * fds_image_side_size()];
		memset(dst, FDS_DISK_GAP, fds_image_side_size());
		is = &finfo->sides[side];
		position = 0;

		fds_image_memset(&dst[size], FDS_DISK_GAP, FDS_GAP_START);
		size += FDS_GAP_START;

		for (position = 0; position < fds_disk_side_size(finfo->format);) {
			fds_examine_block(src, position, &fib);
			finfo->protection.magic_card_trainer = fib.magic_card_trainer;
			finfo->protection.quick_hunter = fib.quick_hunter;
			finfo->protection.ouji = fib.ouji;
			finfo->protection.kgk = fib.kgk;

			if (finfo->protection.quick_hunter | finfo->protection.kgk | finfo->protection.ouji) {
				finfo->write_protected = 0x04;
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
			if (fib.stop) {
				// gestione trainer e protezioni (blocco aggiuntivo)
				if (finfo->protection.magic_card_trainer || finfo->protection.kgk || finfo->protection.quick_hunter) {
					BYTE btype = finfo->protection.magic_card_trainer
						? 0x05
						: finfo->protection.kgk ? 0x12 : finfo->protection.quick_hunter ? 0x00 : 0x00;
					uint32_t cpos = 0, cblength = 0;
					WORD crc = 0;

					fib.blength = finfo->protection.magic_card_trainer
						? 0x200
						: finfo->protection.kgk ? 0x2500 : finfo->protection.quick_hunter ? 0x3000 : 0;
					// indico l'inizio del blocco
					fds_image_memset(&dst[size], FDS_DISK_BLOCK_MARK, 1);
					size += 1;
					// inizilizzo le variabili per il calcolo del crc
					cpos = size;
					cblength = fib.blength;
					// nell'fds originale manca il tipo di blocco (0x00)
					if (finfo->protection.quick_hunter) {
						fds_image_memset(&dst[size], btype, 1);
						size += 1;
						cblength += 1;
					}
					// copio il blocco
					fds_image_memcpy(&src[position], &dst[size], fib.blength);
					size += fib.blength;
					// crc
					crc = fds_crc_block(&dst[cpos], cblength);
					fds_image_memset(&dst[size], (crc >> 0), 1);
					size += 1;
					fds_image_memset(&dst[size], (crc >> 8), 1);
					size += 1;
				}
				break;
			}

			if (src[position]) {
				uint32_t cpos = 0, cblength = 0;
				WORD crc = 0;

				// indico l'inizio del blocco
				fds_image_memset(&dst[size], FDS_DISK_BLOCK_MARK, 1);
				size += 1;
				// inizilizzo le variabili per il calcolo del crc
				cpos = size;
				cblength = fib.blength;
				// copio il blocco
				fds_image_memcpy(&src[position], &dst[size], fib.blength);
				size += fib.blength;
				// crc
				if (finfo->format == FDS_FORMAT) {
					crc = fds_crc_block(&dst[cpos], cblength);
				} else {
					fds_image_memcpy(&src[position + fib.blength], (BYTE *)&crc, 2);
				}
				fds_image_memset(&dst[size], (crc >> 0), 1);
				size += 1;
				fds_image_memset(&dst[size], (crc >> 8), 1);
				size += 1;
				// inizializzo il blocco di gap
				fds_image_memset(&dst[size], FDS_DISK_GAP, FDS_GAP_BLOCK);
				size += FDS_GAP_BLOCK;

				// in caso di write di un file questo altro blocco di gap serve per ottenere il
				// il giusto posizionamento (nell'immagine) al blocco dei dati del file (0x04):
				// - scrittura blocco header (0x03)
				// - scrittura crc
				// - il bios scrive altri 4 byte (0x00) dopo il crc
				// - scrive 121 byte di gap
				// - scrive il blocco dei dati (0x04)
				// - scrittura crc
				// - il bios scrive altri 4 byte (0x00) dopo il crc
				// - scrive 121 byte di gap
				if (src[position] == BL_FILE_HEADER) {
					fds_image_memset(&dst[size], FDS_DISK_GAP, FDS_GAP_FILE_BLOCK);
					size += FDS_GAP_FILE_BLOCK;
				}
			}
			position += (fib.blength + (finfo->format == QD_FORMAT ? 2 : 0));
		}

		is->side = side;

		if (size < fds_image_side_size()) {
			fds_image_memset(&dst[size], FDS_DISK_GAP, fds_image_side_size() - size);
			size += (fds_image_side_size() - size);
		}

		is->size = size;
	}
	// ultimo passaggio
	for (BYTE side = 0; side < finfo->total_sides; side++) {
		finfo->sides[side].data = &finfo->image[side * fds_image_side_size()];
	}
	if (mfds) {
		free(mfds);
	}
	return (EXIT_OK);
}
uTCHAR *fds_bcd_data(BYTE *bcd) {
	static uTCHAR date[13];

	if ((!bcd[0] && !bcd[1] && !bcd[2]) || ((bcd[0] == 0xFF) && (bcd[1] == 0xFF) && (bcd[2] == 0xFF))) {
		usnprintf(&date[0], usizeof(date), uL("__-__-____"));
	} else {
		int year = ((bcd[0] >> 4) * 10) + (bcd[0] & 0x0F);
		int month = ((bcd[1] >> 4) * 10) + (bcd[1] & 0x0F);
		int day = ((bcd[2] >> 4) * 10) + (bcd[2] & 0x0F);

		if (year < 55) {
			// Heisei period
			year += 1988;
		} else if (year >= 80) {
			// Western calendar
			year += 1900;
		} else {
			// Showa period
			year += 1925;
		}
		usnprintf(&date[0], usizeof(date), uL("%02d-%02d-%04d"), day, month, year);
	}
	return (&date[0]);
}
void fds_free_fds_info(void) {
	if (fds.info.image) {
		// se richiesto, sovrascrivo il file originale
		if (fds.info.writings_occurred) {
			uTCHAR file[LENGTH_FILE_NAME_LONG];

			fds_diff_file_name(&info.rom.file[0], fds.info.format, &file[0], usizeof(file));
			if (cfg->fds_write_mode == FDS_WR_ORIGINAL_FILE) {
				fds_from_image_to_file(info.rom.file, fds.info.format, fds.info.type);
			} else {
				size_t size = 0;
				BYTE *mfds = fds_from_image_to_mem(fds.info.format, fds.info.type, &size);

				if (!mfds || fds_create_ips(fds.info.data, fds.info.total_size, mfds, size, file)) {
					log_error(uL("FDS;error on writing diff file"));
				}
				if (mfds) {
					free(mfds);
				}
			}
		}
		free(fds.info.image);
	}
	if (fds.info.data) {
		free(fds.info.data);
	}
}
BYTE fds_examine_block(const BYTE *src, uint32_t position, _fds_info_block *fib) {
	switch (src[position]) {
		case BL_DISK_INFO:
			// informazioni sul disco
			// "Jingorou (Japan) (Unl).fds" e "Graphic Editor Hokusai - Ver 1.2 (Japan) (Unl).fds"
			fib->ouji = fds.info.protection.autodetect && !strncmp((void *)&src[position + 0x10], "OUJI", 4);
			fib->kgk = fds.info.protection.autodetect && !strncmp((void *)&src[position + 0x10], "KGK ", 4);
			fib->magic_card_trainer = FALSE;
			fib->count2000 = 0;
			fib->blength = 56;
			return (TRUE);
		case BL_FILE_AMOUNT:
			// il numero dei file nel disco
			fib->files = src[position + 1];
			fib->blength = 2;
			return (TRUE);
		case BL_FILE_HEADER:
			// header del file
			fib->faddress = (src[position + 0x0C] << 8) | src[position + 0x0B];
			fib->flength = (src[position + 0x0E] << 8) | src[position + 0x0D];
			fib->ftype = src[position + 0x0F] & 0x03;
			if (fib->flength_new && fib->files) {
				fib->flength = fib->ftype != 0 ? (fib->flength_ppu ? fib->flength_ppu : fib->flength) : fib->flength_cpu;
			} else if (fib->ouji && (fib->ftype == 0) && (fib->faddress == 0x2000) && (fib->flength == 1) &&
				(++fib->count2000 >= 3)) {
				fib->flength = fib->files ? 0xC000 : 0xE000;
			}
			if (fib->files) {
				fib->files--;
			}
			// "Quick Hunter (Japan) (Unl).fds"
			if (!fib->files && (fib->faddress == 0x2000) && (fib->flength == 0x900) && (fib->ftype == 0) &&
				!strncmp((void *)&src[position + 0x03], "KIYONO.", 7)) {
				fib->quick_hunter = TRUE;
			}
			fib->blength = 16;
			return (TRUE);
		case BL_FILE_DATA:
			// il contenuto del file
			if (fds.info.protection.autodetect && (fib->ftype == 0) && (fib->faddress == 0x4FFF) && (fib->flength == 8)) {
				fib->flength_new = src[position + 0x01] & 0x80;
				fib->magic_card_trainer = (src[position + 0x01] != 0xFE) && ((src[position + 0x01] & 0xC0) == 0xC0) &&
					(src[position + 0x08] == 0x00);
				fib->flength_cpu = 0x8000;
				switch (src[position + 0x02] >> 5) {
					case 4:
					case 5:
						fib->flength_ppu = 32768;
						break;
					case 6:
						fib->flength_ppu = 16384;
						break;
					case 7:
						fib->flength_ppu = 8192;
						break;
					default:
						fib->flength_ppu = 0;
						break;
				}
			}
			fib->blength = fib->flength + 1;
			return (TRUE);
		default:
			fib->blength = 0;
			fib->stop = TRUE;
			return (FALSE);
	}
}
void fds_control_autoinsert(_fds_sinfo *sinfo) {
	_fds_control_autoinsert images[] = {
		// --- auto insert disabilitato
		// Gall Force - Eternal Story (Japan)
		{ "GAL", { 0x1E9969AC, 0xED380FA3, 0x00, 0x00 }, TRUE, TRUE, TRUE },
		// Gall Force - Eternal Story (Japan) [T-En by Gil Galad v1.0]
		{ "GAL", { 0xD049BAFA, 0x3B7198F3, 0x00, 0x00 }, TRUE, TRUE, TRUE },
		// Koneko Monogatari - The Adventures of Chatran (Japan)
		{ "KOM", { 0xE9A457BF, 0x7791A2DD, 0x00, 0x00 }, TRUE, TRUE, TRUE },
		// Koneko Monogatari - The Adventures of Chatran (Japan) (Sample) (1986-07-03)
		{ "KOM", { 0x8C457E04, 0xCB25E05C, 0x00, 0x00 }, TRUE, TRUE, TRUE },
		// Puzzle Boys (Japan) (Disk Writer)
		{ "PUZ", { 0x2385D83D, 0x64488AFD, 0x00, 0x00 }, TRUE, TRUE, TRUE },
		// Puzzle Boys (Japan) (Disk Writer) [T-En by DvD Translations Rev A]
		{ "PUZ", { 0x876DD60C, 0xD487A70E, 0x00, 0x00 }, TRUE, TRUE, TRUE },
		// Samurai Sword (Japan)
		{ "SMU", { 0xD0B342F5, 0xE46A6E3E, 0x00, 0x00 }, TRUE, TRUE, TRUE },
		// Samurai Sword (Japan) [T-En by Mute v1.0]
		{ "SMU", { 0x9080511A, 0x4B8D69C7, 0x00, 0x00 }, TRUE, TRUE, TRUE },
		// Quick Hunter (Japan) (Unl)
		{ "OUJ", { 0x2AD12F7F, 0xBFA73716, 0x00, 0x00 }, TRUE, TRUE, TRUE },

		// --- auto insert r4032 disabilitato e auto insert end_of_head abilitato
		// Egger Land (Japan)
		{ "EGL", { 0x8EE0B051, 0x71119427, 0x00, 0x00 }, FALSE, TRUE, FALSE },
		// Egger Land (Japan) [T-En by Necrosaro v1.0]
		{ "EGL", { 0xCE21494E, 0x71119427, 0x00, 0x00 }, FALSE, TRUE, FALSE },
		// Eggerland (1987)(Hal Laboratory)(J)[tr En]
		{ "EGL", { 0xCE21494E, 0x5FD1F4EB, 0x00, 0x00 }, FALSE, TRUE, FALSE },
		// Egger Land (Japan) (Rev 1) (Possible Proto)
		{ "EGL", { 0xBB887262, 0xC9915867, 0x00, 0x00 }, FALSE, TRUE, FALSE },
		// Igo - Kyuu Roban Taikyoku (Japan)
		{ "IGO", { 0x1E8B0151, 0xF7130E20, 0x00, 0x00 }, FALSE, TRUE, FALSE },
		// Time Twist - Rekishi no Katasumi de (1991)(Nintendo)(J)
		{ "TT1", { 0x6D6014C1, 0x145A90B2, 0xBFB019B9, 0x636083C1 }, FALSE, TRUE, FALSE },
		// Vs. Excitebike
		{ "EBD", { 0x00, 0x00, 0x00, 0x00 }, FALSE, TRUE, FALSE },
	};

	fds.auto_insert.r4032.disabled = FALSE;
	fds.auto_insert.end_of_head.disabled = TRUE;

	// Souseiki Fammy - ROM-QD for 256K+64K (Japan) (Unl)
	if (!strncmp((char *)&sinfo->block1.name[0], "ILE", 3) &&
		((sinfo->crc32prg == 0x01218B9D) || (sinfo->crc32prg == 0xF1BCA55D))) {
		fds.auto_insert.disabled = TRUE;
		gui_unsupported_hardware();
		return;
	}
	if (fds.info.total_sides == 1) {
		// auto insert disabilitato
		fds.auto_insert.disabled = TRUE;
		gui_overlay_info_append_msg_precompiled(40, NULL);
		return;
	}
	if (fds.info.protection.quick_hunter) {
		// auto insert disabilitato
		fds.auto_insert.disabled = TRUE;
		gui_overlay_info_append_msg_precompiled(39, NULL);
		return;
	}
	if (ustrstr(info.rom.file, uL("Zenpen")) || ustrstr(info.rom.file, uL("Kouhen")) ||
		ustrstr(info.rom.file, uL("Disk 1")) || ustrstr(info.rom.file, uL("Disk 2"))) {
		// auto insert disabilitato
		fds.auto_insert.disabled = TRUE;
		gui_overlay_info_append_msg_precompiled(39, NULL);
		return;
	}
	for (unsigned int i = 0; i < LENGTH(images); i++) {
		_fds_control_autoinsert *fca = &images[i];

		if (!strncmp((char *)&sinfo->block1.name[0], &fca->name[0], 3) &&
			((!fca->crc32prg[0] && !fca->crc32prg[1] && !fca->crc32prg[2] && !fca->crc32prg[3]) ||
			(fca->crc32prg[0] && (fca->crc32prg[0] == sinfo->crc32prg)) ||
			(fca->crc32prg[1] && (fca->crc32prg[1] == sinfo->crc32prg)) ||
			(fca->crc32prg[2] && (fca->crc32prg[2] == sinfo->crc32prg)) ||
			(fca->crc32prg[3] && (fca->crc32prg[3] == sinfo->crc32prg)))) {
			if (fca->disable_autoinsert) {
				fds.auto_insert.disabled = TRUE;
				gui_overlay_info_append_msg_precompiled(39, NULL);
				continue;
			}
			fds.auto_insert.r4032.disabled = fca->disable_r4032;
			fds.auto_insert.end_of_head.disabled = fca->disable_end_of_head;
			break;
		}
	}
}
void fds_diff_file_name(uTCHAR *file, BYTE format, uTCHAR *dst, size_t len_buffer_dst) {
	uTCHAR ext[10], basename[255], *last_dot = NULL;

	umemset(dst, 0x00, len_buffer_dst);
	gui_utf_basename(file, basename, usizeof(basename));
	usnprintf(dst, len_buffer_dst, uL("" uPs("") DIFF_FOLDER "/" uPs("")), gui_data_folder(), basename);
	usnprintf(ext, usizeof(ext), uL("" uPs("")), (format == QD_FORMAT ? uL(".ipq") : uL(".ipf")));

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
	const BYTE *src = is->data;

	memset(sinfo, 0x00, sizeof(_fds_sinfo));

	for (position = 0; position < fds_disk_side_size(fds.info.format);) {
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
				flength = (src[position + 0x0E] << 8) | src[position + 0x0D];
				blength = 16;
				break;
			case BL_FILE_DATA:
				// il contenuto del file
				blength = flength + 1;
				break;
			case FDS_DISK_GAP:
			case FDS_DISK_BLOCK_MARK:
				size++;
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
			const uint32_t file = sinfo->counted_files;

			switch (block) {
				case 1:
					sinfo->block1.position = size;
					// name
					fds_image_memcpy_ASCII(&src[position + 0x10], &sinfo->block1.name[0], sizeof(sinfo->block1.name));
					// game version
					sinfo->block1.gversion = src[position + 0x14];
					// side number
					sinfo->block1.snumber = src[position + 0x15];
					// disk number
					sinfo->block1.dnumber = src[position + 0x16];
					// manufacturing date
					sinfo->block1.manufacturing[0] = src[position + 0x1F];
					sinfo->block1.manufacturing[1] = src[position + 0x20];
					sinfo->block1.manufacturing[2] = src[position + 0x21];
					// rewritten disk date
					sinfo->block1.rewritten[0] = src[position + 0x2C];
					sinfo->block1.rewritten[1] = src[position + 0x2D];
					sinfo->block1.rewritten[2] = src[position + 0x2E];
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
					sinfo->crc32prg = emu_crc32_continue((void *)&src[position], blength, sinfo->crc32prg);
					break;
				case 4:
					sinfo->file[file].block4.position = size;
					sinfo->file[file].block4.crc32 = emu_crc32((void *)&src[position + 1], flength);
					sinfo->crc32prg = emu_crc32_continue((void *)&src[position], blength, sinfo->crc32prg);
					sinfo->counted_files++;
					break;
				default:
					break;
			}
			size += blength;
			position += blength;
			// crc
			size += 2;
			position += 2;
		}
	}
}
void fds_image_memset(BYTE *dst, BYTE value, uint32_t lenght) {
	for (uint32_t i = 0; i < lenght; i++) {
		(*dst++) = value;
	}
}
void fds_image_memcpy(const BYTE *src, BYTE *dst, uint32_t lenght) {
	for (uint32_t i = 0; i < lenght; i++) {
		(*dst++) = (*src++);
	}
}
void fds_image_memcpy_ASCII(const BYTE *src, BYTE *dst, size_t lenght) {
	memset(dst, 0x00, lenght);
	for (size_t i = 0; i < (lenght - 1); i++) {
		BYTE ch = src[i] & 0xFF;

		dst[i] = ((ch >= 0x20) && (ch <= 0x7E)) ? ch : 0x20;
	}
}
WORD fds_crc_block(const BYTE *src, uint32_t lenght) {
	// Do not include any existing checksum, not even the blank checksums 00 00 or FF FF.
	// The formula will automatically count 2 0x00 bytes without the programmer adding them manually.
	// Also, do not include the gap terminator (0x80) in the data.
	// If you wish to do so, change sum to 0x0000.
	WORD crc = 0x8000;

	for (uint32_t i = 0; i < (lenght + 2); i++) {
		crc = fds_crc_byte(crc, (i < lenght ? src[i]: 0x00));
	}
	return (crc);
}
BYTE *fds_from_image_to_mem(BYTE format, BYTE type, size_t *size) {
	BYTE *mfds = NULL;

	(*size) = (type == FDS_TYPE_FDS ? 16 : 0) + fds_disk_side_size(format) * fds.info.total_sides;

	// alloco la zona di memoria
	mfds = malloc((*size));
	if (!mfds) {
		(*size) = 0;
		return (mfds);
	}
	memset(mfds, 0x00, (*size));

	if (type == FDS_TYPE_FDS) {
		memcpy(&mfds[0], "FDS", 3);
		mfds[3] = 0x1A;
		mfds[4] = fds.info.total_sides;
	}

	for (BYTE side = 0; side < fds.info.total_sides; side++) {
		uint32_t length = 0, position = 0, total_size = 0;
		_fds_info_side *is = &fds.info.sides[side];
		const BYTE *src = is->data;
		_fds_info_block fib = { 0 };
		WORD crc = 0;

		length = (type == FDS_TYPE_FDS ? 16 : 0) + (fds_disk_side_size(format) * side);

		for (position = 0; position < fds_image_side_size();) {
			if ((src[position] == FDS_DISK_GAP) || (src[position] == FDS_DISK_BLOCK_MARK)) {
				position++;
				continue;
			}
			if (fds_examine_block(src, position, &fib)) {
				if (format == FDS_FORMAT) {
					crc = fds_crc_block(&src[position], fib.blength);
				} else {
					crc = src[position + fib.blength];
					crc = (src[position + fib.blength + 1] << 8) | crc;
				}
			} else {
				break;
			}

			if (src[position]) {
				if ((total_size + fib.blength + (format == QD_FORMAT ? 2 : 0)) < fds_disk_side_size(format)) {
					// troncato
					for (unsigned int i = 0; i < fib.blength; i++) {
						mfds[length] = src[position];
						length++;
						position++;
						total_size++;
					}
					if (format == QD_FORMAT) {
						mfds[length] = (crc >> 0);
						mfds[length + 1] = (crc >> 8);
						length += 2;
						total_size += 2;
					}
					position += 2;
				} else {
					break;
				}
			}
		}
	}
	return (mfds);
}
BYTE *fds_read_ips(BYTE *data, size_t size, const uTCHAR *file) {
	size_t fsize = 0, fpos = 0, mpos = 0;
	FILE *fp = ufopen(file, uL("rb"));
	uint32_t counter = 0;
	BYTE rc = EXIT_OK;
	char header[5];

	if (!fp || !data) {
		return (NULL);
	}

	fseek(fp, 0L, SEEK_END);
	fsize = ftell(fp);
	fseek(fp, 0L, SEEK_SET);

	if (fsize >= 0x1000000) {
		fclose(fp);
		free(data);
		return (NULL);
	}
	if ((fread(&header[0], 1, sizeof(header), fp) < sizeof(header)) || strncmp((void *)header, "PATCH", 5)) {
		fclose(fp);
		free(data);
		return (NULL);
	}
	fpos += sizeof(header);

	// Itera attraverso le patch
	while ((fpos < fsize) && (mpos < size)) {
		size_t offset = 0, length = 0;
		BYTE address[3], len[2];
		BYTE rle = FALSE, ch = 0;

		// Leggo l'offset dalla patch corrente
		if (fread(&address[0], 1, sizeof(address), fp) < sizeof(address)) {
			rc = EXIT_ERROR;
			break;
		}
		fpos += sizeof(address);
		offset = (address[0] << 16) | (address[1] << 8) | address[2];

		// EOF
		if (offset == 0x454F46) {
			break;
		}

		// Leggo la lunghezza dalla patch corrente
		if (fread(&len[0], 1, sizeof(len), fp) < sizeof(len)) {
			rc = EXIT_ERROR;
			break;
		}
		fpos += sizeof(len);
		length = (len[0] << 8) | len[1];

		// Se la lunghezza e' 0, e' un blocco RLE
		if (!length) {
			rle = TRUE;

			// Leggo la lunghezza RLE a 16 bit
			if (fread(&len[0], 1, sizeof(len), fp) < sizeof(len)) {
				rc = EXIT_ERROR;
				break;
			}
			fpos += sizeof(len);
			length = (len[0] << 8) | len[1];

			// Leggo il byte da ripetere
			if (fread(&ch, 1, sizeof(ch), fp) < sizeof(ch)) {
				rc = EXIT_ERROR;
				break;
			}
			fpos += sizeof(ch);
		}

		if ((offset + length) >= size) {
			size_t old_size = size;
			BYTE *new_blk = NULL;

			size = (offset + length);
			new_blk = (BYTE *)realloc(data, size);
			if (!new_blk) {
				rc = EXIT_ERROR;
				break;
			}
			memset(new_blk + old_size, 0x00, size - old_size);
			data = new_blk;
		}

		if (rle) {
			// Applico la ripetizione dei dati in memoria
			for (size_t i = 0; i < length; ++i) {
				data[offset + i] = ch;
			}
		} else {
			// Leggo i dati da applicare dalla patch
			if (fread(&data[offset], 1, length, fp) < length) {
				rc = EXIT_ERROR;
				break;
			}
			fpos += length;
		}
		counter++;
		mpos = offset + length;
	}
	fclose(fp);
	if ((rc == EXIT_ERROR) || !counter) {
		free(data);
		data = NULL;
	}
	return (data);
}
BYTE fds_create_ips(const BYTE *d1, const size_t size1, const BYTE *d2, const size_t size2, const uTCHAR *file) {
	FILE *fp = ufopen(file, uL("w+b"));
	uint32_t counter = 0;
	size_t i = 0, fsize = 0;

	if (!d1 || !d2 || !fp) {
		return (EXIT_ERROR);
	}
	fwrite("PATCH", 5, 1, fp);
	fsize += 5;

	while ((i < size1) && (i < size2)) {
		size_t length = 0;
		BYTE rle = TRUE;

		if (i < size1) {
			if (d1[i] == d2[i]) {
				i++;
				continue;
			}
			while (((i + length) < size1) && ((i + length) < size2) && (length < 65535)) {
				if (d1[i + length] == d2[i + length]) {
					break;
				}
				if (!rle && (length >= 3) && ((length + 3) < size1)) {
					if ((d1[i + length + 0] != d2[i + length + 0]) &&
						(d1[i + length + 1] != d2[i + length + 1]) &&
						(d1[i + length + 2] != d2[i + length + 2]) &&
						(d2[i + length + 0] == d2[i + length + 1]) &&
						(d2[i + length + 1] == d2[i + length + 2])) {
						break;
					}
				}
				if (rle && (length >= 1) && (d2[i + length] != d2[i])) {
					if (length < 3) {
						rle = FALSE;
						continue;
					}
					break;
				}
				length++;
			}
		}

		if (!length) {
			while (((i + length) < size2) && (length < 65535)) {
				if (!rle && (length >= 3) && ((length + 3) < size2)) {
					if ((d2[i + length + 0] == d2[i + length + 1]) &&
						(d2[i + length + 1] == d2[i + length + 2])) {
						break;
					}
				}
				if (rle && (length >= 1) && (d2[i + length] != d2[i])) {
					if (length < 3) {
						rle = FALSE;
						continue;
					}
					break;
				}
				length++;
			}
		}

		if (length == 1) {
			rle = FALSE;
		}

		{
			// Big Endian
			BYTE address[3] = { i >> 16, i >> 8, i & 0xFF  };
			BYTE len[2] = { length >> 8, length & 0xFF };
			BYTE rlen[2] = { 0x00, 0x00 };

			if (rle) {
				rlen[0] = len[0];
				rlen[1] = len[1];
				len[0] = 0x00;
				len[1] = 0x00;
			}
			// Scrivo le istruzioni di patching nel file IPS
			fputc(address[0], fp);
			fputc(address[1], fp);
			fputc(address[2], fp);
			fputc(len[0], fp);
			fputc(len[1], fp);
			fsize += 5;
			if (rle) {
				fputc(rlen[0], fp);
				fputc(rlen[1], fp);
				fputc(d2[i], fp);
				fsize += 3;
			} else {
				fwrite(&d2[i], 1, length, fp);
				fsize += length;
			}

			// forzo la scrittura del file
			fflush(fp);

			if (fsize >= 0x1000000) {
				log_error(uL("ips;error on writing file, too large (max 16MB)"));
				break;
			}
		}

		counter++;
		i += length;
	}
	fclose(fp);

	if (!counter) {
		uremove(file);
	}
	return (EXIT_OK);
}
