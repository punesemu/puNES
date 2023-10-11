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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include "bmp_rw.h"
#include "crt_core.h"

/* First, configure the system you want to emulate by changing the value of
 * 'CRT_SYSTEM' inside crt_core.h
 *
 * Then compile this program using:
 * cc -O3 -o ntscvideo video_convert.c crt_core.c crt_ntsc.c crt_ntscvhs.c crt_pv1k.c crt_template.c bmp_rw.c
 * 
 * mkdir frames
 * mkdir output
 * ffmpeg -r 1 -i your_video.mov -r 1 ./frames/$frame%06d.bmp
 * ./ntscvideo <arguments>
 * ffmpeg -r 30 -f image2 -s 640x480 -i ./output/%06d.bmp -vcodec libx264 -crf 10 -pix_fmt yuv420p out.mp4
 */

#if (CRT_SYSTEM == CRT_SYSTEM_NES)
#error video frame converter does not currently support NES frame loading
#endif

#define DRV_HEADER "NTSC/CRT v%d.%d.%d by EMMIR 2018-2023\n",\
                    CRT_MAJOR, CRT_MINOR, CRT_PATCH

static int dooverwrite = 1;
static int docolor = 1;
static int progressive = 0;
static int scanlines = 1;
#if (CRT_SYSTEM == CRT_SYSTEM_NTSCVHS)
static int doaberration = 0;
#endif

static int
stoint(char *s, int *err)
{
    char *tail;
    long val;

    errno = 0;
    *err = 0;
    val = strtol(s, &tail, 10);
    if (errno == ERANGE) {
        printf("integer out of integer range\n");
        *err = 1;
    } else if (errno != 0) {
        printf("bad string: %s\n", strerror(errno));
        *err = 1;
    } else if (*tail != '\0') {
        printf("integer contained non-numeric characters\n");
        *err = 1;
    }
    return val;
}

static void
usage(char *p)
{
#if (CRT_SYSTEM == CRT_SYSTEM_NTSCVHS)
    printf("usage: %s -m|o|a|p|s|h num_frames outwidth outheight noise\n", p);
#else
    printf("usage: %s -m|o|p|s|h num_frames outwidth outheight noise\n", p);
#endif
    printf("sample usage: %s -oa 5000 640 480 0\n", p);
    printf("sample usage: %s - 1400 832 624 12\n", p);
    printf("-- NOTE: the - after the program name is required\n");
    printf("------------------------------------------------------------\n");
    printf("\tm : monochrome\n");
    printf("\to : do not prompt when overwriting files\n");
    printf("\ta : mess up the bottom of the frame (useful for the VHS look)\n");
    printf("\ts : fill in gaps between scan lines\n");
    printf("\tp : progressive scan (rather than interlaced)\n");
    printf("\th : print help\n");
    printf("\n");
    printf("by default, the image will be full color and interlaced\n");
}

static int
process_args(int argc, char **argv)
{
    char *flags;

    flags = argv[1];
    if (*flags == '-') {
        flags++;
    }
    for (; *flags != '\0'; flags++) {
        switch (*flags) {
            case 'm': docolor = 0;      break;
            case 'o': dooverwrite = 0;  break;
#if (CRT_SYSTEM == CRT_SYSTEM_NTSCVHS)
            case 'a': doaberration = 1; break;
#endif
            case 's': scanlines = 0;    break;
            case 'p': progressive = 1;  break;
            case 'h': usage(argv[0]); return 0;
            default:
                fprintf(stderr, "Unrecognized flag '%c'\n", *flags);
                return 0;
        }
    }
    return 1;
}

static int
fileexist(char *n)
{
    FILE *fp = fopen(n, "r");
    if (fp) {
        fclose(fp);
        return 1;
    }
    return 0;
}

static int
promptoverwrite(char *fn)
{
    if (dooverwrite && fileexist(fn)) {
        do {
            char c = 0;
            printf("\n--- file (%s) already exists, overwrite? (y/n)\n", fn);
            scanf(" %c", &c);
            if (c == 'y' || c == 'Y') {
                return 1;
            }
            if (c == 'n' || c == 'N') {
                return 0;
            }
        } while (1);
    }
    return 1;
}

int
main(int argc, char **argv)
{
    char buf[256];
    struct NTSC_SETTINGS ntsc;
    struct CRT crt;
    int *img = NULL;
    int imgw, imgh;
    int *output = NULL;
    int outw = 640;
    int outh = 480;
    int noise = 12;
    int err = 0;
    int nframes = 0;
    
    printf(DRV_HEADER);
    printf("This program does not operate on video files, only sequences of\n");
    printf("images. Please make sure you have the FFMPEG command line tools\n");
    printf("installed and follow these instructions to convert a video\n");
    printf("using the NTSC/CRT library:\n");
    printf("  mkdir frames\n");
    printf("  mkdir output\n");
    printf("  ffmpeg -r 1 -i your_video.mov -r 1 ./frames/$frame%%06d.bmp\n");
    printf("  ./%s <arguments>\n", argv[0]);
    printf("  ffmpeg -r 30 -f image2 -s 640x480 -i ./output/%%06d.bmp -vcodec libx264 -crf 10 -pix_fmt yuv420p out.mp4\n");
    printf("\n");
    printf("------------------------------------------------------------\n");

    if (argc < 5) {
        usage(argv[0]);
        return EXIT_FAILURE;
    }
    
    if (!process_args(argc, argv)) {
        return EXIT_FAILURE;
    }

    nframes = stoint(argv[2], &err);
    if (err) {
        return EXIT_FAILURE;
    }
    if (nframes <= 0) {
        printf("num_frames must be greater than 0!\n");
        return EXIT_FAILURE;
    }
    outw = stoint(argv[3], &err);
    if (err) {
        return EXIT_FAILURE;
    }
    if (outw <= 0) {
        printf("outwidth must be greater than 0!\n");
        return EXIT_FAILURE;
    }
    outh = stoint(argv[4], &err);
    if (err) {
        return EXIT_FAILURE;
    }
    if (outh <= 0) {
        printf("outheight must be greater than 0!\n");
        return EXIT_FAILURE;
    }
    noise = stoint(argv[5], &err);
    if (err) {
        return EXIT_FAILURE;
    }

    if (noise < 0) noise = 0;

    /* seed standard library PRNG */
    srand(time(0));
    
    output = calloc(outw * outh, sizeof(int));
    if (output == NULL) {
        printf("out of memory\n");
        return EXIT_FAILURE;
    }

    crt_init(&crt, outw, outh, CRT_PIX_FORMAT_BGRA, output);

    ntsc.format = CRT_PIX_FORMAT_BGRA;

    ntsc.as_color = docolor;
    ntsc.field = 0;
    ntsc.raw = 0;
    ntsc.hue = 0;
    ntsc.frame = 0;
#if (CRT_SYSTEM == CRT_SYSTEM_NTSCVHS)
    ntsc.do_aberration = doaberration;
#endif

    crt.blend = 0;
    crt.scanlines = scanlines;
    crt.saturation = 10;

    printf("converting to %dx%d...\n", outw, outh);
    err = 1;
    
    while (err < nframes) {
        if (img != NULL) {
            free(img);
            img = NULL;
        }
        sprintf(buf, "frames/%06d.bmp", err);
        if (!bmp_read24(buf, &img, &imgw, &imgh, calloc)) {
            printf("unable to read image %s\n", buf);
            return EXIT_FAILURE;
        }
        ntsc.data = img;
        ntsc.w = imgw;
        ntsc.h = imgh;
        crt_modulate(&crt, &ntsc);
        crt_demodulate(&crt, noise);
        if (!progressive) {
            ntsc.field ^= 1;
            if ((err & 1) == 0) {
                /* a frame is two fields */
                ntsc.frame ^= 1;
            }
        }
        sprintf(buf, "output/%06d.bmp", err);
        if (promptoverwrite(buf)) {
            if (!bmp_write24(buf, output, outw, outh)) {
                printf("unable to write image %s\n", buf);
                return EXIT_FAILURE;
            }
        }
        err++;
        printf("frame %d / %d\n", err, nframes);
    }
    
    printf("done\n");
    return EXIT_SUCCESS;
}
