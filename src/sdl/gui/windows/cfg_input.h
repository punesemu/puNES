/*
 * cfg_input.h
 *
 *  Created on: 27/nov/2013
 *      Author: fhorse
 */

#ifndef CFG_INPUT_H_
#define CFG_INPUT_H_

#include "common.h"
#define __GUI_BASE__
#include "gui.h"
#undef __GUI_BASE__
#include "cfg_file.h"

typedef struct {
	BYTE id;
	_port port;
} _cfg_port;

struct _cfg_input {
	HWND father;
	HWND child;
	_config_input settings;

	_cfg_port port[PORT_MAX];
} cfg_input;

void cfg_input_dialog(HWND hwnd);

#endif /* CFG_INPUT_H_ */
