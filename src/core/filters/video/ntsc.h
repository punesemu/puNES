/*
 * ntsc.h
 *
 *  Created on: 18/ago/2010
 *      Author: fhorse
 */

#ifndef NTSC_H_
#define NTSC_H_

#include "common.h"
#include "gfx.h"
#include "nes_ntsc.h"
#include "palette.h"

enum ntsc_mode { COMPOSITE, SVIDEO, RGBMODE };

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC gfx_filter_function(ntsc_surface);
EXTERNC BYTE ntsc_init(BYTE effect, BYTE color, BYTE *palette_base, BYTE *palette_in,
		BYTE *palette_out);
EXTERNC void ntsc_quit(void);
EXTERNC void ntsc_set(BYTE effect, BYTE color, BYTE *palette_base, BYTE *palette_in,
		BYTE *palette_out);

#undef EXTERNC

#endif /* NTSC_H_ */
