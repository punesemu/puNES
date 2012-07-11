/*
 * bisquit.c
 *
 *  Created on: 14/nov/2011
 *      Author: fhorse
 */

#include "common.h"

#define wave(p, color) ((color+8+p)%12 < 6)
#define cosf(p) cos(3.141592653 * p / 6)
#define gammafix(f) (f <= 0.f ? 0.f : pow(f, 2.2f / gamma))
#define clamp(v) (v < 0 ? 0 : v > 255 ? 255 : v)

SDL_Surface *s;

const int xres=256*1, yres=240*1;

static unsigned int framecounter=0;
static unsigned int prev2[3][240][xres]={{{}}};
static unsigned int colorbursts[240];
static unsigned int Colorburst=4;
static u16 prev1[3][240][256]={{{}}}; // NES pixels corresponding to each screen location in each tweak offset
static bool diffs[240] = {false};     // Whether this scanline has changed from what it was the last time

void PutPixel(unsigned px,unsigned py, unsigned pixel)
{
	u16 v = 0x8000^pixel;
	u16 &p = prev1[Colorburst/4][py][px];
	if(p != v) { p = v; diffs[py] = true; }
}
void FlushScanline(unsigned py, unsigned length) /* length is in pixels: 340 or 341. */
{
	if(py < 240) colorbursts[py] = Colorburst;
	if(py == 239)
	{
		//#pragma omp parallel for schedule(guided)
		for(py=0; py<240; ++py)
		{
			unsigned y1 = (py  )*yres/240;
			unsigned y2 = (py+1)*yres/240;
			unsigned colorburst = colorbursts[py];
			unsigned int *target = prev2[colorburst/4][py];
			unsigned int *line   = prev1[colorburst/4][py];
			if(diffs[py])
			{
				float sigbuf[256*8], d07=0.f, d15=0.f;
				float sigi[256*8], sigq[256*8]; // Match I & Q at each position.
				for(unsigned p=0; p<256*8; ++p)
				{
					// Retrieve NTSC signal from PPU
					int pixel = line[p/8]%512, offset = (colorburst+p) % 12;
					// Decode the color index.
					int color = (pixel & 0x0F), level = color<0xE ? (pixel>>4) & 3 : 1;
					// Voltage levels, relative to synch voltage
					static const float black=.518f, white=1.962f, attenuation=.746f,
							levels[8] = {.350f, .518f, .962f,1.550f,  // Signal low
									1.094f,1.506f,1.962f,1.962f}; // Signal high
					// NES NTSC modulator (square wave between two voltage levels):
					float spot = levels[level + 4*(color <= 12*wave(offset, color))];
					// De-emphasis bits attenuate a part of the signal:
					if(((pixel & 0x40) && wave(offset,12))
							|| ((pixel & 0x80) && wave(offset, 4))
							|| ((pixel &0x100) && wave(offset, 8))) spot *= attenuation;
					// Normalize:
					float v = (spot - black) / (white-black);
					// Apply slight signal degradation to it
					v = v-0.5f;
					d07 = d07*0.3f + 0.7f*v;
					d15 = d15*-.5f + 1.5f*v;
					v = 0.5f + d07*0.7f + d15*0.3f;
					sigbuf[p] = v;
					sigi[p] = v * cosf(p+12+colorburst);
					sigq[p] = v * cosf(p+21+colorburst);
				}
				float gamma = 1.8f;
				for(unsigned x=0; x<xres; ++x)
				{
					float i=0.f, q=0.f, y=0.f;
					for(int s = x*256*8 / xres, p=0; p<12; ++p, ++s)
						if(s >= 0 && s < 256*8)
						{ i += sigi[s];
						q += sigq[s];
						y += sigbuf[s]; }
					i /= 12.f; q /= 12.f; y /= 12.f;
					//float amplitude = std::sqrt(i*i + q*q);
					//y = 0.9f * y + 0.1f * sigbuf[x * 256*8 / xres];
					//float y = sigbuf[x * 256*8 / xres] - amplitude;
					target[x] = 0x10000 * clamp(255.9f* gammafix(y +  0.946882f*i +  0.623557f*q))
					+ 0x00100 * clamp(255.9f* gammafix(y + -0.274788f*i + -0.635691f*q))
					+ 0x00001 * clamp(255.9f* gammafix(y + -1.108545f*i +  1.709007f*q));
				}
				diffs[py] = false;
			}
			for(unsigned y=y1; y<y2; ++y)
			{
				u32* pix = ((u32*) s->pixels) + y*xres;
				memcpy(pix, target, sizeof(target));
			}
		}
		if(++framecounter%1 == 0) SDL_Flip(s);
	}
	Colorburst = (Colorburst + length*8) % 12;
}
