/*
 *  Copyright (C) 2010-2024 Fabio Cavallo (aka FHorse)
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

#ifndef XBRZ_H_
#define XBRZ_H_

#include "common.h"

void xBRZ(BYTE nidx);
void xBRZ_mt(BYTE nidx);

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC void xbrz_scale(BYTE factor, const WORD *src, uint32_t *trg, uint32_t *palette, int width, int height);
EXTERNC void xbrz_scale_mt(BYTE factor, const WORD *src, uint32_t *trg, uint32_t *palette, int width, int height);

#undef EXTERNC

#endif /* XBRZ_H_ */
