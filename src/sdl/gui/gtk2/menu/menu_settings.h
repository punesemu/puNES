/*
 * menu_settings.h
 *
 *  Created on: 16/dic/2011
 *      Author: fhorse
 */

#ifndef MENU_SETTINGS_H_
#define MENU_SETTINGS_H_

#include "../gtk2.h"
#include "menu_mode.h"
#include "menu_video.h"
#include "menu_audio.h"
#include "menu_input.h"
#ifdef __NETPLAY__
#include "menu_netplay.h"
#endif
#include "menu_gamegenie.h"

void menu_settings(GtkWidget *mainmenu, GtkAccelGroup *accel_group);
void menu_settings_check(void);

#endif /* MENU_SETTINGS_H_ */
