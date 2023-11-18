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

#include "bck_states.h"
#include "mappers.h"
#include "info.h"
#include "nes.h"
#include "apu.h"
#include "fds.h"
#include "gui.h"
#include "tas.h"

#define bck_states_on_struct(_mode, _strct, _data, _index, _size_buf)\
	switch (_mode) {\
		case BCK_STATES_OP_SAVE_ON_MEM:\
			memcpy(((_data) + (_index)), &(_strct), sizeof(_strct));\
			(_index) += sizeof(_strct);\
			break;\
		case BCK_STATES_OP_READ_FROM_MEM:\
			memcpy(&(_strct), ((_data) + (_index)), sizeof(_strct));\
			(_index) += sizeof(_strct);\
			break;\
		case BCK_STATES_OP_COUNT:\
			(_size_buf) += sizeof(_strct);\
			break;\
    	default:\
    		break;\
	}
#define bck_states_on_mem(_mode, _mem, _size, _data, _index, _size_buf)\
	switch (_mode) {\
		case BCK_STATES_OP_SAVE_ON_MEM:\
			memcpy((_data) + (_index), _mem, _size);\
			(_index) += (_size);\
			break;\
		case BCK_STATES_OP_READ_FROM_MEM:\
			memcpy(_mem, ((_data) + (_index)), _size);\
			(_index) += (_size);\
			break;\
		case BCK_STATES_OP_COUNT:\
			(_size_buf) += (_size);\
			break;\
    	default:\
    		break;\
	}

void bck_states_op_screen(BYTE mode, void *data, size_t *index, uint64_t *size_buff) {
	for (int nesidx = 0; nesidx < info.number_of_nes; nesidx++) {
		bck_states_on_mem(mode, nes[nesidx].p.ppu_screen.rd->data, (screen_size()), data, (*index), (*size_buff))
	}
}
void bck_states_op_keyframe(BYTE mode, void *data, size_t *index, uint64_t *size_buff) {
	for (int nesidx = 0; nesidx < info.number_of_nes; nesidx++) {
		// CPU
		bck_states_on_struct(mode, nes[nesidx].c.cpu, data, (*index), (*size_buff))
		bck_states_on_struct(mode, nes[nesidx].c.irq, data, (*index), (*size_buff))
		bck_states_on_struct(mode, nes[nesidx].c.nmi, data, (*index), (*size_buff))

		// PPU
		bck_states_on_struct(mode, nes[nesidx].p.ppu, data, (*index), (*size_buff))
		bck_states_on_struct(mode, nes[nesidx].p.ppu_openbus, data, (*index), (*size_buff))
		bck_states_on_struct(mode, nes[nesidx].p.r2000, data, (*index), (*size_buff))
		bck_states_on_struct(mode, nes[nesidx].p.r2001, data, (*index), (*size_buff))
		bck_states_on_struct(mode, nes[nesidx].p.r2002, data, (*index), (*size_buff))
		bck_states_on_struct(mode, nes[nesidx].p.r2003, data, (*index), (*size_buff))
		bck_states_on_struct(mode, nes[nesidx].p.r2004, data, (*index), (*size_buff))
		bck_states_on_struct(mode, nes[nesidx].p.r2006, data, (*index), (*size_buff))
		bck_states_on_struct(mode, nes[nesidx].p.r2007, data, (*index), (*size_buff))
		bck_states_on_struct(mode, nes[nesidx].p.spr_ev, data, (*index), (*size_buff))
		bck_states_on_struct(mode, nes[nesidx].p.sprite, data, (*index), (*size_buff))
		bck_states_on_struct(mode, nes[nesidx].p.sprite_plus, data, (*index), (*size_buff))
		bck_states_on_struct(mode, nes[nesidx].p.tile_render, data, (*index), (*size_buff))
		bck_states_on_struct(mode, nes[nesidx].p.tile_fetch, data, (*index), (*size_buff))
	}
	// APU
	bck_states_on_struct(mode, apu, data, (*index), (*size_buff))
	bck_states_on_struct(mode, r4011, data, (*index), (*size_buff))
	bck_states_on_struct(mode, r4015, data, (*index), (*size_buff))
	bck_states_on_struct(mode, r4017, data, (*index), (*size_buff))
	bck_states_on_struct(mode, S1, data, (*index), (*size_buff))
	bck_states_on_struct(mode, S2, data, (*index), (*size_buff))
	bck_states_on_struct(mode, TR, data, (*index), (*size_buff))
	bck_states_on_struct(mode, NS, data, (*index), (*size_buff))
	bck_states_on_struct(mode, DMC, data, (*index), (*size_buff))

	// mem map
	for (int nesidx = 0; nesidx < info.number_of_nes; nesidx++) {
		if (ram_size(nesidx)) {
			bck_states_on_mem(mode, ram_pnt(nesidx), ram_size(nesidx), data, (*index), (*size_buff))
		}
	}
	if (wram_size()) {
		bck_states_on_mem(mode, wram_pnt(), wram_size(), data, (*index), (*size_buff))
	}
	for (int nesidx = 0; nesidx < info.number_of_nes; nesidx++) {
		if (vram_size(nesidx)) {
			bck_states_on_mem(mode, vram_pnt(nesidx), vram_size(nesidx), data, (*index), (*size_buff))
		}
		if (nmt_size(nesidx)) {
			bck_states_on_mem(mode, nmt_pnt(nesidx), nmt_size(nesidx), data, (*index), (*size_buff))
		}
		bck_states_on_struct(mode, nes[nesidx].p.oam, data, (*index), (*size_buff))
		bck_states_on_struct(mode, nes[nesidx].m.memmap_palette, data, (*index), (*size_buff))
	}

	// mapper
	bck_states_on_struct(mode, mapper, data, (*index), (*size_buff))
	for (unsigned int i = 0; i < LENGTH(mapper.internal_struct); i++) {
		if (mapper.internal_struct[i]) {
			bck_states_on_mem(mode, mapper.internal_struct[i], mapper.internal_struct_size[i], data, (*index), (*size_buff))
		}
	}

	for (int nesidx = 0; nesidx < info.number_of_nes; nesidx++) {
		// irqA12
		if (nes[nesidx].irqA12.present) {
			bck_states_on_struct(mode, nes[nesidx].irqA12, data, (*index), (*size_buff))
		}

		// irql2f
		if (nes[nesidx].irql2f.present) {
			bck_states_on_struct(mode, nes[nesidx].irql2f, data, (*index), (*size_buff))
		}
	}

	// dipswitch
	bck_states_on_struct(mode, dipswitch, data, (*index), (*size_buff))

	// FDS
	if (fds.info.enabled) {
		BYTE old_side_inserted = fds.drive.side_inserted;

		bck_states_on_struct(mode, fds.drive, data, (*index), (*size_buff))
		bck_states_on_struct(mode, fds.snd, data, (*index), (*size_buff))
		bck_states_on_struct(mode, fds.info.last_operation, data, (*index), (*size_buff))
		bck_states_on_struct(mode, fds.auto_insert, data, (*index), (*size_buff))

		// in caso di ripristino di una snapshot, se era caricato
		// un altro side del disco, devo ricaricarlo.
		if ((mode == BCK_STATES_OP_READ_FROM_MEM) && (old_side_inserted != fds.drive.side_inserted)) {
			fds_disk_op(FDS_DISK_SELECT_FROM_REWIND, fds.drive.side_inserted, FALSE);
			gui_update();
		}
	}

	if (mode == BCK_STATES_OP_READ_FROM_MEM) {
		prgrom_reset_chunks();
		chrrom_reset_chunks();
		ram_reset_chunks();
		wram_reset_chunks();
		nmt_reset_chunks();
		// ripristino i puntatori
		extcl_after_mapper_init();
	}
}
void bck_states_op_input(BYTE mode, void *data, size_t *index, uint64_t *size_buff) {
	int i = 0;

	// standard controller
	for (i = PORT1; i < PORT_MAX; i++) {
		bck_states_op_input_port(i, mode, data, index, size_buff);
	}

	// zapper, mouse, arkanoid, oeka_kids_tablet
	bck_states_on_struct(mode, gmouse.x, data, (*index), (*size_buff))
	bck_states_on_struct(mode, gmouse.y, data, (*index), (*size_buff))
	bck_states_on_struct(mode, gmouse.left, data, (*index), (*size_buff))
	bck_states_on_struct(mode, gmouse.right, data, (*index), (*size_buff))

	// generic keyboard
	bck_states_on_struct(mode, generic_keyboard.row, data, (*index), (*size_buff))
	bck_states_on_struct(mode, generic_keyboard.column, data, (*index), (*size_buff))
	bck_states_on_struct(mode, generic_keyboard.enable, data, (*index), (*size_buff))
	bck_states_on_struct(mode, generic_keyboard.state, data, (*index), (*size_buff))
	bck_states_on_struct(mode, generic_keyboard.data, data, (*index), (*size_buff))

	// lag frames
	bck_states_on_struct(mode, info.lag_frame.totals, data, (*index), (*size_buff))
	bck_states_on_struct(mode, info.lag_frame.consecutive, data, (*index), (*size_buff))
}
void bck_states_op_input_port(BYTE id, BYTE mode, void *data, size_t *index, uint64_t *size_buff) {
	bck_states_on_struct(mode, port[id].type_pad, data, (*index), (*size_buff))
	for (int nesidx = 0; nesidx < info.number_of_nes; nesidx++) {
		bck_states_on_struct(mode, nes[nesidx].c.input.pindex, data, (*index), (*size_buff))
		bck_states_on_struct(mode, nes[nesidx].c.input.fsindex, data, (*index), (*size_buff))
	}
	bck_states_on_struct(mode, port[id].data, data, (*index), (*size_buff))
	bck_states_on_struct(mode, port[id].turbo, data, (*index), (*size_buff))
}
