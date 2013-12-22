/*
 * gui.h
 *
 *  Created on: 02/dic/2011
 *      Author: fhorse
 */

#if defined __GUI_BASE__
#if defined MINGW32 || defined MINGW64
#include "gui/windows/win.h"
#endif
#if defined GTK
#include "gui/gtk2/gtk2.h"
#endif
#endif /* __GUI_BASE__ */

#if defined __GUI_SND__
#if defined MINGW32 || defined MINGW64
#include "gui/windows/snd.h"
#endif
#if defined GTK
#include "gui/gtk2/snd.h"
#endif
#endif /* __GUI_SND__ */
