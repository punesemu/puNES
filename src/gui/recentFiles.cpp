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

#include <QtCore/QtGlobal>
#include <QtCore/QTextStream>
#include <QtCore/QFileInfo>
#include <QtCore/QFile>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QtCore/QStringEncoder>
#endif
#include "recentFiles.hpp"
#include "conf.h"
#include "settings.h"
#include "gui.h"

void recentFiles::init(QString recent_file_name) {
	QFile file;

	file_name = recent_file_name;

	clear();
	file.setFileName(file_name);

	// se non esiste lo creo
	if (!file.exists()) {
		file.open(QIODevice::WriteOnly);
		file.close();
	}
}
void recentFiles::add(uTCHAR *file) {
	int index, rr_index = 1, count = 0;
	_recent_list tmp;
	QString utf;
	uTCHAR *rom;

	if ((cfg->cheat_mode == GAMEGENIE_MODE) && (gamegenie.phase != GG_LOAD_ROM)) {
		return;
	}
	if (file[0] == 0) {
		return;
	}

	if ((rom = uncompress_storage_archive_name(file)) == nullptr) {
		rom = file;
	}

	utf = uQString(rom);

	// normalizzo il path
	utf.replace('\\', '/');

	for (index = 0; index < RECENT_FILES_MAX; index++) {
		if (list.item[index].isEmpty()) {
			break;
		}

		if (list.item[index] == utf) {
			list.item[index] = "";
		}
	}

	tmp.item[0] = tmp.current = utf;

	for (index = 0; index < RECENT_FILES_MAX; index++) {
		if (list.item[index].isEmpty()) {
			continue;
		}
		if (++count < RECENT_FILES_MAX) {
			tmp.item[rr_index++] = list.item[index];
		}
	}

	// copio la lista temporanea nella lista reale
	list.count = tmp.count;

	for (index = 0; index < RECENT_FILES_MAX; index++) {
		list.item[index] = tmp.item[index];
	}

	save();
}
void recentFiles::parse(void) {
	int count = 0;
	QFile file;
	QString line;

	clear();

	file.setFileName(file_name);

	// apro il file che contiene la lista
	if (!file.open(QIODevice::ReadOnly)) {
		return;
	}

	QTextStream in(&file);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	in.setCodec("UTF-8");
#else
	in.setEncoding(QStringEncoder::Utf8);
#endif

	while (!in.atEnd()) {
		// elimino il ritorno a capo
		line = in.readLine().remove(NEWLINE);
		// normalizzo il path
		line.replace('\\', '/');

		// se il file non esiste passo alla riga successiva
		if (!QFileInfo::exists(line)) {
			continue;
		}

		for (QString &index : list.item) {
			if (index.isEmpty()) {
				index = line;
				break;
			}
			if (index ==  line) {
				break;
			}
		}

		list.count = ++count;

		if (count == RECENT_FILES_MAX) {
			break;
		}
	}

	file.close();
}
void recentFiles::save(void) {
	int index;
	QFile recent;

	recent.setFileName(file_name);

	// apro il file
	if (!recent.open(QIODevice::WriteOnly)) {
		return;
	}

	QTextStream out(&recent);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	out.setCodec("UTF-8");
#else
	out.setEncoding(QStringEncoder::Utf8);
#endif
	out.setGenerateByteOrderMark(false);

	for (index = 0; index < RECENT_FILES_MAX; index++) {
		if (list.item[index].isEmpty()) {
			break;
		}
		out << list.item[index] << NEWLINE;
	}

	out.flush();
	recent.close();
	list.count = index;
}
int recentFiles::count(void) {
	return (list.count);
}
const char *recentFiles::item(int index) {
	return ((const char *)list.item[index].constData());
}
int recentFiles::item_size(int index) {
	return (list.item[index].length());
}
const char *recentFiles::current(void) {
	return ((const char *)list.current.constData());
}
int recentFiles::current_size(void) {
	return (list.current.length());
}

void recentFiles::clear(void) {
	list.count = 0;

	for (QString &index : list.item) {
		index = "";
	}
}
