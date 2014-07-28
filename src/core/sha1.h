/*
 * sha1.h
 *
 *  Created on: 08/giu/2011
 *      Author: fhorse
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
