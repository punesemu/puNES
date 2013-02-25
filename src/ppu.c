/*
 * ppu.c
 *
 *  Created on: 28/mar/2010
 *      Author: fhorse
 */

#include <string.h>
#include <stdio.h>
#include "cpu6502.h"
#include "clock.h"
#include "memmap.h"
#include "ppuinline.h"
#include "sdlgfx.h"
#include "mappers.h"
#include "irqA12.h"
#include "irql2f.h"

#define SLCYCLES 341
#define SHORTSLCYCLES 340

#define OVERFLOWSPR 3

#define FETCHAT()\
{\
	BYTE shiftAT;\
	WORD tmp;\
	tmp = ((r2006.value & 0x0380) >> 4) | ((r2006.value & 0x001C) >> 2);\
	tmp = ppuRdMem(0x23C0 | (r2006.value & 0x0C00) | tmp);\
	shiftAT = ((r2006.value & 0x40) >> 4) | (r2006.value & 0x02);\
	tileFetch.attrib = (tileFetch.attrib >> 8)\
		| (((tmp >> shiftAT) & 0x03) << 8);\
}
#define FETCHLB()\
{\
	ppuBckAdr();\
	tileFetch.lByte = (tileFetch.lByte >> 8)\
		| (invCHR[ppuRdMem(ppu.bckAdr)] << 8);\
}
#define FETCHHB()\
{\
	tileFetch.hByte = (tileFetch.hByte >> 8)\
		| (invCHR[ppuRdMem(ppu.bckAdr | 0x0008)] << 8);\
	((r2006.value & 0x1F) == 0x1F) ? (r2006.value ^= 0x041F) : (r2006.value++);\
}
#define PPUTICKET()\
{\
	ppu.cycles -= machine.ppuDivide;\
	ppu.frameX++;\
	r2006.changedFromOP = 0;\
	continue;\
}
#define PUTPIXEL(clr)\
{\
	WORD pixel = r2001.emphasis | clr;\
	screen.line[ppu.screenY][ppu.frameX] = pixel;\
}
#define PUTEMPHASIS(clr)\
	PUTPIXEL((palette.color[clr] & r2001.colorMode))
#define PUTBG PUTEMPHASIS(colorBG)
#define PUTSP PUTEMPHASIS(colorSP | 0x10)

static const BYTE invCHR[256] = {
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

void ppuTick(WORD cycles_cpu) {
	/* aggiungo i cicli della cpu trascorsi */
	ppu.cycles += (cycles_cpu * machine.cpuDivide);

	while (ppu.cycles >= machine.ppuDivide) {
		/* controllo se sono all'inizio della dummy line */
		if (ppu.frameY == machine.vintLines) {
			/*
			 * disabilito il vblank al ciclo 0 della scanline,
			 * a differenza dell'abilitazione del vblank che
			 * avviene al ciclo 341.
			 */
			if (ppu.frameX == 0) {
				ppu.screenY = 0;
				/* setto a 0 il bit 5, 6 ed il 7 del $2002 */
				r2002.spriteOverflow = r2002.sprite0Hit = r2002.vblank = FALSE;
			} else if (machine.type == NTSC) {
				/*
				 * nei frame NTSC dispari, la dummy line e' lunga 340
				 * cicli invece dei soliti 341. Visto che la lettura
				 * e la scrittura del registro $2001 (visible) potrebbe
				 * avvenire un ciclo CPU (3 PPU) prima della fine
				 * della dummy line, ne anticipo il controllo e
				 * l'eventuale modifica.
				 */
				if ((ppu.frameX == (SHORTSLCYCLES - machine.ppuFor1CPU)) && ppu.oddFrame
						&& r2001.bckVisible) {
					ppu.slineCycles = SHORTSLCYCLES;
				}
			}
		}

		if (extclPPU000to34x) {
			/*
			 * utilizzato dalle mappers :
			 * MMC3
			 * Taito
			 */
			extclPPU000to34x();
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
		if (ppu.frameX < SCRROWS) {
			/*
			 * controllo:
			 * 1) di non essere nel vblank
			 * 2) di star disegnando lo schermo perche'
			 *    subito dopo aver finito il rendering
			 *    la PPU rimane assolutamente ferma per una
			 *    scanline.
			 */
			if (!r2002.vblank && (ppu.screenY < SCRLINES)) {
				if (extclPPU000to255) {
					/*
					 * utilizzato dalle mappers :
					 * MMC3
					 * Taito
					 * Tengen
					 */
					extclPPU000to255();
				}
				/* controllo di non essere nella dummy line */
				if (ppu.frameY > machine.vintLines) {
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
						BYTE colorBG = 0, colorSP = 0, visibleSpr = 0;
						BYTE flagSP = FALSE, flagBG = FALSE;
/* -------------------------- FETCH DATI PER TILE SUCCESSIVO --------------------------------- */
						/*
						 * a seconda del pixel che sto renderizzando
						 * (quindi in base al ciclo PPU) faccio
						 * cio' che serve.
						 */
						if (ppu.pixelTile == 0) {
							/*
							 * inizializzo i buffer che utilizzero'
							 * per renderizzare i prossimi 8 pixels.
							 */
							tileRender = tileFetch;
							/* applico il fine X (cioe' lo scrolling) */
							tileRender.lByte >>= ppu.fineX;
							tileRender.hByte >>= ppu.fineX;
							/*
							 * visto che in questo buffer c'e' l'MSB dei
							 * 2 bit che, a loro volta sono i 2 bit LSB
							 * del colore, lo shifto di un bit a sinistra,
							 * in modo da ritrovarmelo in posizione per
							 * l'OR che faro' nel rendering del background.
							 */
							tileRender.hByte <<= 1;
						} else if (ppu.pixelTile == 1) {
							/* faccio il fetch del byte degli attributi */
							FETCHAT()
						} else if (ppu.pixelTile == 3) {
							/*
							 * faccio il fetch dei primi 8 bit che
							 * che compongono il tile (che hanno un
							 * peso minore ai secondi).
							 */
							FETCHLB()
						} else if (ppu.pixelTile == 5) {
							/*
							 * faccio il fetch dei secondi 8 bit che
							 * compongono il tile (che hanno un peso maggiore
							 * rispetto ai primi).
							 */
							FETCHHB()
							if (extclRdChrAfter) {
								/*
								 * utilizzato dalle mappers :
								 * MMC5
								 * MMC2/4
								 */
								extclRdChrAfter(ppu.bckAdr);
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
							if (ppu.frameX == 253) {
								WORD tileY;

								/*
								 * se il $2006 viene aggiornato (tramite istruzione)
								 * proprio al ciclo 253 della PPU, questo incremento
								 * viene ignorato.
								 * Rom interessata :
								 * Cosmic Wars (J) [!].nes
								 * (avviare la rom e non premere niente. Dopo la scritta
								 * 260 iniziale e le esplosioni che seguono, si apre una
								 * schermata con la parte centrale che saltella senza
								 * questo controllo.
								 */
								if (r2006.changedFromOP != 253) {
									/* controllo se fine Y e' uguale a 7 */
									if ((r2006.value & 0x7000) == 0x7000) {
										/* azzero il fine Y */
										r2006.value &= 0x0FFF;
										/* isolo il tile Y */
										tileY = (r2006.value & 0x03E0);
										/* quindi lo esamino */
										if (tileY == 0x03A0) {
											/* nel caso di 29 */
											r2006.value ^= 0x0BA0;
										} else if (tileY == 0x03E0) {
											/* nel caso di 31 */
											r2006.value ^= 0x03E0;
										} else {
											/* incremento tile Y */
											r2006.value += 0x20;
										}
									} else {
										/* incremento di 1 fine Y */
										r2006.value += 0x1000;
									}
								}
								/*
								 * alla fine di ogni scanline
								 * reinizializzo il $2006.
								 */
								r2006EndScanline();
							}
						}
/* ---------------------------------- RENDERING BACKGROUND ----------------------------------- */
						/*
						 * se non e' settato il bit 3 del $2001 il
						 * background e' invisibile.
						 */
						if (r2001.bckVisible) {
							/*
							 * se il bit 1 del $2001 e' a 0 vuol dire
							 * che e' abilitato il clipping del background
							 * (in poche parole non vengono disegnati i primi
							 * 8 pixel dello screen).
							 */
							if (r2001.bckClipping || (ppu.frameX >= 8)) {
								/* sto trattando un pixel del background */
								flagBG = TRUE;
								/* recupero i 2 bit LSB del pixel */
								colorBG = (tileRender.lByte & 0x01) | (tileRender.hByte & 0x02);
								/*
								 * shifto di un bit (leggi un pixel) i
								 * due bitmap buffers
								 */
								tileRender.lByte >>= 1;
								tileRender.hByte >>= 1;
								/*
								 * se i 2 bit LSB del colore non sono uguali a
								 * 0, vuol dire che il pixel non e' trasparente
								 * e quindi vi aggiungo i 2 bit MSB.
								 */
								if (colorBG) {
									/*
									 * recupero i 2 bit MSB del pixel.
									 * Note:
									 * per opera dello scrolling durante il
									 * rendering degli 8 pixel a video posso
									 * passare da uno tile al successivo. Se la
									 * somma tra pixelTile e fineX e' inferiore
									 * a 7 sono ancora nel tile corrente,
									 * altrimenti sono nel tile successivo.
									 */
									if ((ppu.pixelTile + ppu.fineX) < 8) {
										colorBG |= (tileRender.attrib << 2);
									} else {
										colorBG |= (tileRender.attrib >> 6);
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
						if (r2001.sprVisible) {
							BYTE a;
							/* esamino se ci sono sprite da renderizzare */
							for (a = 0; a < sprEv.count; a++) {
								/*
								 * per essercene, la differenza tra frameX e la
								 * cordinata X dello sprite deve essere
								 * inferiore a 8 (per questo uso il WORD, per
								 * avere risultati unsigned).
								 */
								if ((WORD) (ppu.frameX - sprite[a].xC) < 8) {
									/*
									 * se il bit 2 del $2001 e' a 0 vuol dire
									 * che e' abilitato il clipping degli sprite
									 * (in poche parole non vengono disegnati i
									 * primi 8 pixel dello screen).
									 */
									if (r2001.sprClipping || (ppu.frameX >= 8)) {
										/* indico che uno sprite e' stato trovato */
										flagSP = TRUE;
										/*
										 * nel caso il colore dello sprite
										 * sia a zero vuol dire che nessuno
										 * sprite e' stato ancora trovato e
										 * quindi devo disegnare quello
										 * attualmente in esame (sprite
										 * intendo).
										 */
										if (!colorSP) {
											colorSP = ((sprite[a].lByte & 0x01)
													| (sprite[a].hByte & 0x02));
											/*
											 * se i 2 bit LSB del colore non sono uguali a
											 * 0, vuol dire che il pixel non e' trasparente
											 * e quindi vi aggiungo i 2 bit MSB.
											 */
											if (colorSP) {
												colorSP |= ((sprite[a].attrib & 0x03) << 2);
												/* questo sprite non e' invisibile */
												visibleSpr = a;
											}
										}
									}
									/*
									 * shifto di un bit i due bitmap buffers di ogni
									 * sprite della scanlines, compresi quelli non
									 * visibili.
									 */
									sprite[a].lByte >>= 1;
									sprite[a].hByte >>= 1;
								}
							}
						}
/* ------------------------------------ MULTIPLEXER ------------------------------------------ */
						/* tratto i pixel del background e dello sprite */
						if (!(flagBG | flagSP)) {
							/*
							 * se sono nella condizione in cui il BG e' invisibile
							 * e non ho trovato nessuno sprite allora visualizzo
							 * un pixel nero.
							 */
							PUTPIXEL(palette.color[0])
						} else if (!colorSP) {
							/*
							 * se il colore dello sprite e' trasparente,
							 * utilizzo quello del background.
							 */
							PUTBG
						} else if (!colorBG) {
							/*
							 * se invece e' il colore del background ad essere
							 * trasparente, utilizzo quello dello sprite.
							 */
							PUTSP
						} else {
							if (sprite[visibleSpr].attrib & 0x20) {
								/*
								 * se non lo sono tutti e due, controllo la
								 * profondita' dello sprite e se e' settata su
								 * "dietro il background" utilizzo il colore
								 * dello sfondo.
								 */
								PUTBG
							} else {
								/* altrimenti quello dello sprite */
								PUTSP
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
							if (!r2002.sprite0Hit && !sprite[visibleSpr].number
									&& (ppu.frameX != 255)) {
								r2002.sprite0Hit = 0x40;
							}
						}

/* ------------------------------- CONTROLLO SPRITE SCANLINE+1 ------------------------------- */
						if (ppu.frameX < 64) {
							r2004.value = 0xFF;
							/* inizializzo le varibili per il ciclo 64 */
							if (ppu.frameX == 63) {
								/*
								 * inizializzo i vari indici
								 *
								 * Note:
								 * imposto indexByte e index ai
								 * loro valori massimi solo perche'
								 * nel 64° ciclo, come prima cosa
								 * li incremento azzerandoli.
								 */
								sprEv.timing = 0;
								sprEv.index = 0xFF;
								/* la fase 1 e 2 corrispondono */
								sprEv.phase = 2;
							}
						} else if (ppu.frameX < 256) {
/* --------------------------------------- FASE 1 E 2 ---------------------------------------- */
							/*
							 * in questa fase esamino e salvo i primi 8 sprites
							 * che trovo essere nel range. Trovato l'ottavo passo
							 * alla fase 3. Se invece esamino tutti e 64 gli sprites
							 * dell'OAM, passo alla fase 4.
							 */
							if (sprEv.phase == 2) {
								if (!sprEv.timing) {
									/* in caso di overflow dell'indice degli sprite ... */
									if (++sprEv.index == 64) {
										/* ...azzero l'indice... */
										sprEv.index = 0;
										/* ...passo alla fase 4... */
										sprEv.phase = 4;
										/*
										 * ...di cui questo stesso ciclo sara' il
										 * timing = 0, quindi il prossimo sara' l'1.
										 */
										sprEv.timing = 1;
										/* leggo la coordinata Y dello sprite 0 */
										r2004.value = oam.element[0][YC];
									} else {
										/* leggo dall'OAM il byte 0 dell'elemento in esame */
										r2004.value = oam.element[sprEv.index][YC];
										/*
										 * calcolo la differenza tra la posizione
										 * iniziale dello sprite e la posizione Y
										 * del pixel che sto renderizzando. Se e'
										 * inferiore a 8 o 16 (dipende dalla dimensione
										 * dello sprite) allora puo' essere disegnato.
										 */
										sprEv.range = ppu.screenY - r2004.value;
										/*
										 * se sono nel range e lo sprite ha una
										 * posizione Y inferiore o uguale a 0xEF,
										 * lo esamino.
										 */
										if ((r2004.value <= 0xEF)
												&& (sprEv.range < r2000.sizeSPR)) {
											sprEv.evaluate = TRUE;
										} else {
											/* questo sprite non mi interessa */
											sprEv.evaluate = FALSE;
										}
										/* incremento timing */
										sprEv.timing++;
									}
								} else if (sprEv.timing == 1) {
									/*
									 * esamino lo sprites e se necessario
									 * inizio a memorizzare le informazioni.
									 */
									if (sprEv.evaluate) {
										/*
										 * memorizzo la prima parte delle
										 * informazione dello sprite nel buffer.
										 */
										oam.elementPlus[sprEv.countPlus][YC] = r2004.value;
										spritePlus[sprEv.countPlus].number = sprEv.index;
										spritePlus[sprEv.countPlus].flipV = sprEv.range;
										/* continuo a trattare questo sprite */
										sprEv.timing++;
									} else {
										/* passo al prossimo sprite */
										sprEv.timing = 0;
									}
								/* tratto i cicli pari */
								} else if (!(sprEv.timing & 0x01)) {
									/* leggo il prossimo byte dell'OAM */
									r2004.value = oam.element[sprEv.index][sprEv.timing >> 1];
									/* passo al ciclo successivo */
									sprEv.timing++;
								/* tratto i cicli dispari */
								} else {
									/* memorizzo il valore letto nel ciclo prima */
									oam.elementPlus[sprEv.countPlus][sprEv.timing >> 1] =
											r2004.value;
									/* l'unico ciclo diverso e' l'ultimo */
									if (sprEv.timing == 7) {
										/*
										 * se ho gia' trovato 8 sprites allora
										 * devo avviare la fase 3.
										 */
										if (++sprEv.countPlus == 8) {
											sprEv.phase = 3;
											/*
											 * inizilizzo le variabili che
											 * mi serviranno. byteOAM = 3
											 * perche' verra' aumentata e quindi
											 * riportata a 0 nel primo ciclo
											 * della fase 3.
											 */
											sprEv.evaluate = FALSE;
											sprEv.byteOAM = 3;
											sprEv.indexPlus = 0;
										} else {
											/*
											 * indexPlus non superera'
											 * mai il valore 7.
											 */
											sprEv.indexPlus = sprEv.countPlus;
										}
										/* passo al prossimo sprite */
										sprEv.timing = 0;
									} else {
										/* se non sono nel 7° continuo a esaminare lo sprite */
										sprEv.timing++;
									}
								}
/* ------------------------------------------- FASE 3 ---------------------------------------- */
							} else if (sprEv.phase == 3) {
								/* cicli pari */
								if (!(sprEv.timing & 0x01)) {
									/*
									 * se non ho ancora trovato il nono sprite devo
									 * aumentare sia byteOAM che index. Questo
									 * e' un'errore della PPU che, invece di controllare
									 * la coordinata Y (byte 0), tratta il byte puntato
									 * da byteOAM come se fosse la coordinata Y.
									 */
									if (sprEv.evaluate == FALSE) {
										/* incremento l'indice del byte da leggere */
										if (++sprEv.byteOAM == 4) {
											sprEv.byteOAM = 0;
										}
										/* in caso di overflow dell'indice degli sprite ... */
										if (++sprEv.index == 64) {
											sprEv.timing = 1;
											/* ...azzero l'indice... */
											sprEv.index = 0;
											/* ...e passo alla fase 4... */
											sprEv.phase = 4;
											/*
											 * ...di cui questo stesso ciclo sara' il
											 * timing = 0, quindi il prossimo sara' l'1.
											 */
											sprEv.timing = 1;
											/* leggo la coordinata Y dello sprite 0 */
											r2004.value = oam.element[0][YC];
										} else {
											/*
											 * leggo dall'OAM il byte byteOAM
											 * dell'elemento in esame.
											 */
											r2004.value =
													oam.element[sprEv.index][sprEv.byteOAM];
											/* l'unica differenza nei cicli pari e' lo 0 */
											if (sprEv.timing == 0) {
												/*
												 * calcolo la differenza tra la posizione
												 * iniziale dello sprite e la posizione Y
												 * del pixel che sto renderizzando. Se e'
												 * inferiore a 8 o 16 (dipende dalla dimensione
												 * dello sprite) allora puo' essere disegnato.
												 */
												sprEv.range = ppu.screenY - r2004.value;
												/*
												 * se sono nel range e lo sprite ha una
												 * posizione Y inferiore o uguale a 0xEF,
												 * vuol dire che sono al nono sprite.
												 */
												if ((r2004.value <= 0xEF)
														&& (sprEv.range < r2000.sizeSPR)) {
													/* setto il bit 5 (overflow) del $2002 */
													r2002.spriteOverflow = 0x20;
													/*
													 * devo esaminare i 3 byte
													 * consequenziali a questo.
													 */
													sprEv.evaluate = TRUE;
												}
											}
											/* continuo a esaminare lo sprite */
											sprEv.timing++;
										}
									/*
									 * se ho esaminato tutti i 4 byte del nono allora
									 * devo riprendere a esaminare le coordinate Y degli
									 * sprites.
									 */
									} else if (sprEv.evaluate == OVERFLOWSPR) {
										/* in caso di overflow dell'indice degli sprite ... */
										if (++sprEv.index == 64) {
											/* ...azzero l'indice... */
											sprEv.index = 0;
											/* ...e passo alla fase 4... */
											sprEv.phase = 4;
										}
										/* leggo la coordinata Y dello sprite in esame */
										r2004.value = oam.element[sprEv.index][YC];
										/* continuo a esaminare lo sprite */
										sprEv.timing++;
									/*
									 * sto esaminando il nono sprite e devo farlo controllando
									 * i 3 byte dell'OAM successivi a quello che ho considerato
									 * come coordinata Y anche se questi finiscono nell'elemento
									 * dell'OAM successivo.
									 */
									} else if (sprEv.evaluate == TRUE) {
										/* incremento l'indice del byte da leggere */
										if (++sprEv.byteOAM == 4) {
											/*
											 * c'e' la possibilita' che finisca
											 * nell'elemento dell'OAM successivo.
											 */
											sprEv.byteOAM = 0;
											/* in caso di overflow dell'indice degli sprite ... */
											if (++sprEv.index == 64) {
												/* ...azzero l'indice... */
												sprEv.index = 0;
												/* ...e passo alla fase 4... */
												sprEv.phase = 4;
												/*
												 * l'ho imposto a 0 perche' in uscita da
												 * questo if sara' aumentato.
												 */
												sprEv.timing = 0;
											}
										}
										/*
										 * leggo il byte successivo (in caso di passaggio
										 * alla fase 4 questo corrispondera' alla coordinata Y
										 * dello sprite 0
										 */
										r2004.value = oam.element[sprEv.index][sprEv.byteOAM];
										/* continuo a esaminare lo sprite */
										sprEv.timing++;
									}
								/* cicli dispari */
								} else {
									/* leggo la coordinata Y dello sprite in esame */
									r2004.value = oam.elementPlus[sprEv.indexPlus][YC];
									/* se sto esaminando il nono sprite... */
									if (sprEv.evaluate == TRUE) {
										/* ...e sono nell'ultimo ciclo...*/
										if (sprEv.timing == 7) {
											/* ...indico la nuova modalita'... */
											sprEv.evaluate = OVERFLOWSPR;
											/* ...passo al prossimo sprite.. */
											sprEv.timing = 0;
											/*
											 * ...anche se devo riesaminare questo
											 * stesso sprite (ricordo che incremento
											 * index al timing = 0).
											 */
											sprEv.index--;
										} else {
											/* ... e non sono nell'ultimo ciclo,
											 * continuo a esaminare lo sprite.
											 */
											sprEv.timing++;
										}
									} else {
										/*
										 * se non sono nel nono sprite
										 * allora passo al prossimo.
										 */
										sprEv.timing = 0;
									}
								}
/* ------------------------------------------- FASE 4 ---------------------------------------- */
							/* e' composto solo da due cicli (0 e 1) */
							} else if (sprEv.phase == 4) {
								/* ciclo 0 */
								if (!sprEv.timing) {
									/* in caso di overflow dell'indice degli sprite ... */
									if (++sprEv.index == 64) {
										/* ...azzero l'indice */
										sprEv.index = 0;
									}
									/* leggo la coordinata Y dello sprite OAM in esame */
									r2004.value = oam.element[sprEv.index][YC];
									/* passo al ciclo successivo */
									sprEv.timing = 1;
								/* ciclo 1 */
								} else {
									/* leggo la coordinata Y dello sprite in esame */
									r2004.value = oam.elementPlus[sprEv.indexPlus][YC];
									/* passo al prossimo sprite */
									sprEv.timing = 0;
								}
							}
						}
/* ------------------------------------------------------------------------------------------- */
					} else {
						if ((r2006.value & 0xFF00) == 0x3F00) {
							/*
							 * se background e sprites non sono visibili
							 * e $2006 e' nel range 0x3F00/0x3FFF (nella
							 * paletta insomma) allora a video devo
							 * visualizzare il colore puntato dal registro.
							 */
							PUTEMPHASIS(r2006.value & 0x1F)
						} else {
							/*
							 * altrimenti visualizzo un pixel del
							 * colore 0 della paletta.
							 */
							PUTPIXEL(palette.color[0])
						}
					}
				}
				/* incremento in contatore dei pixel interni al tile */
				if (++ppu.pixelTile > 7) {
					ppu.pixelTile = 0;
				}
				PPUTICKET()
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
		if (ppu.frameX < 320) {
			if (!r2002.vblank && r2001.visible && (ppu.screenY < SCRLINES)) {
				if (extclPPU256to319) {
					/*
					 * utilizzato dalle mappers :
					 * MMC3
					 * Taito
					 * MMC5
					 * Tengen
					 */
					extclPPU256to319();
				}
				if (ppu.frameX == 256) {
					sprEv.timing = sprEv.tmpSprPlus = 0;
				}
				/* controllo se ci sono sprite per la (scanline+1) */
				if (sprEv.tmpSprPlus < sprEv.countPlus) {
					/*
					 * utilizzo pixelTile come contatore di cicli per
					 * esaminare uno sprite ogni 8 cicli.
					 */
					if (!sprEv.timing) {
						ppuSprAdr(sprEv.tmpSprPlus);
						/*
						 * significato bit 6 del byte degli attributi:
						 *  0 -> no flip orizzontale
						 *  1 -> si flip orizzontale
						 */
						if (oam.elementPlus[sprEv.tmpSprPlus][AT] & 0x40) {
							/* salvo i primi 8 bit del tile dello sprite */
							spritePlus[sprEv.tmpSprPlus].lByte = ppuRdMem(ppu.sprAdr);
							/* salvo i secondi 8 bit del tile dello sprite */
							spritePlus[sprEv.tmpSprPlus].hByte = (ppuRdMem(ppu.sprAdr | 0x08) << 1);
						} else {
							spritePlus[sprEv.tmpSprPlus].lByte = invCHR[ppuRdMem(ppu.sprAdr)];
							/* salvo i secondi 8 bit del tile dello sprite */
							spritePlus[sprEv.tmpSprPlus].hByte =
									(invCHR[ppuRdMem(ppu.sprAdr | 0x08)] << 1);
						}
						r2004.value = oam.elementPlus[sprEv.tmpSprPlus][YC];
						if (extclRdChrAfter) {
							/*
							 * utilizzato dalle mappers :
							 * MMC5
							 * MMC2/4
							 */
							extclRdChrAfter(ppu.sprAdr);
						}
					} else if (sprEv.timing < 4) {
						r2004.value = oam.elementPlus[sprEv.tmpSprPlus][sprEv.timing];
					} else if (sprEv.timing < 8) {
						r2004.value = oam.elementPlus[sprEv.tmpSprPlus][XC];
					}
					/* incremento il contatore del ciclo interno */
					if (++sprEv.timing == 8) {
						/* passo al prossimo sprite */
						sprEv.timing = 0;
						/* incremento l'indice temporaneo degli sprites */
						sprEv.tmpSprPlus++;
					}
				} else {
					if (!sprEv.timing) {
						r2004.value = oam.element[63][YC];
					} else if (sprEv.timing < 7) {
						r2004.value = 0xFF;
					}
					if (++sprEv.timing == 8) {
						sprEv.timing = 0;
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
				if ((ppu.screenY == 238) && (ppu.frameX == 319)) {
					r2003.value = 0;
				}
				PPUTICKET()
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
		if (!r2002.vblank && r2001.visible && (ppu.screenY < SCRLINES)) {
			if (extclPPU320to34x) {
				/*
				 * utilizzato dalle mappers :
				 * MMC3
				 * Taito
				 * MMC5
				 * Tengen
				 */
				extclPPU320to34x();
			}
			if (ppu.frameX == 323) {
				if (ppu.frameY == machine.vintLines) {
					/*
					 * all'inizio di ogni frame reinizializzo
					 * l'indirizzo della PPU.
					 */
					r2006.value = ppu.tmpVRAM;
				}
				FETCHAT()
			} else if (ppu.frameX == 331) {
				FETCHAT()
			} else if ((ppu.frameX == 325) || (ppu.frameX == 333)) {
				FETCHLB()
			} else if ((ppu.frameX == 327) || (ppu.frameX == 335)) {
				FETCHHB()
				if (extclRdChrAfter) {
					/*
					 * utilizzato dalle mappers :
					 * MMC5
					 * MMC2/4
					 */
					extclRdChrAfter(ppu.bckAdr);
				}
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
		if (ppu.frameX < ppu.slineCycles) {
			if (sprEv.countPlus) {
				r2004.value = oam.elementPlus[0][YC];
			} else {
				r2004.value = oam.element[63][YC];
			}
			ppu.cycles -= machine.ppuDivide;
			ppu.frameX++;
			/* deve essere azzerato alla fine di ogni ciclo PPU */
			r2006.changedFromOP = 0;
			/*
			 * se e' iniziato il ciclo 341, vuol dire
			 * che in realta' e' iniziato il ciclo 0
			 * della scanline successiva.
			 */
			if (ppu.frameX != ppu.slineCycles) {
				continue;
			}
		}

		/*
		 * schema dettaglio frame completo
		 *
		 *        NTSC                             PAL                            Dendy
		 * frameY  | |  screenY            frameY  | |  screenY            frameY  | |  screenY
		 * --------------------            --------------------            --------------------
		 * |0                0|            |0                0|            |0                0|
		 * |.                .|   VBLANK   |.                .|   VBLANK   |.                .|
		 * |.                .|            |.                .|            |19               0|
		 * |19               0|            |69               0|            --------------------
		 * --------------------            -------------------- dummy line-|20               0|
		 * |20               0| dummy line |70               0|/           --------------------
		 * --------------------            --------------------            |21               0|
		 * |21               0|            |71               0|   screen   |.                .|
		 * |.                .|   screen   |.                .| (rendering)|260            239|
		 * |.                .| (rendering)|.                .|            --------------------
		 * |260            239|            |310            239|            |261            240|
		 * --------------------            --------------------            |.                .|
		 * |261            240| PPU ferma  |311            240| PPU ferma  |311            290|
		 * --------------------            --------------------            --------------------
		 * Totale 262 scanlines            Totale 312 scanlines            Totale 312 scanlines
		 */
		/* controllo di essere nel range [dummy...rendering screen] */
		if ((ppu.frameY >= machine.vintLines) && (ppu.screenY < SCRLINES)) {
			BYTE a;
			/* verifico di non trattare la dummy line */
			if (ppu.frameY > machine.vintLines) {
				/* incremento il contatore delle scanline renderizzate */
				ppu.screenY++;
			}
			/*
			 * l'indice degli sprite per la (scanline+1)
			 * diventa quello attuale (visto che sto per
			 * incrementare la scanline).
			 */
			sprEv.count = sprEv.countPlus;
			/* azzero l'indice per la (scanline+1) */
			sprEv.countPlus = 0;
			/*
			 * sposto il buffer degli sprites della scanline
			 * successiva (scanline+1) nel buffer di quella
			 * che sto per trattare.
			 */
			for (a = 0; a < sprEv.count; a++) {
				sprite[a].yC = oam.elementPlus[a][YC];
				sprite[a].tile = oam.elementPlus[a][TL];
				sprite[a].attrib = oam.elementPlus[a][AT];
				sprite[a].xC = oam.elementPlus[a][XC];
				sprite[a].number = spritePlus[a].number;
				sprite[a].flipV = spritePlus[a].flipV;
				sprite[a].lByte = spritePlus[a].lByte;
				sprite[a].hByte = spritePlus[a].hByte;
			}
		}

		/*
		 * incremento frameY, reinizializzo
		 * slineCycles ed e' estremamente importante
		 * che lo faccia esattamente qui, cosi'
		 * come e' importante che l'azzeramento del
		 * ppu.framex lo faccia dopo il settaggio dell'nmi.
		 */
		ppu.frameY++;
		ppu.slineCycles = SLCYCLES;

		/* controllo se ho completato il frame */
		if (ppu.frameY == machine.totalLines) {
			/* incremento il contatore dei frames */
			ppu.frames++;
			/* azzero frameY */
			ppu.frameY = 0;
			/*
			 * setto il flag che indica che un frame
			 * e' stato completato.
			 */
			info.executeCPU = FALSE;
			/* e' un frame dispari? */
			ppu.oddFrame = !ppu.oddFrame;
			/* abilito il vblank */
			r2002.vblank = 0x80;
			/*
			 * quando il bit 7 del $2002 e il bit 7
			 * del $2000 sono a 1 devo generare un NMI.
			 */
			if (r2000.NMIenable) {
				nmi.high = TRUE;
				nmi.frameX = ppu.frameX;
			}
		}
		/*
		 * azzero frameX, ed e' estremamente
		 * importante che lo faccia esattamente
		 * dopo il settaggio dell'nmi.
		 */
		ppu.frameX = 0;
		/* deve essere azzerato alla fine di ogni ciclo PPU */
		r2006.changedFromOP = 0;
	}
}
BYTE ppuTurnON(void) {
	if (info.reset >= HARD) {
		memset(&ppu, 0x00, sizeof(ppu));
		memset(&ppuOpenbus, 0x00, sizeof(ppuOpenbus));
		memset(&r2000, 0x00, sizeof(r2000));
		memset(&r2001, 0x00, sizeof(r2001));
		memset(&r2002, 0x00, sizeof(r2002));
		memset(&r2003, 0x00, sizeof(r2003));
		memset(&r2004, 0x00, sizeof(r2004));
		memset(&r2006, 0x00, sizeof(r2006));
		memset(&r2007, 0x00, sizeof(r2007));
		memset(&sprEv, 0x00, sizeof(sprEv));
		memset(&sprite, 0x00, sizeof(sprite));
		memset(&spritePlus, 0x00, sizeof(spritePlus));
		memset(&tileRender, 0x00, sizeof(tileRender));
		memset(&tileFetch, 0x00, sizeof(tileFetch));
		/*
		 * "Time Lord (U) [!].nes"
		 * funziona correttamente (altrimenti avviato il gioco
		 * la parte di sotto si sporca e non appaiono sprites).
		 */
		ppu.frameY = machine.vintLines + 1;
		ppu.slineCycles = SLCYCLES;
		r2000.r2006Inc = 1;
		r2000.sizeSPR = 8;
		r2001.colorMode = NORMAL;

		/* riservo una zona di memoria per lo screen */
		if ((info.reset == CHANGEROM) || (info.reset == POWERUP)) {
			BYTE a;
			WORD x, y;

			if (screen.data) {
				free(screen.data);
			}

			if (!(screen.data = malloc(screenSize()))) {
				fprintf(stderr, "Out of memory\n");
				return (EXIT_ERROR);
			}

			/*
			 * creo una tabella di indici che puntano
			 * all'inizio di ogni linea dello screen.
			 */
			for (a = 0; a < SCRLINES; ++a) {
				screen.line[a] = (WORD *) (screen.data + (a * SCRROWS));
			}
			/* inizializzo lo screen */
			for (y = 0; y < SCRLINES; y++) {
				for (x = 0; x < SCRROWS; x++) {
					screen.line[y][x] = 0x000D;
				}
			}
			/*
			 * inizializzo la Object Attribute Memory
			 * utilizzata per conservare le informazioni
			 * inerenti gli sprites.
			 */
			memset(oam.data, 0xFF, sizeof(oam.data));
			/*
			 * tabella di indici che puntano ad ogni
			 * elemento dell'OAM (4 bytes ciascuno).
			 */
			for (a = 0; a < 64; ++a) {
				oam.element[a] = &oam.data[(a * 4)];
			}
			memset(oam.plus, 0xFF, sizeof(oam.plus));
			for (a = 0; a < 8; ++a) {
				oam.elementPlus[a] = &oam.plus[(a * 4)];
			}
			/* inizializzo nametables */
			memset(ntbl.data, 0x00, sizeof(ntbl.data));
			/* e paletta dei colori */
			memset(palette.color, 0x3F, sizeof(palette.color));
		}
	} else {
		memset(&r2000, 0x00, sizeof(r2000));
		memset(&r2001, 0x00, sizeof(r2001));
		memset(&r2002, 0x00, sizeof(r2002));
		memset(&r2007, 0x00, sizeof(r2007));

		ppu.frameX = ppu.screenY = ppu.pixelTile = 0;
		ppu.frameY = machine.vintLines + 1;
		ppu.tmpVRAM = ppu.fineX = 0;
		ppu.sprAdr = ppu.bckAdr = 0;
		ppu.slineCycles = SLCYCLES;
		ppu.oddFrame = 0;
		ppu.cycles = 0;
		r2000.r2006Inc = 1;
		r2000.sizeSPR = 8;
		r2001.colorMode = NORMAL;
	}

	return (EXIT_OK);
}
void ppuQuit(void) {
	/* libero la memoria riservata */
	if (screen.data) {
		free(screen.data);
	}
}
