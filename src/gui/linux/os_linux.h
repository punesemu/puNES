/*
 * os_linux.h
 *
 *  Created on: 21/ott/2014
 *      Author: fhorse
 */

#ifndef OS_LINUX_H_
#define OS_LINUX_H_

#include <time.h>

double high_resolution_ms(void);
int __nsleep(const struct timespec *req, struct timespec *rem);

void gui_init(int *argc, char **argv) {
	//setenv("QT_NO_GLIB", "1", 1);

	memset(&gui, 0, sizeof(gui));
	memset(&qt, 0, sizeof(qt));

	qt.app = new QApplication((*argc), argv);

	info.gui = TRUE;
	gui.in_update = FALSE;
	gui.main_win_lfp = TRUE;

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
			//toStdString().c_str()
		}

		if (info.portable) {
			char path[sizeof(info.base_folder)], *dname;
			int length = readlink("/proc/self/exe", path, sizeof(path));

			if (length < 0) {
				fprintf(stderr, "INFO: Error resolving symlink /proc/self/exe.\n");
				info.portable = FALSE;
			} else if (length >= (signed int) sizeof(info.base_folder)) {
				fprintf(stderr, "INFO: Path too long. Truncated.\n");
				info.portable = FALSE;
			} else {
				/*
				 * I don't know why, but the string this readlink() function
				 * returns is appended with a '@'.
				 */
				if (path[length] == '@') {
					path[length] = 0;
				}

				dname = dirname(path);
				strcpy(info.base_folder, dname);
			}
		}

		if (!info.portable) {
			sprintf(info.base_folder, "%s/.%s", gui.home, NAME);
		}
 	}

	gettimeofday(&gui.counterStart, NULL);
	gui_get_ms = high_resolution_ms;
}
void gui_sleep(double ms) {
	struct timespec req = { 0 }, rem = { 0 };
	time_t sec;

	if (ms <= 0) {
		return;
	}

	sec = (time_t) (ms / 1000.0f);
	ms = ms - ((double) sec * 1000.0f);
	req.tv_sec = sec;
	req.tv_nsec = ms * 1000000L;
	__nsleep(&req, &rem);
}
int gui_screen_id(void) {
	int wid = qt.screen->winId();

	return (wid);
}

double high_resolution_ms(void) {
	struct timeval time;

	double elapsed_seconds;
	double elapsed_useconds;

	gettimeofday(&time, NULL);

	elapsed_seconds  = time.tv_sec  - gui.counterStart.tv_sec;
	elapsed_useconds = time.tv_usec - gui.counterStart.tv_usec;

	//return ((elapsed_seconds * 1000) + (elapsed_useconds / 1000.0f) + 0.5f);
	return ((elapsed_seconds * 1000.0f) + (elapsed_useconds / 1000.0f));
}
int __nsleep(const struct timespec *req, struct timespec *rem) {
	struct timespec temp_rem;

	if (nanosleep(req, rem) == -1) {
		__nsleep(rem, &temp_rem);
	} else {
		return (EXIT_ERROR);
	}

	return (EXIT_OK);
}

#endif /* OS_LINUX_H_ */
