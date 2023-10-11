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

#if (CRT_SYSTEM == CRT_SYSTEM_NES)
#include <stdlib.h>
#include <string.h>

/* generate the square wave for a given 9-bit pixel and phase */
static int
square_sample(int p, int phase)
{
    /* amplified IRE = ((mV / 7.143) - 312 / 7.143) * 1024 */
    /* https://www.nesdev.org/wiki/NTSC_video#Brightness_Levels */
    static int IRE[16] = {
     /* 0d     1d     2d      3d */
       -12042, 0,     34406,  81427,
     /* 0d     1d     2d      3d emphasized */
       -17203,-8028,  19497,  57342,
     /* 00     10     20      30 */
        43581, 75693, 112965, 112965,
     /* 00     10     20      30 emphasized */
        26951, 52181, 83721,  83721
    };
    static int active[6] = {
        0300, 0100,
        0500, 0400,
        0600, 0200
    };
    int hue;
    int e, l, v;

    hue = (p & 0x0f);

    /* last two columns are black */
    if (hue >= 0x0e) {
        return 0;
    }

    v = (((hue + phase) % 12) < 6);

    /* red 0100, green 0200, blue 0400 */
    e = (((p & 0700) & active[(phase >> 1) % 6]) > 0);
    switch (hue) {
        case 0x00: l = 1; break;
        case 0x0d: l = 0; break;
        default:   l = v; break;
    }
    return IRE[(l << 3) + (e << 2) + ((p >> 4) & 3)];
}

#define NES_OPTIMIZED 1
/* toggle drawing of NES border
 * (normally not in visible region, but it depends on your emulator)
 * highly recommended to keep disabled for better performance if the border
 * is not visible in your emulator.
 */
#define NES_BORDER    0

/* the optimized version is NOT the most optimized version, it just performs
 * some simple refactoring to prevent a few redundant computations
 */
#if NES_OPTIMIZED


/* this function is an optimization
 * basically factoring out the field setup since as long as CRT->analog
 * does not get cleared, all of this should remain the same every update
 */
static void
setup_field(struct CRT *v)
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
crt_modulate(struct CRT *v, struct NTSC_SETTINGS *s)
{
    int x, y, xo, yo;
    int destw = AV_LEN;
    int desth = CRT_LINES;
    int n, phase;
    int iccf[CRT_CC_VPER][CRT_CC_SAMPLES];
    int ccburst[CRT_CC_VPER][CRT_CC_SAMPLES]; /* color phase for burst */
    int sn, cs;
    static int phasetab[CRT_CC_VPER] = { 0, 4, 8 };
        
    if (!s->field_initialized) {
        setup_field(v);
        s->field_initialized = 1;
    }

    for (y = 0; y < CRT_CC_VPER; y++) {
        xo = (y + s->dot_crawl_offset) * (360 / CRT_CC_VPER);
        for (x = 0; x < CRT_CC_SAMPLES; x++) {
            n = (s->hue + x * (360 / CRT_CC_SAMPLES) + xo + 33) % 360;
            crt_sincos14(&sn, &cs, n * 8192 / 180);
            ccburst[y][x] = sn >> 10;
        }
    }

    xo = AV_BEG  + s->xoffset;
    yo = CRT_TOP + s->yoffset;
         
    /* align signal */
    xo = (xo & ~3);
    
#if NES_BORDER
    for (n = CRT_TOP; n <= (CRT_BOT + 2); n++) {
        int t; /* time */
        signed char *line = &v->analog[n * CRT_HRES];
        
        t = LINE_BEG;
 
        phase = phasetab[(n + s->dot_crawl_offset) % CRT_CC_VPER] + 6;
        t = LAV_BEG;
        while (t < CRT_HRES) {
            int ire, p;
            p = s->border_color;
            if (t == LAV_BEG) p = 0xf0;
            ire = BLACK_LEVEL + v->black_point;
            ire += square_sample(p, phase + 0);
            ire += square_sample(p, phase + 1);
            ire += square_sample(p, phase + 2);
            ire += square_sample(p, phase + 3);
            ire = (ire * v->white_point / 100) >> 12;
            line[t++] = ire;
            phase += 3;
        }
    }
#endif
    for (y = 0; y < desth; y++) {
        signed char *line;  
        int t, cb;
        int sy = (y * s->h) / desth;
        
        if (sy >= s->h) sy = s->h;
        if (sy < 0) sy = 0;
 
        n = (y + yo);
        line = &v->analog[n * CRT_HRES];
        
        /* CB_CYCLES of color burst at 3.579545 Mhz */
        for (t = CB_BEG; t < CB_BEG + (CB_CYCLES * CRT_CB_FREQ); t++) {
            cb = ccburst[n % CRT_CC_VPER][t % CRT_CC_SAMPLES];
            line[t] = (BLANK_LEVEL + (cb * BURST_LEVEL)) >> 5;
            iccf[n % CRT_CC_VPER][t % CRT_CC_SAMPLES] = line[t];
        }
        sy *= s->w;
        phase = phasetab[(y + yo + s->dot_crawl_offset) % CRT_CC_VPER];
        for (x = 0; x < destw; x++) {
            int ire, p;
            
            p = s->data[((x * s->w) / destw) + sy];
            ire = BLACK_LEVEL + v->black_point;
            ire += square_sample(p, phase + 0);
            ire += square_sample(p, phase + 1);
            ire += square_sample(p, phase + 2);
            ire += square_sample(p, phase + 3);
            ire = (ire * v->white_point / 100) >> 12;
            v->analog[(x + xo) + (y + yo) * CRT_HRES] = ire;
            phase += 3;
        }
    }
    
    for (n = 0; n < CRT_CC_VPER; n++) {
        for (x = 0; x < CRT_CC_SAMPLES; x++) {
            v->ccf[n][x] = iccf[n][x] << 7;
        }
    }
}
#else
/* NOT NES_OPTIMIZED */
extern void
crt_modulate(struct CRT *v, struct NTSC_SETTINGS *s)
{
    int x, y, xo, yo;
    int destw = AV_LEN;
    int desth = CRT_LINES;
    int n, phase;
    int iccf[CRT_CC_VPER][CRT_CC_SAMPLES];
    int ccburst[CRT_CC_VPER][CRT_CC_SAMPLES]; /* color phase for burst */
    int sn, cs;
    static int phasetab[CRT_CC_VPER] = { 0, 4, 8 };

    for (y = 0; y < CRT_CC_VPER; y++) {
        xo = (y + s->dot_crawl_offset) * (360 / CRT_CC_VPER);
        for (x = 0; x < CRT_CC_SAMPLES; x++) {
            n = (s->hue + x * (360 / CRT_CC_SAMPLES) + xo + 33) % 360;
            crt_sincos14(&sn, &cs, n * 8192 / 180);
            ccburst[y][x] = sn >> 10;
        }
    }

    xo = AV_BEG  + s->xoffset;
    yo = CRT_TOP + s->yoffset;
     
    /* align signal */
    xo = (xo & ~3);
    
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
            int cb;
            /* prerender/postrender/video scanlines */
            while (t < SYNC_BEG) line[t++] = BLANK_LEVEL; /* FP */
            while (t < BW_BEG) line[t++] = SYNC_LEVEL;  /* SYNC */
            while (t < CB_BEG) line[t++] = BLANK_LEVEL; /* BW + CB + BP */
            /* CB_CYCLES of color burst at 3.579545 Mhz */
            for (t = CB_BEG; t < CB_BEG + (CB_CYCLES * CRT_CB_FREQ); t++) {
                cb = ccburst[n % CRT_CC_VPER][t % CRT_CC_SAMPLES];
                line[t] = (BLANK_LEVEL + (cb * BURST_LEVEL)) >> 5;
                iccf[n % CRT_CC_VPER][t % CRT_CC_SAMPLES] = line[t];
            }
            while (t < LAV_BEG) line[t++] = BLANK_LEVEL;
#if NES_BORDER
            if (n >= CRT_TOP && n <= (CRT_BOT + 2)) {
                phase = phasetab[(n + s->dot_crawl_offset) % CRT_CC_VPER] + 6;
                while (t < CRT_HRES) {
                    int ire, p;
                    p = s->border_color;
                    if (t == LAV_BEG) p = 0xf0;
                    ire = BLACK_LEVEL + v->black_point;
                    ire += square_sample(p, phase + 0);
                    ire += square_sample(p, phase + 1);
                    ire += square_sample(p, phase + 2);
                    ire += square_sample(p, phase + 3);
                    ire = (ire * v->white_point / 100) >> 12;
                    line[t++] = ire;
                    phase += 3;
                }
            } else {
#endif
                while (t < CRT_HRES) line[t++] = BLANK_LEVEL;
#if NES_BORDER
            }
#endif
        }
    }

    for (y = 0; y < desth; y++) {
        int sy = (y * s->h) / desth;
        if (sy >= s->h) sy = s->h;
        if (sy < 0) sy = 0;
        
        sy *= s->w;
        phase = phasetab[(y + yo + s->dot_crawl_offset) % CRT_CC_VPER];
        for (x = 0; x < destw; x++) {
            int ire, p;
            
            p = s->data[((x * s->w) / destw) + sy];
            ire = BLACK_LEVEL + v->black_point;
            ire += square_sample(p, phase + 0);
            ire += square_sample(p, phase + 1);
            ire += square_sample(p, phase + 2);
            ire += square_sample(p, phase + 3);
            ire = (ire * v->white_point / 100) >> 12;
            v->analog[(x + xo) + (y + yo) * CRT_HRES] = ire;
            phase += 3;
        }
    }
    
    for (n = 0; n < CRT_CC_VPER; n++) {
        for (x = 0; x < CRT_CC_SAMPLES; x++) {
            v->ccf[n][x] = iccf[n][x] << 7;
        }
    }
}
#endif

#endif
