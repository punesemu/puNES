/*
 *  Copyright (C) 2010-2026 Fabio Cavallo (aka FHorse)
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
#include "video/gfx.h"
#include "nes.h"
#define CRT_SYSTEM CRT_SYSTEM_NTSC
#include "extra/NTSC-CRT/crt_core.h"

const _ntsc_lmp88959_setup_t ntsc_lmp88959_default = { 8, 10, 165, 0, 2, 110, 12, 0, 1, 0 };
_ntsc_lmp88959_setup_t ntsc_lmp88959 = { 8, 10, 165, 0, 2, 110, 12, 0, 1, 0 };
static struct CRT crt = { 0 };
static struct NTSC_SETTINGS ntsc = { 0 };
static int field = 0;

BYTE ntsc_lmp88959_init(void) {
	memset(&crt, 0x00, sizeof(crt));
	memset(&ntsc, 0x00, sizeof(ntsc));
	field = 0;
	crt_init(&crt, 0, 0, CRT_PIX_FORMAT_BGRA, NULL);
	return (EXIT_OK);
}
void ntsc_lmp88959_surface(BYTE nidx) {
	if (crt.out != gfx.filter.data.pix) {
		ntsc_lmp88959_init();
		crt_init(&crt, SCR_COLUMNS * gfx.filter.factor, SCR_ROWS * gfx.filter.factor,
			CRT_PIX_FORMAT_BGRA, (unsigned char *)gfx.filter.data.pix);
	}
	crt.hue = ntsc_lmp88959.hue;
	crt.brightness = ntsc_lmp88959.brightness;
	crt.contrast = ntsc_lmp88959.contrast;
	crt.saturation = ntsc_lmp88959.saturation;
	crt.black_point = ntsc_lmp88959.black_point;
	crt.white_point = ntsc_lmp88959.white_point;
	crt.blend = ntsc_lmp88959.vertical_blend;
	crt.scanlines = ntsc_lmp88959.scanline;
	ntsc.data = (const unsigned char *)nes[nidx].p.ppu_screen.rd->data;
	ntsc.format = CRT_PIX_FORMAT_BGRA;
	ntsc.palette = (uint32_t *)gfx.filter.data.palette;
	ntsc.w = SCR_COLUMNS;
	ntsc.h = SCR_ROWS;
	ntsc.as_color = TRUE;
	ntsc.field = ntsc_lmp88959.merge_fields ? field & 0x01 : 0;
	ntsc.raw = 0;
//	ntsc.hue = ntsc_lmp88959.hue;
	if (ntsc_lmp88959.merge_fields && (ntsc.field == 0)) {
		ntsc.frame ^= nes[nidx].p.ppu.frames & 0x01;
	}
	crt_modulate(&crt, &ntsc);
	crt_demodulate(&crt, ntsc_lmp88959.noise);
	if (ntsc_lmp88959.merge_fields) {
		field ^= 0x01;
	}
	lmp88959_phosphor_decay();
}

void ntsc_lmp88959_filter_parameters_changed(void) {
	ntsc_lmp88959_init();
}
void ntsc_lmp88959_filter_parameters_default(void) {
	ntsc_lmp88959 = ntsc_lmp88959_default;
}

void ntsc_lmp88959_filter_parameter_default(int index) {
	const _ntsc_lmp88959_setup_t *format = &ntsc_lmp88959_default;

	switch (index) {
		default:
		case 0:
			ntsc_lmp88959.brightness = format->brightness;
			break;
		case 1:
			ntsc_lmp88959.hue = format->hue;
			break;
		case 2:
			ntsc_lmp88959.saturation = format->saturation;
			break;
		case 3:
			ntsc_lmp88959.contrast = format->contrast;
			break;
		case 4:
			ntsc_lmp88959.black_point = format->black_point;
			break;
		case 5:
			ntsc_lmp88959.white_point = format->white_point;
			break;
		case 6:
			ntsc_lmp88959.noise = format->noise;
			break;
	}
}
void ntsc_lmp88959_filter_parameter_smv_default(void) {
	const _ntsc_lmp88959_setup_t *format = &ntsc_lmp88959_default;

	ntsc_lmp88959.scanline = format->scanline;
	ntsc_lmp88959.merge_fields = format->merge_fields;
	ntsc_lmp88959.vertical_blend = format->vertical_blend;
}

#include "extra/NTSC-CRT/crt_core.c"
#include "extra/NTSC-CRT/crt_ntsc.c"

// -------------------------------------------------------------------------------------------------------------------

#include "thread_def.h"

#define LMP88959_POSPHOR_NUM_SLICE 4

static thread_funct(lmp88959_thread_phosphor_decay, void *arg);

void lmp88959_phosphor_decay(void) {
	_lmp88959_thread_phospohor param[LMP88959_POSPHOR_NUM_SLICE];
	thread_t thread[LMP88959_POSPHOR_NUM_SLICE];
	int i = 0;

	// creo i threads
	for (i = 0; i < LMP88959_POSPHOR_NUM_SLICE; i++) {
		param[i].width = gfx.filter.data.width;
		param[i].height = gfx.filter.data.height / LMP88959_POSPHOR_NUM_SLICE;
		param[i].pix =  gfx.filter.data.pix + (param[i].width * (i * param[i].height) * 4);
		thread_create(thread[i], lmp88959_thread_phosphor_decay, &param[i]);
	}
	// attendo che i threads concludano
	for (i = 0; i < LMP88959_POSPHOR_NUM_SLICE; i++) {
		thread_join(thread[i]);
		thread_free(thread[i]);
	}
}

static thread_funct(lmp88959_thread_phosphor_decay, void *arg) {
	_lmp88959_thread_phospohor *p = (_lmp88959_thread_phospohor *)arg;

	for (int i = 0; i < p->width * p->height; i++) {
		uint32_t c = p->pix[i] & 0xffffff;

		p->pix[i] =
			(c >> 1 & 0x7f7f7f) +
			(c >> 2 & 0x3f3f3f) +
			(c >> 3 & 0x1f1f1f) +
			(c >> 4 & 0x0f0f0f);
	}
	thread_funct_return();
}

