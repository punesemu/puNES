/*
 * menu_mode.h
 *
 *  Created on: 16/dic/2011
 *      Author: fhorse
 */

#ifndef MENU_MODE_H_
#define MENU_MODE_H_

#include "../gtk2.h"

void menu_mode(GtkWidget *settings, GtkAccelGroup *accel_group);
void menu_mode_check(void);
void menu_mode_set_mode(int mode);

#endif /* MENU_MODE_H_ */
