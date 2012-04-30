/*
 * menu_nes_fds.h
 *
 *  Created on: 31/mar/2012
 *      Author: fhorse
 */

#ifndef MENU_NES_FDS_H_
#define MENU_NES_FDS_H_

#include "../gtk2.h"

void menu_nes_fds(GtkWidget *mainmenu, GtkAccelGroup *accel_group);
void menu_nes_fds_check(void);
void menu_nes_fds_eject_insert_disk(void);
void menu_nes_fds_select_side(int side);

#endif /* MENU_NES_FDS_H_ */
