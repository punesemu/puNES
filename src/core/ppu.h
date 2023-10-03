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

#ifndef PPU_H_
#define PPU_H_

#include "common.h"
#include "nes.h"

enum ppu_sprite_byte { YC, TL, AT, XC };
enum ppu_color_mode { PPU_CM_GRAYSCALE = 0x30, PPU_CM_NORMAL = 0x3F };
enum ppu_scanline_cycles { SHORT_SLINE_CYCLES = 340, SLINE_CYCLES };
enum ppu_alignment { PPU_ALIGMENT_DEFAULT, PPU_ALIGMENT_RANDOMIZE, PPU_ALIGMENT_INC_AT_RESET };

#define screen_size() ((SCR_ROWS * SCR_COLUMNS) * sizeof(WORD))
#define _ppu_spr_adr(spr, epl, spl, sadr)\
{\
	BYTE flip_v;\
	/*\
	 * significato bit 7 del byte degli attributi:\
	 *  0 -> no flip verticale\
	 *  1 -> si flip verticale\
	 */\
	if (nes[nidx].p.oam.epl[spr][AT] & 0x80) {\
		/* flip verticale */\
		flip_v = ~(spl)[spr].flip_v;\
	} else {\
		/* no flip verticale */\
		flip_v = (spl)[spr].flip_v;\
	}\
	/*\
	 * significato bit 5 del $2000:\
	 *  0 -> sprite 8x8 (1 tile)\
	 *  1 -> sprite 8x16 (2 tile)\
	 */\
	if (nes[nidx].p.r2000.size_spr == 16) {\
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
		(sadr) = (nes[nidx].p.oam.epl[spr][TL] & 0xFE) | ((flip_v & 0x08) >> 3);\
		/* recupero la posizione nella vram del tile */\
		(sadr) = ((nes[nidx].p.oam.epl[spr][TL] & 0x01) << 12) | ((sadr) << 4);\
	} else {\
		/* -- 8x8 --\
		 *\
		 * sprite_plus[x].tile = numero del tile nella vram.\
		 */\
		/* recupero la posizione nella vram del tile */\
		(sadr) = nes[nidx].p.r2000.spt_adr | (nes[nidx].p.oam.epl[spr][TL] << 4);\
	}\
	/* aggiungo la cordinata Y dello sprite */\
	(sadr) += (flip_v & 0x07);\
}
#define ppu_spr_adr(spr) _ppu_spr_adr(spr, ele_plus, nes[nidx].p.sprite_plus, nes[nidx].p.ppu.spr_adr)
#define ppu_bck_adr(r2000bck, r2006vl)\
	nes[nidx].p.ppu.bck_adr = (r2000bck) | ((ppu_rd_mem(nidx, 0x2000 | ((r2006vl) & 0x0FFF)) << 4)\
		| (((r2006vl) & 0x7000) >> 12))
#define r2006_inc()\
	/* controllo se fine Y e' uguale a 7 */\
	if ((nes[nidx].p.r2006.value & 0x7000) == 0x7000) {\
		WORD tile_y;\
		/* azzero il fine Y */\
		nes[nidx].p.r2006.value &= 0x0FFF;\
		/* isolo il tile Y */\
		tile_y = (nes[nidx].p.r2006.value & 0x03E0);\
		/* quindi lo esamino */\
		if (tile_y == 0x03A0) {\
			/* nel caso di 29 */\
			nes[nidx].p.r2006.value ^= 0x0BA0;\
		} else if (tile_y == 0x03E0) {\
			/* nel caso di 31 */\
			nes[nidx].p.r2006.value ^= 0x03E0;\
		} else {\
			/* incremento tile Y */\
			nes[nidx].p.r2006.value += 0x20;\
		}\
	} else {\
		/* incremento di 1 fine Y */\
		nes[nidx].p.r2006.value += 0x1000;\
	}
#define r2006_end_scanline() nes[nidx].p.r2006.value = (nes[nidx].p.r2006.value & 0xFBE0) | (nes[nidx].p.ppu.tmp_vram & 0x041F)
#define ppu_overclock_update()\
	if (nes[nidx].p.overclock.DMC_in_use) {\
		nes[nidx].p.ppu_sclines.total = machine.total_lines;\
		nes[nidx].p.ppu_sclines.frame = machine.total_lines;\
		nes[nidx].p.ppu_sclines.vint = machine.vint_lines;\
		nes[nidx].p.ppu_sclines.vint_extra = 0;\
	} else {\
		nes[nidx].p.ppu_sclines.total = machine.total_lines + nes[nidx].p.overclock.sclines.total;\
		nes[nidx].p.ppu_sclines.frame = machine.total_lines + nes[nidx].p.overclock.sclines.vb;\
		nes[nidx].p.ppu_sclines.vint = machine.vint_lines + nes[nidx].p.overclock.sclines.vb;\
		nes[nidx].p.ppu_sclines.vint_extra = nes[nidx].p.overclock.sclines.vb;\
	}
#define ppu_overclock_control()\
	nes[nidx].p.overclock.in_extra_sclines = TRUE;\
	if ((nes[nidx].p.ppu.frame_y >= nes[nidx].p.ppu_sclines.vint_extra) && (nes[nidx].p.ppu.frame_y < nes[nidx].p.ppu_sclines.frame)) {\
		nes[nidx].p.overclock.in_extra_sclines = FALSE;\
	}

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC void ppu_init(void);
EXTERNC void ppu_quit(void);

EXTERNC void ppu_tick(BYTE nidx);
EXTERNC BYTE ppu_turn_on(void);
EXTERNC void ppu_overclock(BYTE nidx, BYTE reset_dmc_in_use);

EXTERNC void ppu_draw_screen_pause(void);
EXTERNC void ppu_draw_screen_continue(void);

EXTERNC void ppu_draw_screen_pause_with_count(int *count);
EXTERNC void ppu_draw_screen_continue_with_count(int *count);
EXTERNC void ppu_draw_screen_continue_ctrl_count(int *count);
EXTERNC BYTE ppu_alloc_screen_buffer(_ppu_screen_buffer *sb);

EXTERNC void ppu_alignment_reset(void);

#undef EXTERNC

#endif /* PPU_H_ */
