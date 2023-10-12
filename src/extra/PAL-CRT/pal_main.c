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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "ppm_rw.h"
#include "bmp_rw.h"
#include "pal_core.h"


#ifndef CMD_LINE_VERSION
#define CMD_LINE_VERSION 1
#endif

/* there is no NES command line version */
#if ((PAL_SYSTEM == PAL_SYSTEM_NES) && CMD_LINE_VERSION)
#error NES mode does not have a command line version
#endif
static int
cmpsuf(char *s, char *suf, int nc)
{
    return strcmp(s + strlen(s) - nc, suf);
}

#define DRV_HEADER "PAL/CRT v%d.%d.%d by EMMIR 2018-2023\n",\
                    PAL_MAJOR, PAL_MINOR, PAL_PATCH

#if CMD_LINE_VERSION

static int dooverwrite = 1;
static int docolor = 1;
static int progressive = 0;
static int raw = 0;
static int chroma_c = 0;
static int save_analog = 0;

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
    printf(DRV_HEADER);
    printf("usage: %s -m|o|c|p|r|h|a outwidth outheight noise field phase_error infile outfile\n", p);
    printf("sample usage: %s -oc 640 480 24 0 2 in.ppm out.ppm\n", p);
    printf("sample usage: %s -pcr 768 576 24 5 0 in.ppm out.ppm\n", p);
    printf("sample usage: %s - 832 624 0 2 1 in.ppm out.ppm\n", p);
    printf("-- NOTE: the - after the program name is required\n");
    printf("\tfield number is only meaningful in progressive mode\n");
    printf("\tphase error is 0, 1, or 2. 0 being none, and 2 being the most\n");
    printf("------------------------------------------------------------\n");
    printf("\tm : monochrome\n");
    printf("\to : do not prompt when overwriting files\n");
    printf("\tc : do Hanover bar correction\n");
    printf("\tp : progressive scan (rather than interlaced)\n");
    printf("\tr : raw image (needed for images that use artifact colors)\n");
    printf("\ta : save analog signal as image instead of decoded image\n");
    printf("\th : print help\n");
    printf("\n");
    printf("by default, the image will be full color, interlaced, and scaled to the output dimensions\n");
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
            case 'm': docolor = 0;     break;
            case 'o': dooverwrite = 0; break;
            case 'c': chroma_c = 1;    break;
            case 'p': progressive = 1; break;
            case 'r': raw = 1;         break;
            case 'a': save_analog = 1; break;
            case 'h': usage(argv[0]);  return 0;
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
    struct PAL_SETTINGS pal;
    struct PAL_CRT crt;
    int *img;
    int imgw, imgh;
    int *output = NULL;
    int outw = 832;
    int outh = 624;
    int noise = 24;
    unsigned field = 0;
    int phase_error = 0;
    char *input_file;
    char *output_file;
    int err = 0;

    if (argc < 9) {
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    if (!process_args(argc, argv)) {
        return EXIT_FAILURE;
    }

    printf(DRV_HEADER);

    outw = stoint(argv[2], &err);
    if (err) {
        return EXIT_FAILURE;
    }

    outh = stoint(argv[3], &err);
    if (err) {
        return EXIT_FAILURE;
    }

    noise = stoint(argv[4], &err);
    if (err) {
        return EXIT_FAILURE;
    }

    if (noise < 0) noise = 0;

    field = stoint(argv[5], &err);
    if (err) {
        return EXIT_FAILURE;
    }

    phase_error = stoint(argv[6], &err);
    if (err) {
        return EXIT_FAILURE;
    }
    phase_error %= 3;
    
    output = calloc(outw * outh, sizeof(int));
    if (output == NULL) {
        printf("out of memory\n");
        return EXIT_FAILURE;
    }

    input_file = argv[7];
    output_file = argv[8];

    if (cmpsuf(input_file, ".ppm", 4) == 0) {
        if (!ppm_read24(input_file, &img, &imgw, &imgh, calloc)) {
            printf("unable to read image\n");
            return EXIT_FAILURE;
        }
    } else {
        if (!bmp_read24(input_file, &img, &imgw, &imgh, calloc)) {
            printf("unable to read image\n");
            return EXIT_FAILURE;
        }
    }
    printf("loaded %d %d\n", imgw, imgh);

    if (!promptoverwrite(output_file)) {
        return EXIT_FAILURE;
    }

    pal_init(&crt, outw, outh, PAL_PIX_FORMAT_BGRA, output);

    pal.data = img;
    pal.format = PAL_PIX_FORMAT_BGRA;
    pal.w = imgw;
    pal.h = imgh;
    pal.as_color = docolor;
    pal.field = field;
    pal.raw = raw;
    pal.hue = 0;
    pal.color_phase_error = phase_error;
    pal.yoffset = 8;
    
    crt.blend = 1;
    crt.scanlines = 1;
    crt.chroma_correction = chroma_c;

    printf("converting to %dx%d...\n", outw, outh);
    err = 0;
   
    /* accumulate 8 frames */
    while (err < 8) {
        pal_modulate(&crt, &pal);
        pal_demodulate(&crt, noise);
        if (!progressive) {
            pal.field++;
            pal_modulate(&crt, &pal);
            pal_demodulate(&crt, noise);
        }
        err++;
    }
        
    if (save_analog) {
        int i, norm;
        
        free(output);
        output = calloc(PAL_HRES * PAL_VRES, sizeof(int));
        for (i = 0; i < (PAL_HRES * PAL_VRES); i++) {
            norm = crt.analog[i] + 128;
            output[i] = norm << 16 | norm << 8 | norm;
        }
        outw = PAL_HRES;
        outh = PAL_VRES;
    }
    
    if (cmpsuf(output_file, ".ppm", 4) == 0) {
        if (!ppm_write24(output_file, output, outw, outh)) {
            printf("unable to write image\n");
            return EXIT_FAILURE;
        }
    } else {
        if (!bmp_write24(output_file, output, outw, outh)) {
            printf("unable to write image\n");
            return EXIT_FAILURE;
        }
    }
    printf("done\n");
    return EXIT_SUCCESS;
}
#else
#include "fw.h"
#if 0
#define XMAX 624
#define YMAX 832
#else
#define XMAX 832
#define YMAX 624
#endif
static int *video = NULL;
static VIDINFO *info;

static struct PAL_CRT crt;
static struct PAL_SETTINGS pal;

static int *img;
static int imgw;
static int imgh;

static int color = 1;
static int noise = 12;
static unsigned field = 0;
static int progressive = 0;
static int raw = 0;
static int hue = 0;
static int fadephos = 1; /* fade phosphors each frame */

static void
updatecb(void)
{
    if (pkb_key_pressed(FW_KEY_ESCAPE)) {
        sys_shutdown();
    }

    if (pkb_key_held('q')) {
        crt.black_point += 1;
        printf("crt.black_point   %d\n", crt.black_point);
    }
    if (pkb_key_held('a')) {
        crt.black_point -= 1;
        printf("crt.black_point   %d\n", crt.black_point);
    }

    if (pkb_key_held('w')) {
        crt.white_point += 1;
        printf("crt.white_point   %d\n", crt.white_point);
    }
    if (pkb_key_held('s')) {
        crt.white_point -= 1;
        printf("crt.white_point   %d\n", crt.white_point);
    }

    if (pkb_key_held(FW_KEY_ARROW_UP)) {
        crt.brightness += 1;
        printf("%d\n", crt.brightness);
    }
    if (pkb_key_held(FW_KEY_ARROW_DOWN)) {
        crt.brightness -= 1;
        printf("%d\n", crt.brightness);
    }
    if (pkb_key_held(FW_KEY_ARROW_LEFT)) {
        crt.contrast -= 1;
        printf("%d\n", crt.contrast);
    }
    if (pkb_key_held(FW_KEY_ARROW_RIGHT)) {
        crt.contrast += 1;
        printf("%d\n", crt.contrast);
    }
    if (pkb_key_held('1')) {
        crt.saturation -= 1;
        printf("%d\n", crt.saturation);
    }
    if (pkb_key_held('2')) {
        crt.saturation += 1;
        printf("%d\n", crt.saturation);
    }
    if (pkb_key_held('3')) {
        noise -= 1;
        if (noise < 0) {
            noise = 0;
        }
        printf("%d\n", noise);
    }
    if (pkb_key_held('4')) {
        noise += 1;
        printf("%d\n", noise);
    }
    if (pkb_key_held('5')) {
        hue--;
        if (hue < 0) {
            hue = 359;
        }
        printf("%d\n", hue);
    }
    if (pkb_key_held('6')) {
        hue++;
        if (hue > 359) {
            hue = 0;
        }
        printf("%d\n", hue);
    }

    if (pkb_key_pressed('7')) {
        pal.color_phase_error++;
        pal.color_phase_error %= 3;
        printf("color_phase_error: %d\n", pal.color_phase_error);
    }
    if (pkb_key_pressed('8')) {
        crt.chroma_correction ^= 1;
        printf("chroma_correction: %d\n", crt.chroma_correction);
    }
    
    if (pkb_key_pressed(FW_KEY_SPACE)) {
        color ^= 1;
    }
    
    if (pkb_key_pressed('m')) {
        fadephos ^= 1;
        printf("fadephos: %d\n", fadephos);
    }
    if (pkb_key_pressed('r')) {
        pal_reset(&crt);
    }
    if (pkb_key_pressed('g')) {
        crt.scanlines ^= 1;
        printf("crt.scanlines: %d\n", crt.scanlines);
    }
    if (pkb_key_pressed('b')) {
        crt.blend ^= 1;
        printf("crt.blend: %d\n", crt.blend);
    }
    if (pkb_key_pressed('k')) {
        crt.chroma_lag++;
        if (crt.chroma_lag > 8) {
            crt.chroma_lag = -8;
        }
        printf("chroma_lag: %d\n", crt.chroma_lag);
    }
    if (pkb_key_pressed('f')) {
        field++;
        printf("field: %d\n", field);
    }
    if (pkb_key_pressed('e')) {
        progressive ^= 1;
        printf("progressive: %d\n", progressive);
    }
    if (pkb_key_pressed('t')) {
        /* Analog array must be cleared since it normally doesn't get zeroed each frame
         * so active video portions that were written to in non-raw mode will not lose
         * their values resulting in the previous image being
         * displayed where the new, smaller image is not
         */
#if (PAL_SYSTEM == PAL_SYSTEM_PAL)
        /* clearing the analog buffer with optimized NES mode will cause the
         * image to break since the field does not get repopulated
         */
        memset(crt.analog, 0, sizeof(crt.analog));
#endif
        raw ^= 1;
        printf("raw: %d\n", raw);
    }
}

static void
fade_phosphors(void)
{
    int i, *v;
    unsigned int c;

    v = video;

    for (i = 0; i < info->width * info->height; i++) {
        c = v[i] & 0xffffff;
        v[i] = (c >> 1 & 0x7f7f7f) +
               (c >> 2 & 0x3f3f3f) +
               (c >> 3 & 0x1f1f1f) +
               (c >> 4 & 0x0f0f0f);
    }
}

static void
displaycb(void)
{  
    if (fadephos) {
        fade_phosphors();
    } else {
        memset(video, 0, info->width * info->height * sizeof(int));
    }
    /* not necessary to clear if you're rendering on a constant region of the display */
    /* memset(crt.analog, 0, sizeof(crt.analog)); */
#if (PAL_SYSTEM == PAL_SYSTEM_NES)
    pal.data = ppu_output_256x240;
    pal.w = 256;
    pal.h = 240;
    pal.hue = hue;
#else
    pal.data = img;
    pal.format = PAL_PIX_FORMAT_BGRA;
    pal.w = imgw;
    pal.h = imgh;
    pal.as_color = color;
    pal.field = field;
    pal.raw = raw;
    pal.hue = hue;
#endif
    pal_modulate(&crt, &pal);
    pal_demodulate(&crt, noise);
    if (!progressive) {
        field++;
    }
    vid_blit();
    vid_sync();
}

int
main(int argc, char **argv)
{
    int werr;
    char *input_file;

    sys_init();
    sys_updatefunc(updatecb);
    sys_displayfunc(displaycb);
    sys_keybfunc(pkb_keyboard);
    sys_keybupfunc(pkb_keyboardup);

    clk_mode(FW_CLK_MODE_HIRES);
    pkb_reset();
    sys_sethz(50);
    sys_capfps(1);

    werr = vid_open("pal-crt", XMAX, YMAX, 1, FW_VFLAG_VIDFAST);
    if (werr != FW_VERR_OK) {
        FW_error("unable to create window\n");
        return EXIT_FAILURE;
    }

    info = vid_getinfo();
    video = info->video;
    
    printf(DRV_HEADER);

    pal_init(&crt, info->width, info->height, PAL_PIX_FORMAT_BGRA, video);
    crt.blend = 1;
    crt.scanlines = 1;

    if (argc == 1) {
        fprintf(stderr, "Please specify PPM or BMP image input file.\n");
        return EXIT_FAILURE;
    }
    input_file = argv[1];
    
    if (cmpsuf(input_file, ".ppm", 4) == 0) {
        if (!ppm_read24(input_file, &img, &imgw, &imgh, calloc)) {
            fprintf(stderr, "unable to read image\n");
            return EXIT_FAILURE;
        }
    } else {
        if (!bmp_read24(input_file, &img, &imgw, &imgh, calloc)) {
            fprintf(stderr, "unable to read image\n");
            return EXIT_FAILURE;
        }
    }

    printf("loaded %d %d\n", imgw, imgh);

    sys_start();

    sys_shutdown();
    return EXIT_SUCCESS;
}

#endif
