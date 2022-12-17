/*
 *  Copyright (C) 2010-2023 Fabio Cavallo (aka FHorse)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef EMU_THREAD_H_
#define EMU_THREAD_H_

#include "common.h"

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC BYTE emu_thread_init(void);
EXTERNC void emu_thread_quit(void);

EXTERNC void emu_thread_pause(void);
EXTERNC void emu_thread_continue(void);

EXTERNC void emu_thread_pause_with_count(int *count);
EXTERNC void emu_thread_continue_with_count(int *count);
EXTERNC void emu_thread_continue_ctrl_count(int *count);

#undef EXTERNC

#endif /* EMU_THREAD_H_ */

