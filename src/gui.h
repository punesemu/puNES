/*
 * gui.h
 *
 *  Created on: 02/dic/2011
 *      Author: fhorse
 */

#ifndef GUI_H_
#define GUI_H_

#if (defined MINGW32 || defined MINGW64) && defined SDL
#include "gui/nogui/nogui.h"
#elif defined MINGW32 || defined MINGW64
#include "gui/windows/win.h"
#endif
#if defined GTK
#include "gui/gtk2/gtk2.h"
#elif defined SDL
#include "gui/nogui/nogui.h"
#endif


#endif /* GUI_H_ */
