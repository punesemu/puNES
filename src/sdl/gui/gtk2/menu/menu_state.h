/*
 * menu_state.h
 *
 *  Created on: 16/dic/2011
 *      Author: fhorse
 */

#ifndef MENU_STATE_H_
#define MENU_STATE_H_

#include "../gtk2.h"

enum { INC, DEC };
enum { SAVE, LOAD };

void menu_state(GtkWidget *mainmenu, GtkAccelGroup *accel_group);
void menu_state_check(void);
void menu_state_saveslot_incdec(BYTE mode);
void menu_state_saveslot_action(BYTE mode);
void menu_state_saveslot_set(BYTE slot);

#endif /* MENU_STATE_H_ */
