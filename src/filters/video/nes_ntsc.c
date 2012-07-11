/* nes_ntsc 0.2.2. http://www.slack.net/~ant/ */

#include <SDL.h>
#include "nes_ntsc.h"
#include "sdlgfx.h"
#include "overscan.h"

/* Copyright (C) 2006-2007 Shay Green. This module is free software; you
can redistribute it and/or modify it under the terms of the GNU Lesser
General Public License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version. This
module is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
details. You should have received a copy of the GNU Lesser General Public
License along with this module; if not, write to the Free Software Foundation,
Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA */

nes_ntsc_setup_t const nes_ntsc_monochrome = { 0,-1, 0, 0,.2,  0,.2,-.2,-.2,-1, 1, 0, 0, 0, 0 };
nes_ntsc_setup_t const nes_ntsc_composite  = { 0, 0, 0, 0, 0,  0, 0,  0,  0, 0, 1, 0, 0, 0, 0 };
nes_ntsc_setup_t const nes_ntsc_svideo     = { 0, 0, 0, 0,.2,  0,.2, -1, -1, 0, 1, 0, 0, 0, 0 };
nes_ntsc_setup_t const nes_ntsc_rgb        = { 0, 0, 0, 0,.2,  0,.7, -1, -1,-1, 1, 0, 0, 0, 0 };

#define alignment_count 3
#define burst_count     3
#define rescale_in      8
#define rescale_out     7

#define artifacts_mid   1.0f
#define fringing_mid    1.0f
#define std_decoder_hue -15

#define STD_HUE_CONDITION( setup ) !(setup->base_palette || setup->palette)
/* implementazione */
/* Common implementation of NTSC filters */

#define DISABLE_CORRECTION 0

#undef PI
#define PI 3.14159265358979323846f

#ifndef LUMA_CUTOFF
	#define LUMA_CUTOFF 0.20
#endif
#ifndef gamma_size
	#define gamma_size 1
#endif
#ifndef rgb_bits
	#define rgb_bits 8
#endif
#ifndef artifacts_max
	#define artifacts_max (artifacts_mid * 1.5f)
#endif
#ifndef fringing_max
	#define fringing_max (fringing_mid * 2)
#endif
#ifndef STD_HUE_CONDITION
	#define STD_HUE_CONDITION( setup ) 1
#endif

#define ext_decoder_hue     (std_decoder_hue + 15)
#define rgb_unit            (1 << rgb_bits)
#define rgb_offset          (rgb_unit * 2 + 0.5f)

enum { burst_size  = nes_ntsc_entry_size / burst_count };
enum { kernel_half = 16 };
enum { kernel_size = kernel_half * 2 + 1 };

typedef struct init_t {
	float to_rgb[burst_count * 6];
	float to_float[gamma_size];
	float contrast;
	float brightness;
	float artifacts;
	float fringing;
	float kernel[rescale_out * kernel_size * 2];
} init_t;

#define ROTATE_IQ( i, q, sin_b, cos_b ) {\
	float t;\
	t = i * cos_b - q * sin_b;\
	q = i * sin_b + q * cos_b;\
	i = t;\
}

static void init_filters(init_t* impl, nes_ntsc_setup_t const* setup) {
#if rescale_out > 1
	float kernels[kernel_size * 2];
#else
	float* const kernels = impl->kernel;
#endif

	/* generate luma (y) filter using sinc kernel */
	{
		/* sinc with rolloff (dsf) */
		float const rolloff = 1 + (float) setup->sharpness * (float) 0.032;
		float const maxh = 32;
		float const pow_a_n = (float) pow(rolloff, maxh);
		float sum;
		int i;
		/* quadratic mapping to reduce negative (blurring) range */
		float to_angle = (float) setup->resolution + 1;
		to_angle = PI / maxh * (float) LUMA_CUTOFF * (to_angle * to_angle + 1);

		kernels[kernel_size * 3 / 2] = maxh; /* default center value */

		for (i = 0; i < kernel_half * 2 + 1; i++) {
			int x = i - kernel_half;
			float angle = x * to_angle;
			/* instability occurs at center point with rolloff very close to 1.0 */
			if (x || pow_a_n > (float) 1.056 || pow_a_n < (float) 0.981) {
				float rolloff_cos_a = rolloff * (float) cos(angle);
				float num = 1 - rolloff_cos_a - pow_a_n * (float) cos(maxh * angle)
				        		+ pow_a_n * rolloff * (float) cos((maxh - 1) * angle);
				float den = 1 - rolloff_cos_a - rolloff_cos_a + rolloff * rolloff;
				float dsf = num / den;
				kernels[kernel_size * 3 / 2 - kernel_half + i] = dsf - (float) 0.5;
			}
		}

		/* apply blackman window and find sum */
		sum = 0;
		for (i = 0; i < kernel_half * 2 + 1; i++) {
			float x = PI * 2 / (kernel_half * 2) * i;
			float blackman = 0.42f - 0.5f * (float) cos(x) + 0.08f * (float) cos(x * 2);
			sum += (kernels[kernel_size * 3 / 2 - kernel_half + i] *= blackman);
		}

		/* normalize kernel */
		sum = 1.0f / sum;
		for (i = 0; i < kernel_half * 2 + 1; i++) {
			int x = kernel_size * 3 / 2 - kernel_half + i;
			kernels[x] *= sum;
			assert( kernels [x] == kernels [x]);
			/* catch numerical instability */
		}
	}

	/* generate chroma (iq) filter using gaussian kernel */
	{
		float const cutoff_factor = -0.03125f;
		float cutoff = (float) setup->bleed;
		int i;

		if (cutoff < 0) {
			/* keep extreme value accessible only near upper end of scale (1.0) */
			cutoff *= cutoff;
			cutoff *= cutoff;
			cutoff *= cutoff;
			cutoff *= -30.0f / 0.65f;
		}
		cutoff = cutoff_factor - 0.65f * cutoff_factor * cutoff;

		for (i = -kernel_half; i <= kernel_half; i++)
			kernels[kernel_size / 2 + i] = (float) exp(i * i * cutoff);

		/* normalize even and odd phases separately */
		for (i = 0; i < 2; i++) {
			float sum = 0;
			int x;
			for (x = i; x < kernel_size; x += 2)
				sum += kernels[x];

			sum = 1.0f / sum;
			for (x = i; x < kernel_size; x += 2) {
				kernels[x] *= sum;
				assert( kernels [x] == kernels [x]);
				/* catch numerical instability */
			}
		}
	}

	/*
	printf( "luma:\n" );
	for ( i = kernel_size; i < kernel_size * 2; i++ )
		printf( "%f\n", kernels [i] );
	printf( "chroma:\n" );
	for ( i = 0; i < kernel_size; i++ )
		printf( "%f\n", kernels [i] );
	*/

	/* generate linear rescale kernels */
#if rescale_out > 1
	{
		float weight = 1.0f;
		float* out = impl->kernel;
		int n = rescale_out;
		do {
			float remain = 0;
			int i;
			weight -= 1.0f / rescale_in;
			for (i = 0; i < kernel_size * 2; i++) {
				float cur = kernels[i];
				float m = cur * weight;
				*out++ = m + remain;
				remain = cur - m;
			}
		} while (--n);
	}
#endif
}

static float const default_decoder[6] = { 0.956f, 0.621f, -0.272f, -0.647f, -1.105f, 1.702f };

static void init(init_t* impl, nes_ntsc_setup_t const* setup) {
	impl->brightness = (float) setup->brightness * (0.5f * rgb_unit) + rgb_offset;
	impl->contrast = (float) setup->contrast * (0.5f * rgb_unit) + rgb_unit;
#ifdef default_palette_contrast
	if ( !setup->palette )
		impl->contrast *= default_palette_contrast;
#endif

	impl->artifacts = (float) setup->artifacts;
	if (impl->artifacts > 0)
		impl->artifacts *= artifacts_max - artifacts_mid;
	impl->artifacts = impl->artifacts * artifacts_mid + artifacts_mid;

	impl->fringing = (float) setup->fringing;
	if (impl->fringing > 0)
		impl->fringing *= fringing_max - fringing_mid;
	impl->fringing = impl->fringing * fringing_mid + fringing_mid;

	init_filters(impl, setup);

	/* generate gamma table */
	if (gamma_size > 1) {
		float const to_float = 1.0f / (gamma_size - (gamma_size > 1));
		float const gamma = 1.1333f - (float) setup->gamma * 0.5f;
		/* match common PC's 2.2 gamma to TV's 2.65 gamma */
		int i;
		for (i = 0; i < gamma_size; i++)
			impl->to_float[i] = (float) pow(i * to_float, gamma) * impl->contrast
			+ impl->brightness;
	}

	/* setup decoder matricies */
	{
		float hue = (float) setup->hue * PI + PI / 180 * ext_decoder_hue;
		float sat = (float) setup->saturation + 1;
		float const* decoder = setup->decoder_matrix;
		if (!decoder) {
			decoder = default_decoder;
			if (STD_HUE_CONDITION( setup ))
				hue += PI / 180 * (std_decoder_hue - ext_decoder_hue);
		}

		{
			float s = (float) sin(hue) * sat;
			float c = (float) cos(hue) * sat;
			float* out = impl->to_rgb;
			int n;

			n = burst_count;
			do {
				float const* in = decoder;
				int n = 3;
				do {
					float i = *in++;
					float q = *in++;
					*out++ = i * c - q * s;
					*out++ = i * s + q * c;
				} while (--n);
				if (burst_count <= 1)
					break;ROTATE_IQ( s, c, 0.866025f, -0.5f);
				/* +120 degrees */
			} while (--n);
		}
	}
}

/* kernel generation */

#define RGB_TO_YIQ( r, g, b, y, i ) (\
	(y = (r) * 0.299f + (g) * 0.587f + (b) * 0.114f),\
	(i = (r) * 0.596f - (g) * 0.275f - (b) * 0.321f),\
	((r) * 0.212f - (g) * 0.523f + (b) * 0.311f)\
)

#define YIQ_TO_RGB( y, i, q, to_rgb, type, r, g ) (\
	r = (type) (y + to_rgb [0] * i + to_rgb [1] * q),\
	g = (type) (y + to_rgb [2] * i + to_rgb [3] * q),\
	(type) (y + to_rgb [4] * i + to_rgb [5] * q)\
)

#define PACK_RGB( r, g, b ) ((r) << 21 | (g) << 11 | (b) << 1)

enum { rgb_kernel_size = burst_size / alignment_count };
enum { rgb_bias = rgb_unit * 2 * nes_ntsc_rgb_builder };

typedef struct pixel_info_t {
	int offset;
	float negate;
	float kernel[4];
} pixel_info_t;

#if rescale_in > 1
	#define PIXEL_OFFSET_( ntsc, scaled ) \
		(kernel_size / 2 + ntsc + (scaled != 0) + (rescale_out - scaled) % rescale_out + \
				(kernel_size * 2 * scaled))

	#define PIXEL_OFFSET( ntsc, scaled ) \
		PIXEL_OFFSET_( ((ntsc) - (scaled) / rescale_out * rescale_in),\
			(((scaled) + rescale_out * 10) % rescale_out) ),\
			(1.0f - (((ntsc) + 100) & 2))
#else
	#define PIXEL_OFFSET( ntsc, scaled ) \
		(kernel_size / 2 + (ntsc) - (scaled)),\
		(1.0f - (((ntsc) + 100) & 2))
#endif

extern pixel_info_t const nes_ntsc_pixels [alignment_count];

/* Generate pixel at all burst phases and column alignments */
static void gen_kernel(init_t* impl, float y, float i, float q, nes_ntsc_rgb_t* out) {
	/* generate for each scanline burst phase */
	float const* to_rgb = impl->to_rgb;
	int burst_remain = burst_count;
	y -= rgb_offset;
	do {
		/* Encode yiq into *two* composite signals (to allow control over artifacting).
		 Convolve these with kernels which: filter respective components, apply
		 sharpening, and rescale horizontally. Convert resulting yiq to rgb and pack
		 into integer. Based on algorithm by NewRisingSun. */
		pixel_info_t const* pixel = nes_ntsc_pixels;
		int alignment_remain = alignment_count;
		do {
			/* negate is -1 when composite starts at odd multiple of 2 */
			float const yy = y * impl->fringing * pixel->negate;
			float const ic0 = (i + yy) * pixel->kernel[0];
			float const qc1 = (q + yy) * pixel->kernel[1];
			float const ic2 = (i - yy) * pixel->kernel[2];
			float const qc3 = (q - yy) * pixel->kernel[3];

			float const factor = impl->artifacts * pixel->negate;
			float const ii = i * factor;
			float const yc0 = (y + ii) * pixel->kernel[0];
			float const yc2 = (y - ii) * pixel->kernel[2];

			float const qq = q * factor;
			float const yc1 = (y + qq) * pixel->kernel[1];
			float const yc3 = (y - qq) * pixel->kernel[3];

			float const* k = &impl->kernel[pixel->offset];
			int n;
			++pixel;
			for (n = rgb_kernel_size; n; --n) {
				float i = k[0] * ic0 + k[2] * ic2;
				float q = k[1] * qc1 + k[3] * qc3;
				float y = k[kernel_size + 0] * yc0 + k[kernel_size + 1] * yc1
						+ k[kernel_size + 2] * yc2 + k[kernel_size + 3] * yc3 + rgb_offset;
				if (rescale_out <= 1)
					k--;
				else if (k < &impl->kernel[kernel_size * 2 * (rescale_out - 1)])
					k += kernel_size * 2 - 1;
				else
					k -= kernel_size * 2 * (rescale_out - 1) + 2;
				{
					int r, g, b = YIQ_TO_RGB( y, i, q, to_rgb, int, r, g );
					*out++ = PACK_RGB( r, g, b ) - rgb_bias;
				}
			}
		} while (alignment_count > 1 && --alignment_remain);

		if (burst_count <= 1)
			break;

		to_rgb += 6;

		ROTATE_IQ( i, q, -0.866025f, -0.5f);
		/* -120 degrees */
	} while (--burst_remain);
}

static void correct_errors(nes_ntsc_rgb_t color, nes_ntsc_rgb_t* out);

#if DISABLE_CORRECTION
	#define CORRECT_ERROR( a ) { out [i] += rgb_bias; }
	#define DISTRIBUTE_ERROR( a, b, c ) { out [i] += rgb_bias; }
#else
	#define CORRECT_ERROR( a ) { out [a] += error; }
	#define DISTRIBUTE_ERROR( a, b, c ) {\
		nes_ntsc_rgb_t fourth = (error + 2 * nes_ntsc_rgb_builder) >> 2;\
		fourth &= (rgb_bias >> 1) - nes_ntsc_rgb_builder;\
		fourth -= rgb_bias >> 2;\
		out [a] += fourth;\
		out [b] += fourth;\
		out [c] += fourth;\
		out [i] += error - (fourth * 3);\
	}
#endif

#define RGB_PALETTE_OUT( rgb, out_ )\
{\
	unsigned char* out = (out_);\
	nes_ntsc_rgb_t clamped = (rgb);\
	NES_NTSC_CLAMP_( clamped, (8 - rgb_bits) );\
	out [0] = (unsigned char) (clamped >> 21);\
	out [1] = (unsigned char) (clamped >> 11);\
	out [2] = (unsigned char) (clamped >>  1);\
}

/* blitter related */

#ifndef restrict
	#if defined (__GNUC__)
		#define restrict __restrict__
	#elif defined (_MSC_VER) && _MSC_VER > 1300
		#define restrict __restrict
	#else
		/* no support for restricted pointers */
		#define restrict
	#endif
#endif

/* 3 input pixels -> 8 composite samples */
pixel_info_t const nes_ntsc_pixels [alignment_count] = {
	{ PIXEL_OFFSET( -4, -9 ), { 1, 1, .6667f, 0 } },
	{ PIXEL_OFFSET( -2, -7 ), {       .3333f, 1, 1, .3333f } },
	{ PIXEL_OFFSET(  0, -5 ), {                  0, .6667f, 1, 1 } },
};

static void merge_kernel_fields(nes_ntsc_rgb_t* io) {
	int n;
	for (n = burst_size; n; --n) {
		nes_ntsc_rgb_t p0 = io[burst_size * 0] + rgb_bias;
		nes_ntsc_rgb_t p1 = io[burst_size * 1] + rgb_bias;
		nes_ntsc_rgb_t p2 = io[burst_size * 2] + rgb_bias;
		/* merge colors without losing precision */
		io[burst_size * 0] = ((p0 + p1 - ((p0 ^ p1) & nes_ntsc_rgb_builder)) >> 1) - rgb_bias;
		io[burst_size * 1] = ((p1 + p2 - ((p1 ^ p2) & nes_ntsc_rgb_builder)) >> 1) - rgb_bias;
		io[burst_size * 2] = ((p2 + p0 - ((p2 ^ p0) & nes_ntsc_rgb_builder)) >> 1) - rgb_bias;
		++io;
	}
}
static void correct_errors(nes_ntsc_rgb_t color, nes_ntsc_rgb_t* out) {
	int n;
	for (n = burst_count; n; --n) {
		unsigned i;
		for (i = 0; i < rgb_kernel_size / 2; i++) {
			nes_ntsc_rgb_t error = color - out[i] - out[(i + 12) % 14 + 14]
			        - out[(i + 10) % 14 + 28] - out[i + 7] - out[i + 5 + 14] - out[i + 3 + 28];
			DISTRIBUTE_ERROR( i+3+28, i+5+14, i+7);
		}
		out += alignment_count * rgb_kernel_size;
	}
}
void nes_ntsc_init(nes_ntsc_t* ntsc, nes_ntsc_setup_t const* setup) {
	int merge_fields;
	int entry;
	init_t impl;
	float gamma_factor;

	if (!setup)
		setup = &nes_ntsc_composite;
	init(&impl, setup);

	/* setup fast gamma */
	{
		float gamma = (float) setup->gamma * -0.5f;
		if (STD_HUE_CONDITION( setup ))
			gamma += 0.1333f;

		gamma_factor = (float) pow((float) fabs(gamma), 0.73f);
		if (gamma < 0)
			gamma_factor = -gamma_factor;
	}

	merge_fields = setup->merge_fields;
	if (setup->artifacts <= -1 && setup->fringing <= -1)
		merge_fields = 1;

	for (entry = 0; entry < nes_ntsc_palette_size; entry++) {
		/* Base 64-color generation */
		static float const lo_levels[4] = { -0.12f, 0.00f, 0.31f, 0.72f };
		static float const hi_levels[4] = { 0.40f, 0.68f, 1.00f, 1.00f };
		int level = entry >> 4 & 0x03;
		float lo = lo_levels[level];
		float hi = hi_levels[level];

		int color = entry & 0x0F;
		if (color == 0)
			lo = hi;
		if (color == 0x0D)
			hi = lo;
		if (color > 0x0D)
			hi = lo = 0.0f;

		{
			/* phases [i] = cos( i * PI / 6 ) */
			static float const phases[0x10 + 3] = { -1.0f, -0.866025f, -0.5f, 0.0f, 0.5f, 0.866025f,
					1.0f, 0.866025f, 0.5f, 0.0f, -0.5f, -0.866025f, -1.0f, -0.866025f, -0.5f, 0.0f,
					0.5f, 0.866025f, 1.0f };
#define TO_ANGLE_SIN( color )   phases [color]
#define TO_ANGLE_COS( color )   phases [(color) + 3]

			/* Convert raw waveform to YIQ */
			float sat = (hi - lo) * 0.5f;
			float i = TO_ANGLE_SIN( color ) * sat;
			float q = TO_ANGLE_COS( color ) * sat;
			float y = (hi + lo) * 0.5f;

			/* Optionally use base palette instead */
			if (setup->base_palette) {
				unsigned char const* in = &setup->base_palette[(entry & 0x3F) * 3];
				static float const to_float = 1.0f / 0xFF;
				float r = to_float * in[0];
				float g = to_float * in[1];
				float b = to_float * in[2];
				q = RGB_TO_YIQ( r, g, b, y, i );
			}

			/* Apply color emphasis */
#ifdef NES_NTSC_EMPHASIS
			{
				int tint = entry >> 6 & 7;
				if (tint && color <= 0x0D) {
					static float const atten_mul = 0.79399f;
					static float const atten_sub = 0.0782838f;

					if (tint == 7) {
						y = y * (atten_mul * 1.13f) - (atten_sub * 1.13f);
					} else {
						static unsigned char const tints[8] = { 0, 6, 10, 8, 2, 4, 0, 0 };
						int const tint_color = tints[tint];
						float sat = hi * (0.5f - atten_mul * 0.5f) + atten_sub * 0.5f;
						y -= sat * 0.5f;
						if (tint >= 3 && tint != 4) {
							/* combined tint bits */
							sat *= 0.6f;
							y -= sat;
						}
						i += TO_ANGLE_SIN( tint_color ) * sat;
						q += TO_ANGLE_COS( tint_color ) * sat;
					}
				}
			}
#endif

			/* Optionally use palette instead */
			if (setup->palette) {
				unsigned char const* in = &setup->palette[entry * 3];
				static float const to_float = 1.0f / 0xFF;
				float r = to_float * in[0];
				float g = to_float * in[1];
				float b = to_float * in[2];
				q = RGB_TO_YIQ( r, g, b, y, i );
			}

			/* Apply brightness, contrast, and gamma */
			y *= (float) setup->contrast * 0.5f + 1;
			/* adjustment reduces error when using input palette */
			y += (float) setup->brightness * 0.5f - 0.5f / 256;

			{
				float r, g, b = YIQ_TO_RGB( y, i, q, default_decoder, float, r, g );

				/* fast approximation of n = pow( n, gamma ) */
				r = (r * gamma_factor - gamma_factor) * r + r;
				g = (g * gamma_factor - gamma_factor) * g + g;
				b = (b * gamma_factor - gamma_factor) * b + b;

				q = RGB_TO_YIQ( r, g, b, y, i );
			}

			i *= rgb_unit;
			q *= rgb_unit;
			y *= rgb_unit;
			y += rgb_offset;

			/* Generate kernel */
			{
				int r, g, b = YIQ_TO_RGB( y, i, q, impl.to_rgb, int, r, g );
				/* blue tends to overflow, so clamp it */
				nes_ntsc_rgb_t rgb = PACK_RGB( r, g, (b < 0x3E0 ? b: 0x3E0) );

				if (setup->palette_out)
					RGB_PALETTE_OUT( rgb, &setup->palette_out [entry * 3]);

				if (ntsc) {
					nes_ntsc_rgb_t* kernel = ntsc->table[entry];
					gen_kernel(&impl, y, i, q, kernel);
					if (merge_fields)
						merge_kernel_fields(kernel);
					correct_errors(rgb, kernel);
				}
			}
		}
	}
}
#ifndef NES_NTSC_NO_BLITTERS
#define NTSCX2(type)\
{\
	type *restrict line_out = (type *) rgb_out;\
	for ( n = chunk_count; n; --n ) {\
		NES_NTSC_COLOR_IN( 0, NES_NTSC_ADJ_IN( line_in [0] ) );\
		NES_NTSC_RGB_OUT( 0, line_out [0], depth );\
		NES_NTSC_RGB_OUT( 1, line_out [1], depth );\
		NES_NTSC_COLOR_IN( 1, NES_NTSC_ADJ_IN( line_in [1] ) );\
		NES_NTSC_RGB_OUT( 2, line_out [2], depth );\
		NES_NTSC_RGB_OUT( 3, line_out [3], depth );\
		NES_NTSC_COLOR_IN( 2, NES_NTSC_ADJ_IN( line_in [2] ) );\
		NES_NTSC_RGB_OUT( 4, line_out [4], depth );\
		NES_NTSC_RGB_OUT( 5, line_out [5], depth );\
		NES_NTSC_RGB_OUT( 6, line_out [6], depth );\
		line_in  += 3;\
		line_out += 7;\
	}\
	if (!overscan.enabled) {\
		NES_NTSC_COLOR_IN( 0, nes_ntsc_black );\
		NES_NTSC_RGB_OUT( 0, line_out [0], depth );\
		NES_NTSC_RGB_OUT( 1, line_out [1], depth );\
		NES_NTSC_COLOR_IN( 1, nes_ntsc_black );\
		NES_NTSC_RGB_OUT( 2, line_out [2], depth );\
		NES_NTSC_RGB_OUT( 3, line_out [3], depth );\
		NES_NTSC_COLOR_IN( 2, nes_ntsc_black );\
		NES_NTSC_RGB_OUT( 4, line_out [4], depth );\
		NES_NTSC_RGB_OUT( 5, line_out [5], depth );\
		NES_NTSC_RGB_OUT( 6, line_out [6], depth );\
	}\
	burst_phase = (burst_phase + 1) % nes_ntsc_burst_count;\
	input += in_row_width;\
	rgb_out = (char*) rgb_out + out_pitch;\
}
void nes_ntscx2(nes_ntsc_t const* ntsc, NES_NTSC_IN_T const* input, long in_row_width,
		int burst_phase, int in_width, int in_height, void* rgb_out, long out_pitch, int depth) {

	int chunk_count = (in_width - 1) / nes_ntsc_in_chunk;
	for (; in_height; --in_height) {
		int n;
		NES_NTSC_IN_T const* line_in = input;
		NES_NTSC_BEGIN_ROW( ntsc, burst_phase, nes_ntsc_black, nes_ntsc_black,
				NES_NTSC_ADJ_IN( *line_in ));

		if (overscan.enabled) {
			unsigned color_;

			chunk_count++;
			line_in += overscan.left;

			kernel0 = (color_ = (line_in[0]), NES_NTSC_ENTRY_( ktable, color_ ));
			kernel1 = (color_ = (line_in[1]), NES_NTSC_ENTRY_( ktable, color_ ));
			kernel2 = (color_ = (line_in[2]), NES_NTSC_ENTRY_( ktable, color_ ));

			line_in += nes_ntsc_in_chunk;
		} else {
			++line_in;
		}

		switch (depth) {
			case 15:
			case 16:
				NTSCX2(Uint16)
				break;
			case 24:
			case 32:
				NTSCX2(Uint32)
				break;
		}
	}
}
#define NTSCX3(type)\
{\
	type *restrict line_out = (type *) rgb_out;\
	for ( n = chunk_count; n; --n ) {\
		NES_NTSC_COLOR_IN( 0, NES_NTSC_ADJ_IN( line_in [0] ) );\
		NES_NTSC_RGB_OUT( 0, line_out [0], depth );\
		NES_NTSC_RGB_OUT( 1, line_out [1], depth );\
		NES_NTSC_RGB_OUT( 1, line_out [2], depth );\
		NES_NTSC_COLOR_IN( 1, NES_NTSC_ADJ_IN( line_in [1] ) );\
		NES_NTSC_RGB_OUT( 2, line_out [3], depth );\
		NES_NTSC_RGB_OUT( 3, line_out [4], depth );\
		NES_NTSC_RGB_OUT( 3, line_out [5], depth );\
		NES_NTSC_COLOR_IN( 2, NES_NTSC_ADJ_IN( line_in [2] ) );\
		NES_NTSC_RGB_OUT( 4, line_out [6], depth );\
		NES_NTSC_RGB_OUT( 5, line_out [7], depth );\
		NES_NTSC_RGB_OUT( 5, line_out [8], depth );\
		NES_NTSC_RGB_OUT( 6, line_out [9], depth );\
		line_in  += 3;\
		line_out += 10;\
	}\
	if (!overscan.enabled) {\
		NES_NTSC_COLOR_IN( 0, nes_ntsc_black );\
		NES_NTSC_RGB_OUT( 0, line_out [0], depth );\
		NES_NTSC_RGB_OUT( 1, line_out [1], depth );\
		NES_NTSC_RGB_OUT( 1, line_out [2], depth );\
		NES_NTSC_COLOR_IN( 1, nes_ntsc_black );\
		NES_NTSC_RGB_OUT( 2, line_out [3], depth );\
		NES_NTSC_RGB_OUT( 3, line_out [4], depth );\
		NES_NTSC_RGB_OUT( 3, line_out [5], depth );\
		NES_NTSC_COLOR_IN( 2, nes_ntsc_black );\
		NES_NTSC_RGB_OUT( 4, line_out [6], depth );\
		NES_NTSC_RGB_OUT( 5, line_out [7], depth );\
		NES_NTSC_RGB_OUT( 5, line_out [8], depth );\
		NES_NTSC_RGB_OUT( 6, line_out [9], depth );\
	}\
	burst_phase = (burst_phase + 1) % nes_ntsc_burst_count;\
	input += in_row_width;\
	rgb_out = (char*) rgb_out + out_pitch;\
}
void nes_ntscx3(nes_ntsc_t const* ntsc, NES_NTSC_IN_T const* input, long in_row_width,
		int burst_phase, int in_width, int in_height, void* rgb_out, long out_pitch, int depth) {

	int chunk_count = (in_width - 1) / nes_ntsc_in_chunk;
	for (; in_height; --in_height) {
		int n;
		NES_NTSC_IN_T const* line_in = input;
		NES_NTSC_BEGIN_ROW( ntsc, burst_phase, nes_ntsc_black, nes_ntsc_black,
				NES_NTSC_ADJ_IN( *line_in ));

		if (overscan.enabled) {
			unsigned color_;

			chunk_count++;
			line_in += overscan.left;

			kernel0 = (color_ = (line_in[0]), NES_NTSC_ENTRY_( ktable, color_ ));
			kernel1 = (color_ = (line_in[1]), NES_NTSC_ENTRY_( ktable, color_ ));
			kernel2 = (color_ = (line_in[2]), NES_NTSC_ENTRY_( ktable, color_ ));

			line_in += nes_ntsc_in_chunk;
		} else {
			++line_in;
		}

		switch (depth) {
			case 15:
			case 16:
				NTSCX3(Uint16)
				break;
			case 24:
			case 32:
				NTSCX3(Uint32)
				break;
		}
	}
}
#define NTSCX4(type)\
{\
	type *restrict line_out = (type *) rgb_out;\
	for ( n = chunk_count; n; --n ) {\
		NES_NTSC_COLOR_IN( 0, NES_NTSC_ADJ_IN( line_in [0] ) );\
		NES_NTSC_RGB_OUT( 0, line_out [0], depth );\
		NES_NTSC_RGB_OUT( 0, line_out [1], depth );\
		NES_NTSC_RGB_OUT( 1, line_out [2], depth );\
		NES_NTSC_RGB_OUT( 1, line_out [3], depth );\
		NES_NTSC_COLOR_IN( 1, NES_NTSC_ADJ_IN( line_in [1] ) );\
		NES_NTSC_RGB_OUT( 2, line_out [4], depth );\
		NES_NTSC_RGB_OUT( 2, line_out [5], depth );\
		NES_NTSC_RGB_OUT( 3, line_out [6], depth );\
		NES_NTSC_RGB_OUT( 3, line_out [7], depth );\
		NES_NTSC_COLOR_IN( 2, NES_NTSC_ADJ_IN( line_in [2] ) );\
		NES_NTSC_RGB_OUT( 4, line_out [8], depth );\
		NES_NTSC_RGB_OUT( 4, line_out [9], depth );\
		NES_NTSC_RGB_OUT( 5, line_out [10], depth );\
		NES_NTSC_RGB_OUT( 5, line_out [11], depth );\
		NES_NTSC_RGB_OUT( 6, line_out [12], depth );\
		NES_NTSC_RGB_OUT( 6, line_out [13], depth );\
		line_in  += 3;\
		line_out += 14;\
	}\
	if (!overscan.enabled) {\
		NES_NTSC_COLOR_IN( 0, nes_ntsc_black );\
		NES_NTSC_RGB_OUT( 0, line_out [0], depth );\
		NES_NTSC_RGB_OUT( 0, line_out [1], depth );\
		NES_NTSC_RGB_OUT( 1, line_out [2], depth );\
		NES_NTSC_RGB_OUT( 1, line_out [3], depth );\
		NES_NTSC_COLOR_IN( 1, nes_ntsc_black );\
		NES_NTSC_RGB_OUT( 2, line_out [4], depth );\
		NES_NTSC_RGB_OUT( 2, line_out [5], depth );\
		NES_NTSC_RGB_OUT( 3, line_out [6], depth );\
		NES_NTSC_RGB_OUT( 3, line_out [7], depth );\
		NES_NTSC_COLOR_IN( 2, nes_ntsc_black );\
		NES_NTSC_RGB_OUT( 4, line_out [8], depth );\
		NES_NTSC_RGB_OUT( 4, line_out [9], depth );\
		NES_NTSC_RGB_OUT( 5, line_out [10], depth );\
		NES_NTSC_RGB_OUT( 5, line_out [11], depth );\
		NES_NTSC_RGB_OUT( 6, line_out [12], depth );\
		NES_NTSC_RGB_OUT( 6, line_out [13], depth );\
	}\
	burst_phase = (burst_phase + 1) % nes_ntsc_burst_count;\
	input += in_row_width;\
	rgb_out = (char*) rgb_out + out_pitch;\
}
void nes_ntscx4(nes_ntsc_t const* ntsc, NES_NTSC_IN_T const* input, long in_row_width,
		int burst_phase, int in_width, int in_height, void* rgb_out, long out_pitch, int depth) {

	int chunk_count = (in_width - 1) / nes_ntsc_in_chunk;
	for (; in_height; --in_height) {
		int n;
		NES_NTSC_IN_T const* line_in = input;
		NES_NTSC_BEGIN_ROW( ntsc, burst_phase, nes_ntsc_black, nes_ntsc_black,
				NES_NTSC_ADJ_IN( *line_in ));

		if (overscan.enabled) {
			unsigned color_;

			chunk_count++;
			line_in += overscan.left;

			kernel0 = (color_ = (line_in[0]), NES_NTSC_ENTRY_( ktable, color_ ));
			kernel1 = (color_ = (line_in[1]), NES_NTSC_ENTRY_( ktable, color_ ));
			kernel2 = (color_ = (line_in[2]), NES_NTSC_ENTRY_( ktable, color_ ));

			line_in += nes_ntsc_in_chunk;
		} else {
			++line_in;
		}

		switch (depth) {
			case 15:
			case 16:
				NTSCX4(Uint16)
				break;
			case 24:
			case 32:
				NTSCX4(Uint32)
				break;
		}
	}
}
#endif
