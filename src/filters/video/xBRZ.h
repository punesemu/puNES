/*
 * xBRZ.h
 *
 *  Created on: 16/lug/2014
 *      Author: fhorse
 */

#ifndef XBRZ_H_
#define XBRZ_H_

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

#include "common.h"
#include "gfx.h"

void xBRZ_init(void);
gfx_filter_function(xBRZ);

EXTERNC void xbrz_scale(BYTE factor, const WORD *src, uint32_t *trg, uint32_t *palette,
        int no_overscan_width, int startx, int width, int height);

#undef EXTERNC

#endif /* XBRZ_H_ */
