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

#include "bmp_rw.h"

/*
 * BMP image reader/writer kindly provided by 'deqmega' https://github.com/DEQ2000-cyber
 */

static unsigned char *
loadBMP(char *file, unsigned int *w, unsigned int *h, unsigned char *bpp,
        void *(*calloc_func)(size_t, size_t))
{
    FILE *f;
    unsigned char header[54];
    unsigned char pad[3];
    unsigned char *data = NULL;
    unsigned int width, height, size;
    unsigned char BPP, padding;
    unsigned int X;
    int Y;
    
    f = fopen(file, "rb");
    if (f == NULL) {
        return NULL;
    }
    fread(header, sizeof(unsigned char), 54, f);
    width = *(int*) &header[18];
    height = *(int*) &header[22];
    BPP = *(int*) &header[28];
    size = (width * height * (BPP / 8));
    padding = ((4 - (width * (BPP / 8)) % 4) % 4);
    data = calloc_func(size, (BPP / 8));
    if (data == NULL) {
        return NULL;
    }
    fseek(f, 54, SEEK_SET);
    for (Y = height - 1; Y >= 0; Y--) {
        for (X = 0; X < width; X++) {
            fread(&data[(Y * width + X) * (BPP / 8)], (BPP / 8), 1, f);
        }
        fread(pad, padding, 1, f);
    }
    fclose(f), f = NULL;
    *w = width;
    *h = height;
    *bpp = BPP;
    return data;
}

static void *
loadBMPconverter(char *file, int *w, int *h,
        void *(*calloc_func)(size_t, size_t))
{
    unsigned int *data = NULL;
    unsigned int x, y, i;
    unsigned char n;
    unsigned char *p;
    unsigned int *pix;
    
    p = loadBMP(file, &x, &y, &n, calloc_func);
    if (p == NULL) {
        return NULL;
    }
    *w = x;
    *h = y;
    data = calloc_func(x * y, sizeof(unsigned int));
    if (data == NULL) {
        return NULL;
    }
    pix = data;
    if ((n / 8) == 4) {
        memcpy(pix, p, (x * y * sizeof(unsigned int)));
        free(p);
        return data;
    }
    for (i = 0; i < (x * y * (n / 8)); i += (n / 8)) {
        *(pix++) = (p[i] << 0) | (p[i + 1] << 8) | (p[i + 2] << 16) | (255 << 24);
    }
    free(p);
    return data;
}

static int
saveBMP(char *file, int *data, unsigned int w,
        unsigned int h)
{
    FILE *f;
    unsigned int filesize;
    unsigned char pad[3], header[14], info[40];
    unsigned char padding;
    unsigned int X;
    int Y, bpp = 4;
    
    if (data == NULL) {
        return 0;
    }
    padding = ((4 - (w * bpp) % 4) % 4);
    memset(header, 0, sizeof(header));
    memset(info, 0, sizeof(info));
    filesize = 14 + 40 + w * h * bpp + padding * w;
    header[0] = 'B';
    header[1] = 'M';
    header[2] = filesize;
    header[3] = filesize >> 8;
    header[4] = filesize >> 16;
    header[5] = filesize >> 24;
    header[10] = 14 + 40;
    info[0] = 40;
    info[4]  = (w >>  0) & 0xff;
    info[5]  = (w >>  8) & 0xff;
    info[6]  = (w >> 16) & 0xff;
    info[7]  = (w >> 24) & 0xff;
    info[8]  = (h >>  0) & 0xff;
    info[9]  = (h >>  8) & 0xff;
    info[10] = (h >> 16) & 0xff;
    info[11] = (h >> 24) & 0xff;
    info[12] = 1;
    info[14] = bpp * 8;
    f = fopen(file, "wb");
    if (f == NULL) {
        return 0;
    }
    fwrite(header, 14, 1, f);
    fwrite(info, 40, 1, f);
    for (Y = h - 1; Y >= 0; Y--) {
        for (X = 0; X < w; X++) {
            fwrite(&data[Y * w + X], sizeof(int), 1, f);
        }
        fwrite(pad, padding, 1, f);
    }
    fclose(f);
    return 1;
}

extern int
bmp_read24(char *file, int **out_color, int *out_w, int *out_h,
        void *(*calloc_func)(size_t, size_t))
{
    *out_color = loadBMPconverter(file, out_w, out_h, calloc_func);
    return (*out_color != NULL);
}

extern int
bmp_write24(char *name, int *color, int w, int h)
{
    return saveBMP(name, color, (unsigned int) w, (unsigned int) h);
}
