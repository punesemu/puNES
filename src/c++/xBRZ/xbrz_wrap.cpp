/*
 *  Copyright (C) 2010-2017 Fabio Cavallo (aka FHorse)
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
#if defined (__linux__)
#include <pthread.h>
#endif

extern "C" void xbrz_scale(BYTE factor, const WORD *src, uint32_t *trg, uint32_t *palette,
		int width, int height) {
#if defined (WITH_D3D9)
	xbrz::scale(factor, src, trg, palette, width, height, xbrz::ColorFormat::ARGB);
#elif defined (WITH_OPENGL)
	xbrz::scale(factor, src, trg, palette, width, height, xbrz::ColorFormat::RGB);
#endif
}

extern "C" void xbrz_scale_mt(BYTE factor, const WORD *src, uint32_t *trg, uint32_t *palette,
		int width, int height) {
#if defined (__WIN32__)
	HANDLE thread[XBRZ_NUM_SLICE];
	DWORD id[XBRZ_NUM_SLICE];
#elif defined (__linux__)
	pthread_t thread[XBRZ_NUM_SLICE];
#endif
	_xbrz_wrap param[XBRZ_NUM_SLICE];
	int i;

	for (i = 0; i < XBRZ_NUM_SLICE; i++) {
		param[i].slice = i;
		param[i].factor = factor;
		param[i].src = src;
		param[i].trg = trg;
		param[i].palette = palette;
		param[i].srcWidth = width;
		param[i].srcHeight = height;
	#if defined (WITH_D3D9)
		param[i].colFmt = (int) xbrz::ColorFormat::ARGB;
	#elif defined (WITH_OPENGL)
		param[i].colFmt = (int) xbrz::ColorFormat::RGB;
	#endif
#if defined (__WIN32__)
		thread[i] = CreateThread(NULL, 0, xbrz::scale_mt, &param[i], 0, &id[i]);
#elif defined (__linux__)
		pthread_create(&thread[i], NULL, xbrz::scale_mt, &param[i]);
#endif
	}

#if defined (__WIN32__)
	WaitForMultipleObjects(XBRZ_NUM_SLICE, thread, TRUE, INFINITE);
	for (i = 0; i < XBRZ_NUM_SLICE; i++) {
		CloseHandle(thread[i]);
	}
#elif defined (__linux__)
	for (i = 0; i < XBRZ_NUM_SLICE; i++) {
		pthread_join(thread[i], NULL);
	}
#endif
}
