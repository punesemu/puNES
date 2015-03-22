/*
 * cheatObject.cpp
 *
 *  Created on: 10/mar/2015
 *      Author: fhorse
 */

#include <QtCore/QtGlobal>
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
#include <QtGui/QMessageBox>
#else
#include <QtWidgets/QMessageBox>
#endif
#include <QtCore/QFileInfo>
#include <QtCore/QTextStream>
#include "cheatObject.moc"
#include "info.h"
#include "conf.h"


#include <QtCore/QDebug>



#define CHEATFILENAME QString(info.base_folder) + QString(CHEAT_FOLDER) + "/" +\
	QFileInfo(info.rom_file).completeBaseName() + ".xml"

#define CHEAT_XML_VERSION "1.0"

cheatObject::cheatObject(QObject *parent = 0) : QObject(parent) {
	clear_list();
}
cheatObject::~cheatObject() {}
void cheatObject::read_game_cheats() {
	clear_list();

	if (info.no_rom) {
		return;
	}

	import_XML(CHEATFILENAME);

	if (cfg->cheat_mode == CHEATSLIST_MODE) {
		apply_cheats();
	}
}
void cheatObject::save_game_cheats() {
	if (info.no_rom) {
		return;
	}

	if ((cheats.count() == 0) && QFile(CHEATFILENAME).exists()) {
		 QFile::remove(CHEATFILENAME);
		 return;
	}

	if (cfg->cheat_mode == CHEATSLIST_MODE) {
		save_XML(CHEATFILENAME);
	}

	clear_list();
}
void cheatObject::clear_list() {
	if (cheats.count() > 0) {
		cheats.clear();
	}
}
void cheatObject::apply_cheats() {
	cheatslist_blank();

	if (cheats.count() == 0) {
		return;
	}

	chl_map cheat;
	_cheat *rom, *ram;

	for (int i = 0; i < cheats.count(); i++) {
		cheat = cheats.at(i);

		rom = &cheats_list.rom.cheat[cheats_list.rom.counter];
		ram = &cheats_list.ram.cheat[cheats_list.ram.counter];

		if (cheat["genie"] != "-") {
			if ((cheat["enabled"].toInt() == 1) && (cheats_list.rom.counter <= CL_CHEATS)
					&& (decode_gg(cheat["genie"], rom) == EXIT_OK)) {
				cheats_list.rom.counter++;
			}
		} else if (cheat["rocky"] != "-") {
			if ((cheat["enabled"].toInt() == 1) && (cheats_list.rom.counter <= CL_CHEATS)
					&& (decode_rocky(cheat["genie"], rom) == EXIT_OK)) {
				cheats_list.rom.counter++;
			}
		} else {
			if ((cheat["enabled"].toInt() == 1) && (cheats_list.ram.counter <= CL_CHEATS)
					&& (decode_ram(cheat, ram) == EXIT_OK)) {
				cheats_list.ram.counter++;
			}
		}
	}
}
bool cheatObject::is_equal(int index, chl_map *find, bool description) {
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
int cheatObject::find_cheat(chl_map *find, bool description) {
	for (int i = 0; i < cheats.count(); i++) {
		if (is_equal(i, find, description)) {
			return (i);
		}
	}

	return (-1);
}
void cheatObject::import_XML(QString file_XML) {
	QFile *file = new QFile(file_XML);

	if (file->open(QIODevice::ReadOnly | QIODevice::Text)) {
		QXmlStreamReader xmlReader(file);

		while (!xmlReader.atEnd() && !xmlReader.hasError()) {
			QXmlStreamReader::TokenType token = xmlReader.readNext();

			if (token == QXmlStreamReader::StartDocument) {
				continue;
			}

			if (token == QXmlStreamReader::StartElement) {
				if (xmlReader.name() == "cheats") {
					continue;
				}
				if (xmlReader.name() == "cheat") {
					chl_map cheat = parse_xml_cheat(xmlReader);

					if ((cheat.count() > 0) && (find_cheat(&cheat, true) == -1)) {
						cheats.append(cheat);
					}
				}
			}
		}
		if (xmlReader.hasError()) {
			QMessageBox::critical(0, "Error on reading the file", xmlReader.errorString(),
			        QMessageBox::Ok);
		}
		xmlReader.clear();

		file->close();
	}
}
void cheatObject::import_CHT(QString file_CHT) {
	QFile *file = new QFile(file_CHT);

	if (file->open(QIODevice::ReadOnly | QIODevice::Text)) {
		QTextStream in(file);

		while (!in.atEnd()) {
			QStringList splitted = in.readLine().split(":");
			chl_map cheat;

			/* 0 */
			{
				int index = 0;

				if (splitted.at(0).at(index) != QLatin1Char('S')) {
					continue;
				} else {
					index++;
				}

				if ((splitted.at(0).length() > index)
						&& (splitted.at(0).at(index) == QLatin1Char('C'))) {
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

			/* 1 */
			cheat.insert("address", QString("0x" + splitted.at(1).toUpper()));

			/* 2 */
			cheat.insert("value", QString("0x" + splitted.at(2).toUpper()));

			/* 3 */
			if (cheat["enabled_compare"] == "0") {
				cheat.insert("compare", "-");
				splitted.insert(4, splitted.at(3));
			} else {
				cheat.insert("compare", QString("0x" + splitted.at(3).toUpper()));
			}

			/* 4 */
			cheat.insert("description", splitted.at(4));

			{
				_cheat ch;

				decode_ram(cheat, &ch);
				cheat.insert("genie", encode_gg(&ch));
			}

			if (find_cheat(&cheat, true) == -1) {
				cheats.append(cheat);
			}
		}

		file->close();
	}
}
void cheatObject::save_XML(QString file_XML) {
	if (cheats.count() == 0) {
		return;
	}

	QFile *file = new QFile(file_XML);

	if (!file->open(QIODevice::WriteOnly)) {
		QMessageBox::warning(0, "Read only", "The file is in read only mode");
	} else {
		QXmlStreamWriter* xmlWriter = new QXmlStreamWriter(file);

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
}
void cheatObject::complete_gg(chl_map *cheat) {
	_cheat ch;

	if (decode_gg((*cheat)["genie"], &ch) == EXIT_ERROR) {
		cheat->clear();
		return;
	}

	cheat->insert("rocky", "-");
	complete_from_code(cheat, &ch);
}
void cheatObject::complete_rocky(chl_map *cheat) {
	_cheat ch;

	if (decode_rocky((*cheat)["rocky"], &ch) == EXIT_ERROR) {
		cheat->clear();
		return;
	}

	cheat->insert("genie", "-");
	complete_from_code(cheat, &ch);
}
void cheatObject::complete_ram(chl_map *cheat) {
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
void cheatObject::complete_from_code(chl_map *cheat, _cheat *ch) {
	cheat->insert("address", QString("0x" + QString("%1").arg(ch->address, 4, 16,
			QChar('0')).toUpper()));
	cheat->insert("value", QString( "0x" + QString("%1").arg(ch->replace, 2, 16,
			QChar('0')).toUpper()));
	cheat->insert("enabled_compare", QString("%1").arg(ch->enabled_compare));
	if (ch->enabled_compare) {
		cheat->insert("compare", QString("0x" + QString("%1").arg(ch->compare, 2, 16,
				QChar('0')).toUpper()));
	} else {
		cheat->insert("compare", "-");
	}
}
bool cheatObject::decode_gg(QString code, _cheat *cheat) {
	BYTE codes[8];
	int length = code.length();

	if (code.isEmpty() || ((length != 6) && (length != 8))) {
		return (EXIT_ERROR);
	}

	for (int i = 0; i < length; ++i) {
		switch (code[i].toLower().toLatin1()) {
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
			length = 6;
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

	if (length == 8) {
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

	cheat->disabled = 0x00;

	return (EXIT_OK);
}
bool cheatObject::decode_rocky(QString code, _cheat *cheat) {
	DBWORD input = 0, output = 0, key;
	static const BYTE rocky_table[] = {
		 3, 13, 14,  1,  6,  9,  5,  0,
		12,  7,  2,  8, 10, 11,  4, 19,
		21, 23, 22, 20, 17, 16, 18, 29,
		31, 24, 26, 25, 30, 27, 28
	};

	if (code.isEmpty()) {
		return (EXIT_ERROR);
	}

	for (int i = 0; i < 8; ++i) {
		int character = (int) code[i ^ 7].toLatin1();
		DBWORD num;

		if (character >= '0' && character <= '9') {
			num = character - '0';
		} else if (character >= 'A' && character <= 'F') {
			num = character - 'A' + 0x0A;
		} else if (character >= 'a' && character <= 'f') {
			num = character - 'a' + 0x0A;
		} else {
			return (EXIT_ERROR);
		}
		input |= num << (i * 4);
	}

	key = 0xFCBDD274;

	for (DBWORD i = 31; i--; input <<= 1) {
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
bool cheatObject::decode_ram(chl_map ch, _cheat *cheat) {
	bool ok;

	cheat->address = ch["address"].toInt(&ok, 16);
	cheat->replace = ch["value"].toInt(&ok, 16);
	cheat->enabled_compare = ch["enabled_compare"].toInt();
	if (ch["compare"] != "-") {
		cheat->compare = ch["compare"].toInt(&ok, 16);
	}

	return (EXIT_OK);
}
QString cheatObject::encode_gg(_cheat *cheat) {
	QString gg;

	if (cheat->address < 0x8000) {
		return ("-");
	}

	int i = (cheat->enabled_compare ? 8 : 6);
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
		static const QChar table[] = {
			'A', 'P', 'Z', 'L',
			'G', 'I', 'T', 'Y',
			'E', 'O', 'X', 'U',
			'K', 'S', 'V', 'N'
		};
		gg.append(table[codes[a]]);
	};

	return (gg);
}
chl_map cheatObject::parse_xml_cheat(QXmlStreamReader &xml) {
	chl_map cheat;

	if (xml.tokenType() != QXmlStreamReader::StartElement && xml.name() == "cheat") {
		return (cheat);
	}

	QXmlStreamAttributes attributes = xml.attributes();

	if (attributes.hasAttribute("enabled")) {
		cheat["enabled"] = attributes.value("enabled").toString();
	}

	xml.readNext();

	while (!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "cheat")) {
		if (xml.tokenType() == QXmlStreamReader::StartElement) {
			if (xml.name() == "genie") {
				add_element_data_to_map(xml, cheat);
			}
			if (xml.name() == "rocky") {
				add_element_data_to_map(xml, cheat);
			}
			if (xml.name() == "description") {
				add_element_data_to_map(xml, cheat);
			}
			if (xml.name() == "address") {
				add_element_data_to_map(xml, cheat);
			}
			if (xml.name() == "value") {
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
void cheatObject::add_element_data_to_map(QXmlStreamReader &xml, chl_map &map) const {
	if (xml.tokenType() != QXmlStreamReader::StartElement) {
		return;
	}
	QString elementName = xml.name().toString();
	xml.readNext();
	if (xml.tokenType() != QXmlStreamReader::Characters) {
		return;
	}
	map.insert(elementName, xml.text().toString());
}
