/*
 * Copyright (C) 2003 Maxim Stepin ( maxst@hiend3d.com )
 *
 * Copyright (C) 2010 Cameron Zemek ( grom@zeminvaders.net)
 * Copyright (C) 2011 Francois Gannaz <mytskine@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef __HQX_H_
#define __HQX_H_

#include "common.h"
#include "palette.h"

struct _hqx {
	WORD sx;
	WORD sy;
	WORD startx;
	WORD rows;
	WORD lines;
	WORD dst_rows;
} hqnx;

void hqx_init(void);
void hqNx(WORD *screen, WORD **screen_index, uint32_t *palette, BYTE bpp, uint32_t pitch,
        void *pix, WORD rows, WORD lines, WORD width, WORD height, BYTE factor);
void hq2x_32_rb(WORD *screen, void *pix, uint32_t *palette);
void hq3x_32_rb(WORD *screen, void *pix, uint32_t *palette);
void hq4x_32_rb(WORD *screen, void *pix, uint32_t *palette);

#endif
