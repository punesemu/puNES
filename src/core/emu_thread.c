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

#include "emu_thread.h"
#include "emu.h"
#include "gui.h"
#include "info.h"

THFUNCT emu_thread_loop(void *data);

static struct _emu_thread {
	LCKTYPE lock;
	THTYPE loop;
#if defined (__WIN32__)
	DWORD id;
#endif
} emu_thread;

void emu_thread_init(void) {
#if defined (__unix__)
	pthread_mutex_init(&emu_thread.lock, NULL);
	pthread_create(&emu_thread.loop, NULL, emu_thread_loop, NULL);
#elif defined (__WIN32__)
	emu_thread.lock = CreateSemaphore(NULL, 1, 2, NULL);
	emu_thread.loop = CreateThread(NULL, 0, emu_thread_loop, 0, 0, &emu_thread.id);
#endif
}
void emu_thread_quit(void) {
#if defined (__unix__)
	pthread_join(emu_thread.loop, NULL);
	pthread_mutex_destroy(&emu_thread.lock);
#elif defined (__WIN32__)
	WaitForSingleObject(emu_thread.loop, INFINITE);
	CloseHandle(emu_thread.loop);
	CloseHandle(emu_thread.lock);
#endif
}
void emu_thread_lock(void) {
#if defined (__unix__)
	pthread_mutex_lock(&emu_thread.lock);
#elif defined (__WIN32__)
	WaitForSingleObject(emu_thread.lock, INFINITE);
#endif
}
void emu_thread_unlock(void) {
#if defined (__unix__)
	pthread_mutex_unlock(&emu_thread.lock);
#elif defined (__WIN32__)
	ReleaseSemaphore(emu_thread.lock, 1, NULL);
#endif
}

THFUNCT emu_thread_loop(void *data) {
	while (info.stop == FALSE) {
		emu_frame();
	}
#if defined (__unix__)
	pthread_exit((void *)EXIT_OK);
#elif defined (__WIN32__)
	return (0);
#endif
}
