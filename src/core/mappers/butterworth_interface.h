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

#ifndef BUTTERWORTH_INTERFACE_H_
#define BUTTERWORTH_INTERFACE_H_

#include <stdio.h>
#include "common.h"

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

typedef void *bfilter;

EXTERNC bfilter butterworth_create(int _n, double s, double f);
EXTERNC void butterworth_free(bfilter b);
EXTERNC double butterworth_output(bfilter b, double x);

#undef EXTERNC

#endif /* BUTTERWORTH_INTERFACE_H_ */
