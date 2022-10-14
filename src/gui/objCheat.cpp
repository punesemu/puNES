/*
 *  Copyright (C) 2010-2022 Fabio Cavallo (aka FHorse)
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

#include <QtCore/QFileInfo>
#include <QtCore/QTextStream>
#include <QtCore/QRegularExpression>
#include <QtCore/QSettings>
#include <QtWidgets/QMessageBox>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QtCore/QStringEncoder>
#endif
#include "objCheat.hpp"
#include "info.h"
#include "conf.h"
#include "gui.h"

#define CHEAT_XML_VERSION "1.0"
#define CHEATFILENAME uQString(gui_config_folder()) + QString(CHEAT_FOLDER) + "/" +\
	QFileInfo(uQString(info.rom.file)).completeBaseName() + ".xml"

static bool libretro_value(QSettings *set, QString key, QString &value);
static bool libretro_rd_file(QIODevice &device, QSettings::SettingsMap &map);

static const QChar gg_table[] = {
	'A', 'P', 'Z', 'L',
	'G', 'I', 'T', 'Y',
	'E', 'O', 'X', 'U',
	'K', 'S', 'V', 'N'
};
static const DBWORD rocky_key = 0xFCBDD274;
static const BYTE rocky_table[] = {
	 3, 13, 14,  1,  6,  9,  5,  0,
	12,  7,  2,  8, 10, 11,  4, 19,
	21, 23, 22, 20, 17, 16, 18, 29,
	31, 24, 26, 25, 30, 27, 28
};

objCheat::objCheat(QObject *parent) : QObject(parent) {
	clear_list();
}
objCheat::~objCheat() {}

void objCheat::read_game_cheats(QWidget *parent) {
	clear_list();

	if (info.no_rom) {
		return;
	}

	import_Nestopia_xml(parent, CHEATFILENAME);

	if (cfg->cheat_mode == CHEATSLIST_MODE) {
		apply_cheats();
	}
}
void objCheat::save_game_cheats(QWidget *parent) {
	if (info.no_rom) {
		return;
	}

	if (cheats.count() == 0) {
		if (QFile(CHEATFILENAME).exists()) {
			 QFile::remove(CHEATFILENAME);
		}
		return;
	}

	save_Nestopia_xml(parent, CHEATFILENAME);
}
void objCheat::clear_list(void) {
	if (cheats.count() > 0) {
		cheats.clear();
	}
}
void objCheat::apply_cheats(void) {
	_cheat *actual;
	chl_map cheat;
	int i;

	cheatslist_blank();

	if (cheats.count() == 0) {
		return;
	}

	for (i = 0; i < cheats.count(); i++) {
		cheat = cheats.at(i);

		actual = &cheats_list.cheat[cheats_list.counter];

		if (cheat["genie"].compare("-") != 0) {
			if ((cheat["enabled"].toInt() == 1) && (cheats_list.counter <= CL_CHEATS)
				&& (decode_gg(cheat["genie"], actual) == EXIT_OK)) {
				cheats_list.counter++;
			}
		} else if (cheat["rocky"].compare("-") != 0) {
			if ((cheat["enabled"].toInt() == 1) && (cheats_list.counter <= CL_CHEATS)
				&& (decode_rocky(cheat["rocky"], actual) == EXIT_OK)) {
				cheats_list.counter++;
			}
		} else {
			if ((cheat["enabled"].toInt() == 1) && (cheats_list.counter <= CL_CHEATS)
				&& (decode_ram(cheat, actual) == EXIT_OK)) {
				cheats_list.counter++;
			}
		}
	}

	if (cheats_list.counter == 1) {
		gui_overlay_info_append_msg_precompiled(17, &cheats_list.counter);
	} else if (cheats_list.counter > 1) {
		gui_overlay_info_append_msg_precompiled(18, &cheats_list.counter);
	}
}
bool objCheat::is_equal(int index, chl_map *find, bool description) {
	if (index >= cheats.count()) {
		return (false);
	}

	chl_map src = cheats.at(index);

	if (description == true) {
		if ((src["genie"] == (*find)["genie"]) &&
			(src["rocky"] == (*find)["rocky"]) &&
			(src["address"] == (*find)["address"]) &&
			(src["value"] == (*find)["value"]) &&
			(src["compare"] == (*find)["compare"]) &&
			(src["description"] == (*find)["description"]) &&
			(src["enabled_compare"] == (*find)["enabled_compare"])) {
			return (true);
		}
	} else {
		if ((src["genie"] == (*find)["genie"]) &&
			(src["rocky"] == (*find)["rocky"]) &&
			(src["address"] == (*find)["address"]) &&
			(src["value"] == (*find)["value"]) &&
			(src["compare"] == (*find)["compare"]) &&
			(src["enabled_compare"] == (*find)["enabled_compare"])) {
			return (true);
		}
	}

	return (false);
}
int objCheat::find_cheat(chl_map *find, bool description) {
	for (int i = 0; i < cheats.count(); i++) {
		if (is_equal(i, find, description)) {
			return (i);
		}
	}

	return (-1);
}

bool objCheat::decode_ram(chl_map ch, _cheat *cheat) {
	uint address = 0, replace = 0, enabled_compare = 0, compare = 0;
	bool ok;

	memset(cheat, 0x00, sizeof(_cheat));

	address = ch["address"].toUInt(&ok, 16);
	if (!ok || (address > 0xFFFF)) {
		return (EXIT_ERROR);
	}

	replace = ch["value"].toUInt(&ok, 16);
	if (!ok || (replace > 0xFF)) {
		return (EXIT_ERROR);
	}
	enabled_compare = ch["enabled_compare"].toUInt();
	if (!ok || (enabled_compare > 1)) {
		return (EXIT_ERROR);
	}
	if (ch["compare"].compare("-") != 0) {
		compare = ch["compare"].toUInt(&ok, 16);
		if (!ok || (compare > 0xFF)) {
			return (EXIT_ERROR);
		}
	}

	cheat->address = address;
	cheat->replace = replace;
	cheat->enabled_compare = enabled_compare;
	cheat->compare = compare;

	return (EXIT_OK);
}
void objCheat::complete_ram(chl_map *cheat) {
	if ((*cheat)["address"].isEmpty()) {
		cheat->clear();
		return;
	}

	if (cheat->contains("value") && (*cheat)["value"].isEmpty()) {
		cheat->clear();
		return;
	}

	if (cheat->contains("compare")) {
		if ((*cheat)["compare"].isEmpty()) {
			(*cheat)["compare"] = "-";
			cheat->insert("enabled_compare", "0");
		} else {
			cheat->insert("enabled_compare", "1");
		}
	} else {
		cheat->insert("compare", "-");
		cheat->insert("enabled_compare", "0");
	}

	cheat->insert("genie", "-");
	cheat->insert("rocky", "-");
}

bool objCheat::decode_gg(QString code, _cheat *cheat) {
	QByteArray lat1 = code.toLower().toLatin1();
	int len = lat1.length();
	BYTE codes[8];

	memset(cheat, 0x00, sizeof(_cheat));

	if (code.isEmpty() || ((len != 6) && (len != 8))) {
		return (EXIT_ERROR);
	}

	for (int i = 0; i < len; i++) {
		switch (lat1.at(i)) {
			case 'a':
				codes[i] = 0x00;
				break;
			case 'p':
				codes[i] = 0x01;
				break;
			case 'z':
				codes[i] = 0x02;
				break;
			case 'l':
				codes[i] = 0x03;
				break;
			case 'g':
				codes[i] = 0x04;
				break;
			case 'i':
				codes[i] = 0x05;
				break;
			case 't':
				codes[i] = 0x06;
				break;
			case 'y':
				codes[i] = 0x07;
				break;
			case 'e':
				codes[i] = 0x08;
				break;
			case 'o':
				codes[i] = 0x09;
				break;
			case 'x':
				codes[i] = 0x0A;
				break;
			case 'u':
				codes[i] = 0x0B;
				break;
			case 'k':
				codes[i] = 0x0C;
				break;
			case 's':
				codes[i] = 0x0D;
				break;
			case 'v':
				codes[i] = 0x0E;
				break;
			case 'n':
				codes[i] = 0x0F;
				break;
			default:
				return (EXIT_ERROR);
		}

		if ((i == 2) && !(codes[2] & 0x08)) {
			len = 6;
		}
	}

	cheat->address = 0x8000	|
		((codes[4] & 0x01)       | (codes[4] & 0x02)       | (codes[4] & 0x04)       |
		 (codes[3] & 0x08)       | (codes[2] & 0x01) << 4  | (codes[2] & 0x02) << 4  |
		 (codes[2] & 0x04) << 4  | (codes[1] & 0x08) << 4  | (codes[5] & 0x01) << 8  |
		 (codes[5] & 0x02) << 8  | (codes[5] & 0x04) << 8  | (codes[4] & 0x08) << 8  |
		 (codes[3] & 0x01) << 12 | (codes[3] & 0x02) << 12 | (codes[3] & 0x04) << 12);

	cheat->replace = (
		(codes[0] & 0x01)      | (codes[0] & 0x02)      | (codes[0] & 0x04)      |
		(codes[1] & 0x01) << 4 | (codes[1] & 0x02) << 4 | (codes[1] & 0x04) << 4 |
		(codes[0] & 0x08) << 4);

	if (len == 8) {
		cheat->enabled_compare = TRUE;
		cheat->replace |= codes[7] & 0x08;
		cheat->compare = (
			(codes[6] & 0x01)      | (codes[6] & 0x02)      | (codes[6] & 0x04)      |
			(codes[5] & 0x08)      | (codes[7] & 0x01) << 4 | (codes[7] & 0x02) << 4 |
			(codes[7] & 0x04) << 4 | (codes[6] & 0x08) << 4);
	} else {
		cheat->enabled_compare = FALSE;
		cheat->replace |= codes[5] & 0x08;
		cheat->compare = 0x00;
	}

	cheat->disabled = FALSE;

	return (EXIT_OK);
}
QString objCheat::encode_gg(_cheat *cheat) {
	QString gg;
	int i;

	if (cheat->address < 0x8000) {
		return ("-");
	}

	i = (cheat->enabled_compare ? 8 : 6);
	const int codes[8] = {
		(cheat->replace       & 0x07) | (cheat->replace >> 4 & 0x08),
		(cheat->replace >> 4  & 0x07) | (cheat->address >> 4 & 0x08),
		(cheat->address >> 4  & 0x07) | (cheat->enabled_compare ? 0x08 : 0x00),
		(cheat->address >> 12 & 0x07) | (cheat->address      & 0x08),
		(cheat->address       & 0x07) | (cheat->address >> 8 & 0x08),
		(cheat->address >> 8  & 0x07) | ((cheat->enabled_compare ? cheat->compare : cheat->replace) & 0x08),
		(cheat->enabled_compare ? ((cheat->compare & 0x07)      | (cheat->compare >> 4 & 0x08)) : 0),
		(cheat->enabled_compare ? ((cheat->compare >> 4 & 0x07) | (cheat->replace      & 0x08)) : 0)
	};

	for (int a = 0; a < i; a++) {
		gg.append(gg_table[codes[a]]);
	};

	return (gg);
}
void objCheat::complete_gg(chl_map *cheat) {
	_cheat ch;

	if (decode_gg((*cheat)["genie"], &ch) == EXIT_ERROR) {
		cheat->clear();
		return;
	}

	cheat->insert("rocky", "-");
	complete_from_code(cheat, &ch);
}

bool objCheat::decode_rocky(QString code, _cheat *cheat) {
	DBWORD input = 0, output = 0, key = rocky_key;
	QByteArray lat1 = code.toUpper().toLatin1();
	int i, len = lat1.length();

	memset(cheat, 0x00, sizeof(_cheat));

	if (code.isEmpty() || (len != 8)) {
		return (EXIT_ERROR);
	}

	for (i = 0; i < 8; i++) {
		char character = lat1.at(i ^ 7);
		DBWORD num;

		if (character >= '0' && character <= '9') {
			num = character - '0';
		} else if (character >= 'A' && character <= 'F') {
			num = character - 'A' + 0x0A;
		} else {
			return (EXIT_ERROR);
		}
		input |= num << (i * 4);
	}

	for (i = 31; i--; input <<= 1) {
		if ((key ^ input) & 0x80000000) {
			output |= 0x00000001 << rocky_table[i];
			key ^= 0xB8309722;
		}
		key <<= 1;
	}

	cheat->address = (output & 0x7FFF) | 0x8000;
	cheat->enabled_compare = TRUE;
	cheat->compare = (output >> 16) & 0xFF;
	cheat->replace = (output >> 24) & 0xFF;

	return (EXIT_OK);
}
QString objCheat::encode_rocky(_cheat *cheat) {
	DBWORD input, output = 0, key = rocky_key;
	QString rocky;
	int i;

	if ((cheat->enabled_compare == FALSE) || (cheat->address < 0x8000)) {
		return ("-");
	}

	input = (cheat->address & 0x7FFF) | (cheat->compare << 16) | (cheat->replace << 24);

	for (i = 31; i--; key = key << 1 & 0xFFFFFFFF) {
		const uint ctrl = input >> rocky_table[i] & 0x1;

		output |= (key >> 31 ^ ctrl) << (i+1);
		if (ctrl) {
			key ^= 0xB8309722;
		}
	}

	for (i = 8; i--;) {
		const char value = (output >> (i * 4)) & 0xF;

		rocky.append((value >= 0xA) ? (char)(value - 0xA + 'A') : (char)(value + '0'));
	}

	return (rocky);
}
void objCheat::complete_rocky(chl_map *cheat) {
	_cheat ch;

	if (decode_rocky((*cheat)["rocky"], &ch) == EXIT_ERROR) {
		cheat->clear();
		return;
	}

	cheat->insert("genie", "-");
	complete_from_code(cheat, &ch);
}

void objCheat::import_Nestopia_xml(QWidget *parent, QString path) {
	QFile *file = new QFile(path);

	if (file->open(QIODevice::ReadOnly | QIODevice::Text)) {
		QXmlStreamReader xmlReader(file);

		while (!xmlReader.atEnd() && !xmlReader.hasError()) {
			QXmlStreamReader::TokenType token = xmlReader.readNext();

			if (token == QXmlStreamReader::StartDocument) {
				continue;
			}

			if (token == QXmlStreamReader::StartElement) {
				if (!xmlReader.name().toString().compare("cheats", Qt::CaseInsensitive)) {
					continue;
				}
				if (!xmlReader.name().toString().compare("cheat", Qt::CaseInsensitive)) {
					chl_map cheat = parse_nestopia_cheat(xmlReader);

					if ((cheat.count() > 0) && (find_cheat(&cheat, true) == -1)) {
						cheats.append(cheat);
					}
				}
			}
		}
		if (xmlReader.hasError()) {
			QMessageBox::critical(parent, tr("Error on reading the file"), xmlReader.errorString(), QMessageBox::Ok);
		}
		xmlReader.clear();

		file->close();
	}
	delete (file);
}
void objCheat::import_MAME_xml(QWidget *parent, QString path) {
	QFile *file = new QFile(path);

	if (file->open(QIODevice::ReadOnly | QIODevice::Text)) {
		QXmlStreamReader xmlReader(file);

		while (!xmlReader.atEnd() && !xmlReader.hasError()) {
			QXmlStreamReader::TokenType token = xmlReader.readNext();

			if (token == QXmlStreamReader::StartDocument) {
				continue;
			}

			if (token == QXmlStreamReader::StartElement) {
				if (!xmlReader.name().toString().compare("mamecheat", Qt::CaseInsensitive)) {
					continue;
				}
				if (!xmlReader.name().toString().compare("cheat", Qt::CaseInsensitive)) {
					QList<chl_map> list = parse_mame_cheat(xmlReader);
					int i;

					for (i = 0; i < list.count(); i++) {
						if ((list[i].count() > 0) && (find_cheat(&list[i], true) == -1)) {
							cheats.append(list[i]);
						}
					}
				}
			}
		}
		if (xmlReader.hasError()) {
			QMessageBox::critical(parent, tr("Error on reading the file"), xmlReader.errorString(), QMessageBox::Ok);
		}
		xmlReader.clear();

		file->close();
	}
	delete (file);
}
void objCheat::import_FCEUX_cht(QString path) {
	QFile *file = new QFile(path);

	if (file->open(QIODevice::ReadOnly | QIODevice::Text)) {
		QTextStream in(file);

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
		in.setCodec("UTF-8");
#else
		in.setEncoding(QStringEncoder::Utf8);
#endif

		while (!in.atEnd()) {
			chl_map cheat = parse_fceux_cheat(in.readLine());

			if ((cheat.count() > 0) && (find_cheat(&cheat, true) == -1)) {
				cheats.append(cheat);
			}
		}

		file->close();
	}

	delete (file);
}
void objCheat::import_libretro_cht(QString path) {
	static const QSettings::Format cfg = QSettings::registerFormat("libretro", libretro_rd_file, NULL);
	QFileInfo file(path);
	QSettings *set;
	QString key, value;
	uint totals = 0, i;

	set = new QSettings(file.canonicalFilePath(), cfg);

	if (set->allKeys().contains("cheats", Qt::CaseInsensitive)) {
		bool ok;

		totals = set->value("cheats").toUInt(&ok);
		if (!ok) {
			totals = 0;
		}
	}

	for (i = 0; i < totals; i++) {
		key = QString("cheat%1_code").arg(i);
		if (libretro_value(set, key, value) == FALSE) {
			QString description = "", enable = "0";
			QList<chl_map> list;
			QStringList splitted;
			int a;

			if (value.contains("+")) {
				splitted = value.split("+");
			} else {
				splitted.append(value);
			}

			for (a = 0; a < splitted.count(); a++) {
				chl_map cheat;

				if (splitted.at(a).contains(":")) {
					QStringList chsplitted = splitted.at(a).split(":");
					uint adr, replace;
					bool ok;

					if (chsplitted.count() != 2) {
						continue;
					}

					adr = chsplitted.at(0).toUInt(&ok, 16);
					if (!ok || (adr > 0xFFFF)) {
						continue;
					}

					replace = chsplitted.at(1).toUInt(&ok, 16);
					if (!ok || (replace > 0xFF)) {
						continue;
					}

					cheat.insert("address", "0x" + QString("%0").arg(adr, 4, 16, QChar('0')));
					cheat.insert("value", "0x" + QString("%0").arg(replace, 2, 16, QChar('0')));

					complete_ram(&cheat);
					ram_to_gg(&cheat);
				} else {
					cheat.insert("genie", splitted.at(a));
					complete_gg(&cheat);
				}
				list.append(cheat);
			}

			key = QString("cheat%1_desc").arg(i);
			if (libretro_value(set, key, value) == FALSE) {
				description = value;
			}

			key = QString("cheat%1_enable").arg(i);
			if (libretro_value(set, key, value) == FALSE) {
				enable = value;
			}

			for (a = 0; a < list.count(); a++) {
				if (list.count() > 1) {
					list[a].insert("description", description + QString(" (%0 of %1)").arg(a + 1).arg(list.count()));
				} else {
					list[a].insert("description", description);
				}
				list[a].insert("enable", enable);
				if ((list[a].count() > 0) && (find_cheat(&list[a], true) == -1)) {
					cheats.append(list[a]);
				}
			}
		} else {
			continue;
		}
	}

	delete (set);
}

void objCheat::save_Nestopia_xml(QWidget *parent, QString path) {
	QFile *file;

	if (cheats.count() == 0) {
		return;
	}

	file = new QFile(path);

	if (!file->open(QIODevice::WriteOnly)) {
		QMessageBox::warning(parent, tr("Read only"), tr("The file is in read only mode"));
	} else {
		QXmlStreamWriter *xmlWriter = new QXmlStreamWriter(file);

		// with QT6 QXmlStreamWriter always encodes XML in UTF-8.
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
		xmlWriter->setCodec("UTF-8");
#endif

		xmlWriter->setAutoFormatting(true);

		xmlWriter->writeStartDocument();

		xmlWriter->writeStartElement("cheats");
		xmlWriter->writeAttribute("version", CHEAT_XML_VERSION);

		for (int i = 0; i < cheats.count(); i++) {
			chl_map cheat = cheats.at(i);

			xmlWriter->writeStartElement("cheat");
			xmlWriter->writeAttribute("enabled", cheat["enabled"]);

			if (cheat["genie"] != "-") {
				xmlWriter->writeStartElement("genie");
				xmlWriter->writeCharacters(cheat["genie"]);
				xmlWriter->writeEndElement();
			} else if (cheat["rocky"] != "-") {
				xmlWriter->writeStartElement("rocky");
				xmlWriter->writeCharacters(cheat["rocky"]);
				xmlWriter->writeEndElement();
			} else {
				if (cheat.contains("address")) {
					xmlWriter->writeStartElement("address");
					xmlWriter->writeCharacters(cheat["address"]);
					xmlWriter->writeEndElement();
				}
				if (cheat.contains("value")) {
					xmlWriter->writeStartElement("value");
					xmlWriter->writeCharacters(cheat["value"]);
					xmlWriter->writeEndElement();
				}
				if (cheat.contains("compare") && (cheat["compare"].compare("-") != 0)) {
					xmlWriter->writeStartElement("compare");
					xmlWriter->writeCharacters(cheat["compare"]);
					xmlWriter->writeEndElement();
				}
			}
			if (cheat.contains("description")) {
				xmlWriter->writeStartElement("description");
				xmlWriter->writeCharacters(cheat["description"]);
				xmlWriter->writeEndElement();
			}

			xmlWriter->writeEndElement();
		}
		xmlWriter->writeEndElement();

		xmlWriter->writeEndDocument();
		delete (xmlWriter);

		file->close();
	}
	delete (file);
}

chl_map objCheat::parse_nestopia_cheat(QXmlStreamReader &xml) {
	chl_map cheat;

	if ((xml.tokenType() != QXmlStreamReader::StartElement) && (xml.name() == QString("cheat"))) {
		return (cheat);
	}

	if (xml.attributes().hasAttribute("enabled")) {
		cheat["enabled"] = xml.attributes().value("enabled").toString();
	}

	xml.readNext();

	while (!((xml.tokenType() == QXmlStreamReader::EndElement) && (xml.name() == QString("cheat")))) {
		if (xml.tokenType() == QXmlStreamReader::StartElement) {
			if (!xml.name().toString().compare("genie", Qt::CaseInsensitive)) {
				add_element_data_to_map(xml, cheat);
			}
			if (!xml.name().toString().compare("rocky", Qt::CaseInsensitive)) {
				add_element_data_to_map(xml, cheat);
			}
			if (!xml.name().toString().compare("description", Qt::CaseInsensitive)) {
				add_element_data_to_map(xml, cheat);
			}
			if (!xml.name().toString().compare("address", Qt::CaseInsensitive)) {
				add_element_data_to_map(xml, cheat);
			}
			if (!xml.name().toString().compare("value", Qt::CaseInsensitive)) {
				add_element_data_to_map(xml, cheat);
			}
			if (!xml.name().toString().compare("compare", Qt::CaseInsensitive)) {
				add_element_data_to_map(xml, cheat);
			}
		}
		xml.readNext();
	}

	if (cheat.contains("genie")) {
		complete_gg(&cheat);
	} else if (cheat.contains("rocky")) {
		complete_rocky(&cheat);
	} else if (cheat.contains("address")) {
		complete_ram(&cheat);
	} else {
		cheat.clear();
	}

	return (cheat);
}
QList<chl_map> objCheat::parse_mame_cheat(QXmlStreamReader &xml) {
	QString desc = "";
	QList<chl_map> list;

	list.clear();

	if ((xml.tokenType() != QXmlStreamReader::StartElement) && (xml.name() == QString("cheat"))) {
		return (list);
	}

	if (xml.attributes().hasAttribute("desc")) {
		desc = xml.attributes().value("desc").toString();
	}

	xml.readNext();

	while (!((xml.tokenType() == QXmlStreamReader::EndElement) && (xml.name() == QString("cheat")))) {
		if (xml.tokenType() == QXmlStreamReader::StartElement) {
			bool enabled = 0;

			if (!xml.name().toString().compare("script", Qt::CaseInsensitive)) {
				enabled = xml.attributes().value("state").toString().compare("run", Qt::CaseInsensitive) == 0 ? false : true;
			}

			if (!xml.name().toString().compare("action", Qt::CaseInsensitive)) {
				QString condition =  xml.attributes().value("condition").toString();

				xml.readNext();

				if (xml.tokenType() == QXmlStreamReader::Characters) {
					QString text = xml.text().toString();
					uint adr1 = 0, adr2 = 0, compare = 0, value = 0;
					bool oka1 = false, oka2 = false, okc = false, okv = false;
					int index = -1;

					// condition
					index = condition.indexOf('@');
					if (index >= 0) {
						QStringList splitted = condition.mid(index + 1).split("==");

						if (splitted.count() == 2) {
							adr1 = splitted.at(0).toUInt(&oka1, 16);
							if (oka1 && (adr1 > 0xFFFF)) {
								oka1 = false;
							}
							compare = splitted.at(1).toUInt(&okc, 16);
							if (okc && (compare > 0xFF)) {
								okc = false;
							}
						}
					}

					// text
					index = text.indexOf('@');
					if (index >= 0) {
						QStringList splitted = text.mid(index + 1).split("=");

						if (splitted.count() == 2) {
							adr2 = splitted.at(0).toUInt(&oka2, 16);
							if (oka2 && (adr2 > 0xFFFF)) {
								oka2 = false;
							}
							value = splitted.at(1).toUInt(&okv, 16);
							if (okv && (value > 0xFF)) {
								okv = false;
							}
						}
					}

					if (oka2 & okv) {
						chl_map cheat;

						add_element_data_to_map("enabled", enabled ? "1" : "0", cheat);
						add_element_data_to_map("description", desc, cheat);
						add_element_data_to_map("address", "0x" + QString("%0").arg(adr2, 4, 16, QChar('0')).toUpper(), cheat);
						add_element_data_to_map("value", "0x" + QString("%0").arg(value, 2, 16, QChar('0')).toUpper(), cheat);
						if (okc) {
							add_element_data_to_map("compare", "0x" + QString("%0").arg(compare, 2, 16, QChar('0')).toUpper(), cheat);
						}

						complete_ram(&cheat);
						ram_to_gg(&cheat);

						list.append(cheat);
					}
				}
			}
		}
		xml.readNext();
	}

	if (list.count() > 1) {
		int i;

		for (i = 0; i < list.count(); i++) {
			list[i]["description"] = list[i]["description"] + QString(" (%0 of %1)").arg(i + 1).arg(list.count());
		}
	}

	return (list);
}
chl_map objCheat::parse_fceux_cheat(QString line) {
	QStringList splitted = line.split(":");
	chl_map cheat;

	if (splitted.length() < 5) {
		return (cheat);
	}

	// 0
	{
		int index = 0;

		if (splitted.at(0).at(index) != QLatin1Char('S')) {
			return (cheat);
		} else {
			index++;
		}

		if ((splitted.at(0).length() > index) && (splitted.at(0).at(index) == QLatin1Char('C'))) {
			cheat.insert("enabled_compare", "1");
			index++;
		} else {
			cheat.insert("enabled_compare", "0");
		}

		if (splitted.at(0).length() < 3) {
			cheat.insert("enabled", "0");
		} else {
			cheat.insert("enabled", "1");
			splitted.insert(1, splitted.at(0).mid(index));
		}
	}

	// 1
	cheat.insert("address", QString("0x" + splitted.at(1).toUpper()));

	// 2
	cheat.insert("value", QString("0x" + splitted.at(2).toUpper()));

	// 3
	if (!cheat["enabled_compare"].compare("0")) {
		cheat.insert("compare", "-");
		splitted.insert(4, splitted.at(3));
	} else {
		cheat.insert("compare", QString("0x" + splitted.at(3).toUpper()));
	}

	// 4
	cheat.insert("description", splitted.at(4));

	ram_to_gg(&cheat);

	return (cheat);
}

void objCheat::complete_from_code(chl_map *cheat, _cheat *ch) {
	cheat->insert("address", QString("0x" + QString("%1").arg(ch->address, 4, 16, QChar('0')).toUpper()));
	cheat->insert("value", QString( "0x" + QString("%1").arg(ch->replace, 2, 16, QChar('0')).toUpper()));
	cheat->insert("enabled_compare", QString("%1").arg(ch->enabled_compare));
	if (ch->enabled_compare) {
		cheat->insert("compare", QString("0x" + QString("%1").arg(ch->compare, 2, 16, QChar('0')).toUpper()));
	} else {
		cheat->insert("compare", "-");
	}
}
void objCheat::ram_to_gg(chl_map *cheat) {
	_cheat ch;

	decode_ram((*cheat), &ch);
	cheat->insert("genie", encode_gg(&ch));
	cheat->insert("rocky", "-");
}
void objCheat::add_element_data_to_map(QString element_name, QString text, chl_map &map) const {
	if (!element_name.compare("genie", Qt::CaseInsensitive) ||
		!element_name.compare("rocky", Qt::CaseInsensitive) ||
		!element_name.compare("compare", Qt::CaseInsensitive)) {
		if (text.isNull() || text.isEmpty() || !text.compare("-")) {
			return;
		}
	}
	map.insert(element_name.toLower(), text);
}
void objCheat::add_element_data_to_map(QXmlStreamReader &xml, chl_map &map) const {
	QString element_name;

	if (xml.tokenType() != QXmlStreamReader::StartElement) {
		return;
	}

	element_name = xml.name().toString();

	xml.readNext();

	if (xml.tokenType() != QXmlStreamReader::Characters) {
		return;
	}

	add_element_data_to_map(element_name, xml.text().toString(), map);
}

// ----------------------------------------- I/O -----------------------------------------

static bool libretro_value(QSettings *set, QString key, QString &value) {
	value = "";

	if (set->allKeys().contains(key, Qt::CaseInsensitive)) {
		value = set->value(key).toString();
		return (FALSE);
	}
	return (TRUE);
}
static bool libretro_rd_file(QIODevice &device, QSettings::SettingsMap &map) {
	QTextStream in(&device);

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	in.setCodec("UTF-8");
#else
	in.setEncoding(QStringEncoder::Utf8);
#endif

	while (!in.atEnd()) {
		QString line = in.readLine();

		if (line.isEmpty() || line.startsWith("#") || line.startsWith("*")) {
			continue;
		}

		QStringList splitted = line.split("=");
		QString key, value;

		if (splitted.count() == 2) {
			key = QString(splitted.at(0)).replace(QRegularExpression("\\s*$"), "");
			value = splitted.at(1).trimmed();
			// rimuovo i commenti che possono esserci sulla riga
			value = value.remove(QRegularExpression("#.*"));
			value = value.remove(QRegularExpression("//.*"));
			value = value.remove('"');
			value = value.trimmed();

			map[key] = value;
		}
	}

	return (true);
}
