/*****************************************************************************/
/*
 * NTSC/CRT - integer-only NTSC video signal encoding / decoding emulation
 *
 *   by EMMIR 2018-2023
 *
 *   YouTube: https://www.youtube.com/@EMMIR_KC/videos
 *   Discord: https://discord.com/invite/hdYctSmyQJ
 */
/*****************************************************************************/
#ifndef _CRT_SNES_H_
#define _CRT_SNES_H_

#ifdef __cplusplus
extern "C" {
#endif

/* crt_snes.h
 *
 * An interface to convert a digital image to an analog NTSC signal in a
 * fashion similar to an SNES.
 * 
 */
#define CRT_CC_LINE 2273

/* NOTE, in general, increasing CRT_CB_FREQ reduces blur and bleed */
#define CRT_CB_FREQ     4 /* carrier frequency relative to sample rate */
#define CRT_HRES        (CRT_CC_LINE * CRT_CB_FREQ / 10) /* horizontal res */
#define CRT_VRES        262                       /* vertical resolution */
#define CRT_INPUT_SIZE  (CRT_HRES * CRT_VRES)

#define CRT_TOP         15     /* first line with active video */
#define CRT_BOT         255    /* final line with active video */
#define CRT_LINES       (CRT_BOT - CRT_TOP) /* number of active video lines */

#define CRT_CC_SAMPLES  4 /* samples per chroma period (samples per 360 deg) */
#define CRT_CC_VPER     3 /* vertical period in which the artifacts repeat */

/* search windows, hsync is in terms of samples, vsync is lines */
#define CRT_HSYNC_WINDOW 6
#define CRT_VSYNC_WINDOW 6

/* accumulated signal threshold required for sync detection.
 * Larger = more stable, until it's so large that it is never reached in which
 *          case the CRT won't be able to sync
 */
#define CRT_HSYNC_THRESH 4
#define CRT_VSYNC_THRESH 94

/* 
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

#define CRT_DO_BANDLIMITING 0    /* enable/disable bandlimiting when encoding */
/* frequencies for bandlimiting */
#define L_FREQ           1431818 /* full line */
#define Y_FREQ           420000  /* Luma   (Y) 4.2  MHz */
#define I_FREQ           150000  /* Chroma (I) 1.5  MHz */
#define Q_FREQ           55000   /* Chroma (Q) 0.55 MHz */

/* IRE units (100 = 1.0V, -40 = 0.0V)
 * IRE is used because it fits nicely in an 8-bit signed char
 */
#define WHITE_LEVEL      100
#define BURST_LEVEL      20
#define BLACK_LEVEL      7
#define BLANK_LEVEL      0
#define SYNC_LEVEL      -40
#define IRE_MAX          110   /* max value is max value of signed char */
#define IRE_MIN          0     /* min value is min value of signed char */

/* how much Q's phase is offset relative to I.
 * Should generally be 90 degrees, however, changing
 * variables like CRT_CB_FREQ and CRT_CC_SAMPLES can
 * cause its sign to change from + to - or vice versa.
 * If you notice the SMPTE bars having colors in the wrong order even
 * when the right hue is set, then this might be what you need to change.
 */
#define Q_OFFSET  (-90)   /* in degrees */

/* burst hue offset */
#define HUE_OFFSET (210)  /* in degrees */

/* define line ranges in which sync is generated
 * the numbers are inclusive
 * Make sure these numbers fit in (0, CRT_VRES)
 */
#define SYNC_REGION_LO 3
#define SYNC_REGION_HI 6
/* same as above but for equalizing pulses */
#define EQU_REGION_A_LO 0
#define EQU_REGION_A_HI 2

#define EQU_REGION_B_LO 7
#define EQU_REGION_B_HI 9

struct NTSC_SETTINGS {
    const unsigned char *data; /* image data */
    int format;     /* pix format (one of the CRT_PIX_FORMATs in crt_core.h) */
    int w, h;       /* width and height of image */
    int raw;        /* 0 = scale image to fit monitor, 1 = don't scale */
    int as_color;   /* 0 = monochrome, 1 = full color */
    int field;      /* unused */
    int frame;      /* unused */
    int hue;        /* 0-359 */
    int xoffset;    /* x offset in sample space. 0 is minimum value */
    int yoffset;    /* y offset in # of lines. 0 is minimum value */
    int dot_crawl_offset; /* 0-3 */
    /* make sure your NTSC_SETTINGS struct is zeroed out before you do anything */
    int iirs_initialized; /* internal state */
};

#ifdef __cplusplus
}
#endif

#endif
