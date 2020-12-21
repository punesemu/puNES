/*
 *  Copyright (C) 2010-2021 Fabio Cavallo (aka FHorse)
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

#ifndef PPU_H_
#define PPU_H_

#include "common.h"

enum ppu_sprite_byte { YC, TL, AT, XC };
enum ppu_color_mode { PPU_CM_GRAYSCALE = 0x30, PPU_CM_NORMAL = 0x3F };
enum ppu_scanline_cycles { SHORT_SLINE_CYCLES = 340, SLINE_CYCLES };

#define screen_size() (SCR_LINES * SCR_ROWS) * sizeof(WORD)
#define _ppu_spr_adr(sprite, epl, spl, sadr)\
{\
	BYTE flip_v;\
	/*\
	 * significato bit 7 del byte degli attributi:\
	 *  0 -> no flip verticale\
	 *  1 -> si flip verticale\
	 */\
	if (oam.epl[sprite][AT] & 0x80) {\
		/* flip verticale */\
		flip_v = ~spl[sprite].flip_v;\
	} else {\
		/* no flip verticale */\
		flip_v = spl[sprite].flip_v;\
	}\
	/*\
	 * significato bit 5 del $2000:\
	 *  0 -> sprite 8x8 (1 tile)\
	 *  1 -> sprite 8x16 (2 tile)\
	 */\
	if (r2000.size_spr == 16) {\
		/* -- 8x16 --\
		 *\
		 * sprite_plus[x].tile:\
		 * 76543210\
		 * ||||||||\
		 * |||||||+- Bank ($0000 or $1000) dei tiles\
		 * +++++++-- numero del tile (da 0 a 127)\
		 */\
		/*\
		 * estrapolo il numero del tile che sara' pari\
		 * per i primi 8x8 pixels (sprite_plus[x].flip_v\
		 * inferiore a 8), e dispari per restanti 8x8\
		 * (sprite_plus[x].flip_v superiore a 8),\
		 * in modo da formare uno sprite 8x16, mentre in\
		 * caso di flip verticale sara' l'esatto contrario,\
		 * dispari per i primi 8x8 e pari per i secondi 8x8.\
		 */\
		sadr = (oam.epl[sprite][TL] & 0xFE) | ((flip_v & 0x08) >> 3);\
		/* recupero la posizione nella vram del tile */\
		sadr = ((oam.epl[sprite][TL] & 0x01) << 12) | (sadr << 4);\
	} else {\
		/* -- 8x8 --\
		 *\
		 * sprite_plus[x].tile = numero del tile nella vram.\
		 */\
		/* recupero la posizione nella vram del tile */\
		sadr = r2000.spt_adr | (oam.epl[sprite][TL] << 4);\
	}\
	/* aggiungo la cordinata Y dello sprite */\
	sadr += (flip_v & 0x07);\
}
#define ppu_spr_adr(sprite) _ppu_spr_adr(sprite, ele_plus, sprite_plus, ppu.spr_adr)
#define ppu_bck_adr(r2000bck, r2006vl)\
	ppu.bck_adr = r2000bck | ((ppu_rd_mem(0x2000 | (r2006vl & 0x0FFF)) << 4)\
		| ((r2006vl & 0x7000) >> 12))
#define r2006_inc()\
	/* controllo se fine Y e' uguale a 7 */\
	if ((r2006.value & 0x7000) == 0x7000) {\
		WORD tile_y;\
		/* azzero il fine Y */\
		r2006.value &= 0x0FFF;\
		/* isolo il tile Y */\
		tile_y = (r2006.value & 0x03E0);\
		/* quindi lo esamino */\
		if (tile_y == 0x03A0) {\
			/* nel caso di 29 */\
			r2006.value ^= 0x0BA0;\
		} else if (tile_y == 0x03E0) {\
			/* nel caso di 31 */\
			r2006.value ^= 0x03E0;\
		} else {\
			/* incremento tile Y */\
			r2006.value += 0x20;\
		}\
	} else {\
		/* incremento di 1 fine Y */\
		r2006.value += 0x1000;\
	}
#define r2006_end_scanline() r2006.value = (r2006.value & 0xFBE0) | (ppu.tmp_vram & 0x041F)
#define ppu_overclock_update()\
	if (overclock.DMC_in_use) {\
		ppu_sclines.total = machine.total_lines;\
		ppu_sclines.frame = machine.total_lines;\
		ppu_sclines.vint = machine.vint_lines;\
		ppu_sclines.vint_extra = 0;\
	} else {\
		ppu_sclines.total = machine.total_lines + overclock.sclines.total;\
		ppu_sclines.frame = machine.total_lines + overclock.sclines.vb;\
		ppu_sclines.vint = machine.vint_lines + overclock.sclines.vb;\
		ppu_sclines.vint_extra = overclock.sclines.vb;\
	}
#define ppu_overclock_control()\
	overclock.in_extra_sclines = TRUE;\
	if ((ppu.frame_y >= ppu_sclines.vint_extra) && (ppu.frame_y < ppu_sclines.frame)) {\
		overclock.in_extra_sclines = FALSE;\
	}

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
		/* questo byte non serve piu' */
		BYTE first_of_tick;
	} sf;
	WORD rnd_adr;
}  _ppu;
typedef struct _screen_buffer {
	BYTE ready;
	uint64_t frame;
	WORD *data;
	WORD *line[SCR_LINES];
} _screen_buffer;
typedef struct _screen {
	BYTE index;
	_screen_buffer *wr;
	_screen_buffer *rd;
	_screen_buffer *last_completed_wr;
	_screen_buffer buff[2];
} _screen;
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

extern _ppu ppu;
extern _screen screen;
extern _ppu_openbus ppu_openbus;
extern _r2000 r2000;
extern _r2001 r2001;
extern _r2002 r2002;
extern _r2006 r2006;
extern _r2xxx r2003, r2004, r2007;
extern _spr_evaluate spr_ev;
extern _spr sprite[8], sprite_plus[8];
extern _spr_evaluate spr_ev_unl;
extern _spr sprite_unl[56], sprite_plus_unl[56];
extern _tile tile_render, tile_fetch;
extern _ppu_sclines ppu_sclines;
extern _overclock overclock;

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC void ppu_init(void);
EXTERNC void ppu_quit(void);

EXTERNC void ppu_tick(void);
EXTERNC BYTE ppu_turn_on(void);
EXTERNC void ppu_overclock(BYTE reset_dmc_in_use);

EXTERNC void ppu_draw_screen_pause(void);
EXTERNC void ppu_draw_screen_continue(void);

EXTERNC void ppu_draw_screen_pause_with_count(int *count);
EXTERNC void ppu_draw_screen_continue_with_count(int *count);
EXTERNC void ppu_draw_screen_continue_ctrl_count(int *count);

#undef EXTERNC

#endif /* PPU_H_ */
