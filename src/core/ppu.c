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

#include <stdlib.h>
#include <string.h>
#include "cpu.h"
#include "info.h"
#include "clock.h"
#include "mem_map.h"
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
	ppu.rnd_adr = ((r2006.value & 0x0380) >> 4) | ((r2006.value & 0x001C) >> 2);\
	ppu.rnd_adr = 0x23C0 | (r2006.value & 0x0C00) | ppu.rnd_adr;\
	tmp = ppu_rd_mem(ppu.rnd_adr);\
	shift_at = ((r2006.value & 0x40) >> 4) | (r2006.value & 0x02);\
	tile_fetch.attrib = (tile_fetch.attrib >> 8) | (((tmp >> shift_at) & 0x03) << 8);\
}
#define fetch_lb(r2000bck, r2006vl)\
	ppu.rnd_adr = 0x2000 | ((r2006.race.ctrl ? r2006.race.value : r2006.value) & 0x0FFF);\
	ppu_bck_adr(r2000bck, r2006vl);\
	tile_fetch.l_byte = (tile_fetch.l_byte >> 8) | (inv_chr[ppu_rd_mem(ppu.bck_adr)] << 8);
#define fetch_hb()\
	ppu.rnd_adr = ppu.bck_adr | 0x0008;\
	tile_fetch.h_byte = (tile_fetch.h_byte >> 8) | (inv_chr[ppu_rd_mem(ppu.rnd_adr)] << 8);\
	((r2006.value & 0x1F) == 0x1F) ? (r2006.value ^= 0x041F) : (r2006.value++);
#define ppu_ticket()\
	ppu.cycles -= machine.ppu_divide;\
	ppu.frame_x++;\
	nmi.cpu_cycles_from_last_nmi++;\
	/* deve essere azzerato alla fine di ogni ciclo PPU */\
	r2006.changed_from_op = 0;
#define put_pixel(clr) screen.wr->line[ppu.screen_y][ppu.frame_x] = r2001.emphasis | clr;
#define put_emphasis(clr) put_pixel((mmap_palette.color[clr] & r2001.color_mode))
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
		if ((WORD)(ppu.frame_x - sp[a].x_C) < 8) {\
			/*\
			 * se il bit 2 del $2001 e' a 0 vuol dire\
			 * che e' abilitato il clipping degli sprite\
			 * (in poche parole non vengono disegnati i\
			 * primi 8 pixel dello screen).\
			 */\
			if ((ppu.frame_x >= 8) || r2001.spr_clipping) {\
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
	if (oam.elp[spenv.tmp_spr_plus][AT] & 0x40) {\
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
static BYTE ppu_alloc_screen_buffer(_screen_buffer *sb);
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
	0x09, 0x01, 0x00, 0x01, 0x00, 0x02, 0x02, 0x0D,
	0x08, 0x10, 0x08, 0x24, 0x00, 0x00, 0x04, 0x2C,
	0x09, 0x01, 0x34, 0x03, 0x00, 0x04, 0x00, 0x14,
	0x08, 0x3A, 0x00, 0x02, 0x00, 0x20, 0x2C, 0x08
};

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
_spr_evaluate spr_ev_unl;
_spr sprite_unl[56], sprite_plus_unl[56];
_tile tile_render, tile_fetch;
_ppu_sclines ppu_sclines;
_overclock overclock;
_ppu_alignment ppu_alignment;

void ppu_init(void) {
	memset(&screen, 0x00, sizeof(screen));
}
void ppu_quit(void) {
	/* libero la memoria riservata */
	BYTE a;

	for (a = 0; a < 2; a++) {
		_screen_buffer *sb = &screen.buff[a];

		if (sb->data) {
			free(sb->data);
			sb->data = NULL;
		}
	}

	if (screen.preview.data) {
		free(screen.preview.data);
		screen.preview.data = NULL;
	}
}

void ppu_tick(void) {
	/* aggiungo i cicli della cpu trascorsi */
	ppu.cycles = (SWORD)(ppu.cycles + machine.cpu_divide);

	while (ppu.cycles >= machine.ppu_divide) {
		r2002.race.sprite_overflow = FALSE;

		/* gestione della condizione di race del $2000 al dot 257 */
		if (r2000.race.ctrl) {
			r2000.race.ctrl = FALSE;
			ppu.tmp_vram = (ppu.tmp_vram & 0xF3FF) | ((r2000.race.value & 0x03) << 10);
		}

		/* gestione del delay del bit del grayscale */
		if (r2001.grayscale_bit.delay && (--r2001.grayscale_bit.delay == 0)) {
			r2001.color_mode = PPU_CM_GRAYSCALE;
		}

		// gestione della seconda scrittura del $2006
		if (r2006.second_write.delay && (--r2006.second_write.delay == 0)) {
			WORD old_r2006 = r2006.value;

			ppu.tmp_vram = r2006.second_write.value;

			if ((!ppu.vblank && r2001.visible && (ppu.screen_y < SCR_ROWS)) && (ppu.frame_y > ppu_sclines.vint)) {
				// split_scroll_test_v2.nes e split_scroll_delay.nes
				if (ppu.frame_x == 255) {
					ppu.tmp_vram &= r2006.value;
				}

				// condizione di race riscontrata in "scanline.nes" e
				// "Knight Rider (U) [!].nes" (i glitch grafici sotto la macchina
				// nell'introduzione sono presenti su hardware reale).
				// Anche "logo (E).nes" e "Ferrari - Grand Prix Challenge (U) [!].nes"
				// ne sono soggetti.
				if (ppu.frame_x < SCR_COLUMNS) {
					if ((ppu.pixel_tile >= 1) && (ppu.pixel_tile <= 3)) {
						r2006.race.ctrl = TRUE;
						r2006.race.value = (r2006.value & 0x00FF) | (ppu.tmp_vram & 0xFF00);
					}
				}

				// aggiorno l'r2006
				r2006.value = ppu.tmp_vram;

				// split_scroll_test_v2.nes e split_scroll_delay.nes
				if (ppu.frame_x == 254) {
					r2006_inc()
				}
			} else {
				// aggiorno l'r2006
				r2006.value = ppu.tmp_vram;
			}

			if (extcl_update_r2006) {
				// utilizzato dalle mappers :
				// MMC3
				// Rex (DBZ)
				// Taito (TC0690)
				// Tengen (Rambo)
				extcl_update_r2006(r2006.value, old_r2006);
			}
		}

		/* controllo se sono all'inizio della dummy line */
		if (ppu.frame_y == ppu_sclines.vint) {
			/*
			 * disabilito il vblank al ciclo 0 della scanline,
			 * a differenza dell'abilitazione del vblank che
			 * avviene al ciclo 341.
			 */
			if (ppu.frame_x == 0) {
				if (chinaersan2.enable) {
					memcpy(chinaersan2.ram, mmcpu.ram, 256);
				}
				ppu.screen_y = 0;
				/* setto a 0 il bit 5, 6 ed il 7 del $2002 */
				r2002.sprite_overflow = r2002.sprite0_hit = r2002.vblank = ppu.vblank = FALSE;
				// serve assolutamente per la corretta lettura delle coordinate del puntatore zapper
				if ((info.zapper_is_present == TRUE) && (fps_fast_forward_enabled() == FALSE)) {
					memset((BYTE *)screen.wr->data, 0, screen_size());
				}
			} else if ((ppu.frame_x == (SHORT_SLINE_CYCLES - 1)) && (machine.type == NTSC)) {
				/*
				 * nei frame NTSC dispari, la dummy line e' lunga 340
				 * cicli invece dei soliti 341. Visto che la lettura
				 * e la scrittura del registro $2001 (visible) potrebbe
				 * avvenire un ciclo CPU (3 PPU) prima della fine
				 * della dummy line, ne anticipo il controllo e
				 * l'eventuale modifica.
				 */
				ppu.sf.prev = ppu.sf.actual;
				ppu.sf.actual = FALSE;
				if (ppu.odd_frame) {
					if (r2001.bck_visible) {
						if ((r2001.race.ctrl == FALSE) || (r2001.race.value & 0x08)) {
							ppu.sline_cycles = SHORT_SLINE_CYCLES;
							ppu.sf.actual = TRUE;
						}
					} else {
						if ((r2001.race.ctrl == TRUE) && (r2001.race.value & 0x08)) {
							ppu.sline_cycles = SHORT_SLINE_CYCLES;
							ppu.sf.actual = TRUE;
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
		if (ppu.frame_x < SCR_COLUMNS) {
			/*
			 * controllo:
			 * 1) di non essere nel vblank
			 * 2) di star disegnando lo schermo perche'
			 *    subito dopo aver finito il rendering
			 *    la PPU rimane assolutamente ferma per una
			 *    scanline.
			 */
			if (ppu.vblank) {
				if ((machine.type == PAL) && (ppu.frame_y > 23)) {
					ppu_oam_evaluation();
				}
			} else if (ppu.screen_y < SCR_ROWS) {
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
				if (ppu.frame_y > ppu_sclines.vint) {
					/*
					 * controllo se background o sprites (basta
					 * solo uno dei due) siano visibili.
					 */
					if (r2001.visible) {
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
						if (ppu.pixel_tile == 0) {
							/*
							 * inizializzo i buffer che utilizzero'
							 * per renderizzare i prossimi 8 pixels.
							 */
							tile_render = tile_fetch;
							/* applico il fine X (cioe' lo scrolling) */
							tile_render.l_byte >>= ppu.fine_x;
							tile_render.h_byte >>= ppu.fine_x;
							/*
							 * visto che in questo buffer c'e' l'MSB dei
							 * 2 bit che, a loro volta sono i 2 bit LSB
							 * del colore, lo shifto di un bit a sinistra,
							 * in modo da ritrovarmelo in posizione per
							 * l'OR che faro' nel rendering del background.
							 */
							tile_render.h_byte <<= 1;
						} else if (ppu.pixel_tile == 1) {
							/* faccio il fetch del byte degli attributi */
							fetch_at()
						} else if (ppu.pixel_tile == 3) {
							/*
							 * faccio il fetch dei primi 8 bit che
							 * che compongono il tile (che hanno un
							 * peso minore rispetto ai secondi).
							 */
							fetch_lb(r2000.bpt_adr, (r2006.race.ctrl ? r2006.race.value : r2006.value))
						} else if (ppu.pixel_tile == 5) {
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
								extcl_after_rd_chr(ppu.bck_adr);
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
							if (ppu.frame_x == 253) {
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
						if (r2001.bck_visible) {
							/*
							 * se il bit 1 del $2001 e' a 0 vuol dire
							 * che e' abilitato il clipping del background
							 * (in poche parole non vengono disegnati i primi
							 * 8 pixel dello screen).
							 */
							if ((ppu.frame_x >= 8) || r2001.bck_clipping) {
								/* sto trattando un pixel del background */
								//flag_bg = TRUE;
								/* recupero i 2 bit LSB del pixel */
								color_bg = (tile_render.l_byte & 0x01) | (tile_render.h_byte & 0x02);
								/*
								 * shifto di un bit (leggi un pixel) i
								 * due bitmap buffers
								 */
								tile_render.l_byte >>= 1;
								tile_render.h_byte >>= 1;
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
									if ((ppu.pixel_tile + ppu.fine_x) < 8) {
										color_bg |= (tile_render.attrib << 2);
									} else {
										color_bg |= (tile_render.attrib >> 6);
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
						if (r2001.spr_visible) {
							BYTE a;

							examine_sprites(spr_ev, sprite, visible_spr, FALSE)

							if (cfg->unlimited_sprites == TRUE) {
								examine_sprites(spr_ev_unl, sprite_unl, visible_spr_unl, TRUE)
							}
						}
/* ------------------------------------ MULTIPLEXER ------------------------------------------ */
						if (!color_sp) {
							/*
							 * se il colore dello sprite e' trasparente,
							 * utilizzo quello del background.
							 */
							if (cfg->hide_background) {
								put_pixel(mmap_palette.color[0])
							} else {
								put_bg
							}
						} else if (!color_bg) {
							/*
							 * se invece e' il colore del background ad essere
							 * trasparente, utilizzo quello dello sprite.
							 */
							if (cfg->hide_sprites) {
								put_pixel(mmap_palette.color[0])
							} else {
								put_sp
							}
						} else {
							if (unlimited_spr == FALSE) {
								if (sprite[visible_spr].attrib & 0x20) {
									/*
									 * se non lo sono tutti e due, controllo la
									 * profondita' dello sprite e se e' settata su
									 * "dietro il background" utilizzo il colore
									 * dello sfondo.
									 */
									if (cfg->hide_background) {
										if (cfg->hide_sprites) {
											put_pixel(mmap_palette.color[0])
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
											put_pixel(mmap_palette.color[0])
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
								if (!r2002.sprite0_hit && !sprite[visible_spr].number && (ppu.frame_x != 255)) {
									r2002.sprite0_hit = 0x40;
								}
							} else {
								if (sprite_unl[visible_spr_unl].attrib & 0x20) {
									if (cfg->hide_background) {
										if (cfg->hide_sprites) {
											put_pixel(mmap_palette.color[0])
										} else {
											put_sp
										}
									} else {
										put_bg
									}
								} else {
									if (cfg->hide_sprites) {
										if (cfg->hide_background) {
											put_pixel(mmap_palette.color[0])
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
						put_pixel(mmap_palette.color[0])

						if ((r2006.value & 0xFF00) == 0x3F00) {
							/*
							 * se background e sprites non sono visibili
							 * e $2006 e' nel range 0x3F00/0x3FFF (nella
							 * paletta insomma) allora a video devo
							 * visualizzare il colore puntato dal registro.
							 */
							put_emphasis(r2006.value & 0x1F)
						}
					}
				}
				/* incremento in contatore dei pixel interni al tile */
				if (++ppu.pixel_tile > 7) {
					ppu.pixel_tile = 0;
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
		if (ppu.frame_x < 320) {
			if (!ppu.vblank && r2001.visible && (ppu.screen_y < SCR_ROWS)) {
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
				if (ppu.frame_x == 256) {
					spr_ev.timing = spr_ev.tmp_spr_plus = 0;
				}
				/* controllo se ci sono sprite per la (scanline+1) */
				if (spr_ev.tmp_spr_plus < spr_ev.count_plus) {
					switch (spr_ev.timing) {
						case 0:
							/*
							 * utilizzo spr_ev.timing come contatore di cicli per
							 * esaminare uno sprite ogni 8 cicli.
							 */
							ppu.rnd_adr = 0x2000 | (r2006.value & 0xFFF);
							ppu_spr_adr(spr_ev.tmp_spr_plus)
							get_sprites(ele_plus, spr_ev, sprite_plus, ppu.spr_adr)
							r2004.value = oam.ele_plus[spr_ev.tmp_spr_plus][YC];
							if (extcl_after_rd_chr) {
								/*
								 * utilizzato dalle mappers :
								 * MMC5
								 * MMC2/4
								 */
								extcl_after_rd_chr(ppu.spr_adr);
							}
							/* incremento il contatore del ciclo interno */
							spr_ev.timing++;
							break;
						case 2:
							ppu.rnd_adr = 0x2000 | (r2006.value & 0xFFF);
							r2004.value = oam.ele_plus[spr_ev.tmp_spr_plus][spr_ev.timing];
							/* incremento il contatore del ciclo interno */
							spr_ev.timing++;
							break;
						case 1:
						case 3:
							r2004.value = oam.ele_plus[spr_ev.tmp_spr_plus][spr_ev.timing];
							/* incremento il contatore del ciclo interno */
							spr_ev.timing++;
							break;
						case 4:
							ppu.rnd_adr = ppu.spr_adr;
							r2004.value = oam.ele_plus[spr_ev.tmp_spr_plus][XC];
							/* incremento il contatore del ciclo interno */
							spr_ev.timing++;
							break;
						case 5:
							r2004.value = oam.ele_plus[spr_ev.tmp_spr_plus][XC];
							/* incremento il contatore del ciclo interno */
							spr_ev.timing++;
							break;
						case 6:
							ppu.rnd_adr = ppu.spr_adr | 0x0008;
							r2004.value = oam.ele_plus[spr_ev.tmp_spr_plus][XC];
							/* incremento il contatore del ciclo interno */
							spr_ev.timing++;
							break;
						case 7:
							r2004.value = oam.ele_plus[spr_ev.tmp_spr_plus][XC];
							/* passo al prossimo sprite */
							spr_ev.timing = 0;
							/* incremento l'indice temporaneo degli sprites */
							if (++spr_ev.tmp_spr_plus == 8) {
								// unlimited sprites
								if ((cfg->unlimited_sprites == TRUE) && (spr_ev_unl.evaluate == TRUE)) {
									for (spr_ev_unl.tmp_spr_plus = 0;
										spr_ev_unl.tmp_spr_plus < spr_ev_unl.count_plus;
										spr_ev_unl.tmp_spr_plus++) {
										WORD spr_adr;

										_ppu_spr_adr(spr_ev_unl.tmp_spr_plus, ele_plus_unl, sprite_plus_unl, spr_adr)
										get_sprites(ele_plus_unl, spr_ev_unl, sprite_plus_unl, spr_adr)
									}
									spr_ev_unl.evaluate = FALSE;
								}
							}
							break;
					 }
				} else {
					if (spr_ev.timing == 0) {
						r2004.value = oam.element[63][YC];
						spr_ev.timing++;
					} else if (spr_ev.timing < 7) {
						r2004.value = 0xFF;
						spr_ev.timing++;
					} else {
						spr_ev.timing = 0;
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
				if ((ppu.frame_x == 319) && (ppu.screen_y == 238)) {
					r2003.value = 0;
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
		if (!ppu.vblank && (r2001.visible || r2001.race.ctrl) && (ppu.screen_y < SCR_ROWS)) {
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
			switch (ppu.frame_x) {
				case 323:
					if (ppu.frame_y == ppu_sclines.vint) {
						/*
						 * all'inizio di ogni frame reinizializzo
						 * l'indirizzo della PPU.
						 */
						r2006.value = ppu.tmp_vram;
					}
					fetch_at()
					break;
				case 331:
					fetch_at()
					break;
				case 325:
				case 333:
					fetch_lb(r2000.bpt_adr, r2006.value)
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
						extcl_after_rd_chr(ppu.bck_adr);
					}
					break;
				case 337:
				case 339:
					ppu.rnd_adr = 0x2000 | (r2006.value & 0x0FFF);
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
		if (ppu.frame_x < ppu.sline_cycles) {
			if (spr_ev.count_plus) {
				r2004.value = oam.ele_plus[0][YC];
			} else {
				r2004.value = oam.element[63][YC];
			}
			ppu_ticket()
			/*
			 * se e' iniziato il ciclo 341, vuol dire
			 * che in realta' e' iniziato il ciclo 0
			 * della scanline successiva.
			 */
			if (ppu.frame_x != ppu.sline_cycles) {
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
		if ((ppu.frame_y >= ppu_sclines.vint) && (ppu.screen_y < SCR_ROWS)) {
			BYTE a;

			/* verifico di non trattare la dummy line */
			if (ppu.frame_y > ppu_sclines.vint) {
				/* incremento il contatore delle scanline renderizzate */
				ppu.screen_y++;
				if ((ppu.screen_y == SCR_ROWS) && (info.no_ppu_draw_screen == 0)) {
					gfx_draw_screen();
				}
			}
			/*
			 * l'indice degli sprite per la (scanline+1)
			 * diventa quello attuale (visto che sto per
			 * incrementare la scanline).
			 */
			spr_ev.count = spr_ev.count_plus;
			/* azzero l'indice per la (scanline+1) */
			spr_ev.count_plus = 0;
			/*
			 * sposto il buffer degli sprites della scanline
			 * successiva (scanline+1) nel buffer di quella
			 * che sto per trattare.
			 */
			for (a = spr_ev.count; a--;) {
				sprite[a].y_C = oam.ele_plus[a][YC];
				sprite[a].tile = oam.ele_plus[a][TL];
				sprite[a].attrib = oam.ele_plus[a][AT];
				sprite[a].x_C = oam.ele_plus[a][XC];
				sprite[a].number = sprite_plus[a].number;
				sprite[a].flip_v = sprite_plus[a].flip_v;
				sprite[a].l_byte = sprite_plus[a].l_byte;
				sprite[a].h_byte = sprite_plus[a].h_byte;
			}
			// unlimited sprites
			if (cfg->unlimited_sprites == TRUE) {
				spr_ev_unl.count = spr_ev_unl.count_plus;
				/* azzero l'indice per la (scanline+1) */
				spr_ev_unl.count_plus = 0;
				/*
				 * sposto il buffer degli sprites della scanline
				 * successiva (scanline+1) nel buffer di quella
				 * che sto per trattare.
				 */
				for (a = spr_ev_unl.count; a--;) {
					sprite_unl[a].y_C = oam.ele_plus_unl[a][YC];
					sprite_unl[a].tile = oam.ele_plus_unl[a][TL];
					sprite_unl[a].attrib = oam.ele_plus_unl[a][AT];
					sprite_unl[a].x_C = oam.ele_plus_unl[a][XC];
					sprite_unl[a].number = sprite_plus_unl[a].number;
					sprite_unl[a].flip_v = sprite_plus_unl[a].flip_v;
					sprite_unl[a].l_byte = sprite_plus_unl[a].l_byte;
					sprite_unl[a].h_byte = sprite_plus_unl[a].h_byte;
				}
			}
		}

		/*
		 * incremento frameY, reinizializzo
		 * slineCycles ed e' estremamente importante
		 * che lo faccia esattamente qui, cosi'
		 * come e' importante che l'azzeramento del
		 * ppu.framex lo faccia dopo il settaggio dell'nmi.
		 */
		ppu.frame_y++;
		ppu.sline_cycles = SLINE_CYCLES;

		/* controllo se ho completato il frame */
		if (ppu.frame_y >= ppu_sclines.total) {
			// aggiorno il numero delle scanlines
			ppu_overclock_update()
			// azzero il flag del DMC dell'overclock
			overclock.DMC_in_use = FALSE;
			/* incremento il contatore ppu dei frames */
			ppu.frames++;
			/* azzero frame_y */
			ppu.frame_y = 0;
			/* setto il flag che indica che un frame e' stato completato */
			info.frame_status = FRAME_FINISHED;
			/* e' un frame dispari? */
			ppu.odd_frame = !ppu.odd_frame;
			/* abilito il vblank */
			r2002.vblank = 0x80;
			ppu.vblank = TRUE;
			if ((ppu.frames == 1) && info.r2002_jump_first_vblank) {
				r2002.vblank = 0x00;
			}
			/*
			 * quando il bit 7 del $2002 e il bit 7
			 * del $2000 sono a 1 devo generare un NMI.
			 */
			if (r2000.nmi_enable) {
				nmi.high = TRUE;
				nmi.frame_x = ppu.frame_x;
				/* azzero i numeri di cicli dall'nmi */
				nmi.cpu_cycles_from_last_nmi = 0;
			}
		}

		// controllo se sono nelle scanlines extra
		ppu_overclock_control()

		/*
		 * azzero frameX, ed e' estremamente
		 * importante che lo faccia esattamente
		 * dopo il settaggio dell'nmi.
		 */
		ppu.frame_x = 0;
		/* deve essere azzerato alla fine di ogni ciclo PPU */
		r2006.changed_from_op = 0;
	}
}
BYTE ppu_turn_on(void) {
	// nel primo frame l'overclocking e' sempre disabilitato
	overclock.DMC_in_use = TRUE;
	ppu_overclock(FALSE);

	if (info.reset >= HARD) {
		memset(&ppu, 0x00, sizeof(ppu));
		memset(&ppu_openbus, 0x00, sizeof(ppu_openbus));
		memset(&r2000, 0x00, sizeof(r2000));
		memset(&r2001, 0x00, sizeof(r2001));
		memset(&r2002, 0x00, sizeof(r2002));
		memset(&r2003, 0x00, sizeof(r2003));
		memset(&r2004, 0x00, sizeof(r2004));
		memset(&r2006, 0x00, sizeof(r2006));
		memset(&r2007, 0x00, sizeof(r2007));
		memset(&spr_ev, 0x00, sizeof(spr_ev));
		memset(&sprite, 0x00, sizeof(sprite));
		memset(&sprite_plus, 0x00, sizeof(sprite_plus));
		memset(&spr_ev_unl, 0x00, sizeof(spr_ev_unl));
		memset(&sprite_unl, 0x00, sizeof(sprite_unl));
		memset(&sprite_plus_unl, 0x00, sizeof(sprite_plus_unl));
		memset(&tile_render, 0x00, sizeof(tile_render));
		memset(&tile_fetch, 0x00, sizeof(tile_fetch));
		/*
		 * "Time Lord (U) [!].nes"
		 * funziona correttamente (altrimenti avviato il gioco
		 * la parte di sotto si sporca e non appaiono sprites).
		 */
		ppu.frame_y = ppu_sclines.vint + 1;
		ppu.sline_cycles = SLINE_CYCLES;
		r2000.r2006_inc = 1;
		r2000.size_spr = 8;
		r2001.color_mode = PPU_CM_NORMAL;

		/* riservo una zona di memoria per lo screen */
		if ((info.reset == CHANGE_ROM) || (info.reset == POWER_UP)) {
			BYTE a;

			screen.rd = &screen.buff[0];
			screen.wr = &screen.buff[1];
			screen.last_completed_wr = screen.wr;

			for (a = 0; a < 2; a++) {
				if (ppu_alloc_screen_buffer(&screen.buff[a]) == EXIT_ERROR) {
					return (EXIT_ERROR);
				}
			}
			if (ppu_alloc_screen_buffer(&screen.preview) == EXIT_ERROR) {
				return (EXIT_ERROR);
			}
			/*
			 * tabella di indici che puntano ad ogni
			 * elemento dell'OAM (4 bytes ciascuno).
			 */
			for (a = 0; a < 64; ++a) {
				oam.element[a] = &oam.data[(a * 4)];
			}
			for (a = 0; a < 8; ++a) {
				oam.ele_plus[a] = &oam.plus[(a * 4)];
			}
			for (a = 0; a < 56; ++a) {
				oam.ele_plus_unl[a] = &oam.plus_unl[(a * 4)];
			}
			ppu_alignment_reset();
		}
		/* reinizializzazione completa della PPU */
		{
			int a, x, y;

			/* inizializzo lo screen */
			for (a = 0; a < 2; a++) {
				_screen_buffer *sb = &screen.buff[a];

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
			memset(oam.data, 0xFF, sizeof(oam.data));
			memset(oam.plus, 0xFF, sizeof(oam.plus));
			memset(oam.plus_unl, 0xFF, sizeof(oam.plus_unl));
			/* inizializzo nametables */
			memset(ntbl.data, 0x00, sizeof(ntbl.data));
			/* e paletta dei colori */
			memcpy(mmap_palette.color, palette_init, sizeof(mmap_palette.color));
		}
		ppu_alignment_init();
	} else {
		memset(&r2000, 0x00, sizeof(r2000));
		memset(&r2001, 0x00, sizeof(r2001));
		memset(&r2002, 0x00, sizeof(r2002));
		memset(&r2007, 0x00, sizeof(r2007));

		ppu.frame_x = ppu.screen_y = ppu.pixel_tile = 0;
		ppu.frame_y = ppu_sclines.vint + 1;
		ppu.tmp_vram = ppu.fine_x = 0;
		ppu.spr_adr = ppu.bck_adr = 0;
		ppu.sline_cycles = SLINE_CYCLES;
		ppu.odd_frame = 0;
		ppu.cycles = 0;
		r2000.r2006_inc = 1;
		r2000.size_spr = 8;
		r2001.color_mode = PPU_CM_NORMAL;
	}

	return (EXIT_OK);
}
void ppu_overclock(BYTE reset_dmc_in_use) {
	if (reset_dmc_in_use) {
		overclock.DMC_in_use = FALSE;
	}

	overclock.sclines.vb = 0;
	overclock.sclines.pr = 0;

	if (cfg->ppu_overclock) {
		overclock.sclines.vb = cfg->extra_vb_scanlines;
		overclock.sclines.pr = cfg->extra_pr_scanlines;
	}

	overclock.sclines.total = overclock.sclines.vb + overclock.sclines.pr;
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

	ppu.cycles = (SWORD)((ppu_alignment.cpu + (-ppu_alignment.ppu + 1)) % machine.cpu_divide);

	if (cfg->ppu_alignment == PPU_ALIGMENT_INC_AT_RESET) {
		if ((ppu_alignment.count.cpu = (ppu_alignment.count.cpu + 1) % machine.cpu_divide) == 0) {
			ppu_alignment.count.ppu = (ppu_alignment.count.ppu + 1) % machine.ppu_divide;
		}
	}

	if (gui.start == TRUE) {
		gui_update_status_bar();
	}
}
static BYTE ppu_alloc_screen_buffer(_screen_buffer *sb) {
	int b;

	sb->ready = FALSE;
	sb->frame = 0;

	if (sb->data) {
		free(sb->data);
	}

	if (!(sb->data = (WORD *)malloc(screen_size()))) {
		fprintf(stderr, "Out of memory\n");
		return (EXIT_ERROR);
	}
	/*
	 * creo una tabella di indici che puntano
	 * all'inizio di ogni linea dello screen.
	 */
	for (b = 0; b < SCR_ROWS; b++) {
		sb->line[b] = (WORD *)(sb->data + (b * SCR_COLUMNS));
	}

	return (EXIT_OK);
}
INLINE static void ppu_oam_evaluation(void) {
/* ------------------------------- CONTROLLO SPRITE SCANLINE+1 ------------------------------- */
	if (ppu.frame_x < 64) {
		r2004.value = 0xFF;
		/* inizializzo le varibili per il ciclo 64 */
		if (ppu.frame_x == 63) {
			/*
			 * inizializzo i vari indici
			 *
			 * Note:
			 * imposto index al suo valore massimo
			 * solo perche' nel 64° ciclo, come prima
			 * cosa lo incremento azzerandolo.
			 */
			spr_ev.timing = 0;
			spr_ev.real = 0;
			spr_ev.index = 0xFF;
			/* la fase 1 e 2 corrispondono */
			spr_ev.phase = 2;
		}
	} else if (ppu.frame_x < 256) {
/* --------------------------------------- FASE 1 E 2 ---------------------------------------- */
		/*
		 * in questa fase esamino e salvo i primi 8 sprites
		 * che trovo essere nel range. Trovato l'ottavo passo
		 * alla fase 3. Se invece esamino tutti e 64 gli sprites
		 * dell'OAM, passo alla fase 4.
		 */
		if (spr_ev.phase == 2) {
			if (spr_ev.timing == 0) {
				/* in caso di overflow dell'indice degli sprite ... */
				if (++spr_ev.index == 64) {
					/* ...azzero l'indice... */
					spr_ev.index = spr_ev.real = 0;
					/* ...passo alla fase 4... */
					spr_ev.phase = 4;
					/*
					 * ...di cui questo stesso ciclo sara' il
					 * timing = 0, quindi il prossimo sara' l'1.
					 */
					spr_ev.timing = 1;
					/* leggo la coordinata Y dello sprite 0 */
					r2004.value = oam.element[0][YC];
					/*
					 * We've since discovered that not only are
					 * sprites 0 and 1 temporarily replaced with
					 * the pair that OAMADDR&0xF8 points to, but
					 * it's permanent: the pair that OAMADDR & 0xF8
					 * points to is copied to the first 8 bytes of
					 * OAM when rendering starts.
					 * The difference should only show up with a game
					 * that doesn't use DMA every frame.
					 */
					{
						static BYTE i;

						for (i = 8; i--;) {
							oam.data[i] = oam.data[(r2003.value & 0xF8) + i];
						}
					}
				} else {
					spr_ev.real = spr_ev.index;
					/* leggo dall'OAM il byte 0 dell'elemento in esame */
					r2004.value = oam.element[spr_ev.real][YC];
					/*
					 * calcolo la differenza tra la posizione
					 * iniziale dello sprite e la posizione Y
					 * del pixel che sto renderizzando. Se e'
					 * inferiore a 8 o 16 (dipende dalla dimensione
					 * dello sprite) allora puo' essere disegnato.
					 */
					spr_ev.range = ppu.screen_y - r2004.value;

					spr_ev.evaluate = FALSE;
					/*
					 * se sono nel range e lo sprite ha una
					 * posizione Y inferiore o uguale a 0xEF,
					 * lo esamino.
					 */
					if ((spr_ev.count_plus < 8) && (r2004.value <= 0xEF) && (spr_ev.range < r2000.size_spr)) {
						spr_ev.evaluate = TRUE;
					}
					/* incremento timing */
					spr_ev.timing++;
				}
			} else if (spr_ev.timing == 1) {
				/*
				 * esamino lo sprites e se necessario
				 * inizio a memorizzare le informazioni.
				 */
				if (spr_ev.evaluate) {
					/*
					 * memorizzo la prima parte delle
					 * informazione dello sprite nel buffer.
					 */
					oam.ele_plus[spr_ev.count_plus][YC] = r2004.value;
					sprite_plus[spr_ev.count_plus].number = spr_ev.index;
					sprite_plus[spr_ev.count_plus].flip_v = spr_ev.range;
					/* continuo a trattare questo sprite */
					spr_ev.timing++;
				} else {
					/* passo al prossimo sprite */
					spr_ev.timing = 0;
				}
			/* tratto i cicli pari */
			} else if (!(spr_ev.timing & 0x01)) {
				/* leggo il prossimo byte dell'OAM */
				r2004.value = oam.element[spr_ev.real][spr_ev.timing >> 1];
				/* passo al ciclo successivo */
				spr_ev.timing++;
			/* tratto i cicli dispari */
			} else {
				/* memorizzo il valore letto nel ciclo prima */
				oam.ele_plus[spr_ev.count_plus][spr_ev.timing >> 1] = r2004.value;
				/* l'unico ciclo diverso e' l'ultimo */
				if (spr_ev.timing == 7) {
					/*
					 * se ho gia' trovato 8 sprites allora
					 * devo avviare la fase 3.
					 */
					if (++spr_ev.count_plus == 8) {
						spr_ev.phase = 3;
						/*
						 * inizilizzo le variabili che
						 * mi serviranno. byte_OAM = 3
						 * perche' verra' aumentata e quindi
						 * riportata a 0 nel primo ciclo
						 * della fase 3.
						 */
						spr_ev.evaluate = FALSE;
						spr_ev.byte_OAM = 3;
						spr_ev.index_plus = 0;

						// unlimited sprites
						if (cfg->unlimited_sprites == TRUE) {
							// https://wiki.nesdev.com/w/index.php/Sprite_overflow_games (Use of excess sprites for masking effects)
							// https://github.com/SourMesen/Mesen/issues/188
							// start - thx to Sour
							BYTE unlimited_sprites = TRUE;

							if (cfg->unlimited_sprites_auto == TRUE) {
								BYTE count = 0,  max_count = 0;
								WORD last_position = 0xFFFF;
								int i;

								for (i = 0; i < 64; i++) {
									BYTE y = oam.element[i][YC];
									WORD range = ppu.screen_y - y;

									if ((y <= 0xEF) && (range < r2000.size_spr)) {
										WORD position = (y << 8) | oam.element[i][XC];

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
								BYTE t2004;

								spr_ev_unl.index = spr_ev.index + 1;
								spr_ev_unl.count_plus = 0;

								for (; spr_ev_unl.index < 64; spr_ev_unl.index++) {
									t2004 = oam.element[spr_ev_unl.index][YC];

									spr_ev_unl.range = ppu.screen_y - t2004;

									if ((t2004 <= 0xEF) && (spr_ev_unl.range < r2000.size_spr)) {
										oam.ele_plus_unl[spr_ev_unl.count_plus][YC] = oam.element[spr_ev_unl.index][YC];
										oam.ele_plus_unl[spr_ev_unl.count_plus][TL] = oam.element[spr_ev_unl.index][TL];
										oam.ele_plus_unl[spr_ev_unl.count_plus][AT] = oam.element[spr_ev_unl.index][AT];
										oam.ele_plus_unl[spr_ev_unl.count_plus][XC] = oam.element[spr_ev_unl.index][XC];
										sprite_plus_unl[spr_ev_unl.count_plus].number = spr_ev_unl.index;
										sprite_plus_unl[spr_ev_unl.count_plus].flip_v = spr_ev_unl.range;
										spr_ev_unl.count_plus++;
									}
								}
								if (spr_ev_unl.count_plus) {
									spr_ev_unl.evaluate = TRUE;
								}
							}
						}
					} else {
						/*
						 * index_plus non superera'
						 * mai il valore 7.
						 */
						spr_ev.index_plus = spr_ev.count_plus;
					}
					/* passo al prossimo sprite */
					spr_ev.timing = 0;
				} else {
					/* se non sono nel 7° continuo a esaminare lo sprite */
					spr_ev.timing++;
				}
			}
/* ------------------------------------------- FASE 3 ---------------------------------------- */
		} else if (spr_ev.phase == 3) {
			/* cicli pari */
			if (!(spr_ev.timing & 0x01)) {
				/*
				 * se non ho ancora trovato il nono sprite devo
				 * aumentare sia byte_OAM che index. Questo
				 * e' un'errore della PPU che, invece di controllare
				 * la coordinata Y (byte 0), tratta il byte puntato
				 * da byte_OAM come se fosse la coordinata Y.
				 */
				if (spr_ev.evaluate == FALSE) {
					/* incremento l'indice del byte da leggere */
					if (++spr_ev.byte_OAM == 4) {
						spr_ev.byte_OAM = 0;
					}
					/* in caso di overflow dell'indice degli sprite ... */
					if (++spr_ev.index == 64) {
						/* ...azzero l'indice... */
						spr_ev.index = 0;
						/* ...e passo alla fase 4... */
						spr_ev.phase = 4;
						/*
						 * ...di cui questo stesso ciclo sara' il
						 * timing = 0, quindi il prossimo sara' l'1.
						 */
						spr_ev.timing = 1;
						/* leggo la coordinata Y dello sprite 0 */
						r2004.value = oam.element[0][YC];
					} else {
						/*
						 * leggo dall'OAM il byte byte_OAM
						 * dell'elemento in esame.
						 */
						r2004.value = oam.element[spr_ev.index][spr_ev.byte_OAM];
						/* l'unica differenza nei cicli pari e' lo 0 */
						if (spr_ev.timing == 0) {
							/*
							 * calcolo la differenza tra la posizione
							 * iniziale dello sprite e la posizione Y
							 * del pixel che sto renderizzando. Se e'
							 * inferiore a 8 o 16 (dipende dalla dimensione
							 * dello sprite) allora puo' essere disegnato.
							 */
							spr_ev.range = ppu.screen_y - r2004.value;
							/*
							 * se sono nel range e lo sprite ha una
							 * posizione Y inferiore o uguale a 0xEF,
							 * vuol dire che sono al nono sprite.
							 */
							if ((r2004.value <= 0xEF) && (spr_ev.range < r2000.size_spr)) {
								/* setto il bit 5 (overflow) del $2002 */
								r2002.sprite_overflow = 0x20;
								r2002.race.sprite_overflow = TRUE;
								/*
								 * devo esaminare i 3 byte
								 * consequenziali a questo.
								 */
								spr_ev.evaluate = TRUE;
							}
						}
						/* continuo a esaminare lo sprite */
						spr_ev.timing++;
					}
				/*
				 * se ho esaminato tutti i 4 byte del nono allora
				 * devo riprendere a esaminare le coordinate Y degli
				 * sprites.
				 */
				} else if (spr_ev.evaluate == PPU_OVERFLOW_SPR) {
					/* in caso di overflow dell'indice degli sprite ... */
					if (++spr_ev.index == 64) {
						/* ...azzero l'indice... */
						spr_ev.index = 0;
						/* ...e passo alla fase 4... */
						spr_ev.phase = 4;
					}
					/* leggo la coordinata Y dello sprite in esame */
					r2004.value = oam.element[spr_ev.index][YC];
					/* continuo a esaminare lo sprite */
					spr_ev.timing++;
				/*
				 * sto esaminando il nono sprite e devo farlo controllando
				 * i 3 byte dell'OAM successivi a quello che ho considerato
				 * come coordinata Y anche se questi finiscono nell'elemento
				 * dell'OAM successivo.
				 */
				} else if (spr_ev.evaluate == TRUE) {
					/* incremento l'indice del byte da leggere */
					if (++spr_ev.byte_OAM == 4) {
						/*
						 * c'e' la possibilita' che finisca
						 * nell'elemento dell'OAM successivo.
						 */
						spr_ev.byte_OAM = 0;
						/* in caso di overflow dell'indice degli sprite ... */
						if (++spr_ev.index == 64) {
							/* ...azzero l'indice... */
							spr_ev.index = 0;
							/* ...e passo alla fase 4... */
							spr_ev.phase = 4;
							/*
							 * l'ho imposto a 0 perche' in uscita da
							 * questo if sara' aumentato.
							 */
							spr_ev.timing = 0;
						}
					}
					/*
					 * leggo il byte successivo (in caso di passaggio
					 * alla fase 4 questo corrispondera' alla coordinata Y
					 * dello sprite 0
					 */
					r2004.value = oam.element[spr_ev.index][spr_ev.byte_OAM];
					/* continuo a esaminare lo sprite */
					spr_ev.timing++;
				}
			/* cicli dispari */
			} else {
				/* leggo la coordinata Y dello sprite in esame */
				r2004.value = oam.ele_plus[spr_ev.index_plus][YC];
				/* se sto esaminando il nono sprite... */
				if (spr_ev.evaluate == TRUE) {
					/* ...e sono nell'ultimo ciclo...*/
					if (spr_ev.timing == 7) {
						/* ...indico la nuova modalita'... */
						spr_ev.evaluate = PPU_OVERFLOW_SPR;
						/* ...passo al prossimo sprite.. */
						spr_ev.timing = 0;
						/*
						 * ...anche se devo riesaminare questo
						 * stesso sprite (ricordo che incremento
						 * index al timing = 0).
						 */
						spr_ev.index--;
					} else {
						/* ... e non sono nell'ultimo ciclo,
						 * continuo a esaminare lo sprite.
						 */
						spr_ev.timing++;
					}
				} else {
					/*
					 * se non sono nel nono sprite
					 * allora passo al prossimo.
					 */
					spr_ev.timing = 0;
				}
			}
/* ------------------------------------------- FASE 4 ---------------------------------------- */
		/* e' composto solo da due cicli (0 e 1) */
		} else if (spr_ev.phase == 4) {
			/* ciclo 0 */
			if (spr_ev.timing == 0) {
				/* in caso di overflow dell'indice degli sprite ... */
				if (++spr_ev.index == 64) {
					/* ...azzero l'indice */
					spr_ev.index = 0;
				}
				/* leggo la coordinata Y dello sprite OAM in esame */
				r2004.value = oam.element[spr_ev.index][YC];
				/* passo al ciclo successivo */
				spr_ev.timing = 1;
				/* ciclo 1 */
			} else {
				/* leggo la coordinata Y dello sprite in esame */
				r2004.value = oam.ele_plus[spr_ev.index_plus][YC];
				/* passo al prossimo sprite */
				spr_ev.timing = 0;
			}
		}
	}
}
