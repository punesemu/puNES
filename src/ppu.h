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

#define screenSize() (SCRLINES * SCRROWS) * sizeof(WORD)
#define ppuSprAdr(sprite)\
{\
	BYTE flipV;\
	/*\
	 * significato bit 7 del byte degli attributi:\
	 *  0 -> no flip verticale\
	 *  1 -> si flip verticale\
	 */\
	if (oam.elementPlus[sprite][AT] & 0x80) {\
		/* flip verticale */\
		flipV = ~spritePlus[sprite].flipV;\
	} else {\
		/* no flip verticale */\
		flipV = spritePlus[sprite].flipV;\
	}\
	/*\
	 * significato bit 5 del $2000:\
	 *  0 -> sprite 8x8 (1 tile)\
	 *  1 -> sprite 8x16 (2 tile)\
	 */\
	if (r2000.sizeSPR == 16) {\
		/* -- 8x16 --\
		 *\
		 * spritePlus[x].tile:\
		 * 76543210\
		 * ||||||||\
		 * |||||||+- Bank ($0000 or $1000) dei tiles\
		 * +++++++-- numero del tile (da 0 a 127)\
		 */\
		/*\
		 * estrapolo il numero del tile che sara' pari\
		 * per i primi 8x8 pixels (spritePlus[x].flipV\
		 * inferiore a 8), e dispari per restanti 8x8\
		 * (spritePlus[x].flipV superiore a 8),\
		 * in modo da formare uno sprite 8x16, mentre in\
		 * caso di flip verticale sara' l'esatto contrario,\
		 * dispari per i primi 8x8 e pari per i secondi 8x8.\
		 */\
		ppu.sprAdr = (oam.elementPlus[sprite][TL] & 0xFE) | ((flipV & 0x08) >> 3);\
		/* recupero la posizione nella vram del tile */\
		ppu.sprAdr = ((oam.elementPlus[sprite][TL] & 0x01) << 12) | (ppu.sprAdr << 4);\
	} else {\
		/* -- 8x8 --\
		 *\
		 * spritePlus[x].tile = numero del tile nella vram.\
		 */\
		/* recupero la posizione nella vram del tile */\
		ppu.sprAdr = r2000.SPTadr | (oam.elementPlus[sprite][TL] << 4);\
	}\
	/* aggiungo la cordinata Y dello sprite */\
	ppu.sprAdr += (flipV & 0x07);\
}
#define ppuBckAdr()\
	ppu.bckAdr = r2000.BPTadr |\
		((ppuRdMem(0x2000 | (r2006.value & 0x0FFF)) << 4)\
		| ((r2006.value & 0x7000) >> 12))
#define r2006inc()\
{\
	WORD tileY;\
	/* controllo se fine Y e' uguale a 7 */\
	if ((r2006.value & 0x7000) == 0x7000) {\
		/* azzero il fine Y */\
		r2006.value &= 0x0FFF;\
		/* isolo il tile Y */\
		tileY = (r2006.value & 0x03E0);\
		/* quindi lo esamino */\
		if (tileY == 0x03A0) {\
			/* nel caso di 29 */\
			r2006.value ^= 0x0BA0;\
		} else if (tileY == 0x03E0) {\
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
#define r2006EndScanline()\
	r2006.value = (r2006.value & 0xFBE0) | (ppu.tmpVRAM & 0x041F)

typedef struct {
	WORD frameX;
	WORD frameY;
	BYTE fineX;
	BYTE screenY;
	WORD pixelTile;
	WORD slineCycles;
	WORD tmpVRAM;
	WORD sprAdr;
	WORD bckAdr;
	BYTE openbus;
	BYTE oddFrame;
	BYTE skipDraw;
	SWORD cycles;
	uint32_t frames;
}  _ppu;
typedef struct {
	WORD *data;
	WORD *line[SCRLINES];
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
} _ppuOpenbus;
typedef struct {
	BYTE value;
} _r2xxx;
typedef struct {
	BYTE value;
	BYTE NMIenable;
	BYTE sizeSPR;
	BYTE r2006Inc;
	WORD SPTadr;
	WORD BPTadr;
} _r2000;
typedef struct {
	BYTE value;
	WORD emphasis;
	BYTE visible;
	BYTE bckVisible;
	BYTE sprVisible;
	BYTE bckClipping;
	BYTE sprClipping;
	BYTE colorMode;
} _r2001;
typedef struct {
	BYTE vblank;
	BYTE sprite0Hit;
	BYTE spriteOverflow;
	BYTE toggle;
} _r2002;
typedef struct {
	WORD value;
	WORD changedFromOP;
} _r2006;
typedef struct {
	WORD range;
	BYTE count;
	BYTE countPlus;
	BYTE tmpSprPlus;
	BYTE evaluate;
	BYTE byteOAM;
	BYTE indexPlus;
	BYTE index;
	BYTE timing;
	BYTE phase;
} _sprEvaluate;
typedef struct {
	BYTE yC;
	BYTE tile;
	BYTE attrib;
	BYTE xC;
	BYTE number;
	BYTE flipV;
	BYTE lByte;
	WORD hByte;
} _spr;
/*
 * le variabili sono di tipo WORD e DBWORD perche',
 * per gestire correttamente lo scrolling, saranno
 * sempre trattati due tiles alla volta, altrimenti
 * sarebbero stati sufficienti BYTE e WORD.
 */
typedef struct {
	WORD attrib;
	WORD lByte;
	DBWORD hByte;
} _tile;

_ppu ppu;
_screen screen;
_ppuOpenbus ppuOpenbus;
_r2000 r2000;
_r2001 r2001;
_r2002 r2002;
_r2006 r2006;
_r2xxx r2003, r2004, r2007;
_sprEvaluate sprEv;
_spr sprite[8], spritePlus[8];
_tile tileRender, tileFetch;

void ppuTick(WORD cyclesCPU);
BYTE ppuTurnON(void);
void ppuQuit(void);

#endif /* PPU_H_ */
