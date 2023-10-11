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
#ifndef _CRT_TEMP_H_
#define _CRT_TEMP_H_

#ifdef __cplusplus
extern "C" {
#endif

/* crt_template.h
 *
 * Your description here.
 * 
 */

/* NOTE: to add this to the main library, simply add it to the list at the 
 * top of crt_core.h and add its header include as another elif clause,
 * that's it!
 */

/* define number of chroma cycles per line
 * (scaled by 10, e.g 228 would be 228, 227.5 would be 2275)
 * 
 * if defining a fractional number of cycles, it would make sense to then
 * modify CRT_CC_VPER to reflect that.
 * For example,
 * 228.0  would repeat every line,
 * 227.5  would repeat every 2 lines,
 * 227.33 would repeat every 3, etc.
 */
#define CRT_CC_LINE 2275

/* NOTE, in general, increasing CRT_CB_FREQ reduces blur and bleed */
#define CRT_CB_FREQ     4 /* carrier frequency relative to sample rate */
#define CRT_HRES        (CRT_CC_LINE * CRT_CB_FREQ / 10) /* horizontal res */
#define CRT_VRES        262                       /* vertical resolution */
#define CRT_INPUT_SIZE  (CRT_HRES * CRT_VRES)

#define CRT_TOP         21     /* first line with active video */
#define CRT_BOT         261    /* final line with active video */
#define CRT_LINES       (CRT_BOT - CRT_TOP) /* number of active video lines */

#define CRT_CC_SAMPLES  4 /* samples per chroma period (samples per 360 deg) */
#define CRT_CC_VPER     2 /* vertical period in which the artifacts repeat */

/* search windows, hsync is in terms of samples, vsync is lines */
#define CRT_HSYNC_WINDOW 8
#define CRT_VSYNC_WINDOW 8

/* accumulated signal threshold required for sync detection.
 * Larger = more stable, until it's so large that it is never reached in which
 *          case the CRT won't be able to sync
 */
#define CRT_HSYNC_THRESH 4
#define CRT_VSYNC_THRESH 94

/*
 *                      FULL HORIZONTAL LINE SIGNAL (~63500 ns)
 * |---------------------------------------------------------------------------|
 *   HBLANK (~10900 ns)                 ACTIVE VIDEO (~52600 ns)
 * |-------------------||------------------------------------------------------|
 *   
 *   
 *   WITHIN HBLANK PERIOD:
 *   
 *   FP (~1500 ns)  SYNC (~4700 ns)  BW (~600 ns)  CB (~2500 ns)  BP (~1600 ns)
 * |--------------||---------------||------------||-------------||-------------|
 *      BLANK            SYNC           BLANK          BLANK          BLANK
 * 
 */
#define LINE_BEG         0
#define FP_ns            1500      /* front porch */
#define SYNC_ns          4700      /* sync tip */
#define BW_ns            600       /* breezeway */
#define CB_ns            2500      /* color burst */
#define BP_ns            1600      /* back porch */
#define AV_ns            52600     /* active video */
#define HB_ns            (FP_ns + SYNC_ns + BW_ns + CB_ns + BP_ns) /* h blank */
/* line duration should be ~63500 ns */
#define LINE_ns          (FP_ns + SYNC_ns + BW_ns + CB_ns + BP_ns + AV_ns)

/* convert nanosecond offset to its corresponding point on the sampled line */
#define ns2pos(ns)       ((ns) * CRT_HRES / LINE_ns)
/* starting points for all the different pulses */
#define FP_BEG           ns2pos(0)
#define SYNC_BEG         ns2pos(FP_ns)
#define BW_BEG           ns2pos(FP_ns + SYNC_ns)
#define CB_BEG           ns2pos(FP_ns + SYNC_ns + BW_ns)
#define BP_BEG           ns2pos(FP_ns + SYNC_ns + BW_ns + CB_ns)
#define AV_BEG           ns2pos(HB_ns)
#define AV_LEN           ns2pos(AV_ns)

/* somewhere between 7 and 12 cycles */
#define CB_CYCLES   10


#define CRT_DO_BANDLIMITING 1    /* enable/disable bandlimiting when encoding */
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
#define HUE_OFFSET (-60)  /* in degrees */

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

/* your NTSC_SETTINGS struct, add or remove data as you see fit */
struct NTSC_SETTINGS {
    const unsigned char *data; /* image data */
    int format;     /* pix format (one of the CRT_PIX_FORMATs in crt_core.h) */
    int w, h;       /* width and height of image */
    int raw;        /* 0 = scale image to fit monitor, 1 = don't scale */
    int as_color;   /* 0 = monochrome, 1 = full color */
    int field;      /* 0 = even, 1 = odd */
    int frame;      /* 0 = even, 1 = odd */
    int hue;        /* 0-359 */
    int xoffset;    /* x offset in sample space. 0 is minimum value */
    int yoffset;    /* y offset in # of lines. 0 is minimum value */
    int dot_crawl_offset; /* 0-5 */
    /* make sure your NTSC_SETTINGS struct is zeroed out before you do anything */
    int iirs_initialized; /* internal state */
};

#ifdef __cplusplus
}
#endif

#endif
