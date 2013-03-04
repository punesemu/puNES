/*
 * cfginput.h
 *
 *  Created on: 04/nov/2011
 *      Author: fhorse
 */

#ifndef CFGINPUT_H_
#define CFGINPUT_H_

#include "common.h"

typedef struct {
	BYTE id;
	_port port;
} _cfg_port;

_cfg_port cfg_port1, cfg_port2;

void cfgInput(HWND hwnd);

#endif /* CFGINPUT_H_ */
