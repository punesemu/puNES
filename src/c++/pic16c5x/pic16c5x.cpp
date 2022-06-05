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

#include "pic16c5x.h"
#include "pic16c5x_m.h"

PIC16C54 *pic = NULL;

void pic16c5x_init(BYTE *rom, pic16c5x_rd_funct rd, pic16c5x_wr_funct wr) {
	if (pic) {
		pic16c5x_quit();
	}
	pic = new PIC16C54(rom, rd, wr);
}
void pic16c5x_quit(void) {
	if (pic) {
		delete pic;
		pic = NULL;
	}
}
void pic16c5x_reset(BYTE type) {
	if (pic) {
		pic->reset(type);
	}
}
void pic16c5x_run(void) {
	if (pic) {
		pic->run();
	}
}
BYTE pic16c5x_save_mapper(BYTE mode, BYTE slot, FILE *fp) {
	if (pic) {
		return (pic->save_mapper(mode, slot, fp));
	}
	return  (EXIT_OK);
}
