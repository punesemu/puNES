/*
 * ntsc.h
 *
 *  Created on: 18/ago/2010
 *      Author: fhorse
 */

#ifndef NTSC_H_
#define NTSC_H_

#include "common.h"
#include "nes_ntsc.h"
#include "palette.h"

enum ntsc_mode { COMPOSITE, SVIDEO, RGBMODE };

void ntsc_surface(WORD *screen, WORD **screen_index, uint32_t *palette, BYTE bpp, uint32_t pitch,
        void *pix, WORD rows, WORD lines, WORD width, WORD height, BYTE factor);
BYTE ntsc_init(BYTE effect, BYTE color, BYTE *palette_base, BYTE *palette_in, BYTE *palette_out);
void ntsc_quit(void);
void ntsc_set(BYTE effect, BYTE color, BYTE *palette_base, BYTE *palette_in, BYTE *palette_out);

#endif /* NTSC_H_ */
