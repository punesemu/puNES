/*
 * menu.h
 *
 *  Created on: 16/dic/2011
 *      Author: fhorse
 */

#ifndef MENU_H_
#define MENU_H_

#include "menu_file.h"
#include "menu_nes.h"
#include "menu_settings.h"
#include "menu_state.h"
#include "menu_help.h"
#include "menu_video_effect.h"
#include "menu_video_fullscreen.h"
#include "menu_video_tv_aspect_ratio.h"
#include "menu_video_interpolation.h"

void menu_create(GtkWidget *win, GtkWidget *mainbox);
void menu_hide(void);
void menu_show(void);

#endif /* MENU_H_ */
