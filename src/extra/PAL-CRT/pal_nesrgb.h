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

#ifndef _PAL_NESRGB_H_
#define _PAL_NESRGB_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* pal_nesrgb.h
 *
 * An interface to convert NES PPU output in RGB form to an analog PAL signal.
 * 
 */
#define PAL_CC_LINE 28417

/* NOTE, in general, increasing PAL_CB_FREQ reduces blur and bleed */
#define PAL_CB_FREQ     4 /* carrier frequency relative to sample rate */

/* https://www.nesdev.org/wiki/NTSC_video#Scanline_Timing */
#define PAL_HRES        (PAL_CC_LINE * PAL_CB_FREQ / 100) /* horizontal res */
#define PAL_VRES        312                       /* vertical resolution */
#define PAL_INPUT_SIZE  (PAL_HRES * PAL_VRES)

#define PAL_TOP         42     /* first line with active video */
#define PAL_BOT         282    /* final line with active video */

#define PAL_LINES       (PAL_BOT - PAL_TOP) /* number of active video lines */

/* NES composite signal is measured in terms of PPU pixels, or cycles
 * https://www.nesdev.org/wiki/NTSC_video#Scanline_Timing
 *
 *                         FULL HORIZONTAL LINE SIGNAL
 *                                 (341 PPU px)
 * |---------------------------------------------------------------------------|
 *   HBLANK (58 PPU px)               ACTIVE VIDEO (283 PPU px)
 * |-------------------||------------------------------------------------------|
 *
 *
 *   WITHIN HBLANK PERIOD:
 *
 *   FP (9 PPU px)  SYNC (25 PPU px) BW (4 PPU px) CB (15 PPU px) BP (5 PPU px)
 * |--------------||---------------||------------||-------------||-------------|
 *      BLANK            SYNC           BLANK          BLANK          BLANK
 *
 *
 *   WITHIN ACTIVE VIDEO PERIOD:
 *
 *   LB (15 PPU px)                 AV (256 PPU px)               RB (11 PPU px)
 * |--------------||--------------------------------------------||-------------|
 *      BORDER                           VIDEO                        BORDER
 *
 */
#define LINE_BEG         0
#define FP_PPUpx         9         /* front porch */
#define SYNC_PPUpx       25        /* sync tip */
#define BW_PPUpx         4         /* breezeway */
#define CB_PPUpx         15        /* color burst */
#define BP_PPUpx         5         /* back porch */
#define PS_PPUpx         1         /* pulse */
#define LB_PPUpx         15        /* left border */
#define AV_PPUpx         256       /* active video */
#define RB_PPUpx         11        /* right border */
#define HB_PPUpx         (FP_PPUpx + SYNC_PPUpx + BW_PPUpx + CB_PPUpx + BP_PPUpx) /* h blank */
/* line duration should be ~63500 ns */
#define LINE_PPUpx       (FP_PPUpx + SYNC_PPUpx + BW_PPUpx + CB_PPUpx + BP_PPUpx + PS_PPUpx + LB_PPUpx + AV_PPUpx + RB_PPUpx)

/* convert pixel offset to its corresponding point on the sampled line */
#define PPUpx2pos(PPUpx) ((PPUpx) * PAL_HRES / LINE_PPUpx)
/* starting points for all the different pulses */
#define FP_BEG           PPUpx2pos(0)                                           /* front porch point */
#define SYNC_BEG         PPUpx2pos(FP_PPUpx)                                    /* sync tip point */
#define BW_BEG           PPUpx2pos(FP_PPUpx + SYNC_PPUpx)                       /* breezeway point */
#define CB_BEG           PPUpx2pos(FP_PPUpx + SYNC_PPUpx + BW_PPUpx)            /* color burst point */
#define BP_BEG           PPUpx2pos(FP_PPUpx + SYNC_PPUpx + BW_PPUpx + CB_PPUpx) /* back porch point */
#define LAV_BEG          PPUpx2pos(HB_PPUpx)                                    /* full active video point */
#define AV_BEG           PPUpx2pos(HB_PPUpx + PS_PPUpx + LB_PPUpx)              /* PPU active video point */
#define AV_LEN           PPUpx2pos(AV_PPUpx)                                    /* active video length */

/* NES does not have the 25 Hz offset applied to each line */
#define OFFSET_25Hz(y)  (0)

/* somewhere between 7 and 12 cycles */
#define CB_CYCLES   10

/* line frequency */
#define L_FREQ           1773448

/* IRE units (100 = 1.0V, -40 = 0.0V) */
#define WHITE_LEVEL      110
#define BURST_LEVEL      30
#define BLACK_LEVEL      0
#define BLANK_LEVEL      0
#define SYNC_LEVEL      -37

struct PAL_SETTINGS {
    const unsigned char *data; /* image data */
    int format;     /* pix format (one of the PAL_PIX_FORMATs in pal_core.h) */
    int w, h;       /* width and height of image */
    /* NOTE: NES mode is always progressive */
    int hue;              /* 0-359 */
    int xoffset;    /* x offset in sample space. 0 is minimum value */
    int yoffset;    /* y offset in # of lines. 0 is minimum value */
    /* make sure your PAL_SETTINGS struct is zeroed out before you do anything */
    int field_initialized; /* internal state */
    uint32_t *palette; /* FHorse */
    
    /* internal data */
    int altline[6]; /* stores alternating line pattern */
};

#ifdef __cplusplus
}
#endif

#endif
