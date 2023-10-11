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

#include "ppm_rw.h"

extern int
ppm_read24(char *file,
           int **out_color, int *out_w, int *out_h,
           void *(*calloc_func)(size_t, size_t))
 {
    FILE *f;
    long beg;
    int *out;
    int i, npix;
    int r, g, b;
    int header = 0;
    char buf[64];
    int maxc = 0xff;

    f = fopen(file, "rb");
    if (f == NULL) {
        printf("[ppm_rw] unable to open ppm: %s\n", file);
        return 0;
    }
    while (header < 3) {
        if (!fgets(buf, sizeof(buf), f)) {
            printf("[ppm_rw] invalid ppm [no data]: %s\n", file);
            goto err;
        }
        if (buf[0] == '#') {
            continue;
        }
        switch (header) {
            case 0:
                if (buf[0] != 'P' || buf[1] != '6') {
                    printf("[ppm_rw] invalid ppm [not P6]: %s\n", file);
                    goto err;
                }
                break;
            case 1:
                if (sscanf(buf, "%d %d", out_w, out_h) != 2) {
                    printf("[ppm_rw] invalid ppm [no dim]: %s\n", file);
                    goto err;
                }
                break;
            case 2:
                maxc = atoi(buf);
                if (maxc > 0xff) {
                    printf("[ppm_rw] invalid ppm [>255]: %s\n", file);
                    goto err;
                }
                break;
            default:
                break;
        }
        header++;
    }
           
    beg = ftell(f);
    npix = *out_w * *out_h;
    *out_color = calloc_func(npix, sizeof(int));
    if (*out_color == NULL) {
        printf("[ppm_rw] out of memory loading ppm: %s\n", file);
        goto err;
    }
    out = *out_color;
    /*printf("ppm 24-bit w: %d, h: %d, s: %d\n", *out_w, *out_h, npix);*/
    for (i = 0; i < npix; i++) {
#define TO_8_BIT(x) (((x) * 255 + (maxc) / 2) / (maxc))
        r = TO_8_BIT(fgetc(f));
        g = TO_8_BIT(fgetc(f));
        b = TO_8_BIT(fgetc(f));
        if (feof(f)) {
            printf("[ppm_rw] early eof: %s\n", file);
            goto err;
        }
        out[i] = (r << 16 | g << 8 | b);
    }
    fseek(f, beg, SEEK_SET);
    fclose(f);
    return 1;
err:
    fclose(f);
    return 0;
}

extern int
ppm_write24(char *name, int *color, int w, int h)
{
    FILE *f;
    int i, npix, c;

    f = fopen(name, "wb");
    if (f == NULL) {
        printf("[ppm_rw] failed to write file: %s\n", name);
        return 0;
    }

    fprintf(f, "P6\n%d %d\n255\n", w, h);

    npix = w * h;
    for (i = 0; i < npix; i++) {
        c = *color++;
        fputc((c >> 16 & 0xff), f);
        fputc((c >> 8  & 0xff), f);
        fputc((c >> 0  & 0xff), f);
    }
    fclose(f);
    return 1;
}
