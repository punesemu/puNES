/*
 * gui_snd.h
 *
 *  Created on: 03/nov/2013
 *      Author: fhorse
 */

#ifndef GUI_SND_H_
#define GUI_SND_H_

#if defined MINGW32 || defined MINGW64
#include "gui/windows/snd.h"
#endif
#if defined GTK
#include "gui/gtk2/snd.h"
#endif

#endif /* GUI_SND_H_ */
