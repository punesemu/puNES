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

#include <stdlib.h>
#include <string.h>
#include "cpu.h"
#include "info.h"
#include "clock.h"
#include "ppu_inline.h"
#include "video/gfx.h"
#include "mappers.h"
#include "irqA12.h"
#include "conf.h"
#include "fps.h"
#include "emu.h"
#include "gui.h"

enum ppu_misc { PPU_OVERFLOW_SPR = 3 };

#define fetch_at()\
{\
	BYTE shift_at, tmp;\
	ppudata.ppu.rnd_adr = ((ppudata.r2006.value & 0x0380) >> 4) | ((ppudata.r2006.value & 0x001C) >> 2);\
	ppudata.ppu.rnd_adr = 0x23C0 | (ppudata.r2006.value & 0x0C00) | ppudata.ppu.rnd_adr;\
	tmp = ppu_rd_mem(ppudata.ppu.rnd_adr);\
	shift_at = ((ppudata.r2006.value & 0x40) >> 4) | (ppudata.r2006.value & 0x02);\
	ppudata.tile_fetch.attrib = (ppudata.tile_fetch.attrib >> 8) | (((tmp >> shift_at) & 0x03) << 8);\
}
#define fetch_lb(r2000bck, r2006vl)\
	ppudata.ppu.rnd_adr = 0x2000 | ((ppudata.r2006.race.ctrl ? ppudata.r2006.race.value : ppudata.r2006.value) & 0x0FFF);\
	ppu_bck_adr(r2000bck, r2006vl);\
	ppudata.tile_fetch.l_byte = (ppudata.tile_fetch.l_byte >> 8) | (inv_chr[ppu_rd_mem(ppudata.ppu.bck_adr)] << 8);
#define fetch_hb()\
	ppudata.ppu.rnd_adr = ppudata.ppu.bck_adr | 0x0008;\
	ppudata.tile_fetch.h_byte = (ppudata.tile_fetch.h_byte >> 8) | (inv_chr[ppu_rd_mem(ppudata.ppu.rnd_adr)] << 8);\
	((ppudata.r2006.value & 0x1F) == 0x1F) ? (ppudata.r2006.value ^= 0x041F) : (ppudata.r2006.value++);
#define ppu_ticket()\
	ppudata.ppu.cycles -= machine.ppu_divide;\
	ppudata.ppu.frame_x++;\
	cpudata.nmi.cpu_cycles_from_last_nmi++;\
	/* deve essere azzerato alla fine di ogni ciclo PPU */\
	ppudata.r2006.changed_from_op = 0;
#define put_pixel(clr) ppudata.ppu_screen.wr->line[ppudata.ppu.screen_y][ppudata.ppu.frame_x] = ppudata.r2001.emphasis | clr;
#define put_emphasis(clr) put_pixel((memmap_palette.color[clr] & ppudata.r2001.color_mode))
#define put_bg put_emphasis(color_bg)
#define put_sp put_emphasis(color_sp | 0x10)
#define examine_sprites(senv, sp, vis, ty)\
	/* esamino se ci sono sprite da renderizzare */\
	for (a = 0; a < senv.count; a++) {\
		/*\
		 * per essercene, la differenza tra frameX e la\
		 * cordinata X dello sprite deve essere\
		 * inferiore a 8 (per questo uso il WORD, per\
		 * avere risultati unsigned).\
		 */\
		if ((WORD)(ppudata.ppu.frame_x - sp[a].x_C) < 8) {\
			/*\
			 * se il bit 2 del $2001 e' a 0 vuol dire\
			 * che e' abilitato il clipping degli sprite\
			 * (in poche parole non vengono disegnati i\
			 * primi 8 pixel dello screen).\
			 */\
			if ((ppudata.ppu.frame_x >= 8) || ppudata.r2001.spr_clipping) {\
				/* indico che uno sprite e' stato trovato */\
				/*flag_sp = TRUE;*/\
				/*\
				 * nel caso il colore dello sprite\
				 * sia a zero vuol dire che nessuno\
				 * sprite e' stato ancora trovato e\
				 * quindi devo disegnare quello\
				 * attualmente in esame (sprite\
				 * intendo).\
				 */\
				if (!color_sp) {\
					color_sp = ((sp[a].l_byte & 0x01) | (sp[a].h_byte & 0x02));\
					/*\
					 * se i 2 bit LSB del colore non sono uguali a\
					 * 0, vuol dire che il pixel non e' trasparente\
					 * e quindi vi aggiungo i 2 bit MSB.\
					 */\
					if (color_sp) {\
						color_sp |= ((sp[a].attrib & 0x03) << 2);\
						/* questo sprite non e' invisibile */\
						vis = a;\
						unlimited_spr = ty;\
					}\
				}\
			}\
			/*\
			 * shifto di un bit i due bitmap buffers di ogni\
			 * sprite della scanlines, compresi quelli non\
			 * visibili.\
			 */\
			sp[a].l_byte >>= 1;\
			sp[a].h_byte >>= 1;\
		}\
	}
#define get_sprites(elp, spenv, spl, sadr)\
	/*\
	 * significato bit 6 del byte degli attributi:\
	 *  0 -> no flip orizzontale\
	 *  1 -> si flip orizzontale\
	 */\
	if (ppudata.oam.elp[spenv.tmp_spr_plus][AT] & 0x40) {\
		/* salvo i primi 8 bit del tile dello sprite */\
		spl[spenv.tmp_spr_plus].l_byte = ppu_rd_mem(sadr);\
		/* salvo i secondi 8 bit del tile dello sprite */\
		spl[spenv.tmp_spr_plus].h_byte = (ppu_rd_mem(sadr | 0x08) << 1);\
	} else {\
		spl[spenv.tmp_spr_plus].l_byte = inv_chr[ppu_rd_mem(sadr)];\
		/* salvo i secondi 8 bit del tile dello sprite */\
		spl[spenv.tmp_spr_plus].h_byte = (inv_chr[ppu_rd_mem(sadr | 0x08)] << 1);\
	}

static void ppu_alignment_init(void);
INLINE static void ppu_oam_evaluation(void);

static const BYTE inv_chr[256] = {
	0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0,
	0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0,
	0x08, 0x88, 0x48, 0xC8,	0x28, 0xA8, 0x68, 0xE8,
	0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8,
	0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4,
	0x14, 0x94, 0x54, 0xD4,	0x34, 0xB4, 0x74, 0xF4,
	0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC,
	0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC,
	0x02, 0x82, 0x42, 0xC2,	0x22, 0xA2, 0x62, 0xE2,
	0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2,
	0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA,
	0x1A, 0x9A, 0x5A, 0xDA,	0x3A, 0xBA, 0x7A, 0xFA,
	0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6,
	0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6,
	0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE,
	0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE,
	0x01, 0x81, 0x41, 0xC1, 0x21, 0xA1, 0x61, 0xE1,
	0x11, 0x91, 0x51, 0xD1,	0x31, 0xB1, 0x71, 0xF1,
	0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9,
	0x19, 0x99, 0x59, 0xD9, 0x39, 0xB9, 0x79, 0xF9,
	0x05, 0x85, 0x45, 0xC5,	0x25, 0xA5, 0x65, 0xE5,
	0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5,
	0x0D, 0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED,
	0x1D, 0x9D, 0x5D, 0xDD,	0x3D, 0xBD, 0x7D, 0xFD,
	0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3,
	0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3,
	0x0B, 0x8B, 0x4B, 0xCB,	0x2B, 0xAB, 0x6B, 0xEB,
	0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB,
	0x07, 0x87, 0x47, 0xC7, 0x27, 0xA7, 0x67, 0xE7,
	0x17, 0x97, 0x57, 0xD7,	0x37, 0xB7, 0x77, 0xF7,
	0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F, 0xEF,
	0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFF
};
static const BYTE palette_init[0x20] = {
//	0x09, 0x01, 0x00, 0x01, 0x00, 0x02, 0x02, 0x0D,
	0x0D, 0x01, 0x00, 0x01, 0x00, 0x02, 0x02, 0x0D,
	0x08, 0x10, 0x08, 0x24, 0x00, 0x00, 0x04, 0x2C,
	0x09, 0x01, 0x34, 0x03, 0x00, 0x04, 0x00, 0x14,
	0x08, 0x3A, 0x00, 0x02, 0x00, 0x20, 0x2C, 0x08
};

_ppu_data ppudata;
_ppu_alignment ppu_alignment;

void ppu_init(void) {
	memset(&ppudata.ppu_screen, 0x00, sizeof(ppudata.ppu_screen));
}
void ppu_quit(void) {
	/* libero la memoria riservata */
	BYTE a = 0;

	for (a = 0; a < 2; a++) {
		_ppu_screen_buffer *sb = &ppudata.ppu_screen.buff[a];

		if (sb->data) {
			free(sb->data);
			sb->data = NULL;
		}
	}
}

void ppu_tick(void) {
	/* aggiungo i cicli della cpu trascorsi */
	ppudata.ppu.cycles = (SWORD)(ppudata.ppu.cycles + machine.cpu_divide);

	while (ppudata.ppu.cycles >= machine.ppu_divide) {
		ppudata.r2002.race.sprite_overflow = FALSE;

		/* gestione della condizione di race del $2000 al dot 257 */
		if (ppudata.r2000.race.ctrl) {
			ppudata.r2000.race.ctrl = FALSE;
			ppudata.ppu.tmp_vram = (ppudata.ppu.tmp_vram & 0xF3FF) | ((ppudata.r2000.race.value & 0x03) << 10);
		}

		/* gestione del delay del bit del grayscale */
		if (ppudata.r2001.grayscale_bit.delay && (--ppudata.r2001.grayscale_bit.delay == 0)) {
			ppudata.r2001.color_mode = PPU_CM_GRAYSCALE;
		}

		// gestione della seconda scrittura del $2006
		if (ppudata.r2006.second_write.delay && (--ppudata.r2006.second_write.delay == 0)) {
			WORD old_r2006 = ppudata.r2006.value;

			ppudata.ppu.tmp_vram = ppudata.r2006.second_write.value;

			if ((!ppudata.ppu.vblank && ppudata.r2001.visible && (ppudata.ppu.screen_y < SCR_ROWS)) && (ppudata.ppu.frame_y > ppudata.ppu_sclines.vint)) {
				// split_scroll_test_v2.nes e split_scroll_delay.nes
				if (ppudata.ppu.frame_x == 255) {
					ppudata.ppu.tmp_vram &= ppudata.r2006.value;
				}

				// condizione di race riscontrata in "scanline.nes" e
				// "Knight Rider (U) [!].nes" (i glitch grafici sotto la macchina
				// nell'introduzione sono presenti su hardware reale).
				// Anche "logo (E).nes" e "Ferrari - Grand Prix Challenge (U) [!].nes"
				// ne sono soggetti.
				if (ppudata.ppu.frame_x < SCR_COLUMNS) {
					if ((ppudata.ppu.pixel_tile >= 1) && (ppudata.ppu.pixel_tile <= 3)) {
						ppudata.r2006.race.ctrl = TRUE;
						ppudata.r2006.race.value = (ppudata.r2006.value & 0x00FF) | (ppudata.ppu.tmp_vram & 0xFF00);
					}
				}

				// aggiorno l'r2006
				ppudata.r2006.value = ppudata.ppu.tmp_vram;

				// split_scroll_test_v2.nes e split_scroll_delay.nes
				if (ppudata.ppu.frame_x == 254) {
					r2006_inc()
				}
			} else {
				// aggiorno l'r2006
				ppudata.r2006.value = ppudata.ppu.tmp_vram;
			}

			if (extcl_update_r2006) {
				extcl_update_r2006(ppudata.r2006.value, old_r2006);
			}
		}

		/* controllo se sono all'inizio della dummy line */
		if (ppudata.ppu.frame_y == ppudata.ppu_sclines.vint) {
			/*
			 * disabilito il vblank al ciclo 0 della scanline,
			 * a differenza dell'abilitazione del vblank che
			 * avviene al ciclo 341.
			 */
			if (ppudata.ppu.frame_x == 0) {
				if (chinaersan2.enable) {
					memcpy(chinaersan2.ram, ram_pnt(), 256);
				}
				ppudata.ppu.screen_y = 0;
				/* setto a 0 il bit 5, 6 ed il 7 del $2002 */
				ppudata.r2002.sprite_overflow = ppudata.r2002.sprite0_hit = ppudata.r2002.vblank = ppudata.ppu.vblank = FALSE;
				// serve assolutamente per la corretta lettura delle coordinate del puntatore zapper
				if (info.zapper_is_present && !fps_fast_forward_enabled()) {
					memset((BYTE *)ppudata.ppu_screen.wr->data, 0, (size_t)screen_size());
				}
			} else if ((ppudata.ppu.frame_x == (SHORT_SLINE_CYCLES - 1)) && (machine.type == NTSC)) {
				/*
				 * nei frame NTSC dispari, la dummy line e' lunga 340
				 * cicli invece dei soliti 341. Visto che la lettura
				 * e la scrittura del registro $2001 (visible) potrebbe
				 * avvenire un ciclo CPU (3 PPU) prima della fine
				 * della dummy line, ne anticipo il controllo e
				 * l'eventuale modifica.
				 */
				ppudata.ppu.sf.prev = ppudata.ppu.sf.actual;
				ppudata.ppu.sf.actual = FALSE;
				if (ppudata.ppu.odd_frame) {
					if (ppudata.r2001.bck_visible) {
						if (!ppudata.r2001.race.ctrl || (ppudata.r2001.race.value & 0x08)) {
							ppudata.ppu.sline_cycles = SHORT_SLINE_CYCLES;
							ppudata.ppu.sf.actual = TRUE;
						}
					} else {
						if (ppudata.r2001.race.ctrl && (ppudata.r2001.race.value & 0x08)) {
							ppudata.ppu.sline_cycles = SHORT_SLINE_CYCLES;
							ppudata.ppu.sf.actual = TRUE;
						}
					}
				}
			}
		}

		if (extcl_ppu_000_to_34x) {
			/*
			 * utilizzato dalle mappers :
			 * MMC3
			 * Taito
			 */
			extcl_ppu_000_to_34x();
		}

		/*
		 * Access PPU Memory : 1 - 128
		 * 		1. Fetch 1 name table byte
		 * 		2. Fetch 1 attribute table byte
		 * 		3. Fetch 2 pattern table bitmap bytes
		 * 		This process is repeated 32 times (32 tiles in una scanline).
		 * Note:
		 * Ricordo che un accesso alla memoria
		 * corrisponde a 2 cicli e che in un ciclo viene
		 * disegnato un pixel a video (per questo motivo
		 * utillizzo frameX per contarli [i cicli]).
		 */
		if (ppudata.ppu.frame_x < SCR_COLUMNS) {
			/*
			 * controllo:
			 * 1) di non essere nel vblank
			 * 2) di star disegnando lo schermo perche'
			 *    subito dopo aver finito il rendering
			 *    la PPU rimane assolutamente ferma per una
			 *    scanline.
			 */
			if (ppudata.ppu.vblank) {
				if ((machine.type == PAL) && (ppudata.ppu.frame_y > 23)) {
					ppu_oam_evaluation();
				}
			} else if (ppudata.ppu.screen_y < SCR_ROWS) {
				if (extcl_ppu_000_to_255) {
					/*
					 * utilizzato dalle mappers :
					 * MMC3
					 * Taito
					 * Tengen
					 */
					extcl_ppu_000_to_255();
				}
				/* controllo di non essere nella dummy line */
				if (ppudata.ppu.frame_y > ppudata.ppu_sclines.vint) {
					/*
					 * controllo se background o sprites (basta
					 * solo uno dei due) siano visibili.
					 */
					if (ppudata.r2001.visible) {
						/*
						 * inizializzo le variabili dei colori e
						 * l'indicatore del numero dello sprite
						 * che eventualmente potra' essere
						 * renderizzato (mi serve per il multiplexer).
						 */
						BYTE color_bg = 0, color_sp = 0;
						BYTE unlimited_spr = FALSE, visible_spr = 0, visible_spr_unl = 0;

/* -------------------------- FETCH DATI PER TILE SUCCESSIVO --------------------------------- */
						/*
						 * a seconda del pixel che sto renderizzando
						 * (quindi in base al ciclo PPU) faccio
						 * cio' che serve.
						 */
						if (ppudata.ppu.pixel_tile == 0) {
							/*
							 * inizializzo i buffer che utilizzero'
							 * per renderizzare i prossimi 8 pixels.
							 */
							ppudata.tile_render = ppudata.tile_fetch;
							/* applico il fine X (cioe' lo scrolling) */
							ppudata.tile_render.l_byte >>= ppudata.ppu.fine_x;
							ppudata.tile_render.h_byte >>= ppudata.ppu.fine_x;
							/*
							 * visto che in questo buffer c'e' l'MSB dei
							 * 2 bit che, a loro volta sono i 2 bit LSB
							 * del colore, lo shifto di un bit a sinistra,
							 * in modo da ritrovarmelo in posizione per
							 * l'OR che faro' nel rendering del background.
							 */
							ppudata.tile_render.h_byte <<= 1;
						} else if (ppudata.ppu.pixel_tile == 1) {
							/* faccio il fetch del byte degli attributi */
							fetch_at()
						} else if (ppudata.ppu.pixel_tile == 3) {
							/*
							 * faccio il fetch dei primi 8 bit che
							 * che compongono il tile (che hanno un
							 * peso minore rispetto ai secondi).
							 */
							fetch_lb(ppudata.r2000.bpt_adr, (ppudata.r2006.race.ctrl ? ppudata.r2006.race.value : ppudata.r2006.value))
						} else if (ppudata.ppu.pixel_tile == 5) {
							/*
							 * faccio il fetch dei secondi 8 bit che
							 * compongono il tile (che hanno un peso maggiore
							 * rispetto ai primi).
							 */
							fetch_hb()

							if (extcl_after_rd_chr) {
								/*
								 * utilizzato dalle mappers :
								 * MMC5
								 * MMC2/4
								 */
								extcl_after_rd_chr(ppudata.ppu.bck_adr);
							}
							/*
							 * Fine Y e' incrementato dopo l'ultimo fetch
							 * della scanline. Ad ogni salto da 7 a 0 di
							 * fine Y, viene incremetato tile Y che arrivato
							 * a 29 dovra' essere azzerato. Quando tile Y
							 * passa da 29 a 0, deve essere flippato l'MSB
							 * del nametable bit. Se tile Y e' settato a 30
							 * o 31 (tramite $2006 o $2005) non ci sara'
							 * ne l'azzeramento ne il flip e, nel caso sia
							 * l'indirizzo, puntera' alla attribut table.
							 */
							if (ppudata.ppu.frame_x == 253) {
								r2006_inc()
								/*
								 * alla fine di ogni scanline
								 * reinizializzo il $2006.
								 */
								r2006_end_scanline();
							}
						}
/* ---------------------------------- RENDERING BACKGROUND ----------------------------------- */
						/*
						 * se non e' settato il bit 3 del $2001 il
						 * background e' invisibile.
						 */
						if (ppudata.r2001.bck_visible) {
							/*
							 * se il bit 1 del $2001 e' a 0 vuol dire
							 * che e' abilitato il clipping del background
							 * (in poche parole non vengono disegnati i primi
							 * 8 pixel dello screen).
							 */
							if ((ppudata.ppu.frame_x >= 8) || ppudata.r2001.bck_clipping) {
								/* sto trattando un pixel del background */
								//flag_bg = TRUE;
								/* recupero i 2 bit LSB del pixel */
								color_bg = (ppudata.tile_render.l_byte & 0x01) | (ppudata.tile_render.h_byte & 0x02);
								/*
								 * shifto di un bit (leggi un pixel) i
								 * due bitmap buffers
								 */
								ppudata.tile_render.l_byte >>= 1;
								ppudata.tile_render.h_byte >>= 1;
								/*
								 * se i 2 bit LSB del colore non sono uguali a
								 * 0, vuol dire che il pixel non e' trasparente
								 * e quindi vi aggiungo i 2 bit MSB.
								 */
								if (color_bg) {
									/*
									 * recupero i 2 bit MSB del pixel.
									 * Note:
									 * per opera dello scrolling durante il
									 * rendering degli 8 pixel a video posso
									 * passare da uno tile al successivo. Se la
									 * somma tra pixel_tile e fine_x e' inferiore
									 * a 7 sono ancora nel tile corrente,
									 * altrimenti sono nel tile successivo.
									 */
									if ((ppudata.ppu.pixel_tile + ppudata.ppu.fine_x) < 8) {
										color_bg |= (ppudata.tile_render.attrib << 2);
									} else {
										color_bg |= (ppudata.tile_render.attrib >> 6);
									}
								}
							}
						}
/* ---------------------------------- RENDERING SPRITE --------------------------------------- */
						/*
						 * se e' settato il bit 5 del $2001 gli
						 * sprite sono visibili (se a 0 sono
						 * invisibili e non devo disegnarli).
						 */
						if (ppudata.r2001.spr_visible) {
							BYTE a = 0;

							examine_sprites(ppudata.spr_ev, ppudata.sprite, visible_spr, FALSE)

							if (cfg->unlimited_sprites) {
								examine_sprites(ppudata.spr_ev_unl, ppudata.sprite_unl, visible_spr_unl, TRUE)
							}
						}
/* ------------------------------------ MULTIPLEXER ------------------------------------------ */
						if (!color_sp) {
							/*
							 * se il colore dello sprite e' trasparente,
							 * utilizzo quello del background.
							 */
							if (cfg->hide_background) {
								put_pixel(memmap_palette.color[0])
							} else {
								put_bg
							}
						} else if (!color_bg) {
							/*
							 * se invece e' il colore del background ad essere
							 * trasparente, utilizzo quello dello sprite.
							 */
							if (cfg->hide_sprites) {
								put_pixel(memmap_palette.color[0])
							} else {
								put_sp
							}
						} else {
							if (!unlimited_spr) {
								if (ppudata.sprite[visible_spr].attrib & 0x20) {
									/*
									 * se non lo sono tutti e due, controllo la
									 * profondita' dello sprite e se e' settata su
									 * "dietro il background" utilizzo il colore
									 * dello sfondo.
									 */
									if (cfg->hide_background) {
										if (cfg->hide_sprites) {
											put_pixel(memmap_palette.color[0])
										} else {
											put_sp
										}
									} else {
										put_bg
									}
								} else {
									/* altrimenti quello dello sprite */
									if (cfg->hide_sprites) {
										if (cfg->hide_background) {
											put_pixel(memmap_palette.color[0])
										} else {
											put_bg
										}
									} else {
										put_sp
									}
								}
								/* -- HIT OBJECT #0 FLAG --
								 *
								 * per sapere se sono nella condizione di
								 * "Sprite 0 Hit" devo controllare:
								 * 1) se 'Sprite 0 Hit' e' gia' settato
								 * 2) se sto trattando lo sprite #0
								 * 3) che non stia renderizzando l'ultimo pixel
								 *    visibile (il 255 appunto) della scanline
								 * Note:
								 * implementato il controllo del pixel 255
								 * la demo scroll.nes spesso mi sporca
								 * la parte finale dello screen dove sono
								 * posizionate le informazioni su tipo di
								 * sistema (pal o nes e frequenza di aggiornamento).
								 */
								if (!ppudata.r2002.sprite0_hit && !ppudata.sprite[visible_spr].number && (ppudata.ppu.frame_x != 255)) {
									ppudata.r2002.sprite0_hit = 0x40;
								}
							} else {
								if (ppudata.sprite_unl[visible_spr_unl].attrib & 0x20) {
									if (cfg->hide_background) {
										if (cfg->hide_sprites) {
											put_pixel(memmap_palette.color[0])
										} else {
											put_sp
										}
									} else {
										put_bg
									}
								} else {
									if (cfg->hide_sprites) {
										if (cfg->hide_background) {
											put_pixel(memmap_palette.color[0])
										} else {
											put_bg
										}
									} else {
										put_sp
									}
								}
							}
						}
						ppu_oam_evaluation();
/* ------------------------------------------------------------------------------------------- */
					} else {
						/*
						 * altrimenti visualizzo un pixel del
						 * colore 0 della paletta.
						 */
						put_pixel(memmap_palette.color[0])

						if ((ppudata.r2006.value & 0xFF00) == 0x3F00) {
							/*
							 * se background e sprites non sono visibili
							 * e $2006 e' nel range 0x3F00/0x3FFF (nella
							 * paletta insomma) allora a video devo
							 * visualizzare il colore puntato dal registro.
							 */
							put_emphasis(ppudata.r2006.value & 0x1F)
						}
					}
				}
				/* incremento in contatore dei pixel interni al tile */
				if (++ppudata.ppu.pixel_tile > 7) {
					ppudata.ppu.pixel_tile = 0;
				}
				ppu_ticket()
				continue;
			}
		}

/* ----------------------------- FETCH DATI SPRITES SCANLINE+1 ------------------------------- */
		/*
		 * Access PPU Memory : 129 - 160
		 * 		1. Garbage name table byte
		 * 		2. Garbage name table byte
		 * 		3. Pattern table bitmap #0 for applicable object (for next scanline)
		 * 		4. Pattern table bitmap #1 for applicable object (for next scanline)
		 * 		This process is repeated 8 times.
		 */
		if (ppudata.ppu.frame_x < 320) {
			if (!ppudata.ppu.vblank && ppudata.r2001.visible && (ppudata.ppu.screen_y < SCR_ROWS)) {
				if (extcl_ppu_256_to_319) {
					/*
					 * utilizzato dalle mappers :
					 * MMC3
					 * Taito
					 * MMC5
					 * Tengen
					 */
					extcl_ppu_256_to_319();
				}
				if (ppudata.ppu.frame_x == 256) {
					ppudata.spr_ev.timing = ppudata.spr_ev.tmp_spr_plus = 0;
				}
				/* controllo se ci sono sprite per la (scanline+1) */
				if (ppudata.spr_ev.tmp_spr_plus < ppudata.spr_ev.count_plus) {
					switch (ppudata.spr_ev.timing) {
						case 0:
							/*
							 * utilizzo ppudata.spr_ev.timing come contatore di cicli per
							 * esaminare uno sprite ogni 8 cicli.
							 */
							ppudata.ppu.rnd_adr = 0x2000 | (ppudata.r2006.value & 0xFFF);
							ppu_spr_adr(ppudata.spr_ev.tmp_spr_plus)
							get_sprites(ele_plus, ppudata.spr_ev, ppudata.sprite_plus, ppudata.ppu.spr_adr)
							ppudata.r2004.value = ppudata.oam.ele_plus[ppudata.spr_ev.tmp_spr_plus][YC];
							if (extcl_after_rd_chr) {
								/*
								 * utilizzato dalle mappers :
								 * MMC5
								 * MMC2/4
								 */
								extcl_after_rd_chr(ppudata.ppu.spr_adr);
							}
							/* incremento il contatore del ciclo interno */
							ppudata.spr_ev.timing++;
							break;
						case 2:
							ppudata.ppu.rnd_adr = 0x2000 | (ppudata.r2006.value & 0xFFF);
							ppudata.r2004.value = ppudata.oam.ele_plus[ppudata.spr_ev.tmp_spr_plus][ppudata.spr_ev.timing];
							/* incremento il contatore del ciclo interno */
							ppudata.spr_ev.timing++;
							break;
						case 1:
						case 3:
							ppudata.r2004.value = ppudata.oam.ele_plus[ppudata.spr_ev.tmp_spr_plus][ppudata.spr_ev.timing];
							/* incremento il contatore del ciclo interno */
							ppudata.spr_ev.timing++;
							break;
						case 4:
							ppudata.ppu.rnd_adr = ppudata.ppu.spr_adr;
							ppudata.r2004.value = ppudata.oam.ele_plus[ppudata.spr_ev.tmp_spr_plus][XC];
							/* incremento il contatore del ciclo interno */
							ppudata.spr_ev.timing++;
							break;
						case 5:
							ppudata.r2004.value = ppudata.oam.ele_plus[ppudata.spr_ev.tmp_spr_plus][XC];
							/* incremento il contatore del ciclo interno */
							ppudata.spr_ev.timing++;
							break;
						case 6:
							ppudata.ppu.rnd_adr = ppudata.ppu.spr_adr | 0x0008;
							ppudata.r2004.value = ppudata.oam.ele_plus[ppudata.spr_ev.tmp_spr_plus][XC];
							/* incremento il contatore del ciclo interno */
							ppudata.spr_ev.timing++;
							break;
						case 7:
							ppudata.r2004.value = ppudata.oam.ele_plus[ppudata.spr_ev.tmp_spr_plus][XC];
							/* passo al prossimo sprite */
							ppudata.spr_ev.timing = 0;
							/* incremento l'indice temporaneo degli sprites */
							if (++ppudata.spr_ev.tmp_spr_plus == 8) {
								// unlimited sprites
								if (cfg->unlimited_sprites && ppudata.spr_ev_unl.evaluate) {
									for (ppudata.spr_ev_unl.tmp_spr_plus = 0;
										ppudata.spr_ev_unl.tmp_spr_plus < ppudata.spr_ev_unl.count_plus;
										ppudata.spr_ev_unl.tmp_spr_plus++) {
										WORD spr_adr = 0;

										_ppu_spr_adr(ppudata.spr_ev_unl.tmp_spr_plus, ele_plus_unl, ppudata.sprite_plus_unl, spr_adr)
										get_sprites(ele_plus_unl, ppudata.spr_ev_unl, ppudata.sprite_plus_unl, spr_adr)
									}
									ppudata.spr_ev_unl.evaluate = FALSE;
								}
							}
							break;
					 }
				} else {
					if (ppudata.spr_ev.timing == 0) {
						ppudata.r2004.value = ppudata.oam.element[63][YC];
						ppudata.spr_ev.timing++;
					} else if (ppudata.spr_ev.timing < 7) {
						ppudata.r2004.value = 0xFF;
						ppudata.spr_ev.timing++;
					} else {
						ppudata.spr_ev.timing = 0;
					}
				}
				/*
				 * dalla riga 239 non devo piu' esaminare
				 * gli sprites quindi azzero l'$2003.
				 * Roms interessate:
				 * Aladdin 2 (Unl).nes
				 * Dusty Diamond's All-Star Softball (U) [!].nes
				 * Bing Kuang Ji Dan Zi - Flighty Chicken (Ch).nes
				 */
				if ((ppudata.ppu.frame_x == 319) && (ppudata.ppu.screen_y == 238)) {
					ppudata.r2003.value = 0;
				}
				ppu_ticket()
				continue;
			}
		}
/* ------------------------------- FETCH TILE 0 e 1 SCANLINE+1 ------------------------------- */
		/*
		 * Access PPU Memory : 161 - 168
		 * 		1. Fetch 1 name table byte
		 * 		2. Fetch 1 attribute table byte
		 * 		3. Fetch 2 pattern table bitmap bytes
		 * 		This process is repeated 2 times.
		 */
		if (!ppudata.ppu.vblank && (ppudata.r2001.visible || ppudata.r2001.race.ctrl) && (ppudata.ppu.screen_y < SCR_ROWS)) {
			if (extcl_ppu_320_to_34x) {
				/*
				 * utilizzato dalle mappers :
				 * MMC3
				 * Taito
				 * MMC5
				 * Tengen
				 */
				extcl_ppu_320_to_34x();
			}
			switch (ppudata.ppu.frame_x) {
				case 323:
					if (ppudata.ppu.frame_y == ppudata.ppu_sclines.vint) {
						/*
						 * all'inizio di ogni frame reinizializzo
						 * l'indirizzo della PPU.
						 */
						ppudata.r2006.value = ppudata.ppu.tmp_vram;
					}
					fetch_at()
					break;
				case 331:
					fetch_at()
					break;
				case 325:
				case 333:
					fetch_lb(ppudata.r2000.bpt_adr, ppudata.r2006.value)
					break;
				case 327:
				case 335:
					fetch_hb()
					if (extcl_after_rd_chr) {
						/*
						 * utilizzato dalle mappers :
						 * MMC5
						 * MMC2/4
						 */
						extcl_after_rd_chr(ppudata.ppu.bck_adr);
					}
					break;
				case 337:
				case 339:
					ppudata.ppu.rnd_adr = 0x2000 | (ppudata.r2006.value & 0x0FFF);
					break;
			}
		}

/* ------------------------------------------------------------------------------------------- */
		/*
		 * Access PPU Memory : 169 - 170 (+1/2)
		 * 		1. Fetch 1 name table byte
		 * 		This process is repeated 2 times.
		 *
		 * I'm unclear of the reason why this particular
		 * access to ram is made. The nametable address
		 * that is accessed 2 times in a row here, is also
		 * the same nametable address that points to the 3rd
		 * tile to be rendered on the screen (or basically,
		 * the first nametable address that will be accessed
		 * when the PPU is fetching background data on the
		 * next scanline).
		 */
		if (ppudata.ppu.frame_x < ppudata.ppu.sline_cycles) {
			if (ppudata.spr_ev.count_plus) {
				ppudata.r2004.value = ppudata.oam.ele_plus[0][YC];
			} else {
				ppudata.r2004.value = ppudata.oam.element[63][YC];
			}
			ppu_ticket()
			/*
			 * se e' iniziato il ciclo 341, vuol dire
			 * che in realta' e' iniziato il ciclo 0
			 * della scanline successiva.
			 */
			if (ppudata.ppu.frame_x != ppudata.ppu.sline_cycles) {
				continue;
			}
		}

		/*
		 * schema dettaglio frame completo
		 *
		 *        NTSC                             PAL                            Dendy
		 * frameY  | |  screenY            frameY  | |  screenY            frameY  | |  screenY
		 * --------------------            --------------------            ---------V----------
		 * |ovb     V        0| ovclock vb |ovb      V       0| ovclock vb |ovb     B        0|
		 * ---------B----------            ----------B---------            ---------L----------
		 * |ovb+0   L        0|            |ovb+0    L       0|            |ovb+0   A        0|
		 * |.       A        .|   VBLANK   |.        A       .|   VBLANK   |.       N        .|
		 * |.       N        .|            |.        N       .|            |ovb+19  K        0|
		 * |ovb+19  K        0|            |ovb+69   K       0|            --------------------
		 * --------------------            -------------------- dummy line-|ovb+20           0|
		 * |ovb+20           0| dummy line |ovb+70           0|/           --------------------
		 * --------------------            --------------------            |ovb+21           0|
		 * |ovb+21           0|            |ovb+71           0|   screen   |.                .|
		 * |.                .|   screen   |.                .| (rendering)|ovb+260        239|
		 * |.                .| (rendering)|.                .|            --------------------
		 * |ovb+260        239|            |ovb+310        239|            |ovb+261        240|
		 * --------------------            --------------------            |.                .|
		 * |ovb+261        240| PPU ferma  |ovb+311        240| PPU ferma  |ovb+311        290|
		 * --------------------            --------------------            --------------------
		 * Tot. ovb+262 sclines            Tot. ovb+312 sclines            Tot. ovb+312 sclines
		 * --------------------            --------------------            --------------------
		 * |ovb+262        240| ovclock pr |ovb+312        240| ovclock pr |ovb+312        290|
		 * --------------------            --------------------            --------------------
		 *
		 */
		/* controllo di essere nel range [dummy...rendering screen] */
		if ((ppudata.ppu.frame_y >= ppudata.ppu_sclines.vint) && (ppudata.ppu.screen_y < SCR_ROWS)) {
			BYTE a = 0;

			/* verifico di non trattare la dummy line */
			if (ppudata.ppu.frame_y > ppudata.ppu_sclines.vint) {
				/* incremento il contatore delle scanline renderizzate */
				ppudata.ppu.screen_y++;
				if ((ppudata.ppu.screen_y == SCR_ROWS) && (info.no_ppu_draw_screen == 0)) {
					gfx_draw_screen();
				}
			}
			/*
			 * l'indice degli sprite per la (scanline+1)
			 * diventa quello attuale (visto che sto per
			 * incrementare la scanline).
			 */
			ppudata.spr_ev.count = ppudata.spr_ev.count_plus;
			/* azzero l'indice per la (scanline+1) */
			ppudata.spr_ev.count_plus = 0;
			/*
			 * sposto il buffer degli sprites della scanline
			 * successiva (scanline+1) nel buffer di quella
			 * che sto per trattare.
			 */
			for (a = ppudata.spr_ev.count; a--;) {
				ppudata.sprite[a].y_C = ppudata.oam.ele_plus[a][YC];
				ppudata.sprite[a].tile = ppudata.oam.ele_plus[a][TL];
				ppudata.sprite[a].attrib = ppudata.oam.ele_plus[a][AT];
				ppudata.sprite[a].x_C = ppudata.oam.ele_plus[a][XC];
				ppudata.sprite[a].number = ppudata.sprite_plus[a].number;
				ppudata.sprite[a].flip_v = ppudata.sprite_plus[a].flip_v;
				ppudata.sprite[a].l_byte = ppudata.sprite_plus[a].l_byte;
				ppudata.sprite[a].h_byte = ppudata.sprite_plus[a].h_byte;
			}
			// unlimited sprites
			if (cfg->unlimited_sprites) {
				ppudata.spr_ev_unl.count = ppudata.spr_ev_unl.count_plus;
				/* azzero l'indice per la (scanline+1) */
				ppudata.spr_ev_unl.count_plus = 0;
				/*
				 * sposto il buffer degli sprites della scanline
				 * successiva (scanline+1) nel buffer di quella
				 * che sto per trattare.
				 */
				for (a = ppudata.spr_ev_unl.count; a--;) {
					ppudata.sprite_unl[a].y_C = ppudata.oam.ele_plus_unl[a][YC];
					ppudata.sprite_unl[a].tile = ppudata.oam.ele_plus_unl[a][TL];
					ppudata.sprite_unl[a].attrib = ppudata.oam.ele_plus_unl[a][AT];
					ppudata.sprite_unl[a].x_C = ppudata.oam.ele_plus_unl[a][XC];
					ppudata.sprite_unl[a].number = ppudata.sprite_plus_unl[a].number;
					ppudata.sprite_unl[a].flip_v = ppudata.sprite_plus_unl[a].flip_v;
					ppudata.sprite_unl[a].l_byte = ppudata.sprite_plus_unl[a].l_byte;
					ppudata.sprite_unl[a].h_byte = ppudata.sprite_plus_unl[a].h_byte;
				}
			}
		}

		/*
		 * incremento frameY, reinizializzo
		 * slineCycles ed e' estremamente importante
		 * che lo faccia esattamente qui, cosi'
		 * come e' importante che l'azzeramento del
		 * ppudata.ppu.framex lo faccia dopo il settaggio dell'nmi.
		 */
		ppudata.ppu.frame_y++;
		ppudata.ppu.sline_cycles = SLINE_CYCLES;

		/* controllo se ho completato il frame */
		if (ppudata.ppu.frame_y >= ppudata.ppu_sclines.total) {
			// aggiorno il numero delle scanlines
			ppu_overclock_update()
			// azzero il flag del DMC dell'overclock
			ppudata.overclock.DMC_in_use = FALSE;
			/* incremento il contatore ppu dei frames */
			ppudata.ppu.frames++;
			/* azzero frame_y */
			ppudata.ppu.frame_y = 0;
			/* setto il flag che indica che un frame e' stato completato */
			info.frame_status = FRAME_FINISHED;
			/* e' un frame dispari? */
			ppudata.ppu.odd_frame = !ppudata.ppu.odd_frame;
			/* abilito il vblank */
			ppudata.r2002.vblank = 0x80;
			ppudata.ppu.vblank = TRUE;
			if ((ppudata.ppu.frames == 1) && info.r2002_jump_first_vblank) {
				ppudata.r2002.vblank = 0x00;
			}
			/*
			 * quando il bit 7 del $2002 e il bit 7
			 * del $2000 sono a 1 devo generare un NMI.
			 */
			if (ppudata.r2000.nmi_enable) {
				cpudata.nmi.high = TRUE;
				cpudata.nmi.frame_x = ppudata.ppu.frame_x;
				/* azzero i numeri di cicli dall'nmi */
				cpudata.nmi.cpu_cycles_from_last_nmi = 0;
			}
		}

		// controllo se sono nelle scanlines extra
		ppu_overclock_control()

		/*
		 * azzero frameX, ed e' estremamente
		 * importante che lo faccia esattamente
		 * dopo il settaggio dell'nmi.
		 */
		ppudata.ppu.frame_x = 0;
		/* deve essere azzerato alla fine di ogni ciclo PPU */
		ppudata.r2006.changed_from_op = 0;
	}
}
BYTE ppu_turn_on(void) {
	// nel primo frame l'overclocking e' sempre disabilitato
	ppudata.overclock.DMC_in_use = TRUE;
	ppu_overclock(FALSE);

	if (info.reset >= HARD) {
		memset(&ppudata.ppu, 0x00, sizeof(_ppu));
		memset(&ppudata.ppu_openbus, 0x00, sizeof(_ppu_openbus));
		memset(&ppudata.r2000, 0x00, sizeof(_r2000));
		memset(&ppudata.r2001, 0x00, sizeof(_r2001));
		memset(&ppudata.r2002, 0x00, sizeof(_r2002));
		memset(&ppudata.r2003, 0x00, sizeof(_r2xxx));
		memset(&ppudata.r2004, 0x00, sizeof(_r2xxx));
		memset(&ppudata.r2006, 0x00, sizeof(_r2006));
		memset(&ppudata.r2007, 0x00, sizeof(_r2xxx));
		memset(&ppudata.spr_ev, 0x00, sizeof(_spr_evaluate));
		memset(&ppudata.sprite, 0x00, sizeof(_spr));
		memset(&ppudata.sprite_plus, 0x00, sizeof(_spr));
		memset(&ppudata.spr_ev_unl, 0x00, sizeof(_spr_evaluate));
		memset(&ppudata.sprite_unl, 0x00, sizeof(_spr));
		memset(&ppudata.sprite_plus_unl, 0x00, sizeof(_spr));
		memset(&ppudata.tile_render, 0x00, sizeof(_tile));
		memset(&ppudata.tile_fetch, 0x00, sizeof(_tile));
		/*
		 * "Time Lord (U) [!].nes"
		 * funziona correttamente (altrimenti avviato il gioco
		 * la parte di sotto si sporca e non appaiono sprites).
		 */
		ppudata.ppu.frame_y = ppudata.ppu_sclines.vint + 1;
		ppudata.ppu.sline_cycles = SLINE_CYCLES;
		ppudata.r2000.r2006_inc = 1;
		ppudata.r2000.size_spr = 8;
		ppudata.r2001.color_mode = PPU_CM_NORMAL;

		/* riservo una zona di memoria per lo screen */
		if ((info.reset == CHANGE_ROM) || (info.reset == POWER_UP)) {
			BYTE a = 0;

			ppudata.ppu_screen.rd = &ppudata.ppu_screen.buff[0];
			ppudata.ppu_screen.wr = &ppudata.ppu_screen.buff[1];
			ppudata.ppu_screen.last_completed_wr = ppudata.ppu_screen.wr;

			for (a = 0; a < 2; a++) {
				if (ppu_alloc_screen_buffer(&ppudata.ppu_screen.buff[a]) == EXIT_ERROR) {
					return (EXIT_ERROR);
				}
			}
			/*
			 * tabella di indici che puntano ad ogni
			 * elemento dell'OAM (4 bytes ciascuno).
			 */
			for (a = 0; a < 64; ++a) {
				ppudata.oam.element[a] = &ppudata.oam.data[(size_t)(a * 4)];
			}
			for (a = 0; a < 8; ++a) {
				ppudata.oam.ele_plus[a] = &ppudata.oam.plus[(size_t)(a * 4)];
			}
			for (a = 0; a < 56; ++a) {
				ppudata.oam.ele_plus_unl[a] = &ppudata.oam.plus_unl[(size_t)(a * 4)];
			}
			ppu_alignment_reset();
		}
		/* reinizializzazione completa della PPU */
		{
			int a = 0, x = 0, y = 0;

			/* inizializzo lo screen */
			for (a = 0; a < 2; a++) {
				_ppu_screen_buffer *sb = &ppudata.ppu_screen.buff[a];

				for (y = 0; y < SCR_ROWS; y++) {
					for (x = 0; x < SCR_COLUMNS; x++) {
						sb->line[y][x] = 0x000D;
					}
				}
			}
			/*
			 * inizializzo la Object Attribute Memory
			 * utilizzata per conservare le informazioni
			 * inerenti gli sprites.
			 */
			memset(ppudata.oam.data, 0xFF, sizeof(ppudata.oam.data));
			memset(ppudata.oam.plus, 0xFF, sizeof(ppudata.oam.plus));
			memset(ppudata.oam.plus_unl, 0xFF, sizeof(ppudata.oam.plus_unl));
			/* inizializzo nametables */
			nmt_memset();
			/* e paletta dei colori */
			memcpy(memmap_palette.color, palette_init, sizeof(memmap_palette.color));

			// power_up_palette.nes
			if (info.crc32.total == 0xDD941E82) {
				memmap_palette.color[0] = 0x09;
			}
		}
		ppu_alignment_init();
	} else {
		memset(&ppudata.r2000, 0x00, sizeof(_r2000));
		memset(&ppudata.r2001, 0x00, sizeof(_r2001));
		memset(&ppudata.r2002, 0x00, sizeof(_r2002));
		memset(&ppudata.r2007, 0x00, sizeof(_r2xxx));

		ppudata.ppu.frame_x = ppudata.ppu.screen_y = ppudata.ppu.pixel_tile = 0;
		ppudata.ppu.frame_y = ppudata.ppu_sclines.vint + 1;
		ppudata.ppu.tmp_vram = ppudata.ppu.fine_x = 0;
		ppudata.ppu.spr_adr = ppudata.ppu.bck_adr = 0;
		ppudata.ppu.sline_cycles = SLINE_CYCLES;
		ppudata.ppu.odd_frame = 0;
		ppudata.ppu.cycles = 0;
		ppudata.r2000.r2006_inc = 1;
		ppudata.r2000.size_spr = 8;
		ppudata.r2001.color_mode = PPU_CM_NORMAL;
	}

	return (EXIT_OK);
}
void ppu_overclock(BYTE reset_dmc_in_use) {
	if (reset_dmc_in_use) {
		ppudata.overclock.DMC_in_use = FALSE;
	}

	ppudata.overclock.sclines.vb = 0;
	ppudata.overclock.sclines.pr = 0;

	if (cfg->ppu_overclock) {
		ppudata.overclock.sclines.vb = cfg->extra_vb_scanlines;
		ppudata.overclock.sclines.pr = cfg->extra_pr_scanlines;
	}

	ppudata.overclock.sclines.total = ppudata.overclock.sclines.vb + ppudata.overclock.sclines.pr;
	ppu_overclock_update()
	ppu_overclock_control()
}

void ppu_draw_screen_pause(void) {
	info.no_ppu_draw_screen++;
}
void ppu_draw_screen_continue(void) {
	if (--info.no_ppu_draw_screen < 0) {
		info.no_ppu_draw_screen = 0;
	}
}
void ppu_draw_screen_pause_with_count(int *count) {
	ppu_draw_screen_pause();
	(*count)++;
}
void ppu_draw_screen_continue_with_count(int *count) {
	ppu_draw_screen_continue();
	(*count)--;
}
void ppu_draw_screen_continue_ctrl_count(int *count) {
	for (; (*count) > 0; (*count)--) {
		ppu_draw_screen_continue();
	}
	(*count) = 0;
}
BYTE ppu_alloc_screen_buffer(_ppu_screen_buffer *sb) {
	int b = 0;

	sb->ready = FALSE;
	sb->frame = 0;

	if (sb->data) {
		free(sb->data);
	}

	sb->data = (WORD *)malloc((size_t)screen_size());
	if (!sb->data) {
		log_error(uL("ppu;out of memory"));
		return (EXIT_ERROR);
	}
	/*
	 * creo una tabella di indici che puntano
	 * all'inizio di ogni linea dello screen.
	 */
	for (b = 0; b < SCR_ROWS; b++) {
		sb->line[b] = (WORD *)(sb->data + ((size_t)b * SCR_COLUMNS));
	}

	return (EXIT_OK);
}

void ppu_alignment_reset(void) {
	ppu_alignment.count.cpu = 0;
	ppu_alignment.count.ppu = 0;
}

static void ppu_alignment_init(void) {
	switch (cfg->ppu_alignment) {
		default:
		case PPU_ALIGMENT_DEFAULT:
			ppu_alignment.cpu = 0;
			ppu_alignment.ppu = 1;
			break;
		case PPU_ALIGMENT_RANDOMIZE:
			ppu_alignment.cpu = emu_irand(100) % machine.cpu_divide;
			ppu_alignment.ppu = emu_irand(100) % machine.ppu_divide;
			break;
		case PPU_ALIGMENT_INC_AT_RESET:
			ppu_alignment.cpu = ppu_alignment.count.cpu;
			ppu_alignment.ppu = ppu_alignment.count.ppu;
			break;
	}

	ppudata.ppu.cycles = 0; //(SWORD)(machine.ppu_divide * -8);
	ppudata.ppu.cycles += (SWORD)((ppu_alignment.cpu + (-ppu_alignment.ppu + 1)) % machine.cpu_divide);

	if (cfg->ppu_alignment == PPU_ALIGMENT_INC_AT_RESET) {
		ppu_alignment.count.cpu = (ppu_alignment.count.cpu + 1) % machine.cpu_divide;
		if (!ppu_alignment.count.cpu) {
			ppu_alignment.count.ppu = (ppu_alignment.count.ppu + 1) % machine.ppu_divide;
		}
	}

	if (gui.start) {
		gui_update_status_bar();
	}
	if ((cfg->ppu_alignment == PPU_ALIGMENT_DEFAULT) || (info.reset == CHANGE_ROM) || (info.reset == POWER_UP)) {
		return;
	} else if (info.reset >= HARD) {
		log_info(uL("CPU/PPU alig.;PPU %d/%d, CPU %d/%d"),
			ppu_alignment.ppu, (machine.ppu_divide - 1),
			ppu_alignment.cpu, (machine.cpu_divide - 1));
	}
}

INLINE static void ppu_oam_evaluation(void) {
/* ------------------------------- CONTROLLO SPRITE SCANLINE+1 ------------------------------- */
	if (ppudata.ppu.frame_x < 64) {
		ppudata.r2004.value = 0xFF;
		/* inizializzo le varibili per il ciclo 64 */
		if (ppudata.ppu.frame_x == 63) {
			/*
			 * inizializzo i vari indici
			 *
			 * Note:
			 * imposto index al suo valore massimo
			 * solo perche' nel 64° ciclo, come prima
			 * cosa lo incremento azzerandolo.
			 */
			ppudata.spr_ev.timing = 0;
			ppudata.spr_ev.real = 0;
			ppudata.spr_ev.index = 0xFF;
			/* la fase 1 e 2 corrispondono */
			ppudata.spr_ev.phase = 2;
		}
	} else if (ppudata.ppu.frame_x < 256) {
/* --------------------------------------- FASE 1 E 2 ---------------------------------------- */
		/*
		 * in questa fase esamino e salvo i primi 8 sprites
		 * che trovo essere nel range. Trovato l'ottavo passo
		 * alla fase 3. Se invece esamino tutti e 64 gli sprites
		 * dell'OAM, passo alla fase 4.
		 */
		if (ppudata.spr_ev.phase == 2) {
			if (ppudata.spr_ev.timing == 0) {
				/* in caso di overflow dell'indice degli sprite ... */
				if (++ppudata.spr_ev.index == 64) {
					/* ...azzero l'indice... */
					ppudata.spr_ev.index = ppudata.spr_ev.real = 0;
					/* ...passo alla fase 4... */
					ppudata.spr_ev.phase = 4;
					/*
					 * ...di cui questo stesso ciclo sara' il
					 * timing = 0, quindi il prossimo sara' l'1.
					 */
					ppudata.spr_ev.timing = 1;
					/* leggo la coordinata Y dello sprite 0 */
					ppudata.r2004.value = ppudata.oam.element[0][YC];
					/*
					 * We've since discovered that not only are
					 * sprites 0 and 1 temporarily replaced with
					 * the pair that OAMADDR & 0xF8 points to, but
					 * it's permanent: the pair that OAMADDR & 0xF8
					 * points to is copied to the first 8 bytes of
					 * OAM when rendering starts.
					 * The difference should only show up with a game
					 * that doesn't use DMA every frame.
					 */
					{
						static BYTE i;

						for (i = 8; i--;) {
							ppudata.oam.data[i] = ppudata.oam.data[(ppudata.r2003.value & 0xF8) + i];
						}
					}
				} else {
					ppudata.spr_ev.real = ppudata.spr_ev.index;
					/* leggo dall'OAM il byte 0 dell'elemento in esame */
					ppudata.r2004.value = ppudata.oam.element[ppudata.spr_ev.real][YC];
					/*
					 * calcolo la differenza tra la posizione
					 * iniziale dello sprite e la posizione Y
					 * del pixel che sto renderizzando. Se e'
					 * inferiore a 8 o 16 (dipende dalla dimensione
					 * dello sprite) allora puo' essere disegnato.
					 */
					ppudata.spr_ev.range = ppudata.ppu.screen_y - ppudata.r2004.value;

					ppudata.spr_ev.evaluate = FALSE;
					/*
					 * se sono nel range e lo sprite ha una
					 * posizione Y inferiore o uguale a 0xEF,
					 * lo esamino.
					 */
					if ((ppudata.spr_ev.count_plus < 8) && (ppudata.r2004.value <= 0xEF) && (ppudata.spr_ev.range < ppudata.r2000.size_spr)) {
						ppudata.spr_ev.evaluate = TRUE;
					}
					/* incremento timing */
					ppudata.spr_ev.timing++;
				}
			} else if (ppudata.spr_ev.timing == 1) {
				/*
				 * esamino lo sprites e se necessario
				 * inizio a memorizzare le informazioni.
				 */
				if (ppudata.spr_ev.evaluate) {
					/*
					 * memorizzo la prima parte delle
					 * informazione dello sprite nel buffer.
					 */
					ppudata.oam.ele_plus[ppudata.spr_ev.count_plus][YC] = ppudata.r2004.value;
					ppudata.sprite_plus[ppudata.spr_ev.count_plus].number = ppudata.spr_ev.index;
					ppudata.sprite_plus[ppudata.spr_ev.count_plus].flip_v = ppudata.spr_ev.range;
					/* continuo a trattare questo sprite */
					ppudata.spr_ev.timing++;
				} else {
					/* passo al prossimo sprite */
					ppudata.spr_ev.timing = 0;
				}
			/* tratto i cicli pari */
			} else if (!(ppudata.spr_ev.timing & 0x01)) {
				/* leggo il prossimo byte dell'OAM */
				ppudata.r2004.value = ppudata.oam.element[ppudata.spr_ev.real][ppudata.spr_ev.timing >> 1];
				/* passo al ciclo successivo */
				ppudata.spr_ev.timing++;
			/* tratto i cicli dispari */
			} else {
				/* memorizzo il valore letto nel ciclo prima */
				ppudata.oam.ele_plus[ppudata.spr_ev.count_plus][ppudata.spr_ev.timing >> 1] = ppudata.r2004.value;
				/* l'unico ciclo diverso e' l'ultimo */
				if (ppudata.spr_ev.timing == 7) {
					/*
					 * se ho gia' trovato 8 sprites allora
					 * devo avviare la fase 3.
					 */
					if (++ppudata.spr_ev.count_plus == 8) {
						ppudata.spr_ev.phase = 3;
						/*
						 * inizilizzo le variabili che
						 * mi serviranno. byte_OAM = 3
						 * perche' verra' aumentata e quindi
						 * riportata a 0 nel primo ciclo
						 * della fase 3.
						 */
						ppudata.spr_ev.evaluate = FALSE;
						ppudata.spr_ev.byte_OAM = 3;
						ppudata.spr_ev.index_plus = 0;

						// unlimited sprites
						if (cfg->unlimited_sprites) {
							// https://wiki.nesdev.com/w/index.php/Sprite_overflow_games (Use of excess sprites for masking effects)
							// https://github.com/SourMesen/Mesen/issues/188
							// start - thx to Sour
							BYTE unlimited_sprites = TRUE;

							if (cfg->unlimited_sprites_auto) {
								BYTE count = 0,  max_count = 0;
								WORD last_position = 0xFFFF;
								int i = 0;

								for (i = 0; i < 64; i++) {
									BYTE y = ppudata.oam.element[i][YC];
									WORD range = ppudata.ppu.screen_y - y;

									if ((y <= 0xEF) && (range < ppudata.r2000.size_spr)) {
										WORD position = (y << 8) | ppudata.oam.element[i][XC];

										if (position != last_position) {
											if (count > max_count) {
												max_count = count;
											}
											last_position = position;
											count = 1;
											continue;
										}
										count++;
									}
								}
								unlimited_sprites = (count < 8) & (max_count < 8);
							}
							// end

							if (unlimited_sprites) {
								BYTE t2004 = 0;

								ppudata.spr_ev_unl.index = ppudata.spr_ev.index + 1;
								ppudata.spr_ev_unl.count_plus = 0;

								for (; ppudata.spr_ev_unl.index < 64; ppudata.spr_ev_unl.index++) {
									t2004 = ppudata.oam.element[ppudata.spr_ev_unl.index][YC];

									ppudata.spr_ev_unl.range = ppudata.ppu.screen_y - t2004;

									if ((t2004 <= 0xEF) && (ppudata.spr_ev_unl.range < ppudata.r2000.size_spr)) {
										ppudata.oam.ele_plus_unl[ppudata.spr_ev_unl.count_plus][YC] = ppudata.oam.element[ppudata.spr_ev_unl.index][YC];
										ppudata.oam.ele_plus_unl[ppudata.spr_ev_unl.count_plus][TL] = ppudata.oam.element[ppudata.spr_ev_unl.index][TL];
										ppudata.oam.ele_plus_unl[ppudata.spr_ev_unl.count_plus][AT] = ppudata.oam.element[ppudata.spr_ev_unl.index][AT];
										ppudata.oam.ele_plus_unl[ppudata.spr_ev_unl.count_plus][XC] = ppudata.oam.element[ppudata.spr_ev_unl.index][XC];
										ppudata.sprite_plus_unl[ppudata.spr_ev_unl.count_plus].number = ppudata.spr_ev_unl.index;
										ppudata.sprite_plus_unl[ppudata.spr_ev_unl.count_plus].flip_v = ppudata.spr_ev_unl.range;
										ppudata.spr_ev_unl.count_plus++;
									}
								}
								if (ppudata.spr_ev_unl.count_plus) {
									ppudata.spr_ev_unl.evaluate = TRUE;
								}
							}
						}
					} else {
						/*
						 * index_plus non superera'
						 * mai il valore 7.
						 */
						ppudata.spr_ev.index_plus = ppudata.spr_ev.count_plus;
					}
					/* passo al prossimo sprite */
					ppudata.spr_ev.timing = 0;
				} else {
					/* se non sono nel 7° continuo a esaminare lo sprite */
					ppudata.spr_ev.timing++;
				}
			}
/* ------------------------------------------- FASE 3 ---------------------------------------- */
		} else if (ppudata.spr_ev.phase == 3) {
			/* cicli pari */
			if (!(ppudata.spr_ev.timing & 0x01)) {
				/*
				 * se non ho ancora trovato il nono sprite devo
				 * aumentare sia byte_OAM che index. Questo
				 * e' un'errore della PPU che, invece di controllare
				 * la coordinata Y (byte 0), tratta il byte puntato
				 * da byte_OAM come se fosse la coordinata Y.
				 */
				if (!ppudata.spr_ev.evaluate) {
					/* incremento l'indice del byte da leggere */
					if (++ppudata.spr_ev.byte_OAM == 4) {
						ppudata.spr_ev.byte_OAM = 0;
					}
					/* in caso di overflow dell'indice degli sprite ... */
					if (++ppudata.spr_ev.index == 64) {
						/* ...azzero l'indice... */
						ppudata.spr_ev.index = 0;
						/* ...e passo alla fase 4... */
						ppudata.spr_ev.phase = 4;
						/*
						 * ...di cui questo stesso ciclo sara' il
						 * timing = 0, quindi il prossimo sara' l'1.
						 */
						ppudata.spr_ev.timing = 1;
						/* leggo la coordinata Y dello sprite 0 */
						ppudata.r2004.value = ppudata.oam.element[0][YC];
					} else {
						/*
						 * leggo dall'OAM il byte byte_OAM
						 * dell'elemento in esame.
						 */
						ppudata.r2004.value = ppudata.oam.element[ppudata.spr_ev.index][ppudata.spr_ev.byte_OAM];
						/* l'unica differenza nei cicli pari e' lo 0 */
						if (ppudata.spr_ev.timing == 0) {
							/*
							 * calcolo la differenza tra la posizione
							 * iniziale dello sprite e la posizione Y
							 * del pixel che sto renderizzando. Se e'
							 * inferiore a 8 o 16 (dipende dalla dimensione
							 * dello sprite) allora puo' essere disegnato.
							 */
							ppudata.spr_ev.range = ppudata.ppu.screen_y - ppudata.r2004.value;
							/*
							 * se sono nel range e lo sprite ha una
							 * posizione Y inferiore o uguale a 0xEF,
							 * vuol dire che sono al nono sprite.
							 */
							if ((ppudata.r2004.value <= 0xEF) && (ppudata.spr_ev.range < ppudata.r2000.size_spr)) {
								/* setto il bit 5 (overflow) del $2002 */
								ppudata.r2002.sprite_overflow = 0x20;
								ppudata.r2002.race.sprite_overflow = TRUE;
								/*
								 * devo esaminare i 3 byte
								 * consequenziali a questo.
								 */
								ppudata.spr_ev.evaluate = TRUE;
							}
						}
						/* continuo a esaminare lo sprite */
						ppudata.spr_ev.timing++;
					}
				/*
				 * se ho esaminato tutti i 4 byte del nono allora
				 * devo riprendere a esaminare le coordinate Y degli
				 * sprites.
				 */
				} else if (ppudata.spr_ev.evaluate == PPU_OVERFLOW_SPR) {
					/* in caso di overflow dell'indice degli sprite ... */
					if (++ppudata.spr_ev.index == 64) {
						/* ...azzero l'indice... */
						ppudata.spr_ev.index = 0;
						/* ...e passo alla fase 4... */
						ppudata.spr_ev.phase = 4;
					}
					/* leggo la coordinata Y dello sprite in esame */
					ppudata.r2004.value = ppudata.oam.element[ppudata.spr_ev.index][YC];
					/* continuo a esaminare lo sprite */
					ppudata.spr_ev.timing++;
				/*
				 * sto esaminando il nono sprite e devo farlo controllando
				 * i 3 byte dell'OAM successivi a quello che ho considerato
				 * come coordinata Y anche se questi finiscono nell'elemento
				 * dell'OAM successivo.
				 */
				} else if (ppudata.spr_ev.evaluate) {
					/* incremento l'indice del byte da leggere */
					if (++ppudata.spr_ev.byte_OAM == 4) {
						/*
						 * c'e' la possibilita' che finisca
						 * nell'elemento dell'OAM successivo.
						 */
						ppudata.spr_ev.byte_OAM = 0;
						/* in caso di overflow dell'indice degli sprite ... */
						if (++ppudata.spr_ev.index == 64) {
							/* ...azzero l'indice... */
							ppudata.spr_ev.index = 0;
							/* ...e passo alla fase 4... */
							ppudata.spr_ev.phase = 4;
							/*
							 * l'ho imposto a 0 perche' in uscita da
							 * questo if sara' aumentato.
							 */
							ppudata.spr_ev.timing = 0;
						}
					}
					/*
					 * leggo il byte successivo (in caso di passaggio
					 * alla fase 4 questo corrispondera' alla coordinata Y
					 * dello sprite 0
					 */
					ppudata.r2004.value = ppudata.oam.element[ppudata.spr_ev.index][ppudata.spr_ev.byte_OAM];
					/* continuo a esaminare lo sprite */
					ppudata.spr_ev.timing++;
				}
			/* cicli dispari */
			} else {
				/* leggo la coordinata Y dello sprite in esame */
				ppudata.r2004.value = ppudata.oam.ele_plus[ppudata.spr_ev.index_plus][YC];
				/* se sto esaminando il nono sprite... */
				if (ppudata.spr_ev.evaluate) {
					/* ...e sono nell'ultimo ciclo...*/
					if (ppudata.spr_ev.timing == 7) {
						/* ...indico la nuova modalita'... */
						if (ppudata.spr_ev.evaluate == PPU_OVERFLOW_SPR){
							/* ...passo al prossimo sprite.. */
							ppudata.spr_ev.timing = 0;
						} else {
							ppudata.spr_ev.evaluate = PPU_OVERFLOW_SPR;
							/* ...passo al prossimo sprite.. */
							ppudata.spr_ev.timing = 0;
							/*
							 * ...anche se devo riesaminare questo
							 * stesso sprite (ricordo che incremento
							 * index al timing == 0).
							 */
							ppudata.spr_ev.index--;
						}
					} else {
						/* ... e non sono nell'ultimo ciclo,
						 * continuo a esaminare lo sprite.
						 */
						ppudata.spr_ev.timing++;
					}
				} else {
					/*
					 * se non sono nel nono sprite
					 * allora passo al prossimo.
					 */
					ppudata.spr_ev.timing = 0;
				}
			}
/* ------------------------------------------- FASE 4 ---------------------------------------- */
		/* e' composto solo da due cicli (0 e 1) */
		} else if (ppudata.spr_ev.phase == 4) {
			/* ciclo 0 */
			if (ppudata.spr_ev.timing == 0) {
				/* in caso di overflow dell'indice degli sprite ... */
				if (++ppudata.spr_ev.index == 64) {
					/* ...azzero l'indice */
					ppudata.spr_ev.index = 0;
				}
				/* leggo la coordinata Y dello sprite OAM in esame */
				ppudata.r2004.value = ppudata.oam.element[ppudata.spr_ev.index][YC];
				/* passo al ciclo successivo */
				ppudata.spr_ev.timing = 1;
				/* ciclo 1 */
			} else {
				/* leggo la coordinata Y dello sprite in esame */
				ppudata.r2004.value = ppudata.oam.ele_plus[ppudata.spr_ev.index_plus][YC];
				/* passo al prossimo sprite */
				ppudata.spr_ev.timing = 0;
			}
		}
	}
}
