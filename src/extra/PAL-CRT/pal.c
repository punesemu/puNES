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

#if (PAL_SYSTEM == PAL_SYSTEM_PAL)
#include <stdlib.h>
#include <string.h>

#define EXP_P         11
#define EXP_ONE       (1 << EXP_P)
#define EXP_MASK      (EXP_ONE - 1)
#define EXP_PI        6434
#define EXP_MUL(x, y) (((x) * (y)) >> EXP_P)
#define EXP_DIV(x, y) (((x) << EXP_P) / (y))

static int e11[] = {
    EXP_ONE,
    5567,  /* e   */
    15133, /* e^2 */
    41135, /* e^3 */
    111817 /* e^4 */
}; 

/* fixed point e^x */
static int
expx(int n)
{
    int neg, idx, res;
    int nxt, acc, del;
    int i;

    if (n == 0) {
        return EXP_ONE;
    }
    neg = n < 0;
    if (neg) {
        n = -n;
    }
    idx = n >> EXP_P;
    res = EXP_ONE;
    for (i = 0; i < idx / 4; i++) {
        res = EXP_MUL(res, e11[4]);
    }
    idx &= 3;
    if (idx > 0) {
        res = EXP_MUL(res, e11[idx]);
    }
    
    n &= EXP_MASK;
    nxt = EXP_ONE;
    acc = 0;
    del = 1;
    for (i = 1; i < 17; i++) {
        acc += nxt / del;
        nxt = EXP_MUL(nxt, n);
        del *= i;
        if (del > nxt || nxt <= 0 || del <= 0) {
            break;
        }
    }
    res = EXP_MUL(res, acc);

    if (neg) {
        res = EXP_DIV(EXP_ONE, res);
    }
    return res;
}

/*****************************************************************************/
/********************************* FILTERS ***********************************/
/*****************************************************************************/

/* infinite impulse response low pass filter for bandlimiting YUV */
static struct IIRLP {
    int c;
    int h; /* history */
} iirY, iirU, iirV;

/* freq  - total bandwidth
 * limit - max frequency
 */
static void
init_iir(struct IIRLP *f, int freq, int limit)
{
    int rate; /* cycles/pixel rate */
    
    memset(f, 0, sizeof(struct IIRLP));
    rate = (freq << 9) / limit;
    f->c = EXP_ONE - expx(-((EXP_PI << 9) / rate));
}

static void
reset_iir(struct IIRLP *f)
{
    f->h = 0;
}

/* hi-pass for debugging */
#define HIPASS 0

static int
iirf(struct IIRLP *f, int s)
{
    f->h += EXP_MUL(s - f->h, f->c);
#if HIPASS
    return s - f->h;
#else
    return f->h;
#endif
}

extern void
pal_modulate(struct PAL_CRT *v, struct PAL_SETTINGS *s)
{
    int x, y, xo, yo;
    int destw = AV_LEN;
    int desth = ((PAL_LINES * 64500) >> 16);
    static int iccf[PAL_VRES][4];
    int ccsin[4][4]; /* color phase for mod */
    int ccburst[4][4]; /* color phase for burst */
    int bsign[4];
    int sn, cs, n;
    int framephase;

    int bpp;
    if (!s->iirs_initialized) {
        init_iir(&iirY, L_FREQ, Y_FREQ);
        init_iir(&iirU, L_FREQ, U_FREQ);
        init_iir(&iirV, L_FREQ, V_FREQ);
        s->iirs_initialized = 1;
    }

#if PAL_DO_BLOOM
    if (s->raw) {
        destw = s->w;
        desth = s->h;
        if (destw > ((AV_LEN * 55500) >> 16)) {
            destw = ((AV_LEN * 55500) >> 16);
        }
        if (desth > ((PAL_LINES * 63500) >> 16)) {
            desth = ((PAL_LINES * 63500) >> 16);
        }
    } else {
        destw = (AV_LEN * 55500) >> 16;
        desth = (PAL_LINES * 63500) >> 16;
    }
#else
    if (s->raw) {
        destw = s->w;
        desth = s->h;
        if (destw > AV_LEN) {
            destw = AV_LEN;
        }
        if (desth > ((PAL_LINES * 64500) >> 16)) {
            desth = ((PAL_LINES * 64500) >> 16);
        }
    }
#endif
    /* phase changes by 270 degrees each frame */
    framephase = (s->field & 7) * 270;
    s->field &= 1;
    if (s->as_color) {
        for (y = 0; y < 4; y++) {
            int vert = (y) * 90;
            bsign[y] = ((y & 1) ? -1 : 1);
            for (x = 0; x < 4; x++) {
                int calign;
                n = -vert + s->hue + x * 90 + 45 + framephase;
                n += s->field * 180;
                /* swinging burst */
                ccburst[y][x] = (n + bsign[y] * 45) * 8192 / 180;
                calign = 25 - (s->color_phase_error * 20);
                ccsin[y][x] = (n - (180 + calign)) * 8192 / 180;
            }
        }
    } else {
        memset(ccburst, 0, sizeof(ccburst));
        memset(ccsin, 0, sizeof(ccsin));
    }

    bpp = pal_bpp4fmt(s->format);
    if (bpp == 0) {
        return; /* just to be safe */
    }
    xo = AV_BEG  + s->xoffset + (AV_LEN    - destw) / 2;
    yo = PAL_TOP + s->yoffset + (PAL_LINES - desth) / 2;
    
    /* align signal */
    xo = (xo & ~3);

    for (n = 0; n < PAL_VRES; n++) {
        int t; /* time */
        signed char *line = &v->analog[n * PAL_HRES];
        
        t = LINE_BEG;

        if (n <= 3) {
            /* equalizing pulses - small blips of sync, mostly blank */
            while (t < (46   * PAL_HRES / 100)) line[t++] = BLANK_LEVEL;
            while (t < (50  * PAL_HRES / 100)) line[t++] = SYNC_LEVEL;
            while (t < (96  * PAL_HRES / 100)) line[t++] = BLANK_LEVEL;
            while (t < (100 * PAL_HRES / 100)) line[t++] = SYNC_LEVEL;
        } else if (n >= 4 && n <= 6) {
            int even[4] = { 46, 50, 96, 100 };
            int odd[4] =  { 4, 50, 96, 100 };
            int *offs = even;
            if (s->field == 1) {
                offs = odd;
            }
            /* vertical sync pulse - small blips of blank, mostly sync */
            while (t < (offs[0] * PAL_HRES / 100)) line[t++] = SYNC_LEVEL;
            while (t < (offs[1] * PAL_HRES / 100)) line[t++] = BLANK_LEVEL;
            while (t < (offs[2] * PAL_HRES / 100)) line[t++] = SYNC_LEVEL;
            while (t < (offs[3] * PAL_HRES / 100)) line[t++] = BLANK_LEVEL;
        } else {
            /* video line */
            while (t < SYNC_BEG) line[t++] = BLANK_LEVEL; /* FP */
            while (t < BW_BEG)   line[t++] = SYNC_LEVEL;  /* SYNC */
            /* PAL switch */
            if (bsign[n & 3] == 1) {
                while (t < CB_BEG) line[t++] = BLANK_LEVEL;
            } else {
                while (t < CB_BEG) line[t++] = SYNC_LEVEL;
            }
           
            while (t < AV_BEG)   line[t++] = BLANK_LEVEL; /* CB + BP */
            if (n < PAL_TOP) {
                while (t < PAL_HRES) line[t++] = BLANK_LEVEL;
            }
            for (t = CB_BEG; t < CB_BEG + (CB_CYCLES * PAL_CB_FREQ); t++) {
                pal_sincos14(&sn, &cs, ccburst[n & 3][t & 3] - OFFSET_25Hz(n));
                line[t] = (BLANK_LEVEL + ((sn >> 10) * BURST_LEVEL)) >> 5;
                iccf[(n + 3) % PAL_VRES][t & 3] = line[t];
            }
        }
    }
    
    for (y = 0; y < desth; y++) {
        int field_offset;
        int sy;
        int o25Hz;
        
        field_offset = (s->field * s->h + desth) / desth / 2;
        sy = (y * s->h) / desth;
    
        sy += field_offset;

        if (sy >= s->h) sy = s->h;
        
        sy *= s->w;
        
        reset_iir(&iirY);
        reset_iir(&iirU);
        reset_iir(&iirV);
        
        o25Hz = OFFSET_25Hz(y + yo);
        
        for (x = 0; x < destw; x++) {
            int fy, fu = 0, fv = 0;
            int rA, gA, bA;
            const unsigned char *pix;
            int ire; /* composite signal */
            int voff;
            
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
            /* bandlimit Y,U,V */
            voff = ((y + yo) & 3);
            fy = iirf(&iirY, fy);
            if (s->as_color) {
                pal_sincos14(&sn, &cs, ccsin[voff][(x + xo) & 3] - o25Hz);
                fu = 32244 * ((bA << 2) - fy) >> 16;
                fv = 57475 * ((rA << 2) - fy) >> 16;
                fu = iirf(&iirU, fu) * sn >> 14;
                fv = bsign[voff] * iirf(&iirV, fv) * cs >> 14;
            }
            ire += (fy + fu + fv) * (WHITE_LEVEL * v->white_point / 100) >> 10;
            if (ire < 0)   ire = 0;
            if (ire > 110) ire = 110;

            v->analog[(x + xo) + (y + yo) * PAL_HRES] = ire;
        }
    }
    /* 25 Hz offset makes it so the phase does not repeat in a field */
    for (n = 0; n < PAL_VRES; n++) {
        for (x = 0; x < 4; x++) {
            v->ccf[n][x] = iccf[n][x] << 7;
        }
    }
    v->cc_period = PAL_VRES;
}
#endif
