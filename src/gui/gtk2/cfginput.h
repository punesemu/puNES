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

_cfg_port cfg_port1;
_cfg_port cfg_port2;

GtkWidget *cfg_controllers_toplevel;

void cfgInput(void);
void cfg_input_resize_std_widget(GtkWidget *widget);
GtkWidget *cfg_input_std_button(const char *description);
GtkWidget *cfg_input_ok_cancel(GCallback ok, GCallback cancel, _cfg_port *cfgport);

#endif /* CFGINPUT_H_ */
