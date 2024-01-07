/*
 *  Copyright (C) 2010-2024 Fabio Cavallo (aka FHorse)
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

#include <string.h>
#include "thread_def.h"
#include "emu_thread.h"
#include "emu.h"
#include "info.h"
#include "gui.h"
#include "debugger.h"

static thread_funct(emu_thread_loop, void *arg);

struct _emu_thread {
	thread_t thread;
	BYTE in_run;
	int pause_calls;
} emu_thread;

BYTE emu_thread_init(void) {
	memset(&emu_thread, 0x00, sizeof(emu_thread));
	thread_create(emu_thread.thread, emu_thread_loop, NULL);
	return (EXIT_OK);
}
void emu_thread_quit(void) {
	if (emu_thread.thread) {
		thread_join(emu_thread.thread);
		thread_free(emu_thread.thread);
	}
}

void emu_thread_pause(void) {
	if (emu_thread.in_run == TH_UNINITIALIZED) {
		return;
	}

	emu_thread.pause_calls++;

	while (emu_thread.in_run == TH_TRUE) {
		gui_sleep(1);
	}
}
void emu_thread_continue(void) {
	if (emu_thread.in_run == TH_UNINITIALIZED) {
		return;
	}

	if (--emu_thread.pause_calls < 0) {
		emu_thread.pause_calls = 0;
	}

	if (emu_thread.pause_calls == 0) {
		while (emu_thread.in_run == TH_FALSE) {
			if (info.stop) {
				break;
			}
			gui_sleep(1);
		}
	}
}

void emu_thread_pause_with_count(int *count) {
	emu_thread_pause();
	(*count)++;
}
void emu_thread_continue_with_count(int *count) {
	emu_thread_continue();
	(*count)--;
}
void emu_thread_continue_ctrl_count(int *count) {
	for (; (*count) > 0; (*count)--) {
		emu_thread_continue();
	}
	(*count) = 0;
}

static thread_funct(emu_thread_loop, UNUSED(void *arg)) {
	while (!info.stop) {
		if (emu_thread.pause_calls) {
			emu_thread.in_run = TH_FALSE;
			gui_sleep(1);
			continue;
		}

		emu_thread.in_run = TH_TRUE;

		if (debugger.mode != DBG_NODBG) {
			emu_frame_debugger();
		} else {
			emu_frame();
		}
	}

	emu_thread.in_run = TH_FALSE;

	thread_funct_return();
}
