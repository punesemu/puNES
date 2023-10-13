/*****************************************************************************/
/*
 * PAL/CRT - integer-only PAL video signal encoding / decoding emulation
 *
 *   by EMMIR 2018-2023
 *
 *   GitHub : https://github.com/LMP88959/PAL-CRT
 *   YouTube: https://www.youtube.com/@EMMIR_KC/videos
 *   Discord: https://discord.com/invite/hdYctSmyQJ
 */
/*****************************************************************************/

#include "pal_core.h"

#if (PAL_SYSTEM == PAL_SYSTEM_NESRGB)
#include <stdlib.h>
#include <string.h>

/* this function is an optimization
 * basically factoring out the field setup since as long as PAL_CRT->analog
 * does not get cleared, all of this should remain the same every update
 */
static void
setup_field(struct PAL_CRT_NESRGB *v, struct PAL_SETTINGS *s)
{
    int n, y;
    
    for (y = 0; y < 6; y++) {
        s->altline[y] = ((y & 1) ? -1 : 1);
    }
 
    for (n = 0; n < PAL_VRES; n++) {
        int t; /* time */
        signed char *line = &v->analog[n * PAL_HRES];
 
        t = LINE_BEG;

        if (n <= 3 || (n >= 7 && n <= 9)) {
            /* equalizing pulses - small blips of sync, mostly blank */
            while (t < (4   * PAL_HRES / 100)) line[t++] = SYNC_LEVEL;
            while (t < (50  * PAL_HRES / 100)) line[t++] = BLANK_LEVEL;
            while (t < (54  * PAL_HRES / 100)) line[t++] = SYNC_LEVEL;
            while (t < (100 * PAL_HRES / 100)) line[t++] = BLANK_LEVEL;
        } else if (n >= 4 && n <= 6) {
            int offs[4] = { 46, 50, 96, 100 };
            /* vertical sync pulse - small blips of blank, mostly sync */
            while (t < (offs[0] * PAL_HRES / 100)) line[t++] = SYNC_LEVEL;
            while (t < (offs[1] * PAL_HRES / 100)) line[t++] = BLANK_LEVEL;
            while (t < (offs[2] * PAL_HRES / 100)) line[t++] = SYNC_LEVEL;
            while (t < (offs[3] * PAL_HRES / 100)) line[t++] = BLANK_LEVEL;
        } else {
            /* prerender/postrender/video scanlines */
            while (t < SYNC_BEG) line[t++] = BLANK_LEVEL; /* FP */
            while (t < BW_BEG) line[t++] = SYNC_LEVEL; /* SYNC */
            while (t < PAL_HRES) line[t++] = BLANK_LEVEL;
            if (s->altline[n & 3] == -1) {
                for (t = BW_BEG; t < CB_BEG; t++) {
                    line[t] = SYNC_LEVEL;
                }
            } else {
                for (t = BW_BEG; t < CB_BEG; t++) {
                    line[t] = BLANK_LEVEL;
                }
            }
        }
    }
}
 
extern void
pal_nesrgb_modulate(struct PAL_CRT_NESRGB *v, struct PAL_SETTINGS *s)
{
    int x, y, xo, yo;
    int destw = AV_LEN;
    int desth = PAL_LINES;
    int n;
    int iccf[6][4];
    int ccsin[6][4]; /* color phase for mod */
    int ccburst[6][4]; /* color phase for burst */
    int sn, cs;
    int bpp;

    if (!s->field_initialized) {
        setup_field(v, s);
        s->field_initialized = 1;
    }
    for (y = 0; y < 6; y++) {
        int vert = y * (360 / 6);
        for (x = 0; x < 4; x++) {
            int ang;
            n = vert + x * 90 + 120;
            ang = n + (s->altline[y] * 45) + (s->altline[y] * s->hue);
            /* swinging burst */
            pal_nesrgb_sincos14(&sn, &cs, ang * 8192 / 180);
            ccburst[y][x] = sn;
            ccsin[y][x] = (n - (180 - 15)) * 8192 / 180;
        }
    }

    bpp = pal_nesrgb_bpp4fmt(s->format);
    if (bpp == 0) {
        return; /* just to be safe */
    }
    
    xo = AV_BEG  + s->xoffset;
    yo = PAL_TOP + s->yoffset;
       
    /* align signal */
    xo = (xo & ~3);
    /* no border on PAL according to https://www.nesdev.org/wiki/PAL_video */
    for (y = 0; y < desth; y++) {
        signed char *line;  
        int t, cb, nm6;
        int sy = (y * s->h) / desth;

        if (sy >= s->h) sy = s->h;
        if (sy < 0) sy = 0;
 
        n = (y + yo);
        nm6 = n % 6;
        line = &v->analog[n * PAL_HRES];

        for (t = CB_BEG; t < CB_BEG + (CB_CYCLES * PAL_CB_FREQ); t++) {
            cb = ccburst[nm6][t & 3];
            line[t] = (BLANK_LEVEL + (cb * BURST_LEVEL)) >> 15;
            iccf[nm6][t & 3] = line[t];
        }
        sy *= s->w;

        for (x = 0; x < destw; x++) {
            int fy, fu = 0, fv = 0;
            int rA, gA, bA;
            const unsigned char *pix;
            int ire; /* composite signal */
            
            pix = (unsigned char *)&s->palette[(*(s->data + ((((x * s->w) / destw) + sy) * 2)))];
            switch (s->format) {
                case PAL_PIX_FORMAT_RGB:
                case PAL_PIX_FORMAT_RGBA:
                    rA = pix[0];
                    gA = pix[1];
                    bA = pix[2];
                    break;
                case PAL_PIX_FORMAT_BGR: 
                case PAL_PIX_FORMAT_BGRA:
                    rA = pix[2];
                    gA = pix[1];
                    bA = pix[0];
                    break;
                case PAL_PIX_FORMAT_ARGB:
                    rA = pix[1];
                    gA = pix[2];
                    bA = pix[3];
                    break;
                case PAL_PIX_FORMAT_ABGR:
                    rA = pix[3];
                    gA = pix[2];
                    bA = pix[1];
                    break;
                default:
                    rA = gA = bA = 0;
                    break;
            }
            /* RGB to YUV */
            fy = (19595 * rA + 38470 * gA +  7471 * bA) >> 14;
            ire = BLACK_LEVEL + v->black_point;

            pal_nesrgb_sincos14(&sn, &cs, ccsin[nm6][(x + xo) & 3]);
            fu = 32244 * ((bA << 2) - fy) >> 16;
            fv = 57475 * ((rA << 2) - fy) >> 16;
            fu = fu * sn >> 14;
            fv = s->altline[nm6] * fv * cs >> 14;
        
            ire += (fy + fu + fv) * (WHITE_LEVEL * v->white_point / 110) >> 10;
            if (ire < 0)   ire = 0;
            if (ire > 110) ire = 110;

            v->analog[(x + xo) + (y + yo) * PAL_HRES] = ire;
        }
    }
   
    for (x = 0; x < 4; x++) {
        for (n = 0; n < 6; n++) {
            v->ccf[n][x] = iccf[(n + 3) % 6][x] << 7;
        }
    }
    v->cc_period = 6;
}

#endif
