/*
 * tas.c
 *
 *  Created on: 30/gen/2012
 *      Author: fhorse
 */

#include <string.h>
#include <stdio.h>
#include <libgen.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include "tas.h"
#include "input.h"
#include "sdltext.h"
#include "emu.h"

#define tas_set_data_port_ctrlstd(prt, dt)\
		prt.data[dt] = tas.il[tas.index].prt[dt]
#define tas_increment_index()\
	if (++tas.index == tas.count) {\
		tasRead();\
	}

_port tas_port_bck1, tas_port_bck2;

BYTE tasFile(char *ext, char *file) {
	if (!(strcasecmp(ext, ".fm2")) || !(strcasecmp(ext, ".FM2"))) {
		tas.type = FM2;
		tasHeader = tasHeader_FM2;
		tasRead = tasRead_FM2;
		tasFrame = tasFrame_FM2;
	}

	if (tas.type) {
		BYTE found = FALSE;

		memcpy(&tas_port_bck1, &port1, sizeof(_port));
		memcpy(&tas_port_bck2, &port2, sizeof(_port));

		strncpy(tas.file, file, sizeof(tas.file));

		tas.fp = fopen(tas.file, "r");

		tasHeader(file);

		{
			BYTE i;
			char rom_ext[4][10] = { ".nes\0", ".NES\0", ".fds\0", ".FDS\0" };

			for (i = 0; i < LENGTH(rom_ext); i++) {
				char rom_file[1024];
				struct stat status;

				strncpy(rom_file, info.romFile, sizeof(rom_file));
				strcat(rom_file, rom_ext[i]);

				if (!(access(rom_file, 0))) {
					stat(rom_file, &status);
					if (status.st_mode & S_IFREG) {
						strncpy(info.romFile, rom_file, sizeof(info.romFile));
						found = TRUE;
						break;
					}
				}
			}
		}

		if (found) {
			tasRead();
		} else {
			info.romFile[0] = 0;
			tasQuit();
		}
	}
	return (EXIT_OK);
}
void tasQuit(void) {
	if (tas.fp) {
		fclose(tas.fp);
	}

	tasHeader = NULL;
	tasRead = NULL;

	memcpy(&port1, &tas_port_bck1, sizeof(_port));
	memcpy(&port2, &tas_port_bck2, sizeof(_port));

	inputInit();

	tas.type = NOTAS;
}

void tasFrame_FM2(void) {
	/* il primo frame */
	if (!tas.frame) {
		/*
		 * sembra proprio che il mio reset / powerup
		 * duri un frame in meno rispetto all'fceux.
		 */
		if ((tas.emulator == FCEUX) && !tas.index) {
			textAddLineInfo(1, "enabled FCEUX compatible mode");
			tas.add_fake_frame = TRUE;
		}
		textAddLineInfo(1, "[yellow]silence, the movie has begun[normal]");
	}

	if (tas.il[tas.index].state > 0) {
		if (tas.il[tas.index].state == 1) {
			emuReset(RESET);
		} else if (tas.il[tas.index].state == 2) {
			emuReset(HARD);
		}
		tas.il[tas.index].state = 0;
		/*
		 * sembra proprio che il mio reset / powerup
		 * duri un frame in meno rispetto all'fceux.
		 */
		if (tas.emulator == FCEUX) {
			tas.add_fake_frame = TRUE;
		}
	}

	if (tas.add_fake_frame) {
		tas_increment_index()
		tas.frame++;
		tas.total_lag_frames++;
		tas.add_fake_frame = FALSE;
	}

	if (++tas.frame >= tas.total) {
		if (tas.frame == tas.total) {
			textAddLineSingle(4, FONT_12X10, 200, TXTCENTER, TXTCENTER, 0, 0, "The End");
		} else if (tas.frame == tas.total + 1) {
			/*
			 * finito il video l'FCEUX lascia settato il primo frame non del film
			 * ai valori dell'ultimo frame del film. Se non lo faccio,
			 * aglar-marblemadness.fm2 non finira' il gioco.
			 */
			return;
		} else {
			tasQuit();
			return;
		}
	}

	if (port1.type == STDCTRL) {
		tas_set_data_port_ctrlstd(port1, BUT_A);
		tas_set_data_port_ctrlstd(port1, BUT_B);
		tas_set_data_port_ctrlstd(port1, SELECT);
		tas_set_data_port_ctrlstd(port1, START);
		tas_set_data_port_ctrlstd(port1, UP);
		tas_set_data_port_ctrlstd(port1, DOWN);
		tas_set_data_port_ctrlstd(port1, LEFT);
		tas_set_data_port_ctrlstd(port1, RIGHT);
	}

	if (port2.type == STDCTRL) {
		tas_set_data_port_ctrlstd(port2, BUT_A);
		tas_set_data_port_ctrlstd(port2, BUT_B);
		tas_set_data_port_ctrlstd(port2, SELECT);
		tas_set_data_port_ctrlstd(port2, START);
		tas_set_data_port_ctrlstd(port2, UP);
		tas_set_data_port_ctrlstd(port2, DOWN);
		tas_set_data_port_ctrlstd(port2, LEFT);
		tas_set_data_port_ctrlstd(port2, RIGHT);
	}

	tas_increment_index()
}
void tasHeader_FM2(char *file) {
	char line[256];
	int start;

	tas.emulator = FCEUX;

	while (fgets(line, sizeof(line), tas.fp)) {
		char *key = 0, *value = 0;

		for (start = 0; start < strlen(line); start++) {
			if ((line[start] == ' ') || (line[start] == '\t')) {
				continue;
			}
			break;
		}

		if ((line[start] == '#') || (line[start] == '\r') || (line[start] == '\n')) {
			continue;
		}

		if (!(key = strtok(line + start, " "))) {
			continue;
		}

		if ((value = strtok(NULL, "\n"))) {
			int i;

			for (i = 0; i < strlen(value); i++) {
				if (value[i] == '\r') {
					value[i] = 0;
				}
			}
		}

		if (key[0] == '|') {
			tas.total++;
		} else if (strcasecmp(key, "emulator") == 0) {
			if (strcasecmp(value, "punes") == 0) {
				tas.emulator = PUNES;
			}
		} else if (strcasecmp(key, "emuVersion") == 0) {
			tas.emu_version = atoi(value);
		} else if (strcasecmp(key, "punesStartFrame") == 0) {
			tas.start_frame = atoi(value) + 1;
		} else if (strcasecmp(key, "romFilename") == 0) {
			//sprintf(info.romFile, "%s/%s", dirname(file), value);
			strcpy(info.romFile, dirname(file));
			strcat(info.romFile, "/");
			strcat(info.romFile, value);
		} else if (strcasecmp(key, "port0") == 0) {
			port1.type = atoi(value);
		} else if (strcasecmp(key, "port1") == 0) {
			port2.type = atoi(value);
		}
	}

	if (tas.emulator == FCEUX) {
		info.r4014_precise_timing_disabled = TRUE;
		//if (tas.emu_version <= 9828) {
			info.r2002_race_condition_disabled = TRUE;
			info.r4016_dmc_double_read_disabled = TRUE;
		//}
	}

	fseek(tas.fp, 0, SEEK_SET);
}
void tasRead_FM2(void) {
	int i, start;
	char line[256], *sep;

	tas.count = tas.index = 0;

	while (fgets(line, sizeof(line), tas.fp)) {
		for (start = 0; start < strlen(line); start++) {
			if ((line[start] == ' ') || (line[start] == '\t')) {
				continue;
			}
			break;
		}
		if (line[start] != '|') {
			continue;
		}

		start++;

		sep = strtok(line + start, "|");

		tas.il[tas.count].state = atoi(sep);

		/* port1 */
		sep = strtok(NULL, "|");

		if (port1.type == STDCTRL) {
			for (i = 0; i < 8; i++) {
				tas.il[tas.count].port1[RIGHT - i] = PRESSED;

				if ((sep[i] == ' ') || (sep[i] == '.')) {
					tas.il[tas.count].port1[RIGHT - i] = RELEASED;
				}
			}
		}

		/* port2 */
		sep = strtok(NULL, "|");

		if (port2.type == STDCTRL) {
			for (i = 0; i < 8; i++) {
				tas.il[tas.count].port2[RIGHT - i] = PRESSED;

				if ((sep[i] == ' ') || (sep[i] == '.')) {
					tas.il[tas.count].port2[RIGHT - i] = RELEASED;
				}
			}
		}

		if (++tas.count == LENGTH(tas.il)) {
			break;
		}
	}
}
