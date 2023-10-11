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

#ifndef _CRT_NES_H_
#define _CRT_NES_H_

#ifdef __cplusplus
extern "C" {
#endif

/* crt_nes.h
 *
 * An interface to convert NES PPU output to an analog NTSC signal.
 *
 */

/* 0 = vertical  chroma (228 chroma clocks per line) */
/* 1 = checkered chroma (227.5 chroma clocks per line) */
/* 2 = sawtooth  chroma (227.3 chroma clocks per line) */
#define CRT_CHROMA_PATTERN 2

/* chroma clocks (subcarrier cycles) per line */
#if (CRT_CHROMA_PATTERN == 1)
#define CRT_CC_LINE 2275
#elif (CRT_CHROMA_PATTERN == 2)
#define CRT_CC_LINE 2273
#else
/* this will give the 'rainbow' effect in the famous waterfall scene */
#define CRT_CC_LINE 2280
#endif

/* NOTE, in general, increasing CRT_CB_FREQ reduces blur and bleed */
#define CRT_CB_FREQ     4 /* carrier frequency relative to sample rate */

/* https://www.nesdev.org/wiki/NTSC_video#Scanline_Timing */
#define CRT_HRES        (CRT_CC_LINE * CRT_CB_FREQ / 10) /* horizontal res */
#define CRT_VRES        262                       /* vertical resolution */
#define CRT_INPUT_SIZE  (CRT_HRES * CRT_VRES)

#define CRT_TOP         15     /* first line with active video */
#define CRT_BOT         255    /* final line with active video */
#define CRT_LINES       (CRT_BOT - CRT_TOP) /* number of active video lines */

#define CRT_CC_SAMPLES  4 /* samples per chroma period (samples per 360 deg) */
#define CRT_CC_VPER     3 /* vertical period in which the artifacts repeat */

/* search windows, in samples */
#define CRT_HSYNC_WINDOW 6
#define CRT_VSYNC_WINDOW 6

/* accumulated signal threshold required for sync detection.
 * Larger = more stable, until it's so large that it is never reached in which
 *          case the CRT won't be able to sync
 */
#define CRT_HSYNC_THRESH 4
#define CRT_VSYNC_THRESH 94

/* NES composite signal is measured in terms of PPU pixels, or cycles
 * https://www.nesdev.org/wiki/NTSC_video#Scanline_Timing
 *
 *                         FULL HORIZONTAL LINE SIGNAL
 *             (341 PPU px; one cycle skipped on odd rendered frames)
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
#define PPUpx2pos(PPUpx) ((PPUpx) * CRT_HRES / LINE_PPUpx)
/* starting points for all the different pulses */
#define FP_BEG           PPUpx2pos(0)                                           /* front porch point */
#define SYNC_BEG         PPUpx2pos(FP_PPUpx)                                    /* sync tip point */
#define BW_BEG           PPUpx2pos(FP_PPUpx + SYNC_PPUpx)                       /* breezeway point */
#define CB_BEG           PPUpx2pos(FP_PPUpx + SYNC_PPUpx + BW_PPUpx)            /* color burst point */
#define BP_BEG           PPUpx2pos(FP_PPUpx + SYNC_PPUpx + BW_PPUpx + CB_PPUpx) /* back porch point */
#define LAV_BEG          PPUpx2pos(HB_PPUpx)                                    /* full active video point */
#define AV_BEG           PPUpx2pos(HB_PPUpx + PS_PPUpx + LB_PPUpx)              /* PPU active video point */
#define AV_LEN           PPUpx2pos(AV_PPUpx)                                    /* active video length */

/* somewhere between 7 and 12 cycles */
#define CB_CYCLES   10

/* line frequency */
#define L_FREQ           1431818 /* full line */

/* IRE units (100 = 1.0V, -40 = 0.0V) */
/* https://www.nesdev.org/wiki/NTSC_video#Terminated_measurement */
#define WHITE_LEVEL      110
#define BURST_LEVEL      30
#define BLACK_LEVEL      0
#define BLANK_LEVEL      0
#define SYNC_LEVEL      -37

struct NTSC_SETTINGS {
    const unsigned short *data; /* 6 or 9-bit NES 'pixels' */
    int w, h;       /* width and height of image */
    unsigned int border_color; /* either BG or black */
    int dot_crawl_offset; /* 0, 1, or 2 */
    /* NOTE: NES mode is always progressive */
    int hue;              /* 0-359 */
    int xoffset;    /* x offset in sample space. 0 is minimum value */
    int yoffset;    /* y offset in # of lines. 0 is minimum value */
    /* make sure your NTSC_SETTINGS struct is zeroed out before you do anything */
    int field_initialized; /* internal state */
};

#ifdef __cplusplus
}
#endif

#endif
