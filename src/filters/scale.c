/*
 * scale.c
 *
 *  Created on: 22/mag/2010
 *      Author: fhorse
 */

#include "scale.h"
#include "overscan.h"

/*
 * cio' che non utilizzo in questa funzione
 * e' il parametro WORD *screen.
 */
void scaleSurface(WORD *screen, WORD **screenIndex, Uint32 *palette, SDL_Surface *dst, WORD rows,
        WORD lines, BYTE factor) {
	WORD sx = 0, sy = 0;
	WORD ox = 0, oy = 0;
	WORD startx = 0;
	SDL_Rect rect;

	rect.w = rect.h = factor;

	if (overscan.enabled) {
		sy += overscan.up;
		lines += overscan.up;
		rows += overscan.left;
		startx = overscan.left;
	}

	/* lock della destinazione */
	//SDL_LockSurface(dst);
	/* loop per l'intera altezza dell'immagine da scalare */
	for (; sy < lines; sy++) {
		/*
		 * calcolo la coordinata y del rettangolo che
		 * altro non e' che il pixel ridimensionato al
		 * fattore richiesto.
		 */
		rect.y = oy * factor;

		ox = 0;

		/* loop per l'intera larghezza dell'immagine */
		for (sx = startx; sx < rows; sx++) {
			/* coordinata x del pixellone :) */
			rect.x = ox * factor;
			/*
			 * converto il colore nel formato corretto di visualizzazione
			 * e riempio un rettangolo delle dimensioni del fattore di scala
			 * alle coordinate corrette.
			 */
			SDL_FillRect(dst, &rect, palette[screenIndex[sy][sx]]);

			ox++;
		}
		oy++;
	}
	/* unlock della destinazione */
	//SDL_UnlockSurface(dst);
}
