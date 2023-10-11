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
#ifndef _CRT_PV1K_H_
#define _CRT_PV1K_H_

#ifdef __cplusplus
extern "C" {
#endif

/* crt_pv1k.h
 *
 * An interface to convert a digital image to an analog NTSC signal in a
 * fashion similar to a Casio PV-1000.
 * 
 */
#define CRT_CC_LINE 2304

/* NOTE, in general, increasing CRT_CB_FREQ reduces blur and bleed */
#define CRT_CB_FREQ     5 /* carrier frequency relative to sample rate */
#define CRT_HRES        (CRT_CC_LINE * CRT_CB_FREQ / 6) /* horizontal res */
#define CRT_VRES        262                       /* vertical resolution */
#define CRT_INPUT_SIZE  (CRT_HRES * CRT_VRES)

#define CRT_TOP         21     /* first line with active video */
#define CRT_BOT         261    /* final line with active video */
#define CRT_LINES       (CRT_BOT - CRT_TOP) /* number of active video lines */

#define CRT_CC_SAMPLES  5 /* samples per chroma period (samples per 360 deg) */
#define CRT_CC_VPER     5 /* vertical period in which the artifacts repeat */

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
#define DOT_ns 223
#define DOTx4_ns 892
#define LINE_BEG         0
#define FP_ns            (3*DOTx4_ns)      /* front porch */
#define SYNC_ns          (3*DOTx4_ns)      /* sync tip */
#define BW_ns            (2*DOTx4_ns)       /* breezeway */
#define CB_ns            (4*DOTx4_ns)      /* color burst */
#define BP_ns            (4*DOTx4_ns)      /* back porch */
#define AV_ns            (55*DOTx4_ns)     /* active video */
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

/* frequencies for bandlimiting */
#define L_FREQ           1431818 /* full line */
#define Y_FREQ           420000  /* Luma   (Y) 4.2  MHz */
#define I_FREQ           150000  /* Chroma (I) 1.5  MHz */
#define Q_FREQ           55000   /* Chroma (Q) 0.55 MHz */

/* IRE units (100 = 1.0V, -40 = 0.0V) */
#define WHITE_LEVEL      100
#define BURST_LEVEL      20
#define BLACK_LEVEL      7
#define BLANK_LEVEL      0
#define SYNC_LEVEL      -40

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
