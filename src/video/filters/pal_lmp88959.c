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

#include <string.h>
#include "ntsc_lmp88959.h"
#include "pal_lmp88959.h"
#include "video/gfx.h"
#include "nes.h"
#define PAL_SYSTEM PAL_SYSTEM_PAL
#include "extra/PAL-CRT/pal_core.h"

const _pal_lmp88959_setup_t pal_lmp88959_default = { 16, 165, 0, 2, 85, 12, 0, 0, 1, 0, 1, 1 };
_pal_lmp88959_setup_t pal_lmp88959 = { 16, 165, 0, 2, 85, 12, 0, 0, 1, 0, 1, 1 };
static struct PAL_CRT crt = { 0 };
static struct PAL_SETTINGS pal = { 0 };
static int field = 0;

BYTE pal_lmp88959_init(void) {
	memset(&crt, 0x00, sizeof(crt));
	memset(&pal, 0x00, sizeof(pal));
	field = 0;
	pal_init(&crt, 0, 0, PAL_PIX_FORMAT_BGRA, NULL);
	return (EXIT_OK);
}
void pal_lmp88959_surface(BYTE nidx) {
	if (crt.out != gfx.filter.data.pix) {
		pal_init(&crt, SCR_COLUMNS * gfx.filter.factor, SCR_ROWS * gfx.filter.factor,
			PAL_PIX_FORMAT_BGRA, (unsigned char *)gfx.filter.data.pix);
	}
	crt.brightness = pal_lmp88959.brightness;
	crt.contrast = pal_lmp88959.contrast;
	crt.saturation = pal_lmp88959.saturation;
	crt.black_point = pal_lmp88959.black_point;
	crt.white_point = pal_lmp88959.white_point;
	crt.blend = pal_lmp88959.vertical_blend;
	crt.scanlines = pal_lmp88959.scanline;
	crt.chroma_lag = pal_lmp88959.chroma_lag;
	crt.chroma_correction = pal_lmp88959.chroma_correction;
	pal.data = (const unsigned char *)nes[nidx].p.ppu_screen.rd->data;
	pal.format = PAL_PIX_FORMAT_BGRA;
	pal.palette = (uint32_t *)gfx.filter.data.palette;
	pal.w = SCR_COLUMNS;
	pal.h = SCR_ROWS;
	pal.as_color = TRUE;
	pal.field = pal_lmp88959.merge_fields ? field : 0;
	pal.raw = 0;
	pal.color_phase_error = pal_lmp88959.color_phase;
	pal_modulate(&crt, &pal);
	pal_demodulate(&crt, pal_lmp88959.noise);
	if (pal_lmp88959.merge_fields) {
		field++;
	}
	lmp88959_phosphor_decay();
}

void pal_lmp88959_filter_parameters_changed(void) {
	pal_lmp88959_init();
}
void pal_lmp88959_filter_parameters_default(void) {
	pal_lmp88959 = pal_lmp88959_default;
}
void pal_lmp88959_filter_parameter_default(int index) {
	const _pal_lmp88959_setup_t *format = &pal_lmp88959_default;

	switch (index) {
		default:
		case 0:
			pal_lmp88959.brightness = format->brightness;
			break;
		case 1:
			pal_lmp88959.saturation = format->saturation;
			break;
		case 2:
			pal_lmp88959.contrast = format->contrast;
			break;
		case 3:
			pal_lmp88959.black_point = format->black_point;
			break;
		case 4:
			pal_lmp88959.white_point = format->white_point;
			break;
		case 5:
			pal_lmp88959.noise = format->noise;
			break;
		case 6:
			pal_lmp88959.color_phase = format->color_phase;
			break;
		case 7:
			pal_lmp88959.chroma_lag = format->chroma_lag;
			break;
	}
}
void pal_lmp88959_filter_parameter_smv_default(void) {
	const _pal_lmp88959_setup_t *format = &pal_lmp88959_default;

	pal_lmp88959.scanline = format->scanline;
	pal_lmp88959.merge_fields = format->merge_fields;
	pal_lmp88959.vertical_blend = format->vertical_blend;
}

#include "extra/PAL-CRT/pal_core.c"
#include "extra/PAL-CRT/pal.c"
