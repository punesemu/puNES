/*
 * menu_video_vsync.h
 *
 *  Created on: 02/gen/2012
 *      Author: fhorse
 */

#ifdef OPENGL
#ifndef MENU_VIDEO_VSYNC_H_
#define MENU_VIDEO_VSYNC_H_

#include "../gtk2.h"

void menu_video_vsync(GtkWidget *video, GtkAccelGroup *accel_group);
void menu_video_vsync_check(void);

#endif /* MENU_VIDEO_VSYNC_H_ */
#endif
