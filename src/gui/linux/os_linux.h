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

#ifndef OS_LINUX_H_
#define OS_LINUX_H_

#include <time.h>

static double high_resolution_ms(void);
static int __nsleep(const struct timespec *req, struct timespec *rem);

void gui_init(int *argc, char **argv) {
	memset(&gui, 0, sizeof(gui));
	qt = {};

	qt.app = new QApplication((*argc), argv);

	info.gui = TRUE;
	gui.in_update = FALSE;
	gui.main_win_lfp = 0;

	/*
	// cerco il numero dei cores
	{
		long nprocs = -1;

		nprocs = sysconf(_SC_NPROCESSORS_ONLN);
		gui.cpu_cores = nprocs;
	}
	*/

	// cerco la HOME e imposto la directory base
	{
		gui.home = getenv("HOME");

		if (!gui.home) {
			gui.home = QDir::homePath().toUtf8().constData();
		}

		if (info.portable) {
			const uTCHAR procselfexe[20] = "proc/self/exe";
			uTCHAR path[usizeof(info.base_folder)];
			int length = readlink(procselfexe, uPTCHAR(path), usizeof(path));

			if (length < 0) {
				fprintf(stderr, "INFO: Error resolving symlink /proc/self/exe.\n");
				info.portable = FALSE;
			} else if (length >= (signed int) usizeof(info.base_folder)) {
				fprintf(stderr, "INFO: Path too long. Truncated.\n");
				info.portable = FALSE;
			} else {
				// I don't know why, but the string this readlink() function
				// returns is appended with a '@'.
				if (path[length] == '@') {
					path[length] = 0;
				}
				gui_utf_dirname(path, info.base_folder, usizeof(info.base_folder));
			}
		} else {
			usnprintf(info.base_folder, usizeof(info.base_folder), uL("" uPERCENTs "/." NAME), gui.home);
		}
 	}

	gettimeofday(&gui.counterStart, nullptr);
	gui_get_ms = high_resolution_ms;
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

static double high_resolution_ms(void) {
	struct timeval time;
	double elapsed_seconds;
	double elapsed_useconds;

	gettimeofday(&time, nullptr);

	elapsed_seconds  = time.tv_sec  - gui.counterStart.tv_sec;
	elapsed_useconds = time.tv_usec - gui.counterStart.tv_usec;

	//return ((elapsed_seconds * 1000) + (elapsed_useconds / 1000.0f) + 0.5f);
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

#endif /* OS_LINUX_H_ */
