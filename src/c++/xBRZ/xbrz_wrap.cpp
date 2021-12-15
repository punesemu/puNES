/*
 *  Copyright (C) 2010-2021 Fabio Cavallo (aka FHorse)
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

#include "c++/xBRZ/xbrz_wrap.h"
#include "c++/xBRZ/xbrz.h"
#include "thread_def.h"

#if defined (WITH_D3D9)
#define color_format xbrz::ColorFormat::ARGB
#elif defined (WITH_OPENGL)
#define color_format xbrz::ColorFormat::RGB
#endif

extern "C" void xbrz_scale(BYTE factor, const WORD *src, uint32_t *trg, uint32_t *palette, int width, int height) {
	xbrz::scale(factor, src, trg, palette, width, height, color_format);
}
extern "C" void xbrz_scale_mt(BYTE factor, const WORD *src, uint32_t *trg, uint32_t *palette, int width, int height) {
	_xbrz_wrap param[XBRZ_NUM_SLICE];
	thread_t thread[XBRZ_NUM_SLICE];
	int i;

	// creo i threads
	for (i = 0; i < XBRZ_NUM_SLICE; i++) {
		param[i].slice = i;
		param[i].factor = factor;
		param[i].src = src;
		param[i].trg = trg;
		param[i].palette = palette;
		param[i].srcWidth = width;
		param[i].srcHeight = height;
		param[i].colFmt = (int)color_format;
		thread_create(thread[i], xbrz::scale_mt, &param[i]);
	}

	// attendo che i threads concludano
	for (i = 0; i < XBRZ_NUM_SLICE; i++) {
		thread_join(thread[i]);
		thread_free(thread[i]);
	}
}
