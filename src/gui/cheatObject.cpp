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
#include "cheatObject.moc"
#include "info.h"
#include "conf.h"

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
	if (info.no_rom || (cheats.count() == 0)) {
		return;
	}

	if (cfg->cheat_mode == CHEATSLIST_MODE) {
		save_XML(CHEATFILENAME);
	}

	clear_list();
}
void cheatObject::clear_list() {
	cheats.clear();
}
void cheatObject::apply_cheats() {
	cheatslist_blank();

	if (cheats.count() == 0) {
		return;
	}

	for (int i = 0; i < cheats.count(); i++) {
		QMap<QString, QString> cheat = cheats.at(i);
		_cheat *gg = &cheats_list.gg.cheat[cheats_list.gg.counter];
		_cheat *ram = &cheats_list.ram.cheat[cheats_list.ram.counter];

		if (cheat["genie"] != "-") {
			if ((cheat["enabled"].toInt() == 1) && (cheats_list.gg.counter <= CL_CHEATS)
					&& (gamegenie_decode(cheat["genie"], gg) == EXIT_OK)) {
				cheats_list.gg.counter++;
			}
		} else {
			if ((cheat["enabled"].toInt() == 1) && (cheats_list.ram.counter <= CL_CHEATS)
					&& (ram_decode(cheat, ram) == EXIT_OK)) {
				cheats_list.ram.counter++;
			}
		}
	}

	/*
	for (int i = 0; i < cheats_list.gg.counter; i++) {
		_cheat *gg = &cheats_list.gg.cheat[i];

		printf("gg  : 0x%04X, 0x%02X\n", gg->address, gg->replace);
	}

	for (int i = 0; i < cheats_list.ram.counter; i++) {
		_cheat *ram = &cheats_list.ram.cheat[i];

		printf("ram : 0x%04X, 0x%02X\n", ram->address, ram->replace);
	}
	*/

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
					cheats.append(parse_xml_cheat(xmlReader));
				}
			}
		}
		if (xmlReader.hasError()) {
			QMessageBox::critical(0, "Error on reading the file", xmlReader.errorString(),
			        QMessageBox::Ok);
		}
		xmlReader.clear();
	}

	file->close();
	delete (file);

	for (int i = 0; i < cheats.count(); i++) {
		QMap<QString, QString> cheat = cheats.at(i);
		_cheat ch;

		if (cheat.contains("genie")) {
			gamegenie_decode(cheat["genie"], &ch);
			cheat.insert("address",
					QString("0x" + QString("%1").arg(ch.address, 4, 16, QChar('0')).toUpper()));
			cheat.insert("value",
					QString("0x" + QString("%1").arg(ch.replace, 2, 16, QChar('0')).toUpper()));
			cheat.insert("enabled_compare", QString(QString("%1").arg(ch.enabled_compare)));
			if (ch.enabled_compare) {
				cheat.insert("compare",
						QString("0x" + QString("%1").arg(ch.compare, 2, 16, QChar('0')).toUpper()));
			} else {
				cheat.insert("compare", "-");
			}
			cheats.replace(i, cheat);
		} else {
			cheat.insert("genie", "-");
			cheat.insert("compare", "-");
			cheats.replace(i, cheat);
		}
	}
}
void cheatObject::save_XML(QString file_XML) {
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
			QMap<QString, QString> cheat = cheats.at(i);

			xmlWriter->writeStartElement("cheat");
			xmlWriter->writeAttribute("enabled", cheat["enabled"]);

			if (cheat["genie"] != "-") {
				xmlWriter->writeStartElement("genie");
				xmlWriter->writeCharacters(cheat["genie"]);
				xmlWriter->writeEndElement();
			}
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
	}
	delete (file);
}
bool cheatObject::gamegenie_decode(QString gamegenie_code, _cheat *cheat) {
	BYTE codes[8];
	int length = gamegenie_code.length();

	if (gamegenie_code.isEmpty() || ((length != 6) && (length != 8))) {
		return (EXIT_ERROR);
	}

	for (int i = 0; i < length; ++i) {
		switch (gamegenie_code[i].toLower().toLatin1()) {
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
			((codes[4] & 0x01)       | (codes[4] & 0x02)       |
			 (codes[4] & 0x04)       | (codes[3] & 0x08)       |
			 (codes[2] & 0x01) << 4  | (codes[2] & 0x02) << 4  |
			 (codes[2] & 0x04) << 4  | (codes[1] & 0x08) << 4  |
			 (codes[5] & 0x01) << 8  | (codes[5] & 0x02) << 8  |
			 (codes[5] & 0x04) << 8  | (codes[4] & 0x08) << 8  |
			 (codes[3] & 0x01) << 12 | (codes[3] & 0x02) << 12 |
			 (codes[3] & 0x04) << 12);

	cheat->replace = (
			(codes[0] & 0x01)      | (codes[0] & 0x02)      | (codes[0] & 0x04)      |
			(codes[1] & 0x01) << 4 | (codes[1] & 0x02) << 4 | (codes[1] & 0x04) << 4 |
			(codes[0] & 0x08) << 4);

	if (length == 8) {
		cheat->enabled_compare = TRUE;
		cheat->replace |= codes[7] & 0x08;
		cheat->compare = (
				(codes[6] & 0x01)      | (codes[6] & 0x02)      |
				(codes[6] & 0x04)      | (codes[5] & 0x08)      |
				(codes[7] & 0x01) << 4 | (codes[7] & 0x02) << 4 |
				(codes[7] & 0x04) << 4 | (codes[6] & 0x08) << 4);
	} else {
		cheat->enabled_compare = FALSE;
		cheat->replace |= codes[5] & 0x08;
		cheat->compare = 0x00;
	}

	cheat->disabled = 0x00;

	return (EXIT_OK);
}
/*
bool dlgCheats::gamegenie_encode(QString *gamegenie_code, _cheat *cheat) {
	if (code.address < 0x8000)
		return RESULT_ERR_INVALID_PARAM;

	const byte codes[8] = { (code.value >> 0 & 0x7U) | (code.value >> 4 & 0x8U), (code.value >> 4
	        & 0x7U) | (code.address >> 4 & 0x8U), (code.address >> 4 & 0x7U)
	        | (code.useCompare ? 0x8U : 0x0U), (code.address >> 12 & 0x7U)
	        | (code.address >> 0 & 0x8U), (code.address >> 0 & 0x7U) | (code.address >> 8 & 0x8U),
	    (code.address >> 8 & 0x7U) | ((code.useCompare ? code.compare : code.value) & 0x8U), (
	            code.useCompare ? ((code.compare >> 0 & 0x7U) | (code.compare >> 4 & 0x8U)) : 0), (
	            code.useCompare ? ((code.compare >> 4 & 0x7U) | (code.value >> 0 & 0x8U)) : 0) };

	uint i = (code.useCompare ? 8 : 6);

	characters[i--] = '\0';

	do {
		static const char lut[] = { 'A', 'P', 'Z', 'L', 'G', 'I', 'T', 'Y', 'E', 'O', 'X', 'U', 'K',
		    'S', 'V', 'N' };

		characters[i] = lut[codes[i]];
	} while (i--);

	return RESULT_OK;
}
*/
bool cheatObject::ram_decode(QMap<QString, QString> ch, _cheat *cheat) {
	bool ok;

	cheat->address = ch["address"].toInt(&ok, 16);
	cheat->replace = ch["value"].toInt(&ok, 16);

	return (EXIT_OK);
}
QMap<QString, QString> cheatObject::parse_xml_cheat(QXmlStreamReader& xml) {
	QMap<QString, QString> cheat;

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
	return (cheat);
}
void cheatObject::add_element_data_to_map(QXmlStreamReader& xml,
		QMap<QString, QString>& map) const {
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
