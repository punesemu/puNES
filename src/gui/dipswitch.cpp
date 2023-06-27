/*
 *  Copyright (C) 2010-2023 Fabio Cavallo (aka FHorse)
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
#include <QtCore/QFile>
#include <QtCore/QStandardPaths>
#include <QtCore/QTextStream>
#include "dipswitch.h"
#include "dlgDipswitch.hpp"
#include "version.h"
#include "gui.h"
#include "info.h"
#include "conf.h"

#define DIPCFGFILENAME "dip.cfg"

void search_in_cfg(QFile &file);
BYTE read_next_tag(void);
BYTE tag_game(void);
void tag_crc(void);
BYTE tag_setting(void);
void tag_choiche(void);
BYTE evaluate_crc32(void);
QString name_from_splitted(int index);
extern uint32_t to_uint(const QString &svalue, int base, int def = 0);

struct _dp_read {
	QTextStream in;
	QStringList splitted;
	QString line;
	QString lower;
	bool in_game;
	bool in_type;
	bool finded;
} dp_read;
_dipswitch dipswitch;
extern _dp_internal dp;

void dipswitch_reset(void) {
	::memset((void *)&dipswitch, 0x00, sizeof(dipswitch));
	dp.name = "";
	dp.crc32s.clear();
	dp.types.clear();
}
void dipswitch_search(void) {
	QStringList gdl = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
	QStringList data_locations;
	const QString gdf = uQString(gui_extract_base(gui_data_folder()));
	const QString gaf = uQString(gui_application_folder());

	dipswitch_reset();

	// elimino la gui_data_folder() e la gui_application_folder() dall'elenco perche' voglio che siano le ultime voci.
	gdl.removeAll(gdf);
	if (gdf.compare(gaf)) {
		gdl.removeAll(gaf);
		gdl.append(gdf);
	}
	foreach(const QString path, gdl) {
		data_locations.append(QString("%0/%1").arg(path, NAME));
	}
	data_locations.append(gaf);

	foreach(const QString path, data_locations) {
		QFile file(QString("%0/%1").arg(path, DIPCFGFILENAME));

		if (file.exists()) {
			search_in_cfg(file);
		}
	}
}
void dipswitch_update_value(void) {
	if (dipswitch.used) {
		for (int type = 0; type < dp.types.length(); type++) {
			for (int index = 0; index < dp.types.at(type).values.length(); index++) {
				if ((dipswitch.value & dp.types.at(type).mask) == dp.types.at(type).values.at(index).value) {
					index = (index + 1) % dp.types.at(type).values.length();
					dipswitch.value = (dipswitch.value & ~dp.types.at(type).mask) | dp.types.at(type).values.at(index).value;
					break;
				}
			}
		}
	}
}

void search_in_cfg(QFile &file) {
	if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		dp_read.in.setDevice(&file);
		dp_read.in_game = FALSE;
		dp_read.in_type = FALSE;
		dp_read.finded = FALSE;
		while (read_next_tag()) {
			if (dp_read.lower.startsWith("game name")) {
				if (!dp_read.in_game) {
					dp_read.in_game = tag_game();
				} else {
					if (evaluate_crc32()) {
						break;
					}
					dp_read.in_type = FALSE;
					dp_read.in_game = tag_game();
				}
				continue;
			} else if (dp_read.lower.startsWith("crc")) {
				if (!dp_read.in_game) {
					continue;
				}
				tag_crc();
				continue;
			} else if (dp_read.lower.startsWith("setting mask")) {
				if (!dp_read.in_game) {
					continue;
				}
				if (dp_read.in_type && dp.types.last().values.empty()) {
					dp.types.removeLast();
				}
				dp_read.in_type = tag_setting();
				continue;
			} else if (dp_read.lower.startsWith("choice value")) {
				if (!dp_read.in_game || !dp_read.in_type) {
					continue;
				}
				tag_choiche();
				continue;
			} else {
				continue;
			}
		}
		if (!evaluate_crc32()) {
			dipswitch_reset();
		} else {
			dipswitch.used = TRUE;
			dipswitch.value = 0;
			for (int type = 0; type < dp.types.length(); type++) {
				for (int index = 0; index < dp.types.at(type).values.length(); index++) {
					if (dp.types.at(type).values.at(index).value == dp.types.at(type).def) {
						dipswitch.value = (dipswitch.value & ~dp.types.at(type).mask) | dp.types.at(type).values.at(index).value;
						break;
					}
				}
			}
			if (cfg->dipswitch != -1) {
				dipswitch.value = cfg->dipswitch;
			}
		}
		file.close();
	}
}
BYTE read_next_tag(void) {
	while (!dp_read.in.atEnd()) {
		dp_read.line = dp_read.in.readLine().simplified();
		dp_read.lower = dp_read.line.toLower();
		if (dp_read.line.isEmpty() || dp_read.line.startsWith("#") || dp_read.line.startsWith(";")) {
			continue;
		}
		dp_read.splitted = dp_read.line.split(" ");
		return (TRUE);
	}
	return (FALSE);
}
BYTE tag_game(void) {
	dipswitch_reset();

	//game name "Vs. Super Mario Bros."
	if (dp_read.splitted.count() < 3) {
		return (FALSE);
	} else {
		dp.name = name_from_splitted(2);
	}
	return (TRUE);
}
void tag_crc(void) {
	QString buf;

	// crc 0xED588F00 ; DH3-E
	if ((dp_read.splitted.count() < 2) || (dp_read.splitted.at(0).compare("crc") != 0)) {
		return;
	}
	buf = dp_read.splitted.at(1);
	dp.crc32s.append(to_uint(buf.remove("0x"), buf.contains("0x") ? 16 : 10));
}
BYTE tag_setting(void) {
	_dp_type type;
	QString buf;

	//setting mask 0x07 default 0x00 name "Coinage"
	if ((dp_read.splitted.count() < 7) ||
		(dp_read.splitted.at(1).compare("mask") != 0) ||
		(dp_read.splitted.at(3).compare("default") != 0)||
		(dp_read.splitted.at(5).compare("name") != 0)) {
		return (FALSE);
	}
	buf = dp_read.splitted.at(2);
	type.mask = to_uint(buf.remove("0x"), buf.contains("0x") ? 16 : 10);
	buf = dp_read.splitted.at(4);
	type.def = to_uint(buf.remove("0x"), buf.contains("0x") ? 16 : 10);
	type.name = name_from_splitted(6);
	dp.types.append(type);
	return (TRUE);
}
void tag_choiche(void) {
	_dp_value value;
	QString buf;

	// choice value 0x03 name "5 Coins 1 Credit"
	if ((dp_read.splitted.count() < 5) ||
		(dp_read.splitted.at(1).compare("value") != 0) ||
		(dp_read.splitted.at(3).compare("name") != 0)) {
		return;
	}
	buf = dp_read.splitted.at(2);
	value.value = to_uint(buf.remove("0x"), buf.contains("0x") ? 16 : 10);
	value.name = name_from_splitted(4);
	dp.types.last().values.append(value);
}
BYTE evaluate_crc32(void) {
	if (dp.crc32s.empty()) {
		return (FALSE);
	}
	if (dp_read.finded) {
		return (TRUE);
	}
	dp_read.finded = FALSE;
	for (int i = 0; i < dp.crc32s.size(); ++i) {
		if (dp.crc32s.at(i) == info.crc32.prg) {
			dp_read.finded = TRUE;
			break;
		}
	}
	return (dp_read.finded);
}
QString name_from_splitted(int index) {
	QString name = "";

	for (int i = index; i < dp_read.splitted.length(); i++) {
		name += dp_read.splitted.at(i) + " ";
	}
	return (name.trimmed().remove('"'));
}
