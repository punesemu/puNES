/*
 * scale.h
 *
 *  Created on: 22/mag/2010
 *      Author: fhorse
 */

#ifndef SCALE_H_
#define SCALE_H_

#include "common.h"
#include "gfx.h"
#include "palette.h"

gfx_filter_function(scale_surface);
void scale_surface1x(WORD **screen_index, uint32_t *palette, BYTE bpp, uint32_t pitch, void *pix);
void scale_surface2x(WORD **screen_index, uint32_t *palette, BYTE bpp, uint32_t pitch, void *pix);
void scale_surface3x(WORD **screen_index, uint32_t *palette, BYTE bpp, uint32_t pitch, void *pix);
void scale_surface4x(WORD **screen_index, uint32_t *palette, BYTE bpp, uint32_t pitch, void *pix);

#endif /* SCALE_H_ */
