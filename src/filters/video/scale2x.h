/*
 * scale2x.h
 *
 *  Created on: 21/mag/2010
 *      Author: fhorse
 */

#ifndef SCALE2X_H_
#define SCALE2X_H_

#include "common.h"
#include "gfx.h"
#include "palette.h"

gfx_filter_function(scaleNx);
void scale2x(WORD **screen_index, uint32_t *palette, BYTE bpp, uint32_t pitch, void *pix);
void scale3x(WORD **screen_index, uint32_t *palette, BYTE bpp, uint32_t pitch, void *pix);
void scale4x(WORD **screen_index, uint32_t *palette, BYTE bpp, uint32_t pitch, void *pix);

#endif /* SCALE2X_H_ */
