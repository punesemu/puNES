/*
 *  Copyright (C) 2010-2016 Fabio Cavallo (aka FHorse)
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
#include <stdio.h>
#include <unistd.h>
#include "recent_roms.h"
#include "info.h"
#include "conf.h"
#include "cheat.h"

#define RECENT_ROMS_FILE "recent.cfg"

void recent_roms_init(void) {
	FILE *fp;
	char tmp[RECENT_ROMS_LINE];

	/* apro il file che contiene la lista */
	sprintf(tmp, "%s/%s", info.base_folder, RECENT_ROMS_FILE);

	/* se non esiste lo creo */
	if ((fp = fopen(tmp, "r")) == NULL) {
		if ((fp = fopen(tmp, "w")) == NULL) {
			return;
		}
		return;
	}

	fclose(fp);
}
void recent_roms_add(char *rom) {
	int index = 0, rr_index = 1, count = 0;
	_recent_roms rr_tmp;

	if ((cfg->cheat_mode == GAMEGENIE_MODE) && (gamegenie.phase == GG_LOAD_ROM)) {
		return;
	}

	memset(&rr_tmp, 0x00, sizeof(_recent_roms));

	for (index = 0; index < RECENT_ROMS_MAX; index++) {
		if (recent_roms_list.item[index][0] == 0) {
			break;
		}

		if (strncmp(recent_roms_list.item[index], rom, RECENT_ROMS_LINE) == 0) {
			recent_roms_list.item[index][0] = 0;
		}
	}

	strncpy(rr_tmp.item[0], rom, RECENT_ROMS_LINE);
	strncpy(rr_tmp.current, rom, RECENT_ROMS_LINE);

	for (index = 0; index < RECENT_ROMS_MAX; index++) {
		if (recent_roms_list.item[index][0] == 0) {
			continue;
		}
		if (++count < RECENT_ROMS_MAX) {
			strncpy(rr_tmp.item[rr_index++], recent_roms_list.item[index], RECENT_ROMS_LINE);
		}
	}

	memcpy(&recent_roms_list, &rr_tmp, sizeof(_recent_roms));

	recent_roms_save();
}
void recent_roms_parse(void) {
	int count = 0;
	char tmp[RECENT_ROMS_LINE], line[RECENT_ROMS_LINE];
	FILE *fp;

	memset(&recent_roms_list, 0x00, sizeof(_recent_roms));
	memset(line, 0x00, RECENT_ROMS_LINE);

	/* apro il file che contiene la lista */
	sprintf(tmp, "%s/%s", info.base_folder, RECENT_ROMS_FILE);

	if ((fp = fopen(tmp, "rt")) == NULL) {
		return;
	}

	/* leggo la lista */
	while (fgets(line, sizeof(line), fp)) {
		int index;

		/* elimino il ritorno a capo */
		line[strlen(line) - 1] = 0x00;

		/* se il file non esiste passo alla riga successiva */
		if (access(line, F_OK) == -1) {
			continue;
		}

		for (index = 0; index < RECENT_ROMS_MAX; index++) {
			if (recent_roms_list.item[index][0] == 0) {
				strncpy(recent_roms_list.item[index], line, RECENT_ROMS_LINE);
				break;
			}
			if (strncmp(recent_roms_list.item[index], line, RECENT_ROMS_LINE) == 0) {
				break;
			}
		}

		recent_roms_list.count = ++count;

		if (count == RECENT_ROMS_MAX) {
			break;
		}
	}

	fclose(fp);
}
void recent_roms_save(void) {
	int index = 0;
	char tmp[RECENT_ROMS_LINE];
	FILE *fp;

	/* apro il file */
	sprintf(tmp, "%s/%s", info.base_folder, RECENT_ROMS_FILE);

	if ((fp = fopen(tmp, "wt")) == NULL) {
		return;
	}

	for (index = 0; index < RECENT_ROMS_MAX; index++) {
		if (recent_roms_list.item[index][0] == 0) {
			break;
		}
		fprintf(fp, "%s\n", recent_roms_list.item[index]);
	}

	fclose(fp);

	recent_roms_list.count = index;
}
