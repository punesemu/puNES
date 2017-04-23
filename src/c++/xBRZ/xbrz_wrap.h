/*
 * xbrz_wrap.h
 *
 *  Created on: 23 apr 2017
 *      Author: fhorse
 */

#ifndef XBRZ_WRAP_H_
#define XBRZ_WRAP_H_

#include "common.h"

#define XBRZ_NUM_SLICE 4

typedef struct _xbrz_wrap {
	int slice;
	BYTE factor;
	const WORD* src;
	uint32_t* trg;
	uint32_t* palette;
	int srcWidth;
	int srcHeight;
	int colFmt;
} _xbrz_wrap;

#endif /* XBRZ_WRAP_H_ */
