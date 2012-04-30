/*
 * overscan.h
 *
 *  Created on: 11/gen/2012
 *      Author: fhorse
 */

#ifndef OVERSCAN_H_
#define OVERSCAN_H_

#include "common.h"

struct _overscan {
	BYTE enabled;
	BYTE up;
	BYTE down;
	BYTE left;
	BYTE right;
} overscan;

#endif /* OVERSCAN_H_ */
