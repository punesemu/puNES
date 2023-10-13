/*****************************************************************************/
/*
 * NTSC/CRT - integer-only NTSC video signal encoding / decoding emulation
 *
 *   by EMMIR 2018-2023
 *   modifications for Mesen by Persune
 *   https://github.com/LMP88959/NTSC-CRT
 *
 *   YouTube: https://www.youtube.com/@EMMIR_KC/videos
 *   Discord: https://discord.com/invite/hdYctSmyQJ
 */
/*****************************************************************************/

#include "crt_core.h"

#if (CRT_SYSTEM == CRT_SYSTEM_NESRGB)
#include <stdlib.h>
#include <string.h>

/* this function is an optimization
 * basically factoring out the field setup since as long as CRT->analog
 * does not get cleared, all of this should remain the same every update
 */
static void
setup_field(struct CRT_NESRGB *v)
{
    int n;
 
    for (n = 0; n < CRT_VRES; n++) {
        int t; /* time */
        signed char *line = &v->analog[n * CRT_HRES];
 
        t = LINE_BEG;
 
        /* vertical sync scanlines */
        if (n >= 259 && n <= CRT_VRES) {
           while (t < SYNC_BEG) line[t++] = BLANK_LEVEL; /* FP */
           while (t < PPUpx2pos(327)) line[t++] = SYNC_LEVEL; /* sync separator */
           while (t < CRT_HRES) line[t++] = BLANK_LEVEL; /* blank */
        } else {
            /* prerender/postrender/video scanlines */
            while (t < SYNC_BEG) line[t++] = BLANK_LEVEL; /* FP */
            while (t < BW_BEG) line[t++] = SYNC_LEVEL;  /* SYNC */
            while (t < CRT_HRES) line[t++] = BLANK_LEVEL;
        }
    }
}
 
extern void
crt_nesrgb_modulate(struct CRT_NESRGB *v, struct NTSC_SETTINGS *s)
{
    int x, y, xo, yo;
    int destw = AV_LEN;
    int desth = CRT_LINES;
    int n;
    int iccf[CRT_CC_VPER][CRT_CC_SAMPLES];
    int ccmodI[CRT_CC_VPER][CRT_CC_SAMPLES]; /* color phase for mod */
    int ccmodQ[CRT_CC_VPER][CRT_CC_SAMPLES]; /* color phase for mod */
    int ccburst[CRT_CC_VPER][CRT_CC_SAMPLES]; /* color phase for burst */
    int sn, cs;
    int bpp;
        
    if (!s->field_initialized) {
        setup_field(v);
        s->field_initialized = 1;
    }

    for (y = 0; y < CRT_CC_VPER; y++) {
        xo = (y + s->dot_crawl_offset) * (360 / CRT_CC_VPER);
        for (x = 0; x < CRT_CC_SAMPLES; x++) {
            n = xo + x * (360 / CRT_CC_SAMPLES);
            crt_nesrgb_sincos14(&sn, &cs, (s->hue + 90 + n + 33) * 8192 / 180);
            ccburst[y][x] = sn >> 10;
            crt_nesrgb_sincos14(&sn, &cs, n * 8192 / 180);
            ccmodI[y][x] = sn >> 10;
            crt_nesrgb_sincos14(&sn, &cs, (n - 90) * 8192 / 180);
            ccmodQ[y][x] = sn >> 10;
        }
    }

    bpp = crt_nesrgb_bpp4fmt(s->format);
    if (bpp == 0) {
        return; /* just to be safe */
    }
    
    xo = AV_BEG  + s->xoffset;
    yo = CRT_TOP + s->yoffset;
         
    /* align signal */
    xo = (xo & ~3);
    
    for (y = 0; y < desth; y++) {
        signed char *line;  
        int t, cb;
        int sy = (y * s->h) / desth;
        
        if (sy >= s->h) sy = s->h;
        if (sy < 0) sy = 0;
 
        n = (y + yo);
        line = &v->analog[n * CRT_HRES];
        n %= CRT_CC_VPER;
        
        /* CB_CYCLES of color burst at 3.579545 Mhz */
        for (t = CB_BEG; t < CB_BEG + (CB_CYCLES * CRT_CB_FREQ); t++) {
            cb = ccburst[n][t % CRT_CC_SAMPLES];
            line[t] = (BLANK_LEVEL + (cb * BURST_LEVEL)) >> 5;
            iccf[n][t % CRT_CC_SAMPLES] = line[t];
        }
        sy *= s->w;
        for (x = 0; x < destw; x++) {
            int fy, fi, fq;
            int rA, gA, bA;
            const unsigned char *pix;
            int ire; /* composite signal */
            int xoff;
            
            pix = (unsigned char *)&s->palette[(*(s->data + ((((x * s->w) / destw) + sy) * 2)))];
            switch (s->format) {
                case CRT_PIX_FORMAT_RGB:
                case CRT_PIX_FORMAT_RGBA:
                    rA = pix[0];
                    gA = pix[1];
                    bA = pix[2];
                    break;
                case CRT_PIX_FORMAT_BGR: 
                case CRT_PIX_FORMAT_BGRA:
                    rA = pix[2];
                    gA = pix[1];
                    bA = pix[0];
                    break;
                case CRT_PIX_FORMAT_ARGB:
                    rA = pix[1];
                    gA = pix[2];
                    bA = pix[3];
                    break;
                case CRT_PIX_FORMAT_ABGR:
                    rA = pix[3];
                    gA = pix[2];
                    bA = pix[1];
                    break;
                default:
                    rA = gA = bA = 0;
                    break;
            }
            
            /* RGB to YIQ */
            fy = (19595 * rA + 38470 * gA +  7471 * bA) >> 14;
            fi = (39059 * rA - 18022 * gA - 21103 * bA) >> 14;
            fq = (13894 * rA - 34275 * gA + 20382 * bA) >> 14;
            ire = BLACK_LEVEL + v->black_point;
            
            xoff = (x + xo) % CRT_CC_SAMPLES;
            
            fi = fi * ccmodI[n][xoff] >> 4;
            fq = fq * ccmodQ[n][xoff] >> 4;
            ire += (fy + fi + fq) * (WHITE_LEVEL * v->white_point / 100) >> 10;
            if (ire < 0)   ire = 0;
            if (ire > 110) ire = 110;
            
            v->analog[(x + xo) + (y + yo) * CRT_HRES] = ire;
        }
    }
    
    for (n = 0; n < CRT_CC_VPER; n++) {
        for (x = 0; x < CRT_CC_SAMPLES; x++) {
            v->ccf[n][x] = iccf[n][x] << 7;
        }
    }
}

#endif
