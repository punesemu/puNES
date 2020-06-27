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

#include <QtCore/QtGlobal>
#include <QtCore/QTextStream>
#include <QtCore/QFileInfo>
#include <QtCore/QFile>
#include "recent_roms.h"
#include "info.h"
#include "conf.h"
#include "settings.h"
#include "gui.h"

#define RECENT_ROMS_FILE "recent.cfg"

static void recent_roms_reset_list(void);
static uTCHAR *recent_roms_file(void);

typedef struct _recent_roms {
	int count;
	QString item[RECENT_ROMS_MAX];
	QString current;
} _recent_roms;

_recent_roms recent_roms_list;

void recent_roms_init(void) {
	QFile recent;

	recent_roms_reset_list();

	recent.setFileName(uQString(recent_roms_file()));

	// se non esiste lo creo
	if (recent.exists() == false) {
		recent.open(QIODevice::WriteOnly);
		recent.close();
	}
}
void recent_roms_add(uTCHAR *file) {
	int index = 0, rr_index = 1, count = 0;
	_recent_roms rr_tmp;
	QString utf;
	uTCHAR *rom;

	if ((cfg->cheat_mode == GAMEGENIE_MODE) && (gamegenie.phase != GG_LOAD_ROM)) {
		return;
	}

	if (file[0] == 0) {
		return;
	}

	if ((rom = uncompress_storage_archive_name(file)) == NULL) {
		rom = file;
	}

	utf = uQString(rom);

	// normalizzo il path
	utf.replace('\\', '/');

	for (index = 0; index < RECENT_ROMS_MAX; index++) {
		if (recent_roms_list.item[index].isEmpty()) {
			break;
		}

		if (recent_roms_list.item[index] == utf) {
			recent_roms_list.item[index] = "";
		}
	}

	rr_tmp.item[0] = rr_tmp.current = utf;

	for (index = 0; index < RECENT_ROMS_MAX; index++) {
		if (recent_roms_list.item[index].isEmpty()) {
			continue;
		}
		if (++count < RECENT_ROMS_MAX) {
			rr_tmp.item[rr_index++] = recent_roms_list.item[index];
		}
	}

	// copio la lista temporanea nella lista reale
	recent_roms_list.count = rr_tmp.count;

	for (index = 0; index < RECENT_ROMS_MAX; index++) {
		recent_roms_list.item[index] = rr_tmp.item[index];
	}

	recent_roms_save();
}
void recent_roms_parse(void) {
	int count = 0;
	QFile recent;
	QString line;

	recent_roms_reset_list();

	recent.setFileName(uQString(recent_roms_file()));

	// apro il file che contiene la lista
	if (recent.open(QIODevice::ReadOnly) == false) {
		return;
	}

	QTextStream in(&recent);
	in.setCodec("UTF-8");

	while (in.atEnd() == false) {
		// elimino il ritorno a capo
		line = in.readLine().remove(NEWLINE);
		// normalizzo il path
		line.replace('\\', '/');

		// se il file non esiste passo alla riga successiva
		if (QFileInfo(line).exists() == false) {
			continue;
		}

		for (int index = 0; index < RECENT_ROMS_MAX; index++) {
			if (recent_roms_list.item[index].isEmpty()) {
				recent_roms_list.item[index] = line;
				break;
			}
			if (recent_roms_list.item[index] ==  line) {
				break;
			}
		}

		recent_roms_list.count = ++count;

		if (count == RECENT_ROMS_MAX) {
			break;
		}
	}

	recent.close();
}
void recent_roms_save(void) {
	int index = 0;
	QFile recent;

	recent.setFileName(uQString(recent_roms_file()));

	// apro il file
	if (recent.open(QIODevice::WriteOnly) == false) {
		return;
	}

	QTextStream out(&recent);
	out.setCodec("UTF-8");
	out.setGenerateByteOrderMark(false);

	for (index = 0; index < RECENT_ROMS_MAX; index++) {
		if (recent_roms_list.item[index].isEmpty()) {
			break;
		}
		out << recent_roms_list.item[index] << NEWLINE;
	}

	out.flush();
	recent.close();
	recent_roms_list.count = index;
}
int recent_roms_count(void) {
	return (recent_roms_list.count);
}
const char *recent_roms_item(int index) {
	return ((const char *) recent_roms_list.item[index].constData());
}
int recent_roms_item_size(int index) {
	return (recent_roms_list.item[index].length());
}
const char *recent_roms_current(void) {
	return ((const char *) recent_roms_list.current.constData());
}
int recent_roms_current_size(void) {
	return (recent_roms_list.current.length());
}

static void recent_roms_reset_list(void) {
	recent_roms_list.count = 0;

	for (int index = 0; index < RECENT_ROMS_MAX; index++) {
		recent_roms_list.item[index] = "";
	}
}
static uTCHAR *recent_roms_file(void) {
	static uTCHAR file[LENGTH_FILE_NAME_LONG];

	umemset(file, 0x00, LENGTH_FILE_NAME_LONG);
	usnprintf(file, usizeof(file), uL("" uPERCENTs "/" RECENT_ROMS_FILE), info.base_folder);
	return (file);
}
