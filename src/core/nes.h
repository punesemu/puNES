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

#ifndef NES_H_
#define NES_H_

#include "common.h"

// cpu -------------------------------------------------------------------------------

typedef struct _cpu {
	// Processor Registers
	union _cpu_pc {
		BYTE b[2];
		WORD w;
	} PC;
	BYTE SP; // Stack Pointer
	BYTE AR; // Accumulator
	BYTE XR; // Index Register X
	BYTE YR; // Index Register Y
	/* Processor Status Register */
	BYTE SR; // Status Register
	BYTE cf; // C (bit 0) - Carry flag
	BYTE zf; // Z (bit 1) - Zero flag
	BYTE im; // I (bit 2) - Interrupt mask
	BYTE df; // D (bit 3) - Decimal flag
	BYTE bf; // B (bit 4) - Break flag
	/*            (bit 5) - Always 1 */
	BYTE of; // O (bit 6) - Overflow flag
	BYTE sf; // S (bit 7) - Sign flag or N - Negative flag
	// il codice che identifica l'istruzione
	WORD opcode;
	WORD opcode_PC;
	// il flag che indica se il ciclo della cpu e' dispari
	BYTE odd_cycle;
	// cicli cpu dell'istruzione e delle operazioni di lettura e scrittura
	SWORD cycles;
	// DMC
	WORD opcode_cycle;
	// doppia lettura
	BYTE double_rd;
	// doppia scrittura
	BYTE double_wr;
	// lettura PRG Ram attiva/disattiva
	BYTE prg_ram_rd_active;
	// scrittura PRG Ram attiva/disattiva
	BYTE prg_ram_wr_active;
	// i cicli (senza aggiustamenti) impiegati dall'opcode
	WORD base_opcode_cycles;
	// buffer di lettura
	BYTE openbus;
} _cpu;
typedef struct _irq {
	BYTE high;
	BYTE delay;
	BYTE before;
	BYTE inhibit;
} _irq;
typedef struct _nmi {
	BYTE high;
	BYTE delay;
	BYTE before;
	BYTE inhibit;
	WORD frame_x;
	// i cicli passati dall'inizio dell'NMI
	uint32_t cpu_cycles_from_last_nmi;
} _nmi;
typedef struct _cpu_data {
	_cpu cpu;
	_irq irq;
	_nmi nmi;
} _cpu_data;

// ppu -------------------------------------------------------------------------------

typedef struct _ppu {
	WORD frame_x;
	WORD frame_y;
	BYTE fine_x;
	BYTE screen_y;
	BYTE vblank;
	WORD pixel_tile;
	WORD sline_cycles;
	WORD tmp_vram;
	WORD spr_adr;
	WORD bck_adr;
	BYTE openbus;
	BYTE odd_frame;
	SWORD cycles;
	uint32_t frames;
	struct _short_frame {
		BYTE actual;
		BYTE prev;
	} sf;
	WORD rnd_adr;
}  _ppu;
typedef struct _ppu_screen_buffer {
	BYTE ready;
	uint64_t frame;
	WORD *data;
	WORD *line[SCR_ROWS];
} _ppu_screen_buffer;
typedef struct _screen {
	BYTE index;
	_ppu_screen_buffer *wr;
	_ppu_screen_buffer *rd;
	_ppu_screen_buffer *last_completed_wr;
	_ppu_screen_buffer buff[2];
} _ppu_screen;
typedef struct _ppu_openbus {
	int32_t bit0;
	int32_t bit1;
	int32_t bit2;
	int32_t bit3;
	int32_t bit4;
	int32_t bit5;
	int32_t bit6;
	int32_t bit7;
} _ppu_openbus;
typedef struct _r2xxx {
	BYTE value;
} _r2xxx;
typedef struct _r2000 {
	BYTE value;
	BYTE nmi_enable;
	BYTE size_spr;
	BYTE r2006_inc;
	WORD spt_adr;
	WORD bpt_adr;
	struct _r2000_race {
		WORD ctrl;
		WORD value;
	} race;
} _r2000;
typedef struct _r2001 {
	BYTE value;
	WORD emphasis;
	BYTE visible;
	BYTE bck_visible;
	BYTE spr_visible;
	BYTE bck_clipping;
	BYTE spr_clipping;
	BYTE color_mode;
	struct _r2001_race {
		WORD ctrl;
		WORD value;
	} race;
	struct _r2001_grayscale_bit {
		BYTE delay;
	} grayscale_bit;
} _r2001;
typedef struct _r2002 {
	BYTE vblank;
	BYTE sprite0_hit;
	BYTE sprite_overflow;
	BYTE toggle;
	struct _r2002_race {
		WORD sprite_overflow;
	} race;
} _r2002;
typedef struct _r2006 {
	WORD value;
	// ormai non più utilizzato
	WORD changed_from_op;
	struct _r2006_second_write {
		BYTE delay;
		WORD value;
	} second_write;
	struct _r2006_race {
		WORD ctrl;
		WORD value;
	} race;
} _r2006;
typedef struct _spr_evaluate {
	WORD range;
	BYTE count;
	BYTE count_plus;
	BYTE tmp_spr_plus;
	BYTE evaluate;
	BYTE byte_OAM;
	BYTE index_plus;
	BYTE index;
	BYTE timing;
	BYTE phase;
	BYTE real;
} _spr_evaluate;
typedef struct _spr {
	BYTE y_C;
	BYTE tile;
	BYTE attrib;
	BYTE x_C;
	BYTE number;
	BYTE flip_v;
	BYTE l_byte;
	WORD h_byte;
} _spr;
/*
 * le variabili sono di tipo WORD e DBWORD perche',
 * per gestire correttamente lo scrolling, saranno
 * sempre trattati due tiles alla volta, altrimenti
 * sarebbero stati sufficienti BYTE e WORD.
 */
typedef struct _tile {
	WORD attrib;
	WORD l_byte;
	DBWORD h_byte;
} _tile;
typedef struct _oam {
	BYTE data[256];
	BYTE *element[64];
	BYTE plus[32];
	BYTE *ele_plus[8];
	// unlimited sprites
	BYTE plus_unl[224];
	BYTE *ele_plus_unl[56];
} _oam;
typedef struct _ppu_sclines {
	WORD total;
	WORD frame;
	WORD vint;
	WORD vint_extra;
} _ppu_sclines;
typedef struct _overclock {
	BYTE in_extra_sclines;
	BYTE DMC_in_use;
	struct _extra_sclines {
		WORD vb;
		WORD pr;
		WORD total;
	} sclines;
} _overclock;
typedef struct _ppu_data {
	_ppu ppu;
	_ppu_screen ppu_screen;
	_ppu_openbus ppu_openbus;
	_r2000 r2000;
	_r2001 r2001;
	_r2002 r2002;
	_r2006 r2006;
	_r2xxx r2003, r2004, r2007;
	_spr_evaluate spr_ev;
	_spr sprite[8], sprite_plus[8];
	_spr_evaluate spr_ev_unl;
	_spr sprite_unl[56], sprite_plus_unl[56];
	_tile tile_render, tile_fetch;
	_oam oam;
	_ppu_sclines ppu_sclines;
	_overclock overclock;
} _ppu_data;

// nes -------------------------------------------------------------------------------

typedef struct _nes {
	_cpu_data c;
	_ppu_data p;
} _nes;

//extern _nes __attribute__((aligned(64))) nes;
extern _nes nes;

#endif /* NES_H_ */
