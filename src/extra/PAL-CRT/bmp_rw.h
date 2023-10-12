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

#ifndef _BMP_RW_
#define _BMP_RW_

/* bmp_rw.h
 *
 * Routines to read and write BMP images. Kindly provided by 'deqmega' https://github.com/DEQ2000-cyber
 *
 */

extern int bmp_read24(char *file,
        int **out_color,
        int *out_w, int *out_h,
        void *(*calloc_func)(size_t, size_t));

extern int bmp_write24(char *name, int *color, int w, int h);

#endif
