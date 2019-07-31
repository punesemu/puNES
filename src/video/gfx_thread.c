/*
 *  Copyright (C) 2010-2020 Fabio Cavallo (aka FHorse)
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

#if defined (__unix__)
#include <pthread.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "video/gfx_thread.h"
#include "video/gfx.h"
#include "info.h"
#include "clock.h"
#include "ppu.h"
#include "fps.h"
#include "gui.h"

enum gfx_thread_states {
	GT_UNINITIALIZED,
	GT_FALSE,
	GT_TRUE
};

#if defined (__unix__)
static void *gfx_thread_loop(void *arg);
#elif defined (_WIN32)
static DWORD WINAPI gfx_thread_loop(void *arg);
#endif

struct _gfx_thread {
#if defined (__unix__)
	pthread_t *thread;
	pthread_mutex_t lock;
#elif defined (_WIN32)
	HANDLE thread;
	HANDLE lock;
#endif
	BYTE in_run;
	int pause_calls;
} gfx_thread;

BYTE gfx_thread_init(void) {
	memset(&gfx_thread_public, 0x00, sizeof(gfx_thread_public));
	memset(&gfx_thread, 0x00, sizeof(gfx_thread));

#if defined (__unix__)
	if (pthread_mutex_init(&gfx_thread.lock, NULL) != 0) {
		fprintf(stderr, "Unable to allocate the emu mutex\n");
		return (EXIT_ERROR);
	}
	gfx_thread.thread = malloc(sizeof(pthread_t));
	pthread_create(gfx_thread.thread, NULL, gfx_thread_loop, NULL);
#elif defined (_WIN32)
	if ((gfx_thread.lock = CreateSemaphore(NULL, 1, 2, NULL)) == NULL) {
		fprintf(stderr, "Unable to allocate the emu mutex\n");
		return (EXIT_ERROR);
	}
	gfx_thread.thread = CreateThread(NULL, 0, gfx_thread_loop, NULL, 0, 0);
#endif
	return (EXIT_OK);
}
void gfx_thread_quit(void) {
#if defined (__unix__)
	if (gfx_thread.thread) {
		pthread_join((*gfx_thread.thread), NULL);
		free(gfx_thread.thread);
	}
	pthread_mutex_destroy(&gfx_thread.lock);
#elif defined (_WIN32)
	if (gfx_thread.thread) {
		WaitForSingleObject(gfx_thread.thread, INFINITE);
		CloseHandle(gfx_thread.thread);
	}
	if (gfx_thread.lock) {
		CloseHandle(gfx_thread.lock);
	}
#endif
}

void gfx_thread_lock(void) {
#if defined (__unix__)
	pthread_mutex_lock(&gfx_thread.lock);
#elif defined (_WIN32)
	WaitForSingleObject((HANDLE **)gfx_thread.lock, INFINITE);
#endif
}
void gfx_thread_unlock(void) {
#if defined (__unix__)
	pthread_mutex_unlock(&gfx_thread.lock);
#elif defined (_WIN32)
	ReleaseSemaphore((HANDLE **)gfx_thread.lock, 1, NULL);
#endif
}

void gfx_thread_pause(void) {
	if (gfx_thread.in_run == GT_UNINITIALIZED) {
		return;
	}

	gfx_thread.pause_calls++;

	while (gfx_thread.in_run == GT_TRUE) {
		gui_sleep(1);
	}
}
void gfx_thread_continue(void) {
	if (gfx_thread.in_run == GT_UNINITIALIZED) {
		return;
	}

	if (--gfx_thread.pause_calls < 0) {
		gfx_thread.pause_calls = 0;
	}

	if (gfx_thread.pause_calls == 0) {
		while (gfx_thread.in_run == GT_FALSE) {
			gui_sleep(1);
		}
	}
}

#if defined (__unix__)
static void *gfx_thread_loop(UNUSED(void *arg)) {
#elif defined (_WIN32)
static DWORD WINAPI gfx_thread_loop(UNUSED(void *arg)) {
#endif
	while (info.stop == FALSE) {
		if (gfx_thread.pause_calls) {
			gfx_thread.in_run = GT_FALSE;
			gui_sleep(1);
			continue;
		}

		gfx_thread.in_run = GT_TRUE;

		if (screen.rd->ready == FALSE) {
			gui_sleep(1);
			continue;
		}

		gfx_thread_public.filtering = TRUE;
		screen.rd->ready = FALSE;
		gfx_apply_filter();
		gfx_thread_public.filtering = FALSE;
	}

	gfx_thread.in_run = GT_FALSE;

#if defined (__unix__)
	return (NULL);
#elif defined (_WIN32)
	return (0);
#endif
}
