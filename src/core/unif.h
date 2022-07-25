/*
 *  Copyright (C) 2010-2022 Fabio Cavallo (aka FHorse)
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

#ifndef UNIF_H_
#define UNIF_H_

#include "common.h"

enum unif_mapper { UNIF_MAPPER = 0x1002 };
enum unif_no_types { NO_INES = 65535, NO_UNIF = 65535 };

typedef struct _unif {
	BYTE finded;
	WORD internal_mapper;
	char board[64];
	char *stripped_board;
	char name[256];
	BYTE mirroring;

	struct _unif_dumped {
		char by[100];
		BYTE day;
		BYTE month;
		WORD year;
		char with[100];
	} dumped;
	struct _unif_header {
		char identification[4];
		uint32_t revision;
		BYTE expansion[24];
	} header;
	struct _unif_chunk {
		char id[4];
		uint32_t length;
	} chunk;
	struct _unif_chips {
		WORD prg;
		WORD chr;
	} chips;
} _unif;

extern _unif unif;

BYTE unif_load_rom(void);

#endif /* UNIF_H_ */
