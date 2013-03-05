/*
 * cfg_input.h
 *
 *  Created on: 04/nov/2011
 *      Author: fhorse
 */

#ifndef CFG_INPUT_H_
#define CFG_INPUT_H_

#include "common.h"

typedef struct {
	BYTE id;
	_port port;
} _cfg_port;

_cfg_port cfg_port1, cfg_port2;

void cfg_input(HWND hwnd);

#endif /* CFG_INPUT_H_ */
