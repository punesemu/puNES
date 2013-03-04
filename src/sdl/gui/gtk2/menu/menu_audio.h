/*
 * menu_audio.h
 *
 *  Created on: 16/dic/2011
 *      Author: fhorse
 */

#ifndef MENU_AUDIO_H_
#define MENU_AUDIO_H_

#include "../gtk2.h"

void menu_audio(GtkWidget *settings, GtkAccelGroup *accel_group);
void menu_audio_check(void);
void menu_audio_set_audio_enable(void);

#endif /* MENU_AUDIO_H_ */
