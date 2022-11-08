/*
 *  Copyright (C) 2010-2022 Fabio Cavallo (aka FHorse)
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

#include <stdio.h>
#include <string.h>
#include "thread_def.h"
#include "video/gfx_thread.h"
#include "video/gfx.h"
#include "info.h"
#include "ppu.h"
#include "gui.h"

static thread_funct(gfx_thread_loop, void *arg);

struct _gfx_thread {
	thread_t thread;
	thread_mutex_t lock;
	BYTE in_run;
	int pause_calls;
} gfx_thread;

_gfx_thread_public gfx_thread_public;

BYTE gfx_thread_init(void) {
	memset(&gfx_thread_public, 0x00, sizeof(gfx_thread_public));
	memset(&gfx_thread, 0x00, sizeof(gfx_thread));
	if (thread_mutex_init_error(gfx_thread.lock)) {
		fprintf(stderr, "Unable to allocate the gfx mutex\n");
		return (EXIT_ERROR);
	}
	thread_create(gfx_thread.thread, gfx_thread_loop, NULL);
	return (EXIT_OK);
}
void gfx_thread_quit(void) {
	if (gfx_thread.thread) {
		thread_join(gfx_thread.thread);
		thread_free(gfx_thread.thread);
	}
	thread_mutex_destroy(gfx_thread.lock);
}

void gfx_thread_lock(void) {
	thread_mutex_lock(gfx_thread.lock);
}
void gfx_thread_unlock(void) {
	thread_mutex_unlock(gfx_thread.lock);
}

void gfx_thread_pause(void) {
	if (gfx_thread.in_run == TH_UNINITIALIZED) {
		return;
	}

	gfx_thread.pause_calls++;

	while (gfx_thread.in_run == TH_TRUE) {
		gui_sleep(1);
	}
}
void gfx_thread_continue(void) {
	if (gfx_thread.in_run == TH_UNINITIALIZED) {
		return;
	}

	if (--gfx_thread.pause_calls < 0) {
		gfx_thread.pause_calls = 0;
	}

	if (gfx_thread.pause_calls == 0) {
		while (gfx_thread.in_run == TH_FALSE) {
			if (info.stop == TRUE) {
				break;
			}
			gui_sleep(1);
		}
	}
}

static thread_funct(gfx_thread_loop, UNUSED(void *arg)) {
	while (info.stop == FALSE) {
		if (gfx_thread.pause_calls) {
			gfx_thread.in_run = TH_FALSE;
			gui_sleep(1);
			continue;
		}

		gfx_thread.in_run = TH_TRUE;

		if (screen.rd->ready == FALSE) {
			gui_sleep(1);
			continue;
		}

		gfx_thread_public.filtering = TRUE;
		screen.rd->ready = FALSE;
		gfx_apply_filter();
		gfx_thread_public.filtering = FALSE;
	}

	gfx_thread.in_run = TH_FALSE;

	thread_funct_return();
}
