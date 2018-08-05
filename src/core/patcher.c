/*
 *  Copyright (C) 2010-2019 Fabio Cavallo (aka FHorse)
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
#include <stdio.h>
#include "patcher.h"
#include "rom_mem.h"
#include "info.h"
#include "emu.h"
#include "gui.h"
#include "text.h"
#include "uncompress.h"
#include "cheat.h"
#include "conf.h"

static DBWORD patcher_2byte(_rom_mem *patch);
static DBWORD patcher_3byte(_rom_mem *patch);
static BYTE patcher_ips(_rom_mem *patch, _rom_mem *rom);

void patcher_init(void) {
	memset(&patcher, 0x00, sizeof(patcher));
}
void patcher_quit(void) {
	if (patcher.file) {
		free (patcher.file);
		patcher.file = NULL;
	}
}
BYTE patcher_ctrl_if_exist(uTCHAR *patch) {
	static const uTCHAR patch_ext[][10] = {	uL(".ips\0"), uL(".IPS\0") };
	uTCHAR file[LENGTH_FILE_NAME_LONG], ext[10];
	BYTE i, found = FALSE;

	if (patch) {
		ustrncpy(file, patch, usizeof(file));
	} else if (patcher.file) {
		ustrncpy(file, patcher.file, usizeof(file));
	} else if (gamegenie.patch) {
		ustrncpy(file, gamegenie.patch, usizeof(file));
	} else {
		ustrncpy(file, info.rom.file, usizeof(file));
	}

	if (file[0] == 0) {
		return (EXIT_ERROR);
	}

	memset(&patcher, 0x00, sizeof(patcher));

	{
		_uncompress_archive *archive;
		uTCHAR *upatch = NULL;
		BYTE rc;

		archive = uncompress_archive_alloc(file, &rc);

		if (rc == UNCOMPRESS_EXIT_OK) {
			if (archive->patch.count > 0) {
				switch ((rc = uncompress_archive_extract_file(archive,UNCOMPRESS_TYPE_PATCH))) {
					case UNCOMPRESS_EXIT_OK:
						upatch = uncompress_archive_extracted_file_name(archive, UNCOMPRESS_TYPE_PATCH);
						found = TRUE;
						break;
					case UNCOMPRESS_EXIT_ERROR_ON_UNCOMP:
						break;
					default:
						break;
				}
				if (upatch) {
					ustrncpy(file, upatch, usizeof(file));
				}
			}
			uncompress_archive_free(archive);
		} else if (rc == UNCOMPRESS_EXIT_IS_NOT_COMP) {
			found = TRUE;
		}
	}

	if (found == FALSE) {
		patcher.patched = FALSE;
		return (EXIT_ERROR);
	}

	found = FALSE;

	for (i = 0; i < LENGTH(patch_ext); i++) {
		uTCHAR *last_dot;

		// rintraccio l'ultimo '.' nel nome
		last_dot = ustrrchr(file, uL('.'));
		// elimino l'estensione
		*last_dot = 0x00;
		// aggiungo l'estensione
		ustrcat(file, patch_ext[i]);

		if (emu_file_exist(file) == EXIT_OK) {
			found = TRUE;
			ustrncpy(ext, patch_ext[i], usizeof(ext));
			break;
		}
	}

	if (found == TRUE) {
		patcher.file = emu_ustrncpy(patcher.file, file);
		return (EXIT_OK);
	}

	return (EXIT_ERROR);
}
void patcher_apply(void *rom_mem) {
	_rom_mem patch, *rom = (_rom_mem *)rom_mem;
	uTCHAR *ext;
	FILE *fp;

	if ((cfg->cheat_mode == GAMEGENIE_MODE) && (gamegenie.phase != GG_LOAD_ROM)) {
		return;
	}

	if (patcher.file == NULL) {
		return;
	}

	if ((fp = ufopen(patcher.file, uL("rb"))) == NULL) {
		patcher_quit();
		return;
	}

	fseek(fp, 0L, SEEK_END);
	patch.size = ftell(fp);
	fseek(fp, 0L, SEEK_SET);

	if ((patch.data = (BYTE *) malloc(patch.size)) == NULL) {
		patcher_quit();
		fclose(fp);
		return;
	}

	if (fread(patch.data, 1, patch.size, fp) != patch.size) {
		patcher_quit();
		fclose(fp);
		free(patch.data);
		return;
	}

	fclose(fp);

	patch.position = 0;

	ext = ustrrchr(patcher.file, uL('.'));

	if (ustrcasecmp(ext, uL(".ips")) == 0) {
		if (patcher_ips(&patch, rom) == EXIT_ERROR) {
			text_add_line_info(1, "[red]error loading patch file");
			fprintf(stderr, "error loading patch file\n");
		} else {
			patcher.patched = TRUE;
		}
	}

	patcher_quit();

	free(patch.data);
}

static DBWORD patcher_2byte(_rom_mem *patch) {
	DBWORD dbw = 0;
	BYTE ch;

	if (rom_mem_ctrl_memcpy(&ch, patch, 1) == EXIT_ERROR) {
		return (-1);
	}
	dbw = ch << 8;
	if (rom_mem_ctrl_memcpy(&ch, patch, 1) == EXIT_ERROR) {
		return (-1);
	}
	dbw = (dbw & 0x0000FF00) | ch;

	return (dbw);
}
static DBWORD patcher_3byte(_rom_mem *patch) {
	DBWORD dbw = 0;
	BYTE ch;

	if (rom_mem_ctrl_memcpy(&ch, patch, 1) == EXIT_ERROR) {
		return (-1);
	}
	dbw = ch << 16;
	if (rom_mem_ctrl_memcpy(&ch, patch, 1) == EXIT_ERROR) {
		return (-1);
	}
	dbw = (dbw & 0x00FF0000) | (ch << 8);
	if (rom_mem_ctrl_memcpy(&ch, patch, 1) == EXIT_ERROR) {
		return (-1);
	}
	dbw = (dbw & 0x00FFFF00) | ch;

	return (dbw);
}
static BYTE patcher_ips(_rom_mem *patch, _rom_mem *rom) {
	size_t size = rom->size;
	BYTE *blk;

	if (strncmp((void *)patch->data, "PATCH", 5) != 0) {
		return (EXIT_ERROR);
	}
	patch->position += 5;

	if ((blk = (BYTE *) malloc(size)) == NULL) {
		return (EXIT_ERROR);
	};

	memcpy(blk, rom->data, size);

	while (TRUE) {
		DBWORD address, len;
		BYTE rle = FALSE;
		BYTE ch;

		if (((address = patcher_3byte(patch)) == -1) || (address == 0x454f46)) {
			break;
		}

		if ((len = patcher_2byte(patch)) == -1) {
			free(blk);
			return (EXIT_ERROR);
		}

		// RLE
		if (len == 0) {
			rle = TRUE;

			if ((len = patcher_2byte(patch)) == -1) {
				free(blk);
				return (EXIT_ERROR);
			}
			if (rom_mem_ctrl_memcpy(&ch, patch, 1) == EXIT_ERROR) {
				free(blk);
				return (EXIT_ERROR);
			}
		}

		if ((address + len) > size) {
			size = (address + len);
			blk = (BYTE *)realloc(blk, size);
		}

		if (rle == TRUE) {
			DBWORD i;

			for (i = 0; i < len; i++) {
				blk[address + i] = ch;
			}
		} else {
			if (rom_mem_ctrl_memcpy(blk + address, patch, len) == EXIT_ERROR) {
				free(blk);
				return (EXIT_ERROR);
			}
		}
	}

	free(rom->data);

	rom->data = blk;
	rom->size = size;

	return (EXIT_OK);
}
