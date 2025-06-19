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

#ifndef DETACH_BARCODE_H_
#define DETACH_BARCODE_H_

#include "common.h"
#include "save_slot.h"

typedef struct _detach_barcode {
	BYTE enabled;
	BYTE data[256];
	DBWORD pos;
	DBWORD count;
	BYTE out;
} _detach_barcode;

extern _detach_barcode detach_barcode;

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC void init_detach_barcode(BYTE reset);
EXTERNC BYTE detach_barcode_save_mapper(BYTE mode, BYTE slot, FILE *fp);

EXTERNC int detach_barcode_bcode(const uTCHAR *rcode);

#undef EXTERNC

#endif /* DETACH_BARCODE_H_ */
