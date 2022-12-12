/*
 *  Copyright (C) 2010-2022 Fabio Cavallo (aka FHorse)
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
#include <string.h>
#include "palette.h"
#include "gui.h"

_color_RGB palette_base_file[64];
_palette_RGB palette_RGB;

void palette_save_on_file(const uTCHAR *file) {
	FILE *fp;

	if ((fp = ufopen(file, uL("wb"))) == NULL) {
		log_error(uL("palette;impossible save palette file " uPs("")), file);
		return;
	}

	if (!(fwrite((BYTE *)palette_RGB.noswap, 64 * 3, 1, fp))) {}

	fclose(fp);
}
BYTE palette_load_from_file(const uTCHAR *file) {
	FILE *fp;

	memset((BYTE *)palette_base_file, 0x00, 64 * 3);

	if ((fp = ufopen(file, uL("rb"))) == NULL) {
		log_error(uL("palette; error on open file " uPs("")), file);
		return (EXIT_ERROR);
	}

	fseek(fp, 0, SEEK_END);

	if (ftell(fp) < (64 * 3)) {
		log_error(uL("palette;error on read file " uPs("")), file);
		fclose(fp);
		return (EXIT_ERROR);
	}

	fseek(fp, 0L, SEEK_SET);

	if (!(fread((BYTE *)palette_base_file, 64 * 3, 1, fp))) {}

	fclose(fp);

	return (EXIT_OK);
}
