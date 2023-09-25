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

#ifndef SRC_VIDEO_FILTERS_2XSAI_H_
#define SRC_VIDEO_FILTERS_2XSAI_H_

#include "common.h"

void scale_2xsai(BYTE nidx);
void scale_dotmatrix(BYTE nidx);
void scale_paltv1x(BYTE nidx);
void scale_paltv2x(BYTE nidx);
void scale_paltv3x(BYTE nidx);
void scale_paltv4x(BYTE nidx);
void scale_super2xsai(BYTE nidx);
void scale_supereagle(BYTE nidx);
void scale_tv2x(BYTE nidx);
void scale_tv3x(BYTE nidx);
void scale_tv4x(BYTE nidx);

#endif /* SRC_VIDEO_FILTERS_2XSAI_H_ */
