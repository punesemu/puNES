/*
 * menu_video_pixel_aspect_ratio.h
 *
 *  Created on: 16/mar/2014
 *      Author: fhorse
 */

#ifndef MENU_VIDEO_PIXEL_ASPECT_RATIO_H_
#define MENU_VIDEO_PIXEL_ASPECT_RATIO_H_

#include "../gtk2.h"

void menu_video_pixel_aspect_ratio(GtkWidget *video, GtkAccelGroup *accel_group);
void menu_video_pixel_aspect_ratio_check(void);
void menu_video_pixel_aspect_ratio_set(int par);
void menu_video_par_soft_stretch(void);

#endif /* MENU_VIDEO_PIXEL_ASPECT_RATIO_H_ */
