/*
 *  Copyright (C) 2010-2020 Fabio Cavallo (aka FHorse)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef SHA1_H_
#define SHA1_H_

#include "common.h"

#ifndef _STD_TYPES
#define _STD_TYPES
#define uchar unsigned char
#define uint  unsigned int
#define ulong unsigned long int
#endif

typedef struct {
	ulong total[2];
	ulong state[5];
	uchar buffer[64];
} sha1_context;

/*
 * Core SHA-1 functions
 */
void sha1_starts(sha1_context *ctx);
void sha1_update(sha1_context *ctx, uchar *input, uint length);
void sha1_finish(sha1_context *ctx, uchar digest[20]);

/*
 * Output SHA-1(file contents), returns 0 if successful.
 */
int sha1_file(char *filename, uchar digest[20], char *string);

/*
 * Output SHA-1(buf)
 */
void sha1_csum(uchar *buf, uint buflen, uchar digest[20], char *string, int typecase);

/*
 * Output HMAC-SHA-1(key,buf)
 */
void sha1_hmac(uchar *key, uint keylen, uchar *buf, uint buflen, uchar digest[20]);

/*
 * Checkup routine
 */
int sha1_self_test(void);

#endif /* SHA1_H_ */
