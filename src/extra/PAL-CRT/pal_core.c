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

#include <stdlib.h>
#include <string.h>

/* ensure negative values for x get properly modulo'd */
#define POSMOD(x, n)     (((x) % (n) + (n)) % (n))

static int sigpsin15[18] = { /* significant points on sine wave (15-bit) */
    0x0000,
    0x0c88,0x18f8,0x2528,0x30f8,0x3c50,0x4718,0x5130,0x5a80,
    0x62f0,0x6a68,0x70e0,0x7640,0x7a78,0x7d88,0x7f60,0x8000,
    0x7f60
};

static int
sintabil8(int n)
{
    int f, i, a, b;

    /* looks scary but if you don't change T14_2PI
     * it won't cause out of bounds memory reads
     */
    f = n >> 0 & 0xff;
    i = n >> 8 & 0xff;
    a = sigpsin15[i];
    b = sigpsin15[i + 1];
    return (a + ((b - a) * f >> 8));
}

/* 14-bit interpolated sine/cosine */
extern void
pal_sincos14(int *s, int *c, int n)
{
    int h;

    n &= T14_MASK;
    h = n & ((T14_2PI >> 1) - 1);

    if (h > ((T14_2PI >> 2) - 1)) {
        *c = -sintabil8(h - (T14_2PI >> 2));
        *s = sintabil8((T14_2PI >> 1) - h);
    } else {
        *c = sintabil8((T14_2PI >> 2) - h);
        *s = sintabil8(h);
    }
    if (n > ((T14_2PI >> 1) - 1)) {
        *c = -*c;
        *s = -*s;
    }
}

extern int
pal_bpp4fmt(int format)
{
    switch (format) {
        case PAL_PIX_FORMAT_RGB:
        case PAL_PIX_FORMAT_BGR:
            return 3;
        case PAL_PIX_FORMAT_ARGB:
        case PAL_PIX_FORMAT_RGBA:
        case PAL_PIX_FORMAT_ABGR:
        case PAL_PIX_FORMAT_BGRA:
            return 4;
        default:
            return 0;
    }
}

/*****************************************************************************/
/********************************* FILTERS ***********************************/
/*****************************************************************************/

/* convolution is much faster but the EQ looks softer, more authentic, and more analog */
#define USE_CONVOLUTION 0
#define USE_7_SAMPLE 0
#define USE_6_SAMPLE 0
#define USE_5_SAMPLE 1

#if USE_CONVOLUTION

/* NOT 3 band equalizer, faster convolution instead.
 * eq function names preserved to keep code clean
 */
static struct EQF {
    int h[7];
} eqY, eqU, eqV;

/* params unused to keep the function the same */
static void
init_eq(struct EQF *f,
        int f_lo, int f_hi, int rate,
        int g_lo, int g_mid, int g_hi)
{
    memset(f, 0, sizeof(struct EQF));
}

static void
reset_eq(struct EQF *f)
{
    memset(f->h, 0, sizeof(f->h));
}

static int
eqf(struct EQF *f, int s)
{
    int i;
    int *h = f->h;

    for (i = 6; i > 0; i--) {
        h[i] = h[i - 1];
    }
    h[0] = s;
#if USE_7_SAMPLE
    /* index : 0 1 2 3 4 5 6 */
    /* weight: 1 4 7 8 7 4 1 */
    return (s + h[6] + ((h[1] + h[5]) * 4) + ((h[2] + h[4]) * 7) + (h[3] * 8)) >> 5;
#elif USE_6_SAMPLE
    /* index : 0 1 2 3 4 5 */
    /* weight: 1 3 4 4 3 1 */
    return (s + h[5] + 3 * (h[1] + h[4]) + 4 * (h[2] + h[3])) >> 4;
#elif USE_5_SAMPLE
    /* index : 0 1 2 3 4 */
    /* weight: 1 2 2 2 1 */
    return (s + h[4] + ((h[1] + h[2] + h[3]) << 1)) >> 3;
#else
    /* index : 0 1 2 3 */
    /* weight: 1 1 1 1*/
    return (s + h[3] + h[1] + h[2]) >> 2;
#endif
}

#else

#define HISTLEN     3
#define HISTOLD     (HISTLEN - 1) /* oldest entry */
#define HISTNEW     0             /* newest entry */

#define EQ_P        16 /* if changed, the gains will need to be adjusted */
#define EQ_R        (1 << (EQ_P - 1)) /* rounding */
/* three band equalizer */
static struct EQF {
    int lf, hf; /* fractions */
    int g[3]; /* gains */
    int fL[4];
    int fH[4];
    int h[HISTLEN]; /* history */
} eqY, eqU, eqV;

/* f_lo - low cutoff frequency
 * f_hi - high cutoff frequency
 * rate - sampling rate
 * g_lo, g_mid, g_hi - gains
 */
static void
init_eq(struct EQF *f,
        int f_lo, int f_hi, int rate,
        int g_lo, int g_mid, int g_hi)
{
    int sn, cs;

    memset(f, 0, sizeof(struct EQF));

    f->g[0] = g_lo;
    f->g[1] = g_mid;
    f->g[2] = g_hi;

    pal_sincos14(&sn, &cs, T14_PI * f_lo / rate);
#if (EQ_P >= 15)
    f->lf = 2 * (sn << (EQ_P - 15));
#else
    f->lf = 2 * (sn >> (15 - EQ_P));
#endif
    pal_sincos14(&sn, &cs, T14_PI * f_hi / rate);
#if (EQ_P >= 15)
    f->hf = 2 * (sn << (EQ_P - 15));
#else
    f->hf = 2 * (sn >> (15 - EQ_P));
#endif
}

static void
reset_eq(struct EQF *f)
{
    memset(f->fL, 0, sizeof(f->fL));
    memset(f->fH, 0, sizeof(f->fH));
    memset(f->h, 0, sizeof(f->h));
}

static int
eqf(struct EQF *f, int s)
{
    int i, r[3];

    f->fL[0] += (f->lf * (s - f->fL[0]) + EQ_R) >> EQ_P;
    f->fH[0] += (f->hf * (s - f->fH[0]) + EQ_R) >> EQ_P;

    for (i = 1; i < 4; i++) {
        f->fL[i] += (f->lf * (f->fL[i - 1] - f->fL[i]) + EQ_R) >> EQ_P;
        f->fH[i] += (f->hf * (f->fH[i - 1] - f->fH[i]) + EQ_R) >> EQ_P;
    }

    r[0] = f->fL[3];
    r[1] = f->fH[3] - f->fL[3];
    r[2] = f->h[HISTOLD] - f->fH[3];

    for (i = 0; i < 3; i++) {
        r[i] = (r[i] * f->g[i]) >> EQ_P;
    }

    for (i = HISTOLD; i > 0; i--) {
        f->h[i] = f->h[i - 1];
    }
    f->h[HISTNEW] = s;

    return (r[0] + r[1] + r[2]);
}

#endif

/*****************************************************************************/
/***************************** PUBLIC FUNCTIONS ******************************/
/*****************************************************************************/

extern void
pal_resize(struct PAL_CRT *v, int w, int h, int f, unsigned char *out)
{
    v->outw = w;
    v->outh = h;
    v->out_format = f;
    v->out = out;
}

extern void
pal_reset(struct PAL_CRT *v)
{
    v->saturation = 10;
    v->brightness = 0;
    v->contrast = 180;
    v->black_point = 0;
    v->white_point = 100;
    v->hsync = 0;
    v->vsync = 0;
}

extern void
pal_init(struct PAL_CRT *v, int w, int h, int f, unsigned char *out)
{
    memset(v, 0, sizeof(struct PAL_CRT));
    pal_resize(v, w, h, f, out);
    pal_reset(v);
    v->rn = 194;

    /* kilohertz to line sample conversion */
#define kHz2L(kHz) (PAL_HRES * (kHz * 100) / L_FREQ)

    /* band gains are pre-scaled as 16-bit fixed point
     * if you change the EQ_P define, you'll need to update these gains too
     */
    init_eq(&eqY, kHz2L(1890), kHz2L(3320), PAL_HRES, 65536, 8192, 9175);
    init_eq(&eqU, kHz2L(80),   kHz2L(1320), PAL_HRES, 65536, 65536, 1311);
    init_eq(&eqV, kHz2L(80),   kHz2L(1320), PAL_HRES, 65536, 65536, 1311);
}

/* search windows, in samples */
#define HSYNC_WINDOW 6
#define VSYNC_WINDOW 6

extern void
pal_demodulate(struct PAL_CRT *c, int noise)
{
    struct {
        int y, u, v;
    } outbuf[AV_LEN + 16], *out = outbuf + 8, *yuvA, *yuvB;
    int i, j, line, rn;
    signed char *sig;
    int s = 0;
    int field, ratio;
    int *ccr; /* color carrier signal */
    int huesn, huecs;
    int xnudge = -3, ynudge = 3;
    int bright = c->brightness - (BLACK_LEVEL + c->black_point);
    int bpp, pitch;
#if PAL_DO_BLOOM
    int prev_e; /* filtered beam energy per scan line */
    int max_e; /* approx maximum energy in a scan line */
#endif

    bpp = pal_bpp4fmt(c->out_format);
    if (bpp == 0) {
        return;
    }
    pitch = c->outw * bpp;

    rn = c->rn;
#if !PAL_DO_VSYNC
    /* determine field before we add noise,
     * otherwise it's not reliably recoverable
     */
    for (i = -VSYNC_WINDOW; i < VSYNC_WINDOW; i++) {
        line = POSMOD(c->vsync + i, PAL_VRES);
        sig = c->analog + line * PAL_HRES;
        s = 0;
        for (j = 0; j < PAL_HRES; j++) {
            s += sig[j];
            if (s <= (125 * SYNC_LEVEL)) {
                goto found_field;
            }
        }
    }
found_field:
    /* if vsync signal was in second half of line, odd field */
    field = (j > (PAL_HRES / 2));
    c->vsync = -3;
#endif
    for (i = 0; i < PAL_INPUT_SIZE; i++) {
        rn = (214019 * rn + 140327895);

        /* signal + noise */
        s = c->analog[i] + (((((rn >> 16) & 0xff) - 0x7f) * noise) >> 8);
        if (s >  127) { s =  127; }
        if (s < -127) { s = -127; }
        c->inp[i] = s;
    }
    c->rn = rn;
#if PAL_DO_VSYNC
    /* Look for vertical sync.
     *
     * This is done by integrating the signal and
     * seeing if it exceeds a threshold. The threshold of
     * the vertical sync pulse is much higher because the
     * vsync pulse is a lot longer than the hsync pulse.
     * The signal needs to be integrated to lessen
     * the noise in the signal.
     */
    for (i = -VSYNC_WINDOW; i < VSYNC_WINDOW; i++) {
        line = POSMOD(c->vsync + i, PAL_VRES);
        sig = c->inp + line * PAL_HRES;
        s = 0;
        for (j = 0; j < PAL_HRES; j++) {
            s += sig[j];
            /* increase the multiplier to make the vsync
             * more stable when there is a lot of noise
             */
            if (s <= (125 * SYNC_LEVEL)) {
                goto vsync_found;
            }
        }
    }
vsync_found:
    c->vsync = line; /* vsync found (or gave up) at this line */
    /* if vsync signal was in second half of line, odd field */
    field = (j > (PAL_HRES / 2));
#endif

#if PAL_DO_BLOOM
    max_e = (128 + (noise / 2)) * AV_LEN;
    prev_e = (16384 / 8);
#endif
    /* ratio of output height to active video lines in the signal */
    ratio = (c->outh << 16) / PAL_LINES;
    ratio = (ratio + 32768) >> 16;

    field = (field * (ratio / 2));

    for (line = PAL_TOP; line < PAL_BOT; line++) {
        unsigned pos, ln, scanR;
        int scanL, dx;
        int L, R;
        unsigned char *cL, *cR;
        int wave[4];
        int dcu, dcv; /* decoded U, V */
        int xpos, ypos;
        int beg, end;
        int phasealign;
        int odd;
#if PAL_DO_BLOOM
        int line_w;
#endif

        beg = (line - PAL_TOP + 0) * (c->outh + c->v_fac) / PAL_LINES + field;
        end = (line - PAL_TOP + 1) * (c->outh + c->v_fac) / PAL_LINES + field;

        if (beg >= c->outh) { continue; }
        if (end > c->outh) { end = c->outh; }

        /* Look for horizontal sync.
         * See comment above regarding vertical sync.
         */
        ln = (POSMOD(line + c->vsync, PAL_VRES)) * PAL_HRES;
        sig = c->inp + ln + c->hsync;

        s = 0;
        for (i = -HSYNC_WINDOW; i < HSYNC_WINDOW; i++) {
            s += sig[SYNC_BEG + i];
            if (s <= (4 * SYNC_LEVEL)) {
                break;
            }
        }
#if PAL_DO_HSYNC
        c->hsync = POSMOD(i + c->hsync, PAL_HRES);
#else
        c->hsync = 0;
#endif

        xpos = POSMOD(AV_BEG + c->hsync + xnudge, PAL_HRES);
        ypos = POSMOD(line + c->vsync + ynudge, PAL_VRES);
        pos = xpos + ypos * PAL_HRES;

        sig = c->inp + ln + c->hsync;
        odd = 0; /* PAL switch, odd line has SYNC in breezeway, even is blank */
        s = 0;
        for (i = 0; i < 8; i++) {
            s += sig[BW_BEG + i];
            if (s <= (4 * SYNC_LEVEL)) {
                odd = 1;
                break;
            }
        }
        ccr = c->ccf[ypos % c->cc_period];
        sig = c->inp + ln + (c->hsync & ~3);
        for (i = CB_BEG; i < CB_BEG + (CB_CYCLES * PAL_CB_FREQ); i++) {
            int p, n;
            p = ccr[i & 3] * 127 / 128; /* fraction of the previous */
            n = sig[i];                 /* mixed with the new sample */
            ccr[i & 3] = p + n;
        }

        phasealign = POSMOD(c->hsync, 4);

        if (!odd) {
            phasealign -= 1;
        }
        odd = odd ? -1 : 1;

        pal_sincos14(&huesn, &huecs, 90 * 8192 / 180 - OFFSET_25Hz(line));
        huesn >>= 7; /* make 8-bit */
        huecs >>= 7;

        /* amplitude of carrier = saturation, phase difference = hue */
        dcu = ccr[(phasealign + 1) & 3] - ccr[(phasealign + 3) & 3];
        dcv = ccr[(phasealign + 2) & 3] - ccr[(phasealign + 0) & 3];

        wave[0] = ((dcu * huecs - dcv * huesn) >> 8) * c->saturation;
        wave[1] = ((dcv * huecs + dcu * huesn) >> 8) * c->saturation;
        wave[2] = -wave[0];
        wave[3] = -wave[1];

        sig = c->inp + pos;
#if PAL_DO_BLOOM
        s = 0;
        for (i = 0; i < AV_LEN; i++) {
            s += sig[i]; /* sum up the scan line */
        }
        /* bloom emulation */
        prev_e = (prev_e * 123 / 128) + ((((max_e >> 1) - s) << 10) / max_e);
        line_w = (AV_LEN * 112 / 128) + (prev_e >> 9);

        dx = (line_w << 12) / c->outw;
        scanL = ((AV_LEN / 2) - (line_w >> 1) + 8) << 12;
        scanR = (AV_LEN - 1) << 12;

        L = (scanL >> 12);
        R = (scanR >> 12);
#else
        dx = ((AV_LEN - 1) << 12) / c->outw;
        scanL = 0;
        scanR = (AV_LEN - 1) << 12;
        L = 0;
        R = AV_LEN;
#endif
        reset_eq(&eqY);
        reset_eq(&eqU);
        reset_eq(&eqV);

        for (i = L; i < R; i++) {
            int dmU, dmV;
            int ou, ov;

            dmU = sig[i] * wave[(i + 0) & 3];
            dmV = sig[i] * wave[(i + 3) & 3] * odd;
            if (c->chroma_correction) {
                static struct { int u, v; } delay_line[AV_LEN + 1];
                ou = dmU;
                ov = dmV;
                dmU = (delay_line[i].u + dmU) / 2;
                dmV = (delay_line[i].v + dmV) / 2;
                delay_line[i].u = ou;
                delay_line[i].v = ov;
            }
            out[i].y = eqf(&eqY, sig[i] + bright) << 4;
            out[i + c->chroma_lag].u = eqf(&eqU, dmU >> 9) >> 3;
            out[i + c->chroma_lag].v = eqf(&eqV, dmV >> 9) >> 3;
        }

        cL = c->out + (beg * pitch);
        cR = cL + pitch;

        for (pos = scanL; pos < scanR && cL < cR; pos += dx) {
            int y, u, v;
            int r, g, b;
            int aa, bb;

            R = pos & 0xfff;
            L = 0xfff - R;
            s = pos >> 12;

            yuvA = out + s;
            yuvB = out + s + 1;

            /* interpolate between samples if needed */
            y = ((yuvA->y * L) >>  2) + ((yuvB->y * R) >>  2);
            u = ((yuvA->u * L) >> 14) + ((yuvB->u * R) >> 14);
            v = ((yuvA->v * L) >> 14) + ((yuvB->v * R) >> 14);

            /* YUV to RGB */
            r = (((y + 4669 * v) >> 12) * c->contrast) >> 8;
            g = (((y - 1622 * u - 2380 * v) >> 12) * c->contrast) >> 8;
            b = (((y + 8311 * u) >> 12) * c->contrast) >> 8;

            if (r < 0) r = 0;
            if (g < 0) g = 0;
            if (b < 0) b = 0;
            if (r > 255) r = 255;
            if (g > 255) g = 255;
            if (b > 255) b = 255;

            if (c->blend) {
                aa = (r << 16 | g << 8 | b);

                switch (c->out_format) {
                    case PAL_PIX_FORMAT_RGB:
                    case PAL_PIX_FORMAT_RGBA:
                        bb = cL[0] << 16 | cL[1] << 8 | cL[2];
                        break;
                    case PAL_PIX_FORMAT_BGR:
                    case PAL_PIX_FORMAT_BGRA:
                        bb = cL[2] << 16 | cL[1] << 8 | cL[0];
                        break;
                    case PAL_PIX_FORMAT_ARGB:
                        bb = cL[1] << 16 | cL[2] << 8 | cL[3];
                        break;
                    case PAL_PIX_FORMAT_ABGR:
                        bb = cL[3] << 16 | cL[2] << 8 | cL[1];
                        break;
                    default:
                        bb = 0;
                        break;
                }

                /* blend with previous color there */
                bb = (((aa & 0xfefeff) >> 1) + ((bb & 0xfefeff) >> 1));
            } else {
                bb = (r << 16 | g << 8 | b);
            }

            switch (c->out_format) {
                case PAL_PIX_FORMAT_RGB:
                    cL[0] = bb >> 16 & 0xff;
                    cL[1] = bb >>  8 & 0xff;
                    cL[2] = bb >>  0 & 0xff;
                    break;

                case PAL_PIX_FORMAT_RGBA:
                    cL[0] = bb >> 16 & 0xff;
                    cL[1] = bb >>  8 & 0xff;
                    cL[2] = bb >>  0 & 0xff;
                    cL[3] = 0xff;
                    break;

                case PAL_PIX_FORMAT_BGR:
                    cL[0] = bb >>  0 & 0xff;
                    cL[1] = bb >>  8 & 0xff;
                    cL[2] = bb >> 16 & 0xff;
                    break;

                case PAL_PIX_FORMAT_BGRA:
                    cL[0] = bb >>  0 & 0xff;
                    cL[1] = bb >>  8 & 0xff;
                    cL[2] = bb >> 16 & 0xff;
                    cL[3] = 0xff;
                    break;

                case PAL_PIX_FORMAT_ARGB:
                    cL[0] = 0xff;
                    cL[1] = bb >> 16 & 0xff;
                    cL[2] = bb >>  8 & 0xff;
                    cL[3] = bb >>  0 & 0xff;
                    break;

                case PAL_PIX_FORMAT_ABGR:
                    cL[0] = 0xff;
                    cL[1] = bb >>  0 & 0xff;
                    cL[2] = bb >>  8 & 0xff;
                    cL[3] = bb >> 16 & 0xff;
                    break;

                default:
                    break;
            }

            cL += bpp;
        }

        /* duplicate extra lines */
        for (s = beg + 1; s < (end - c->scanlines); s++) {
            memcpy(c->out + s * pitch, c->out + (s - 1) * pitch, pitch);
        }
    }
}
