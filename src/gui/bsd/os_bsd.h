/*
 *  Copyright (C) 2010-2026 Fabio Cavallo (aka FHorse)
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

#ifndef OS_BSD_H_
#define OS_BSD_H_

#include <sys/types.h>
#include <sys/sysctl.h>
#include <time.h>
#include <stdio.h>

static double high_resolution_ms(void);
static int __nsleep(const struct timespec *req, struct timespec *rem);

INLINE void gui_init_os(void) {
	gettimeofday(&gui.counterStart, nullptr);
	gui_get_ms = high_resolution_ms;
}
INLINE uTCHAR *gui_home(void) {
	static uTCHAR *home = NULL;

	if (!(home = getenv("HOME"))) {
		home = uQStringCD(QDir::homePath());
	}
	return (home);
}

void gui_sleep(double ms) {
	struct timespec req = {}, rem = {};
	time_t sec;

	if (ms <= 0) {
		return;
	}

	sec = (time_t)(ms / 1000.0f);
	ms = ms - ((double)sec * 1000.0f);
	req.tv_sec = sec;
	req.tv_nsec = ms * 1000000L;
	__nsleep(&req, &rem);
}
int gui_screen_id(void) {
	int wid = qt.screen->wogl->winId();

	return (wid);
}
int gui_win_id(void) {
	int wid = qt.mwin->winId();

	return (wid);
}

static double high_resolution_ms(void) {
	struct timeval time;
	double elapsed_seconds;
	double elapsed_useconds;

	gettimeofday(&time, nullptr);

	elapsed_seconds  = time.tv_sec  - gui.counterStart.tv_sec;
	elapsed_useconds = time.tv_usec - gui.counterStart.tv_usec;

	return ((elapsed_seconds * 1000.0f) + (elapsed_useconds / 1000.0f));
}
static int __nsleep(const struct timespec *req, struct timespec *rem) {
	struct timespec temp_rem;

	if (nanosleep(req, rem) == -1) {
		__nsleep(rem, &temp_rem);
	} else {
		return (EXIT_ERROR);
	}

	return (EXIT_OK);
}

#endif /* OS_BSD_H_ */
