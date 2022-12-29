/*
 *  Copyright (C) 2010-2023 Fabio Cavallo (aka FHorse)
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

// based on https://github.com/SourMesen/Mesen/blob/master/Core/BisqwitNtscFilter.cpp and
// https://forums.nesdev.org/viewtopic.php?t=14338

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "video/gfx.h"
#include "thread_def.h"
#include "ppu.h"

#define NTSC_BISQWIT_NUM_SLICE 4

thread_funct(ntsc_bisqwit_mt, void *arg);
INLINE static void ntsc_bisqwit_generate_signal(const WORD *screen, int8_t *ntsc_signal, int *phase, int row_number);
INLINE static void ntsc_bisqwit_recursive_blend(int iteration_count, uint64_t *output, uint64_t *current_line, uint64_t *next_line, int pixels_per_cycle);
INLINE static void ntsc_bisqwit_decode_frame(int start_row, int end_row, const WORD *screen, uint32_t *pix, int start_phase);
INLINE static void ntsc_bisqwit_decode_line(int width, const int8_t *signal, uint32_t *pix, int phase0);

typedef struct _ntsc_bisqwit_thread {
	int slice;
	BYTE factor;
	const WORD *src;
	uint32_t *dst;
	uint32_t *palette;
	int start_row;
	int end_row;
	int start_phase;
} _ntsc_bisqwit_thread;

const nes_ntsc_bisqwit_setup_t nes_ntsc_bisqwit_default  = { 0, 0, 0, 0, 12, 24,  24,  1, 0.88};
nes_ntsc_bisqwit_setup_t nes_ntsc_bisqwit  = { 0, 0, 0, 0, 12, 24,  24,  1, 0.88};

const WORD _bitmask_lut[12] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x100, 0x200, 0x400, 0x800 };
const int _signals_per_pixel = 8;
int _res_divider = 1;
int _yWidth, _iWidth, _qWidth;
int _y;
int _ir, _ig, _ib;
int _qr, _qg, _qb;
// To finetune hue, you would have to recalculate sinetable[]. (Coarse changes can be made with Phase0.)
int8_t _sine_table[27]; // 8*sin(x*2pi/12)
int8_t _signal_low[0x40];
int8_t _signal_high[0x40];
int delay = 0;

void ntsc_bisqwit_init(void) {
	const double pi = atan(1.0) * 4;
	const int8_t signal_luma_low[4] = { -29, -15, 22, 71 };
	const int8_t signal_luma_high[4] = { 32, 66, 105, 105 };
	int contrast = (int)((nes_ntsc_bisqwit.contrast + 1.0) * (nes_ntsc_bisqwit.contrast + 1.0) * 167941);
	int saturation = (int)((nes_ntsc_bisqwit.saturation + 1.0) * (nes_ntsc_bisqwit.saturation + 1.0) * 144044);
	int i;

	delay = 0;

	for(i = 0; i < 27; i++) {
		_sine_table[i] = (int8_t)(8 * sin(i * 2 * pi / 12 + nes_ntsc_bisqwit.hue * pi));
	}

	_res_divider = 8 / gfx.filter.factor;

	_yWidth = nes_ntsc_bisqwit.ywidth;
	_iWidth = nes_ntsc_bisqwit.iwidth;
	_qWidth = nes_ntsc_bisqwit.qwidth;

	_y = contrast / _yWidth;

	_ir = (int)(contrast * 1.994681e-6 * saturation / _iWidth);
	_qr = (int)(contrast * 9.915742e-7 * saturation / _qWidth);

	_ig = (int)(contrast * 9.151351e-8 * saturation / _iWidth);
	_qg = (int)(contrast * -6.334805e-7 * saturation / _qWidth);

	_ib = (int)(contrast * -1.012984e-6 * saturation / _iWidth);
	_qb = (int)(contrast * 1.667217e-6 * saturation / _qWidth);

	//Precalculate the low and high signal chosen for each 64 base colors
	for(i = 0; i <= 0x3F; i++) {
		int r = (i & 0x0F) >= 0x0E ? 0x1D : i;
		int m = signal_luma_low[r / 0x10];
		int q = signal_luma_high[r / 0x10];

		if((r & 0x0F) == 0x0D) {
			q = m;
		} else if((r & 0x0F) == 0) {
			m = q;
		}
		_signal_low[i] = m;
		_signal_high[i] = q;
	}
}
void ntsc_bisqwit_surface(void) {
	_ntsc_bisqwit_thread param[NTSC_BISQWIT_NUM_SLICE];
	thread_t thread[NTSC_BISQWIT_NUM_SLICE];
	int height = (SCR_ROWS / NTSC_BISQWIT_NUM_SLICE);
	static int delay = 0;
	unsigned int i;

	if (delay++ == 3) {
		delay = 0;
	}

	for (i = 0; i < NTSC_BISQWIT_NUM_SLICE; i++) {
		param[i].slice = i;
		param[i].factor = gfx.filter.factor;
		param[i].src = ppu_screen.rd->data;
		param[i].dst = (uint32_t *)gfx.filter.data.pix;
		param[i].palette = (uint32_t *)gfx.filter.data.palette;
		param[i].start_row = height * i;
		param[i].end_row = (param[i].start_row + height) - 1;
		param[i].start_phase = (!delay ? 8 : 0) + ((param[i].start_row * SCR_COLUMNS) * 8);
		thread_create(thread[i], ntsc_bisqwit_mt, &param[i]);
	}

	for (i = 0; i < NTSC_BISQWIT_NUM_SLICE; i++) {
		thread_join(thread[i]);
		thread_free(thread[i]);
	}
}

void ntsc_bisqwit_filter_parameters_changed(void) {
	ntsc_bisqwit_init();
}
void ntsc_bisqwit_filter_parameters_default(void) {
	nes_ntsc_bisqwit = nes_ntsc_bisqwit_default;
}
void ntsc_bisqwit_filter_parameter_default(int index) {
	const nes_ntsc_bisqwit_setup_t *format = &nes_ntsc_bisqwit_default;

	switch (index) {
		default:
		case 0:
			nes_ntsc_bisqwit.hue = format->hue;
			break;
		case 1:
			nes_ntsc_bisqwit.saturation = format->saturation;
			break;
		case 2:
			nes_ntsc_bisqwit.contrast = format->contrast;
			break;
		case 3:
			nes_ntsc_bisqwit.brightness = format->brightness;
			break;
		case 4:
			nes_ntsc_bisqwit.ywidth = format->ywidth;
			break;
		case 5:
			nes_ntsc_bisqwit.iwidth = format->iwidth;
			break;
		case 6:
			nes_ntsc_bisqwit.qwidth = format->qwidth;
			break;
		case 7:
			nes_ntsc_bisqwit.scanline_intensity = format->scanline_intensity;
			break;
	}
}
void ntsc_bisqwit_filter_parameter_v_default(void) {
	const nes_ntsc_bisqwit_setup_t *format = &nes_ntsc_bisqwit_default;

	nes_ntsc_bisqwit.vertical_blend = format->vertical_blend;
}

thread_funct(ntsc_bisqwit_mt, void *arg) {
	_ntsc_bisqwit_thread *p = (_ntsc_bisqwit_thread *)arg;
	uint32_t *dst = p->dst + ((SCR_COLUMNS * gfx.filter.factor * 8) / _res_divider * p->start_row);

	ntsc_bisqwit_decode_frame(p->start_row, p->end_row, p->src, dst, p->start_phase);

	thread_funct_return();
}

INLINE static void ntsc_bisqwit_generate_signal(const WORD *screen, int8_t *ntsc_signal, int *phase, int row_number) {
	int x;

	for (x = 0; x < SCR_COLUMNS ; x++) {
		uint16_t color = screen[(row_number << 8) | (x < 0 ? 0 : (x >= SCR_COLUMNS ? SCR_COLUMNS - 1 : x))];
		int8_t low = _signal_low[color & 0x3F];
		int8_t high = _signal_high[color & 0x3F];
		int8_t emphasis = color >> 6;
		uint16_t phase_bitmask = _bitmask_lut[abs((*phase) - (color & 0x0F)) % 12];
		uint8_t voltage;

		for (int j = 0; j < 8; j++) {
			phase_bitmask <<= 1;
			voltage = high;
			if (phase_bitmask >= 0x40) {
				if (phase_bitmask == 0x1000) {
					phase_bitmask = 1;
				} else {
					voltage = low;
				}
			}
			if (phase_bitmask & emphasis) {
				voltage -= (voltage / 4);
			}
			ntsc_signal[(x << 3) | j] = voltage;
		}
		(*phase) += _signals_per_pixel;
	}
}
INLINE static void ntsc_bisqwit_recursive_blend(int iteration_count, uint64_t *output, uint64_t *current_line, uint64_t *next_line, int pixels_per_cycle) {
	// Blend 2 pixels at once
	uint32_t width = SCR_COLUMNS * pixels_per_cycle / 2, x;
	double scanline_intensity = nes_ntsc_bisqwit.scanline_intensity;
	BYTE enable = FALSE;

	switch (_res_divider) {
		case 1:
			enable = ((scanline_intensity < 1.0) && (iteration_count == 4));
			break;
		case 2:
			enable = ((scanline_intensity < 1.0) && (iteration_count == 2));
			break;
		case 4:
			enable = (scanline_intensity < 1.0);
			break;
		default:
			break;
	}

	if (enable) {
		// Most likely extremely inefficient scanlines, but works
		for (x = 0; x < width; x++) {
			uint64_t mixed;
			uint8_t r1, g1, b1;
			uint8_t r2, g2, b2;

			if (nes_ntsc_bisqwit.vertical_blend) {
				mixed = ((((current_line[x] ^ next_line[x]) & 0xFEFEFEFEFEFEFEFEL) >> 1) + (current_line[x] & next_line[x]));
			} else {
				mixed = current_line[x];
			}

			r1 = (mixed >> 16) & 0xFF;
			g1 = (mixed >>  8) & 0xFF;
			b1 = (mixed >>  0) & 0xFF;
			r2 = (mixed >> 48) & 0xFF;
			g2 = (mixed >> 40) & 0xFF;
			b2 = (mixed >> 32) & 0xFF;
			r1 = (uint8_t)(r1 * scanline_intensity);
			g1 = (uint8_t)(g1 * scanline_intensity);
			b1 = (uint8_t)(b1 * scanline_intensity);
			r2 = (uint8_t)(r2 * scanline_intensity);
			g2 = (uint8_t)(g2 * scanline_intensity);
			b2 = (uint8_t)(b2 * scanline_intensity);

			output[x] = ((uint64_t)r2 << 48) | ((uint64_t)g2 << 40) | ((uint64_t)b2 << 32) | (r1 << 16) | (g1 << 8) | b1;
		}
	} else if (nes_ntsc_bisqwit.vertical_blend) {
		for (x = 0; x < width; x++) {
			output[x] = ((((current_line[x] ^ next_line[x]) & 0xFEFEFEFEFEFEFEFEL) >> 1) + (current_line[x] & next_line[x]));
		}
	} else {
		memcpy(output, current_line, width * sizeof(uint64_t));
	}

	iteration_count /= 2;

	if (iteration_count > 0) {
		ntsc_bisqwit_recursive_blend(iteration_count, output - width * iteration_count,
			current_line, output, pixels_per_cycle);
		ntsc_bisqwit_recursive_blend(iteration_count, output + width * iteration_count,
			output, next_line, pixels_per_cycle);
	}
}
INLINE static void ntsc_bisqwit_decode_frame(int start_row, int end_row, const WORD *screen, uint32_t *pix, int start_phase) {
	int pixels_per_cycle = 8 / _res_divider, phase = start_phase;
	const int line_width = SCR_COLUMNS;
	int8_t row_signal[line_width * _signals_per_pixel];
	uint32_t row_pixel_gap = SCR_COLUMNS * gfx.filter.factor * pixels_per_cycle;
	uint32_t *pix_org = pix;
	int y;

	for (y = start_row; y <= end_row; y++) {
		int start_cycle = phase % 12;

		// Convert the PPU's output to an NTSC signal
		ntsc_bisqwit_generate_signal(screen, row_signal, &phase, y);
		// Convert the NTSC signal to RGB
		ntsc_bisqwit_decode_line(line_width * _signals_per_pixel, row_signal, pix, (start_cycle + 7) % 12);

		pix += row_pixel_gap;
	}

	// Generate the missing vertical lines
	{
		int last_row = SCR_ROWS - 1, y;

		pix = pix_org;
		for (y = start_row; y <= end_row; y++) {
			uint64_t *current_line = (uint64_t *)pix;
			uint64_t *next_line = (y == last_row ? current_line : (uint64_t *)(pix + row_pixel_gap));
			uint64_t *buffer = (uint64_t *)(pix + (row_pixel_gap / 2));

			ntsc_bisqwit_recursive_blend(4 / _res_divider, buffer, current_line, next_line, pixels_per_cycle);
			pix += row_pixel_gap;
		}
	}
}
/*
 * NTSC_DecodeLine(Width, Signal, Target, Phase0)
 *
 * Convert NES NTSC graphics signal into RGB using integer arithmetics only.
 *
 * Width: Number of NTSC signal samples.
 *        For a 256 pixels wide screen, this would be 256*8. 283*8 if you include borders.
 *
 * Signal: An array of Width samples.
 *         The following sample values are recognized:
 *          -29 = Luma 0 low   32 = Luma 0 high (-38 and  6 when attenuated)
 *          -15 = Luma 1 low   66 = Luma 1 high (-28 and 31 when attenuated)
 *           22 = Luma 2 low  105 = Luma 2 high ( -1 and 58 when attenuated)
 *           71 = Luma 3 low  105 = Luma 3 high ( 34 and 58 when attenuated)
 *         In this scale, sync signal would be -59 and colorburst would be -40 and 19,
 *         but these are not interpreted specially in this function.
 *         The value is calculated from the relative voltage with:
 *                   floor((voltage-0.518)*1000/12)-15
 *
 * Target: Pointer to a storage for Width RGB32 samples (00rrggbb).
 *         Note that the function will produce a RGB32 value for _every_ half-clock-cycle.
 *         This means 2264 RGB samples if you render 283 pixels per scanline (incl. borders).
 *         The caller can pick and choose those columns they want from the signal
 *         to render the picture at their desired resolution.
 *
 * Phase0: An integer in range 0-11 that describes the phase offset into colors on this scanline.
 *         Would be generated from the PPU clock cycle counter at the start of the scanline.
 *         In essence it conveys in one integer the same information that real NTSC signal
 *         would convey in the colorburst period in the beginning of each scanline.
 *
 * Ywidth, Iwidth and Qwidth are the filter widths for Y,I,Q respectively.
 * All widths at 12 produce the best signal quality.
 * 12,24,24 would be the closest values matching the NTSC spec.
 * But off-spec values 12,22,26 are used here, to bring forth mild
 * "chroma dots", an artifacting common with badly tuned TVs.
 * Larger values = more horizontal blurring.
*/
INLINE static void ntsc_bisqwit_decode_line(int width, const int8_t *signal, uint32_t *pix, int phase0) {
	int brightness = (int)(nes_ntsc_bisqwit.brightness * 750);
	int ysum = brightness, isum = 0, qsum = 0;
	int r, g, b;

#define bread(pos) (pos >= 0 ? signal[pos] : 0)
#define bcos(pos) _sine_table[(pos + 36) % 12 + phase0]
#define bsin(pos) _sine_table[(pos + 36) % 12 + 3 + phase0]

	for (int s = 0; s < width; s++) {
		ysum += bread(s)           - bread(s - _yWidth);
		isum += bread(s) * bcos(s) - bread(s - _iWidth) * bcos(s - _iWidth);
		qsum += bread(s) * bsin(s) - bread(s - _qWidth) * bsin(s - _qWidth);
		if (!(s % _res_divider)) {
			r = FHMIN(255, FHMAX(0, (ysum * _y + isum * _ir + qsum * _qr) / 65536));
			g = FHMIN(255, FHMAX(0, (ysum * _y + isum * _ig + qsum * _qg) / 65536));
			b = FHMIN(255, FHMAX(0, (ysum * _y + isum * _ib + qsum * _qb) / 65536));
			(*pix) = 0xFF000000 | (r << 16) | (g << 8) | b;
			pix++;
		}
	}

#undef bread
#undef bcos
#undef bsin
}