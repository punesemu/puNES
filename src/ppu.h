/*
 * ppu.h
 *
 *  Created on: 28/mar/2010
 *      Author: fhorse
 */

#ifndef PPU_H_
#define PPU_H_

#include "common.h"

enum { YC, TL, AT, XC };

#define screen_size() (SCR_LINES * SCR_ROWS) * sizeof(WORD)
#define ppu_spr_adr(sprite)\
{\
	BYTE flip_v;\
	/*\
	 * significato bit 7 del byte degli attributi:\
	 *  0 -> no flip verticale\
	 *  1 -> si flip verticale\
	 */\
	if (oam.ele_plus[sprite][AT] & 0x80) {\
		/* flip verticale */\
		flip_v = ~sprite_plus[sprite].flip_v;\
	} else {\
		/* no flip verticale */\
		flip_v = sprite_plus[sprite].flip_v;\
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
		ppu.spr_adr = (oam.ele_plus[sprite][TL] & 0xFE) | ((flip_v & 0x08) >> 3);\
		/* recupero la posizione nella vram del tile */\
		ppu.spr_adr = ((oam.ele_plus[sprite][TL] & 0x01) << 12) | (ppu.spr_adr << 4);\
	} else {\
		/* -- 8x8 --\
		 *\
		 * sprite_plus[x].tile = numero del tile nella vram.\
		 */\
		/* recupero la posizione nella vram del tile */\
		ppu.spr_adr = r2000.spt_adr | (oam.ele_plus[sprite][TL] << 4);\
	}\
	/* aggiungo la cordinata Y dello sprite */\
	ppu.spr_adr += (flip_v & 0x07);\
}
#define ppu_bck_adr()\
	ppu.bck_adr = r2000.bpt_adr |\
		((ppu_rd_mem(0x2000 | (r2006.value & 0x0FFF)) << 4)\
		| ((r2006.value & 0x7000) >> 12))
#define r2006_inc()\
{\
	WORD tile_y;\
	/* controllo se fine Y e' uguale a 7 */\
	if ((r2006.value & 0x7000) == 0x7000) {\
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
	}\
}
#define r2006_end_scanline()\
	r2006.value = (r2006.value & 0xFBE0) | (ppu.tmp_vram & 0x041F)

typedef struct {
	WORD frame_x;
	WORD frame_y;
	BYTE fine_x;
	BYTE screen_y;
	WORD pixel_tile;
	WORD sline_cycles;
	WORD tmp_vram;
	WORD spr_adr;
	WORD bck_adr;
	BYTE openbus;
	BYTE odd_frame;
	BYTE skip_draw;
	SWORD cycles;
	uint32_t frames;
}  _ppu;
typedef struct {
	WORD *data;
	WORD *line[SCR_LINES];
} _screen;
typedef struct {
	int32_t bit0;
	int32_t bit1;
	int32_t bit2;
	int32_t bit3;
	int32_t bit4;
	int32_t bit5;
	int32_t bit6;
	int32_t bit7;
} _ppu_openbus;
typedef struct {
	BYTE value;
} _r2xxx;
typedef struct {
	BYTE value;
	BYTE nmi_enable;
	BYTE size_spr;
	BYTE r2006_inc;
	WORD spt_adr;
	WORD bpt_adr;
} _r2000;
typedef struct {
	BYTE value;
	WORD emphasis;
	BYTE visible;
	BYTE bck_visible;
	BYTE spr_visible;
	BYTE bck_clipping;
	BYTE spr_clipping;
	BYTE color_mode;
} _r2001;
typedef struct {
	BYTE vblank;
	BYTE sprite0_hit;
	BYTE sprite_overflow;
	BYTE toggle;
} _r2002;
typedef struct {
	WORD value;
	WORD changed_from_op;
} _r2006;
typedef struct {
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
} _spr_evaluate;
typedef struct {
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
typedef struct {
	WORD attrib;
	WORD l_byte;
	DBWORD h_byte;
} _tile;

_ppu ppu;
_screen screen;
_ppu_openbus ppu_openbus;
_r2000 r2000;
_r2001 r2001;
_r2002 r2002;
_r2006 r2006;
_r2xxx r2003, r2004, r2007;
_spr_evaluate spr_ev;
_spr sprite[8], sprite_plus[8];
_tile tile_render, tile_fetch;

void ppu_tick(WORD cycles_cpu);
BYTE ppu_turn_on(void);
void ppu_quit(void);

#endif /* PPU_H_ */
