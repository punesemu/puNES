/*
 * uncompress.c
 *
 *  Created on: 17/dic/2013
 *      Author: fhorse
 */

#include <libgen.h>
#include "uncompress.h"
#include "info.h"
#include "gamegenie.h"
#include "c++/l7zip/l7z.h"
#include "gui.h"
#define MINIZ_NO_ARCHIVE_WRITING_APIS
#define MINIZ_NO_TIME
#include "miniz.h"
#undef MINIZ_NO_TIME
#undef MINIZ_NO_ARCHIVE_WRITING_APIS

BYTE (*uncomp_control_in_archive)(void);
BYTE (*uncomp_file_from_archive)(_uncomp_file_data *file);
BYTE (*uncomp_name_file_compress)(_uncomp_file_data *file);
/* zip */
BYTE uncomp_zip_control_in_archive(void);
BYTE uncomp_zip_file_from_archive(_uncomp_file_data *file);
BYTE uncomp_zip_name_file_compress(_uncomp_file_data *file);

BYTE uncomp_init(void) {
	l7z_init();

	memset(&uncomp, 0x00, sizeof(uncomp));

	return (EXIT_OK);
}
void uncomp_quit(void) {
	l7z_quit();

	uncomp_remove();
}
BYTE uncomp_ctrl(char *ext) {
	if (gamegenie.phase == GG_LOAD_ROM) {
		return (EXIT_OK);
	}

	/* azzero singolarmente i campi della struttura uncomp */
	uncomp.files_founded = 0;
	uncomp.file = NULL;
	memset(&uncomp.compress_archive, 0x00, sizeof(uncomp.compress_archive));
	memset(&uncomp.buffer, 0x00, sizeof(uncomp.buffer));

	if ((l7z_present() == TRUE) && (l7z_control_ext(ext) == EXIT_OK)){
		uncomp_control_in_archive = l7z_control_in_archive;
		uncomp_file_from_archive = l7z_file_from_archive;
		uncomp_name_file_compress = l7z_name_file_compress;
	} else if (!strcasecmp(ext, ".zip")) {
		uncomp_control_in_archive = uncomp_zip_control_in_archive;
		uncomp_file_from_archive = uncomp_zip_file_from_archive;
		uncomp_name_file_compress = uncomp_zip_name_file_compress;
	} else {
		return (EXIT_OK);
	}

	if (uncomp_control_in_archive() == EXIT_ERROR) {
		return (EXIT_ERROR);
	}

	switch (uncomp.files_founded) {
		case 0:
			break;
		case 1:
			uncomp_file_from_archive(&uncomp.file[0]);
			break;
		default: {
			int selected = gui_uncompress_selection_dialog();

			if (selected != UNCOMP_NO_FILE_SELECTED) {
				uncomp_file_from_archive(&uncomp.file[selected]);
			}
			break;
		}
	}

	return (EXIT_OK);
}
BYTE uncomp_name_file(_uncomp_file_data *file) {
	return (uncomp_name_file_compress(file));
}

void uncomp_remove(void) {
	if ((gamegenie.phase == GG_LOAD_ROM) || (gamegenie.phase == GG_LOAD_GAMEGENIE)) {
		return;
	}

	if (uncomp.file != NULL) {
		free(uncomp.file);
		uncomp.file = NULL;
	}

	if (info.uncompress_rom == TRUE) {
		remove(uncomp.uncompress_file);
		memset(&uncomp.uncompress_file, 0x00, sizeof(uncomp.uncompress_file));
	}

	info.uncompress_rom = FALSE;
}

BYTE uncomp_zip_control_in_archive(void) {
	mz_zip_archive zip_archive;
	int a, mode;

	memset(&zip_archive, 0, sizeof(zip_archive));

	if (!mz_zip_reader_init_file(&zip_archive, info.rom_file, 0)) {
		fprintf(stderr, "mz_zip_reader_init_file() failed!\n");
		return (EXIT_ERROR);
	}

	for (mode = UNCOMP_CTRL_FILE_COUNT_ROMS; mode <= UNCOMP_CTRL_FILE_SAVE_DATA; mode++) {
		uncomp.files_founded = 0;

		for (a = 0; a < (int) mz_zip_reader_get_num_files(&zip_archive); a++) {
			mz_zip_archive_file_stat file_stat;
			int b;

			if (!mz_zip_reader_file_stat(&zip_archive, a, &file_stat)) {
				fprintf(stderr, "mz_zip_reader_file_stat() failed!\n");
				mz_zip_reader_end(&zip_archive);
				return (EXIT_ERROR);
			}

			/* se e' una directory continuo */
			if (mz_zip_reader_is_file_a_directory(&zip_archive, a)) {
				continue;
			}

			for (b = 0; b < LENGTH(format_supported); b++) {
				char *ext = strrchr(file_stat.m_filename, '.');

				if ((ext != NULL) && (strcasecmp(ext, format_supported[b].ext) == 0)) {
					if (mode == UNCOMP_CTRL_FILE_SAVE_DATA) {
						uncomp.file[uncomp.files_founded].num = file_stat.m_file_index;
						uncomp.file[uncomp.files_founded].format = format_supported[b].id;
					}
					uncomp.files_founded++;
					break;
				}
			}
		}

		if ((mode == UNCOMP_CTRL_FILE_COUNT_ROMS) && (uncomp.files_founded > 0)) {
			uncomp.file = (_uncomp_file_data *) malloc(
				uncomp.files_founded * sizeof(_uncomp_file_data));
		}
	}

	mz_zip_reader_end(&zip_archive);

	return (EXIT_OK);
}
BYTE uncomp_zip_file_from_archive(_uncomp_file_data *file) {
	mz_zip_archive zip_archive;

	memset(&zip_archive, 0, sizeof(zip_archive));

	if (!mz_zip_reader_init_file(&zip_archive, info.rom_file, 0)) {
		fprintf(stderr, "mz_zip_reader_init_file() failed!\n");
		return (EXIT_ERROR);
	}

	mz_zip_reader_get_filename(&zip_archive, file->num, uncomp.buffer, sizeof(uncomp.buffer));

	snprintf(uncomp.uncompress_file, sizeof(uncomp.uncompress_file), "%s" TMP_FOLDER "/%s",
		info.base_folder, basename(uncomp.buffer));

	if (mz_zip_reader_extract_to_file(&zip_archive, file->num, uncomp.uncompress_file, 0)) {
		strncpy(uncomp.compress_archive, info.rom_file, sizeof(uncomp.compress_archive));
		strncpy(info.rom_file, uncomp.uncompress_file, sizeof(info.rom_file));
		info.uncompress_rom = TRUE;
	} else {
		fprintf(stderr, "unzip file failed!\n");
	}

	// Close the archive, freeing any resources it was using
	mz_zip_reader_end(&zip_archive);

	return (EXIT_OK);
}
BYTE uncomp_zip_name_file_compress(_uncomp_file_data *file) {
	mz_zip_archive zip_archive;

	memset(&zip_archive, 0, sizeof(zip_archive));

	if (!mz_zip_reader_init_file(&zip_archive, info.rom_file, 0)) {
		fprintf(stderr, "mz_zip_reader_init_file() failed!\n");
		return (EXIT_ERROR);
	}

	mz_zip_reader_get_filename(&zip_archive, file->num, uncomp.buffer, sizeof(uncomp.buffer));

	// Close the archive, freeing any resources it was using
	mz_zip_reader_end(&zip_archive);

	return (EXIT_OK);
}
