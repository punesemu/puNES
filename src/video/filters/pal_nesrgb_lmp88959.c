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
#include "pal_nesrgb_lmp88959.h"
#include "video/gfx.h"
#include "nes.h"
#define PAL_SYSTEM PAL_SYSTEM_NESRGB
#include "extra/PAL-CRT/pal_core_nesrgb.h"

const _pal_lmp88959_setup_t pal_nesrgb_lmp88959_default = { 16, 165, 0, 2, 85, 12, 0, 0, 0, 0, 1, 1 };
_pal_lmp88959_setup_t pal_nesrgb_lmp88959 = { 16, 165, 0, 2, 85, 12, 0, 0, 0, 0, 1, 1 };
static struct PAL_CRT_NESRGB crt_nesrgb = { 0 };
static struct PAL_SETTINGS pal_nergb = { 0 };
static int field = 0;

BYTE pal_nesrgb_lmp88959_init(void) {
	memset(&crt_nesrgb, 0x00, sizeof(crt_nesrgb));
	memset(&pal_nergb, 0x00, sizeof(pal_nergb));
	field = 0;
	pal_nesrgb_init(&crt_nesrgb, 0, 0, PAL_PIX_FORMAT_BGRA, NULL);
	return (EXIT_OK);
}
void pal_nesrgb_lmp88959_surface(BYTE nidx) {
	if (crt_nesrgb.out != gfx.filter.data.pix) {
		pal_nesrgb_init(&crt_nesrgb, SCR_COLUMNS * gfx.filter.factor, SCR_ROWS * gfx.filter.factor,
			PAL_PIX_FORMAT_BGRA, (unsigned char *)gfx.filter.data.pix);
	}
	crt_nesrgb.brightness = pal_nesrgb_lmp88959.brightness;
	crt_nesrgb.contrast = pal_nesrgb_lmp88959.contrast;
	crt_nesrgb.saturation = pal_nesrgb_lmp88959.saturation;
	crt_nesrgb.black_point = pal_nesrgb_lmp88959.black_point;
	crt_nesrgb.white_point = pal_nesrgb_lmp88959.white_point;
	crt_nesrgb.blend = pal_nesrgb_lmp88959.vertical_blend;
	crt_nesrgb.scanlines = pal_nesrgb_lmp88959.scanline;
	crt_nesrgb.chroma_lag = pal_nesrgb_lmp88959.chroma_lag;
	crt_nesrgb.chroma_correction = pal_nesrgb_lmp88959.chroma_correction;
	pal_nergb.data = (const unsigned char *)nes[nidx].p.ppu_screen.rd->data;
	pal_nergb.format = PAL_PIX_FORMAT_BGRA;
	pal_nergb.palette = (uint32_t *)gfx.filter.data.palette;
	pal_nergb.w = SCR_COLUMNS;
	pal_nergb.h = SCR_ROWS;
	pal_nesrgb_modulate(&crt_nesrgb, &pal_nergb);
	pal_nesrgb_demodulate(&crt_nesrgb, pal_nesrgb_lmp88959.noise);
	lmp88959_phosphor_decay();
}

void pal_nesrgb_lmp88959_filter_parameters_changed(void) {
	pal_nesrgb_lmp88959_init();
}
void pal_nesrgb_lmp88959_filter_parameters_default(void) {
	pal_nesrgb_lmp88959 = pal_nesrgb_lmp88959_default;
}
void pal_nesrgb_lmp88959_filter_parameter_default(int index) {
	const _pal_lmp88959_setup_t *format = &pal_nesrgb_lmp88959_default;

	switch (index) {
		default:
		case 0:
			pal_nesrgb_lmp88959.brightness = format->brightness;
			break;
		case 1:
			pal_nesrgb_lmp88959.saturation = format->saturation;
			break;
		case 2:
			pal_nesrgb_lmp88959.contrast = format->contrast;
			break;
		case 3:
			pal_nesrgb_lmp88959.black_point = format->black_point;
			break;
		case 4:
			pal_nesrgb_lmp88959.white_point = format->white_point;
			break;
		case 5:
			pal_nesrgb_lmp88959.noise = format->noise;
			break;
		case 6:
			pal_nesrgb_lmp88959.chroma_lag = format->chroma_lag;
			break;
	}
}
void pal_nesrgb_lmp88959_filter_parameter_smv_default(void) {
	const _pal_lmp88959_setup_t *format = &pal_nesrgb_lmp88959_default;

	pal_nesrgb_lmp88959.scanline = format->scanline;
	pal_nesrgb_lmp88959.vertical_blend = format->vertical_blend;
}

#include "extra/PAL-CRT/pal_core_nesrgb.c"
#include "extra/PAL-CRT/pal_nesrgb.c"
