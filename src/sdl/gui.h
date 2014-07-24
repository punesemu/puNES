/*
 * gui.h
 *
 *  Created on: 02/dic/2011
 *      Author: fhorse
 */

#if defined (MINGW32) || defined (MINGW64)
#include "gui/windows/win.h"
#endif
#if defined (GTK)
#include "gui/gtk2/gtk2.h"
#endif
