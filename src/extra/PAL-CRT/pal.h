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

#ifndef _PAL_H_
#define _PAL_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* pal.h
 *
 * An interface to convert a digital image to an analog PAL signal.
 * 
 */

/* 283.75 color cycles per line */
#define PAL_CC_LINE 28375

/* NOTE, in general, increasing PAL_CB_FREQ reduces blur and bleed */
#define PAL_CB_FREQ     4 /* carrier frequency relative to sample rate */
#define PAL_HRES        (PAL_CC_LINE * PAL_CB_FREQ / 100) /* horizontal res */
#define PAL_VRES        312                       /* vertical resolution */
#define PAL_INPUT_SIZE  (PAL_HRES * PAL_VRES)

#define PAL_TOP         42     /* first line with active video */
#define PAL_BOT         282    /* final line with active video */
#define PAL_LINES       (PAL_BOT - PAL_TOP) /* number of active video lines */

/*
 *                      FULL HORIZONTAL LINE SIGNAL (~64000 ns)
 * |---------------------------------------------------------------------------|
 *   HBLANK (~12000 ns)                 ACTIVE VIDEO (~52000 ns)
 * |-------------------||------------------------------------------------------|
 *   
 *   
 *   WITHIN HBLANK PERIOD:
 *   
 *   FP (~1600 ns)  SYNC (~4700 ns)  BW (~800 ns)  CB (~2500 ns)  BP (~2400 ns)
 * |--------------||---------------||------------||-------------||-------------|
 *      BLANK            SYNC           BLANK          BLANK          BLANK
 * 
 */
#define LINE_BEG         0
#define FP_ns            1600      /* front porch */
#define SYNC_ns          4700      /* sync tip */
#define BW_ns            800       /* breezeway with PAL switch */
#define CB_ns            2500      /* color burst */
#define BP_ns            2400      /* back porch */
#define AV_ns            52000     /* active video */
#define HB_ns            (FP_ns + SYNC_ns + BW_ns + CB_ns + BP_ns) /* h blank */
/* line duration should be ~64 ns */
#define LINE_ns          (FP_ns + SYNC_ns + BW_ns + CB_ns + BP_ns + AV_ns)

/* convert nanosecond offset to its corresponding point on the sampled line */
#define ns2pos(ns)       ((ns) * PAL_HRES / LINE_ns)
/* starting points for all the different pulses */
#define FP_BEG           ns2pos(0)
#define SYNC_BEG         ns2pos(FP_ns)
#define BW_BEG           ns2pos(FP_ns + SYNC_ns)
#define CB_BEG           ns2pos(FP_ns + SYNC_ns + BW_ns)
#define BP_BEG           ns2pos(FP_ns + SYNC_ns + BW_ns + CB_ns)
#define AV_BEG           ns2pos(HB_ns)
#define AV_LEN           ns2pos(AV_ns)

#define CB_CYCLES        10

/* 25 Hz offset applied to each line */
#define OFFSET_25Hz(y)  ((((y * 10) * 8192) / 3125))

/* frequencies for bandlimiting */
#define L_FREQ           1773448 /* full line, 4 f_SC PAL sampling */
#define Y_FREQ           520000  /* Luma   (Y) 5.2  MHz */
#define U_FREQ           129000  /* Chroma (U) 1.29 MHz */
#define V_FREQ           129000  /* Chroma (V) 1.29 MHz */

/* IRE units (100 = 1.0V, -40 = 0.0V) */
#define WHITE_LEVEL      100
#define BURST_LEVEL      20
#define BLACK_LEVEL      0
#define BLANK_LEVEL      0
#define SYNC_LEVEL      -40

struct PAL_SETTINGS {
    const unsigned char *data; /* image data */
    int format;     /* pix format (one of the PAL_PIX_FORMATs in pal_core.h) */
    int w, h;       /* width and height of image */
    int raw;        /* 0 = scale image to fit monitor, 1 = don't scale */
    int as_color;   /* 0 = monochrome, 1 = full color */
    unsigned field; /* 0 - infinity */
    int hue;        /* 0-359 */
    int xoffset;    /* x offset in sample space. 0 is minimum value */
    int yoffset;    /* y offset in # of lines. 0 is minimum value */
    int color_phase_error; /* 0-2, 0 = none, 1 = mild, 2 = wild */
    /* make sure your PAL_SETTINGS struct is zeroed out before you do anything */
    int iirs_initialized; /* internal state */
    uint32_t *palette; /* FHorse */
};

#ifdef __cplusplus
}
#endif

#endif
