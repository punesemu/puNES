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

#include <stdio.h>
#include <stdlib.h>
#include "video/gfx.h"
#include "ppu.h"
#include "palette.h"
#include "conf.h"
#include "clock.h"
#include "gui.h"

static void change_color(BYTE *color, SWORD min, SWORD mod);

nes_ntsc_setup_t palette;
_ntsc_filter ntsc_filter;

BYTE ntsc_init(void) {
	// quello per la paletta
	palette = nes_ntsc_composite;

	if (!(ntsc_filter.ntsc = (nes_ntsc_t *)malloc(sizeof(nes_ntsc_t)))) {
		log_error(uL("ntsc;out of memory"));
		return (EXIT_ERROR);
	}
	ntsc_set(NULL, TRUE, 0, NULL, NULL, NULL);
	return (EXIT_OK);
}
void ntsc_quit(void) {
	free(ntsc_filter.ntsc);
}
void ntsc_set(nes_ntsc_t *ntsc, BYTE create_palette, BYTE color, const BYTE *palette_base, const BYTE *palette_in, BYTE *palette_out) {
	nes_ntsc_setup_t *format = create_palette ? &palette : &ntsc_filter.format[cfg->ntsc_format];
	double saturation = format->saturation;

	if (!ntsc) {
		ntsc = ntsc_filter.ntsc;
	}
	if (palette_base) {
		format->base_palette = (unsigned char *)palette_base;
	} else {
		format->base_palette = 0;
	}
	if (palette_in) {
		format->palette = (unsigned char *)palette_in;
	} else {
		format->palette = 0;
	}
	if (palette_out) {
		format->palette_out = (unsigned char *)palette_out;
	} else {
		format->palette_out = 0;
	}

	format->decoder_matrix = 0;
	format->swapped = 0;

	switch (color) {
		// Sony CXA2025AS US
		case PALETTE_SONY: {
			static float matrix[6] = { 1.630f, 0.317f, -0.378f, -0.466f, -1.089f, 1.677f };

			format->decoder_matrix = matrix;
			break;
		}
		case PALETTE_MONO:
			format->saturation = -1;
			break;
		default:
			break;
	}

	nes_ntsc_init(ntsc, format);

	// creo la paletta swappata
	if (format->palette_out) {
		format->swapped = 1;
		format->palette_out = (unsigned char *)palette_RGB.swapped;
		nes_ntsc_init(ntsc, format);
	}

	format->saturation = saturation;
}
void ntsc_rgb_modifier(nes_ntsc_t *ntsc, BYTE *palette_out, SWORD min, SWORD red, SWORD green, SWORD blue) {
	_color_RGB *pRGB = (_color_RGB *)palette_out;
	WORD i;

	// prima ottengo la paletta monocromatica
	ntsc_set(ntsc, TRUE, PALETTE_MONO, 0, 0, palette_out);
	// quindi la modifico
	for (i = 0; i < NUM_COLORS; i++, pRGB++) {
		// rosso
		change_color(&pRGB->r, min, red);
		change_color(&pRGB->g, min, green);
		change_color(&pRGB->b, min, blue);
	}
	// ed infine utilizzo la nuova
	ntsc_set(ntsc, TRUE, 0, 0, palette_out, palette_out);
}
void ntsc_effect_parameters_changed(void) {
	ntsc_set(NULL, FALSE, 0, 0, (BYTE *)palette_RGB.noswap, 0);
	ntsc_set(NULL, FALSE, 0, 0, (BYTE *)palette_RGB.swapped, 0);
}
void ntsc_effect_parameters_default(void) {
	switch (cfg->ntsc_format) {
		case COMPOSITE:
			ntsc_filter.format[cfg->ntsc_format] = nes_ntsc_composite;
			break;
		case SVIDEO:
			ntsc_filter.format[cfg->ntsc_format] = nes_ntsc_svideo;
			break;
		case RGBMODE:
			ntsc_filter.format[cfg->ntsc_format] = nes_ntsc_rgb;
			break;
	}
}
void ntsc_effect_parameter_default(int index) {
	nes_ntsc_setup_t *format;

	switch (cfg->ntsc_format) {
		default:
		case COMPOSITE:
			format = (nes_ntsc_setup_t *)&nes_ntsc_composite;
			break;
		case SVIDEO:
			format = (nes_ntsc_setup_t *)&nes_ntsc_svideo;
			break;
		case RGBMODE:
			format = (nes_ntsc_setup_t *)&nes_ntsc_rgb;
			break;
	}
	switch (index) {
		default:
		case 0:
			ntsc_filter.format[cfg->ntsc_format].hue = format->hue;
			break;
		case 1:
			ntsc_filter.format[cfg->ntsc_format].saturation = format->saturation;
			break;
		case 2:
			ntsc_filter.format[cfg->ntsc_format].contrast = format->contrast;
			break;
		case 3:
			ntsc_filter.format[cfg->ntsc_format].brightness = format->brightness;
			break;
		case 4:
			ntsc_filter.format[cfg->ntsc_format].sharpness = format->sharpness;
			break;
		case 5:
			ntsc_filter.format[cfg->ntsc_format].gamma = format->gamma;
			break;
		case 6:
			ntsc_filter.format[cfg->ntsc_format].resolution = format->resolution;
			break;
		case 7:
			ntsc_filter.format[cfg->ntsc_format].artifacts = format->artifacts;
			break;
		case 8:
			ntsc_filter.format[cfg->ntsc_format].fringing = format->fringing;
			break;
		case 9:
			ntsc_filter.format[cfg->ntsc_format].bleed = format->bleed;
			break;
		case 10:
			ntsc_filter.format[cfg->ntsc_format].scanline_intensity = format->scanline_intensity;
			break;
	}
}
void ntsc_effect_parameter_mv_default(void) {
	nes_ntsc_setup_t *format;

	switch (cfg->ntsc_format) {
		default:
		case COMPOSITE:
			format = (nes_ntsc_setup_t *)&nes_ntsc_composite;
			break;
		case SVIDEO:
			format = (nes_ntsc_setup_t *)&nes_ntsc_svideo;
			break;
		case RGBMODE:
			format = (nes_ntsc_setup_t *)&nes_ntsc_rgb;
			break;
	}
	ntsc_filter.format[cfg->ntsc_format].merge_fields = format->merge_fields;
	ntsc_filter.format[cfg->ntsc_format].vertical_blend = format->vertical_blend;
}
void ntsc_surface(void) {
	static int burst_count = 0, burst_phase = 0;
	int y;

	if (gfx.filter.data.palette == NULL) {
		gfx.filter.data.palette = (void *)ntsc_filter.ntsc;
	}

	nes_ntsc_blit((nes_ntsc_t *)gfx.filter.data.palette, ppu_screen.rd->data, SCR_COLUMNS, burst_phase, SCR_COLUMNS, SCR_ROWS,
		gfx.filter.data.pix, (long)gfx.filter.data.pitch);

	if (ntsc_filter.format[cfg->ntsc_format].merge_fields) {
		burst_count = 0;
		burst_phase = 0;
	} else {
		double div = 1.5f;
		int max = 3;

		if (machine.type != NTSC) {
			div = 1.0f;
			max = 2;
		}
		burst_phase = (int)((double)burst_count / div);
		burst_count = (burst_count + 1) % max;
	}

	for (y = ((gfx.filter.data.height / gfx.filter.factor) - 1); --y >= 0;) {
		unsigned char const *in = ((const unsigned char *)gfx.filter.data.pix) + (y * gfx.filter.data.pitch);
		unsigned char *out = ((unsigned char *)gfx.filter.data.pix) + ((y * gfx.filter.factor) * gfx.filter.data.pitch);
		int n;

		for (n = gfx.filter.data.width; n; --n) {
			unsigned prev = *(uint32_t *)in;
			unsigned next = *(uint32_t *)(in + gfx.filter.data.pitch);
			unsigned mixed;

			// mix rgb without losing low bits
			mixed = ntsc_filter.format[cfg->ntsc_format].vertical_blend ? (prev + next + ((prev ^ next) & 0x030303)) >> 1 : prev;

			*(uint32_t *)out = prev | 0xFF000000;
			if (ntsc_filter.format[cfg->ntsc_format].scanline_intensity < 1.0) {
				uint8_t r = (mixed >> 16) & 0xFF, g = (mixed >> 8) & 0xFF, b = mixed & 0xFF;

				r = (uint8_t)(r * ntsc_filter.format[cfg->ntsc_format].scanline_intensity);
				g = (uint8_t)(g * ntsc_filter.format[cfg->ntsc_format].scanline_intensity);
				b = (uint8_t)(b * ntsc_filter.format[cfg->ntsc_format].scanline_intensity);
				*(uint32_t *)(out +  gfx.filter.data.pitch) = (r << 16) | (g << 8) | b | 0xFF000000;
			} else {
				*(uint32_t *)(out +  gfx.filter.data.pitch) = mixed | 0xFF000000;
			}

			in += NES_NTSC_OUT_DEPTH / 8;
			out += NES_NTSC_OUT_DEPTH / 8;
		}
	}
}

static void change_color(BYTE *color, SWORD min, SWORD mod) {
	SWORD tmp = (SWORD)((*color) + mod);

	(*color) = (tmp < 0 ? min : (tmp > 0xFF ? 0xFF : tmp));
}
