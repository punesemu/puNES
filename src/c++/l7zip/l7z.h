/*
 *  Copyright (C) 2010-2019 Fabio Cavallo (aka FHorse)
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

#ifndef L7Z_H_
#define L7Z_H_

#include "common.h"
#include "uncompress.h"

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC BYTE l7z_init(void);
EXTERNC void l7z_quit(void);
EXTERNC BYTE l7z_present(void);
EXTERNC BYTE l7z_control_ext(const uTCHAR *ext);
EXTERNC BYTE l7z_examine_archive(_uncompress_archive *archive);
EXTERNC BYTE l7z_extract_from_archive(_uncompress_archive *archive, uint32_t selected, BYTE type);
EXTERNC uTCHAR *l7z_item_file_name(_uncompress_archive *archive, uint32_t selected, BYTE type);

#undef EXTERNC

#endif /* L7Z_H_ */
