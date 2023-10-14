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
#include "ntsc_nesrgb_lmp88959.h"
#include "video/gfx.h"
#include "nes.h"
#define CRT_SYSTEM CRT_SYSTEM_NESRGB
#include "extra/NTSC-CRT/crt_core_nesrgb.h"

const _ntsc_lmp88959_setup_t ntsc_nesrgb_lmp88959_default = { 0, 16, 165, 0, 2, 85, 12, 0, 0, 1 };
_ntsc_lmp88959_setup_t ntsc_nesrgb_lmp88959 = { 0, 16, 165, 0, 2, 85, 12, 0, 0, 1 };
static struct CRT_NESRGB crt_nesrgb = { 0 };
static struct NTSC_SETTINGS ntsc_nesrgb = { 0 };

BYTE ntsc_nesrgb_lmp88959_init(void) {
	memset(&crt_nesrgb, 0x00, sizeof(crt_nesrgb));
	memset(&ntsc_nesrgb, 0x00, sizeof(ntsc_nesrgb));
	crt_nesrgb_init(&crt_nesrgb, 0, 0, CRT_PIX_FORMAT_BGRA, NULL);
	return (EXIT_OK);
}
void ntsc_nesrgb_lmp88959_surface(BYTE nidx) {
	if (crt_nesrgb.out != gfx.filter.data.pix) {
		crt_nesrgb_init(&crt_nesrgb, SCR_COLUMNS * gfx.filter.factor, SCR_ROWS * gfx.filter.factor,
			CRT_PIX_FORMAT_BGRA, (unsigned char *)gfx.filter.data.pix);
	}
	crt_nesrgb.hue = ntsc_nesrgb_lmp88959.hue;
	crt_nesrgb.brightness = ntsc_nesrgb_lmp88959.brightness;
	crt_nesrgb.contrast = ntsc_nesrgb_lmp88959.contrast;
	crt_nesrgb.saturation = ntsc_nesrgb_lmp88959.saturation;
	crt_nesrgb.black_point = ntsc_nesrgb_lmp88959.black_point;
	crt_nesrgb.white_point = ntsc_nesrgb_lmp88959.white_point;
	crt_nesrgb.blend = ntsc_nesrgb_lmp88959.vertical_blend;
	crt_nesrgb.scanlines = ntsc_nesrgb_lmp88959.scanline;
	ntsc_nesrgb.data = (const unsigned char *)nes[nidx].p.ppu_screen.rd->data;
	ntsc_nesrgb.format = CRT_PIX_FORMAT_BGRA;
	ntsc_nesrgb.palette = (uint32_t *)gfx.filter.data.palette;
	ntsc_nesrgb.w = SCR_COLUMNS;
	ntsc_nesrgb.h = SCR_ROWS;
	ntsc_nesrgb.dot_crawl_offset = (ntsc_nesrgb.dot_crawl_offset + 1) % 2;
//	ntsc_nesrgb.hue = ntsc_nesrgb_lmp88959.hue;
	crt_nesrgb_modulate(&crt_nesrgb, &ntsc_nesrgb);
	crt_nesrgb_demodulate(&crt_nesrgb, ntsc_nesrgb_lmp88959.noise);
	lmp88959_phosphor_decay();
}

void ntsc_nesrgb_lmp88959_filter_parameters_changed(void) {
	ntsc_nesrgb_lmp88959_init();
}
void ntsc_nesrgb_lmp88959_filter_parameters_default(void) {
	ntsc_nesrgb_lmp88959 = ntsc_nesrgb_lmp88959_default;
}

void ntsc_nesrgb_lmp88959_filter_parameter_default(int index) {
	const _ntsc_lmp88959_setup_t *format = &ntsc_nesrgb_lmp88959_default;

	switch (index) {
		default:
		case 0:
			ntsc_nesrgb_lmp88959.brightness = format->brightness;
			break;
		case 1:
			ntsc_nesrgb_lmp88959.hue = format->hue;
			break;
		case 2:
			ntsc_nesrgb_lmp88959.saturation = format->saturation;
			break;
		case 3:
			ntsc_nesrgb_lmp88959.contrast = format->contrast;
			break;
		case 4:
			ntsc_nesrgb_lmp88959.black_point = format->black_point;
			break;
		case 5:
			ntsc_nesrgb_lmp88959.white_point = format->white_point;
			break;
		case 6:
			ntsc_nesrgb_lmp88959.noise = format->noise;
			break;
	}
}
void ntsc_nesrgb_lmp88959_filter_parameter_sv_default(void) {
	const _ntsc_lmp88959_setup_t *format = &ntsc_nesrgb_lmp88959_default;

	ntsc_nesrgb_lmp88959.scanline = format->scanline;
	ntsc_nesrgb_lmp88959.vertical_blend = format->vertical_blend;
}

#include "extra/NTSC-CRT/crt_core_nesrgb.c"
#include "extra/NTSC-CRT/crt_nesrgb.c"
