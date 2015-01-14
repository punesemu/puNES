/*
 * overscan.h
 *
 *  Created on: 11/gen/2012
 *      Author: fhorse
 */

#ifndef OVERSCAN_H_
#define OVERSCAN_H_

#include "common.h"

enum overscan_limit {
	OVERSCAN_BORDERS_MIN = 0, OVERSCAN_BORDERS_MAX = 17
};

typedef struct {
	BYTE up;
	BYTE down;
	BYTE left;
	BYTE right;
} _overscan_borders;

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC struct overscan {
	BYTE enabled;
	_overscan_borders *borders;
} overscan;

EXTERNC _overscan_borders overscan_borders[2];

EXTERNC BYTE overscan_set_mode(BYTE mode);

#undef EXTERNC

#endif /* OVERSCAN_H_ */
