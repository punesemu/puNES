/*
 *  Copyright (C) 2010-2020 Fabio Cavallo (aka FHorse)
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
#include "patcher_xdelta3_wrap.h"
#include "rom_mem.h"
#include "info.h"
#include "emu.h"
#include "gui.h"
#include "cheat.h"
#include "conf.h"

enum patcher_bps {
	SourceRead,
	TargetRead,
	SourceCopy,
	TargetCopy
};

static SDBWORD patcher_2byte(_rom_mem *patch);
static SDBWORD patcher_3byte(_rom_mem *patch);
static int64_t patcher_4byte_reverse(_rom_mem *patch);
static uint32_t patcher_crc32(unsigned char *message, unsigned int len);
static BYTE patcher_ips(_rom_mem *patch, _rom_mem *rom);
static BYTE patcher_bps_decode(_rom_mem *patch, size_t *size);
static BYTE patcher_bps(_rom_mem *patch, _rom_mem *rom);

_patcher patcher;

void patcher_init(void) {
	memset(&patcher, 0x00, sizeof(patcher));
}
void patcher_quit(void) {
	if (patcher.file) {
		free(patcher.file);
		patcher.file = NULL;
	}
}
BYTE patcher_ctrl_if_exist(uTCHAR *patch) {
	static const uTCHAR patch_ext[][10] = {
		uL(".ips\0"), uL(".IPS\0"),
		uL(".bps\0"), uL(".BPS\0"),
		uL(".xdelta\0"), uL(".XDELTA\0")
	};
	uTCHAR file[LENGTH_FILE_NAME_LONG], ext[10];
	BYTE i, found = FALSE;

	if (patch) {
		ustrncpy(file, patch, usizeof(file) - 1);
	} else if (patcher.file) {
		ustrncpy(file, patcher.file, usizeof(file) - 1);
	} else if (gamegenie.patch) {
		ustrncpy(file, gamegenie.patch, usizeof(file) - 1);
	} else {
		ustrncpy(file, info.rom.file, usizeof(file));
	}

	if (file[0] == 0) {
		return (EXIT_ERROR);
	}

	patcher_quit();

	memset(&patcher, 0x00, sizeof(patcher));

	{
		_uncompress_archive *archive;
		BYTE rc;

		archive = uncompress_archive_alloc(file, &rc);

		if (rc == UNCOMPRESS_EXIT_OK) {
			if (archive->patch.count > 0) {
				switch ((rc = uncompress_archive_extract_file(archive,UNCOMPRESS_TYPE_PATCH))) {
					case UNCOMPRESS_EXIT_OK:
						ustrncpy(file, uncompress_archive_extracted_file_name(archive, UNCOMPRESS_TYPE_PATCH), usizeof(file) - 1);
						found = TRUE;
						break;
					case UNCOMPRESS_EXIT_ERROR_ON_UNCOMP:
						break;
					default:
						break;
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
		if ((last_dot = ustrrchr(file, uL('.')))) {
			// elimino l'estensione
			(*last_dot) = 0x00;
		};
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

	if ((patch.data = (BYTE *)malloc(patch.size)) == NULL) {
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
			gui_overlay_info_append_msg_precompiled(12, NULL);
			fprintf(stderr, "error loading patch file\n");
		} else {
			patcher.patched = TRUE;
		}
	} else if (ustrcasecmp(ext, uL(".bps")) == 0) {
		if (patcher_bps(&patch, rom) == EXIT_ERROR) {
			gui_overlay_info_append_msg_precompiled(12, NULL);
			fprintf(stderr, "error loading patch file\n");
		} else {
			patcher.patched = TRUE;
		}
	} else if (ustrcasecmp(ext, uL(".xdelta")) == 0) {
		if (patcher_xdelta(&patch, rom) == EXIT_ERROR) {
			gui_overlay_info_append_msg_precompiled(12, NULL);
			fprintf(stderr, "error loading patch file\n");
		} else {
			patcher.patched = TRUE;
		}
	}

	gui_utf_dirname(patcher.file, gui.last_open_patch_path, usizeof(gui.last_open_patch_path) - 1);

	patcher_quit();

	free(patch.data);
}

static SDBWORD patcher_2byte(_rom_mem *patch) {
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
static SDBWORD patcher_3byte(_rom_mem *patch) {
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
static int64_t patcher_4byte_reverse(_rom_mem *patch) {
	uint64_t dbw = 0;
	BYTE ch;

	if (rom_mem_ctrl_memcpy(&ch, patch, 1) == EXIT_ERROR) {
		return (-1);
	}
	dbw = ch;
	if (rom_mem_ctrl_memcpy(&ch, patch, 1) == EXIT_ERROR) {
		return (-1);
	}
	dbw = (dbw & 0x000000FF) | (ch << 8);
	if (rom_mem_ctrl_memcpy(&ch, patch, 1) == EXIT_ERROR) {
		return (-1);
	}
	dbw = (dbw & 0x0000FFFF) | (ch << 16);
	if (rom_mem_ctrl_memcpy(&ch, patch, 1) == EXIT_ERROR) {
		return (-1);
	}
	dbw = (dbw & 0x00FFFFFF) | (ch << 24);

	return (dbw);
}
static uint32_t patcher_crc32(unsigned char *message, unsigned int len) {
	unsigned int byte, crc, mask, i;
	int j;

	crc = 0xFFFFFFFF;

	for (i = 0; i < len; i++) {
		byte = message[i]; // Get next byte.
		crc = crc ^ byte;

		for (j = 7; j >= 0; j--) { // Do eight times
			mask = -(crc & 1);
			crc = (crc >> 1) ^ (0xEDB88320 & mask);
		}
	}
	return (~crc);
}
static BYTE patcher_ips(_rom_mem *patch, _rom_mem *rom) {
	size_t size = rom->size;
	BYTE *blk;

	if (strncmp((void *)patch->data, "PATCH", 5) != 0) {
		return (EXIT_ERROR);
	}
	patch->position += 5;

	if ((blk = (BYTE *)malloc(size)) == NULL) {
		return (EXIT_ERROR);
	};

	memcpy(blk, rom->data, size);

	while (TRUE) {
		SDBWORD len;
		SDBWORD address;
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

		if ((size_t)(address + len) > size) {
			size = (address + len);
			blk = (BYTE *)realloc(blk, size);
		}

		if (rle == TRUE) {
			SDBWORD i;

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
static BYTE patcher_bps_decode(_rom_mem *patch, size_t *size) {
	size_t shift = 1;
	size_t data = 0;

	(*size) = 0;

	while (TRUE) {
		BYTE x;

		if (rom_mem_ctrl_memcpy(&x, patch, 1) == EXIT_ERROR) {
			return (EXIT_ERROR);
		}

		data += (x & 0x7f) * shift;
		if (x & 0x80) {
			break;
		}
		shift <<= 7;
		data += shift;
	}

	(*size) = data;

	return (EXIT_OK);
}
static BYTE patcher_bps(_rom_mem *patch, _rom_mem *rom) {
	uint32_t crc_patch, crc_out, crc_in;
	size_t size_in = 0, size_out = 0, size_metadata = 0;
	size_t output_offset = 0;
	int32_t source_relative = 0, target_relative = 0;
	BYTE *blk;

	if (patch->size < (4 + 3 + 12)) {
		return (EXIT_ERROR);
	}

	if (strncmp((void *)patch->data, "BPS1", 4) != 0) {
		return (EXIT_ERROR);
	}
	patch->position += 4;

	{
		uint32_t position = patch->position;
		int64_t tmp;

		patch->position = patch->size - 4;
		if ((tmp = patcher_4byte_reverse(patch)) == -1) {
			return (EXIT_ERROR);
		}
		crc_patch = tmp;

		patch->position = patch->size - 8;
		if ((tmp = patcher_4byte_reverse(patch)) == -1) {
			return (EXIT_ERROR);
		}
		crc_out = tmp;

		patch->position = patch->size - 12;
		if ((tmp = patcher_4byte_reverse(patch)) == -1) {
			return (EXIT_ERROR);
		}
		crc_in = tmp;

		if (crc_patch != patcher_crc32(patch->data, patch->size - 4)) {
			return (EXIT_ERROR);
		}

		if (crc_in != patcher_crc32(rom->data, rom->size)) {
			return (EXIT_ERROR);
		}

		patch->position = position;
	}

	if (patcher_bps_decode(patch, &size_in) == EXIT_ERROR) {
		return (EXIT_ERROR);
	}

	if (rom->size != size_in) {
		return (EXIT_ERROR);
	}

	if (patcher_bps_decode(patch, &size_out) == EXIT_ERROR) {
		return (EXIT_ERROR);
	}

	if (patcher_bps_decode(patch, &size_metadata) == EXIT_ERROR) {
		return (EXIT_ERROR);
	}

	if ((blk = (BYTE *)malloc(size_out)) == NULL) {
		return (EXIT_ERROR);
	}

	memset(blk, 0x00, size_out);

	if (size_metadata) {
		patch->position += size_metadata;
	}

	while (patch->position < (patch->size - 12)) {
		size_t data, length, tmp;
		BYTE command;

		if (patcher_bps_decode(patch, &data) == EXIT_ERROR) {
			return (EXIT_ERROR);
		}

		command = data & 0x03;
		length = (data >> 2) + 1;

		if ((output_offset + length) > size_out) {
			free(blk);
			return (EXIT_ERROR);
		}

		switch (command) {
			case SourceRead:
				if ((output_offset + length) > rom->size) {
					free(blk);
					return (EXIT_ERROR);
				}
				while (length--) {
					blk[output_offset] = rom->data[output_offset];
					output_offset++;
				}
				break;
			case TargetRead:
				if ((patch->position + length) > patch->size) {
					free(blk);
					return (EXIT_ERROR);
				}
				while (length--) {
					blk[output_offset++] = patch->data[patch->position++];
				}
				break;
			case SourceCopy:
				if (patcher_bps_decode(patch, &tmp) == EXIT_ERROR) {
					free(blk);
					return (EXIT_ERROR);
				}
				source_relative += (tmp & 1 ? -1 : +1) * (tmp >> 1);
				if ((source_relative < 0) || ((source_relative + length) > rom->size)) {
					free(blk);
					return (EXIT_ERROR);
				}
				while (length--) {
					blk[output_offset++] = rom->data[source_relative++];
				}
				break;
			case TargetCopy:
				if (patcher_bps_decode(patch, &tmp) == EXIT_ERROR) {
					free(blk);
					return (EXIT_ERROR);
				}
				target_relative += (tmp & 1 ? -1 : +1) * (tmp >> 1);
				if ((target_relative < 0) || ((target_relative + length) > size_out)) {
					free(blk);
					return (EXIT_ERROR);
				}
				while (length--) {
					blk[output_offset++] = blk[target_relative++];
				}
				break;
		}
	}

	if (patch->position != (patch->size - 12)) {
		free(blk);
		return (EXIT_ERROR);
	}

	if (output_offset != size_out) {
		free(blk);
		return (EXIT_ERROR);
	}

	if (crc_out != patcher_crc32(blk, size_out)) {
		return (EXIT_ERROR);
	}

	free(rom->data);

	rom->data = blk;
	rom->size = size_out;

	return (EXIT_OK);
}
