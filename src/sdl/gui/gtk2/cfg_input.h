/*
 * cfg_input.h
 *
 *  Created on: 17/nov/2013
 *      Author: fhorse
 */

#ifndef CFG_INPUT_H_
#define CFG_INPUT_H_

#include "common.h"
#include "gui.h"
#include "cfg_file.h"

typedef struct {
	BYTE id;
	_port port;
} _cfg_port;

struct _cfg_input {
	GtkBuilder *builder;
	GtkWidget *father;
	GtkWidget *child;

	_config_input settings;

	_cfg_port port[PORT_MAX];
} cfg_input;

void cfg_input_dialog(void);

#endif /* CFG_INPUT_H_ */
