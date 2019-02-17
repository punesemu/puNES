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
#include "emu_thread.h"
#include "emu.h"
#include "info.h"
#include "gui.h"

enum emu_thread_states {
	ET_UNINITIALIZED,
	ET_RUN,
	ET_PAUSE
};

#if defined (__unix__)
static void *emu_thread_loop(void *arg);
#elif defined (_WIN32)
static DWORD WINAPI emu_thread_loop(void *arg);
#endif

struct _emu_thread {
#if defined (__unix__)
	pthread_t *thread;
#elif defined (_WIN32)
	HANDLE thread;
#endif
	BYTE in_run;
	BYTE action;
} emu_thread;

BYTE emu_thread_init(void) {
	memset(&emu_thread, 0x00, sizeof(emu_thread));

	emu_thread.action = ET_PAUSE;

#if defined (__unix__)
	emu_thread.thread = malloc(sizeof(pthread_t));
	pthread_create(emu_thread.thread, NULL, emu_thread_loop, NULL);
#elif defined (_WIN32)
	emu_thread.thread = CreateThread(NULL, 0, emu_thread_loop, NULL, 0, 0);
#endif
	return (EXIT_OK);
}
void emu_thread_quit(void) {
#if defined (__unix__)
	if (emu_thread.thread) {
		pthread_join((*emu_thread.thread), NULL);
		free(emu_thread.thread);
	}
#elif defined (_WIN32)
	if (emu_thread.thread) {
		WaitForSingleObject(emu_thread.thread, INFINITE);
		CloseHandle(emu_thread.thread);
	}
#endif
}

void emu_thread_pause(void) {
	if (emu_thread.action != ET_UNINITIALIZED) {
		emu_thread.action = ET_PAUSE;

		while (emu_thread.in_run == TRUE) {
			gui_sleep(1);
		}
	}
}
void emu_thread_continue(void) {
	if (emu_thread.action != ET_UNINITIALIZED) {
		emu_thread.action = ET_RUN;

		while (emu_thread.in_run == FALSE) {
			gui_sleep(1);
		}
	}
}

#if defined (__unix__)
static void *emu_thread_loop(UNUSED(void *arg)) {
#elif defined (_WIN32)
static DWORD WINAPI emu_thread_loop(UNUSED(void *arg)) {
#endif
	while (TRUE) {
		// gestione uscita
		if (info.stop == TRUE) {
			emu_thread.in_run = FALSE;
			break;
		} else if (emu_thread.action == ET_PAUSE) {
			emu_thread.in_run = FALSE;
			gui_sleep(1);
			continue;
		}
		emu_thread.in_run = TRUE;
		emu_frame();
	}
#if defined (__unix__)
	return (NULL);
#elif defined (_WIN32)
	return (0);
#endif
}
