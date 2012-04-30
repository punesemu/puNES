/*
 * menu_video_fullscreen.h
 *
 *  Created on: 16/dic/2011
 *      Author: fhorse
 */

#ifdef OPENGL
#ifndef MENU_VIDEO_FULLSCREEN_H_
#define MENU_VIDEO_FULLSCREEN_H_

#include "../gtk2.h"

void menu_video_fullscreen(GtkWidget *video, GtkAccelGroup *accel_group);
void menu_video_fullscreen_check(void);
void menu_video_fullscreen_switch_stretch(void);

#endif /* MENU_VIDEO_FULLSCREEN_H_ */
#endif
