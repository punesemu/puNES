/*
 *  Copyright (C) 2010-2019 Fabio Cavallo (aka FHorse)
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
	GT_RUN,
	GT_PAUSE
};

#if defined (__unix__)
static void *gfx_thread_loop(void *arg);
#elif defined (__WIN32__)
static DWORD WINAPI gfx_thread_loop(void *arg);
#endif

struct _gfx_thread {
#if defined (__unix__)
	pthread_t *thread;
	pthread_mutex_t lock;
#elif defined (__WIN32__)
	HANDLE thread;
	HANDLE lock;
#endif
	BYTE in_run;
	BYTE action;
} gfx_thread;

BYTE gfx_thread_init(void) {
	memset(&gfx_thread_public, 0x00, sizeof(gfx_thread_public));
	memset(&gfx_thread, 0x00, sizeof(gfx_thread));

	gfx_thread.action = GT_PAUSE;

#if defined (__unix__)
	if (pthread_mutex_init(&gfx_thread.lock, NULL) != 0) {
		fprintf(stderr, "Unable to allocate the emu mutex\n");
		return (EXIT_ERROR);
	}
	gfx_thread.thread = malloc(sizeof(pthread_t));
	pthread_create(gfx_thread.thread, NULL, gfx_thread_loop, NULL);
#elif defined (__WIN32__)
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
#elif defined (__WIN32__)
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
#elif defined (__WIN32__)
	WaitForSingleObject((HANDLE **)gfx_thread.lock, INFINITE);
#endif
}
void gfx_thread_unlock(void) {
#if defined (__unix__)
	pthread_mutex_unlock(&gfx_thread.lock);
#elif defined (__WIN32__)
	ReleaseSemaphore((HANDLE **)gfx_thread.lock, 1, NULL);
#endif
}

void gfx_thread_pause(void) {
	if (gfx_thread.action != GT_UNINITIALIZED) {
		gfx_thread.action = GT_PAUSE;

		while (gfx_thread.in_run == TRUE) {
			gui_sleep(1);
		}
	}
}
void gfx_thread_continue(void) {
	if (gfx_thread.action != GT_UNINITIALIZED) {
		gfx_thread.action = GT_RUN;

		while (gfx_thread.in_run == FALSE) {
			gui_sleep(1);
		}
	}
}

#if defined (__unix__)
static void *gfx_thread_loop(UNUSED(void *arg)) {
#elif defined (__WIN32__)
static DWORD WINAPI gfx_thread_loop(UNUSED(void *arg)) {
#endif
	while (info.stop == FALSE) {
		if (gfx_thread.action == GT_PAUSE) {
			gfx_thread.in_run = FALSE;
			gui_sleep(1);
			continue;
		}

		gfx_thread.in_run = TRUE;

		if (screen.rd->ready == FALSE) {
			gui_sleep(1);
			continue;
		}

		gfx_thread_public.filtering = TRUE;
		screen.rd->ready = FALSE;
		gfx_apply_filter();
		gfx_thread_public.filtering = FALSE;
	}

	gfx_thread.in_run = FALSE;

#if defined (__unix__)
	return (NULL);
#elif defined (__WIN32__)
	return (0);
#endif
}
