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
#include <QtCore/QMap>
#include <QtCore/QFile>
#include <QtCore/QXmlStreamReader>
#include <QtCore/QStandardPaths>
#include "nes20db.h"
#include "version.h"
#include "gui.h"
#include "info.h"
#include "memmap.h"
#include "vs_system.h"

#define NES20DBFILENAME "nes20db.xml"

typedef QMap<QString, QString> game_map;

bool search_in_xml(QFile &file);
game_map parse_game(QXmlStreamReader &xml);
bool is_game_element(QXmlStreamReader &xml);
void add_element_data_to_map(const QString &element_name, const QString &text, game_map &map);
void add_element_data_to_map(QXmlStreamReader &xml, game_map &map);
void populate_game_info(_nes20db &game_info, game_map &map);
uint32_t to_uint(const QString &svalue, int base, int def = 0);
uint32_t to_mirroring(const QString &svalue);

_nes20db nes20db;

void nes20db_reset(void) {
	::memset((void *)&nes20db, 0x00, sizeof(_nes20db));
}
BYTE nes20db_search(void) {
	QStringList gdl = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
	QStringList data_locations;
	const QString gdf = uQString(gui_extract_base(gui_data_folder()));
	const QString gaf = uQString(gui_application_folder());
	bool finded = false;

	nes20db_reset();

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

	info.mapper.nes20db.in_use = FALSE;

	foreach(const QString path, data_locations) {
		QFile file(QString("%0/%1").arg(path, NES20DBFILENAME));

		if (file.exists()) {
			if (search_in_xml(file)) {
				finded = true;
			}
		}
	}
	return (finded);
}

bool search_in_xml(QFile &file) {
	bool finded = false;

	// Just "rom" is the entirety of the ROM file without header, including PRG-, CHR- and Misc ROM. It's what you look
	// up when being confronted with a headerless ROM file (see note 2).
	// "prgram","prgnvram","chrram","chrnvram","chrrom","miscrom" tags are only present if the respective memory type
	// is present.
	// "sum16" is a simple sum of all bytes in a particular ROM type, truncated to 16 bits; this value can be found in
	// Nintendo's leaked internal spreadsheet.
	// "console type", "console region", "expansion type", "vs hardware" and "vs ppu" are just the direct values defined
	// for the NES 2.0 header; I did not want to use text strings that have to be looked up.
	// "mirroring" can be:
	//	For normal mappers:
	// "H", meaning that byte 6's LSB nibble is set to 0..0
	// "V", meaning that byte 6's LSB nibble is set to 0..1
	// "4", meaning that byte 6's LSB nibble is set to 1..0
	// For mapper 030's idiosyncratic use of the NES header's mirroring bits:
	// "H", meaning that byte 6's LSB nibble is set to 0..0
	// "V", meaning that byte 6's LSB nibble is set to 0..1
	// "1", meaning that byte 6's LSB nibble is set to 1..0
	// "4", meaning that byte 6's LSB nibble is set to 1..1
	// For mapper 218's idiosyncratic use of the NES header's mirroring bits:
	// "H", meaning that byte 6's LSB nibble is set to 0..0
	// "V", meaning that byte 6's LSB nibble is set to 0..1
	// "0", meaning that byte 6's LSB nibble is set to 1..0
	// "1", meaning that byte 6's LSB nibble is set to 1..1
	//
	// Notes:
	//
	// The database should be complete for all dumped licensed and original unlicensed games, and will be continuously
	// updated to include more homebrew games/hacks/translations, pirate dumps and plug-and-play consoles, as well as
	// corrections. As it is created automatically from a master set of headered ROM files, I cannot process pull requests
	// on the XML text data; instead, simply point out errors or omissions by posting replies to this thread.
	// When comparing file hashes against the database's full "rom" hash, be sure to compute two hashes:
	// one that trusts and uses an existing header's ROM size fields to know how many bytes to hash,
	// one that ignores any existing header completely and just hashes the entire file sans header.
	// Searching for the first hash value will fail if an otherwise complete ROM has a header with incorrect size fields.
	// Searching for the second hash value will fail if the file has common trailing garbage at the end,
	// such as "This file downloaded from website x". Searching for both hashes will maximize the chances of identifying
	// ROM files with bad headers or trailing garbage bytes. ROM files that have simultaneously trailing garbage and
	// incorrect sizes specified in the header, or no header at all and trailing garbage, cannot be identified without
	// human intervention. ;)

	if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		QXmlStreamReader xmlReader(&file);

		while (!xmlReader.atEnd() && !xmlReader.hasError()) {
			const QXmlStreamReader::TokenType token = xmlReader.readNext();

			if (token == QXmlStreamReader::StartDocument) {
				continue;
			}

			if (token == QXmlStreamReader::StartElement) {
				if (!xmlReader.name().toString().compare("nes20db", Qt::CaseInsensitive)) {
					continue;
				}
				if (!xmlReader.name().toString().compare("game", Qt::CaseInsensitive)) {
					game_map game = parse_game(xmlReader);

					if (game.count() > 0) {
						populate_game_info(nes20db, game);
						// Ho deciso di usare solo il crc32 total (tralasciando quello della prgrom perche' ci sono
						// roms con lo stesso crc32 ma usano mapper diverse :
						// Esempio :
						//    Angry Birds Week(byCasperdj777).nes (0x2A629F7D mapper 185) e
						//    Compatibility Hacks\Bird Week [m003].nes (0x2A629F7D mapper 003)
						if (nes20db.rom.crc32 == info.crc32.total) {
							//const QString comment = game["comment"];
							const BYTE old_format = info.format;

							info.mapper.nes20db.in_use = TRUE;

							info.format = NES_2_0;

							// visto che con il NES_2_0 non eseguo la ricerca nel
							// database inizializzo queste variabili.
							info.mirroring_db = DEFAULT;

							info.mapper.id = nes20db.pcb.mapper;
							info.mapper.submapper_nes20 = nes20db.pcb.submapper;
							info.mapper.submapper = info.mapper.submapper_nes20;

							wram_set_ram_size(nes20db.prgram.size ? nes20db.prgram.size : 0);
							wram_set_nvram_size(nes20db.prgnvram.size ? nes20db.prgnvram.size : 0);

							vram_set_ram_size(0, nes20db.chrram.size ? nes20db.chrram.size : 0);
							vram_set_nvram_size(0, nes20db.chrnvram.size ? nes20db.chrnvram.size : 0);

							info.mapper.battery = nes20db.pcb.battery;
							info.mapper.expansion = nes20db.expansion.type;

							if (old_format != UNIF_FORMAT) {
								info.mapper.prgrom_size = nes20db.prgrom.size;
								info.mapper.prgrom_banks_16k = (nes20db.prgrom.size / S16K) +
									((nes20db.prgrom.size % S16K) ? 1 : 0);
								info.mapper.chrrom_size = nes20db.chrrom.size;
								info.mapper.chrrom_banks_8k = (nes20db.chrrom.size / S8K) +
									((nes20db.chrrom.size % S8K) ? 1 : 0);
							}

							// There is no such thing as an "initial mirroring" in the NES header.
							// "Initial mirroring" is a peculiar behavior of FCEUX that was carried over from
							// the old Nesticle emulator from a time when mapper 4 did not distinguish between
							// actual MMC3 (with software-selectable mirroring) and
							// Namco 108 (with hard-wired mirroring, now assigned to mapper 206).
							// For mappers with software-selectable mirroring such as mapper 45, the
							// mirroring bit of the NES header has no function. It does not specify
							// an "initial mirroring", and must be ignored. And as the NES 2.0 Wiki
							// article states, "Bit 0 is normally relevant only if the mapper does not
							// allow the mirroring type to be switched. It should be set to zero otherwise.",
							// which is the reason it is set to "Horizontal" (i.e. zero) in nes20db.
							// Although Nintendo's original MMC3 has the power-on register state
							// undefined (and thus potentially random), mapper 45's MMC3 clone,
							// based on all hardware tests, seems to power on with all registers cleared
							// to $00, which in the case of $A000 means Vertical mirroring.
							// Of course, since this is a homebrew ROM file that likely was never
							// tested on real hardware, actual hardware behavior is beside the point.
							if (nes20db.pcb.mirroring != MIRRORING_HORIZONTAL) {
								info.mapper.mirroring = nes20db.pcb.mirroring;
							}

							switch (nes20db.console.type) {
								case 0:
								case 1:
								case 2:
									info.decimal_mode = FALSE;
									info.mapper.ext_console_type = nes20db.console.type;
									vs_system.ppu = nes20db.vs.ppu;
									vs_system.special_mode.type = nes20db.vs.hardware;
									break;
								case 3:
									// usato per esempio da :
									// Othello (rev0).nes
									info.decimal_mode = TRUE;
									break;
								default:
									info.mapper.ext_console_type = nes20db.console.type;
									info.decimal_mode = FALSE;
									break;
							}

							switch (nes20db.console.region) {
								default:
								case 0:
								case 2:
									info.machine[DATABASE] = NTSC;
									break;
								case 1:
									info.machine[DATABASE] = PAL;
									break;
								case 3:
									info.machine[DATABASE] = DENDY;
									break;
							}
							finded = true;
							break;
						}
					}
				}
			}
		}
		if (xmlReader.hasError()) {
			//QMessageBox::critical(parent, tr("Error on reading the file"), xmlReader.errorString(), QMessageBox::Ok);
		}
		xmlReader.clear();
		file.close();
	}
	return (finded);
}
game_map parse_game(QXmlStreamReader &xml) {
	game_map game;

	if ((xml.tokenType() != QXmlStreamReader::StartElement) && is_game_element(xml)) {
		return (game);
	}

	xml.readNext();

	while (!((xml.tokenType() == QXmlStreamReader::EndElement) && is_game_element(xml))) {
		if (xml.tokenType() == QXmlStreamReader::Comment) {
			add_element_data_to_map("comment", xml.text().toString(), game);
		} else if (xml.tokenType() == QXmlStreamReader::StartElement) {
			if (!xml.name().toString().compare("prgrom", Qt::CaseInsensitive)) {
				add_element_data_to_map(xml, game);
			}
			if (!xml.name().toString().compare("chrrom", Qt::CaseInsensitive)) {
				add_element_data_to_map(xml, game);
			}
			if (!xml.name().toString().compare("trainer", Qt::CaseInsensitive)) {
				add_element_data_to_map(xml, game);
			}
			if (!xml.name().toString().compare("miscrom", Qt::CaseInsensitive)) {
				add_element_data_to_map(xml, game);
			}
			if (!xml.name().toString().compare("rom", Qt::CaseInsensitive)) {
				add_element_data_to_map(xml, game);
			}

			if (!xml.name().toString().compare("prgram", Qt::CaseInsensitive)) {
				add_element_data_to_map(xml, game);
			}
			if (!xml.name().toString().compare("prgnvram", Qt::CaseInsensitive)) {
				add_element_data_to_map(xml, game);
			}
			if (!xml.name().toString().compare("chrram", Qt::CaseInsensitive)) {
				add_element_data_to_map(xml, game);
			}
			if (!xml.name().toString().compare("chrnvram", Qt::CaseInsensitive)) {
				add_element_data_to_map(xml, game);
			}

			if (!xml.name().toString().compare("pcb", Qt::CaseInsensitive)) {
				add_element_data_to_map(xml, game);
			}
			if (!xml.name().toString().compare("console", Qt::CaseInsensitive)) {
				add_element_data_to_map(xml, game);
			}
			if (!xml.name().toString().compare("vs", Qt::CaseInsensitive)) {
				add_element_data_to_map(xml, game);
			}
			if (!xml.name().toString().compare("expansion", Qt::CaseInsensitive)) {
				add_element_data_to_map(xml, game);
			}
		}
		xml.readNext();
	}

	if (!game.contains("prgrom_size") ||
		!game.contains("prgrom_crc32") ||
		!game.contains("rom_size") ||
		!game.contains("rom_crc32") ||
		!game.contains("pcb_mapper")) {
		game.clear();
	}

	return (game);
}
bool is_game_element(QXmlStreamReader &xml) {
	return (!xml.name().toString().compare("game", Qt::CaseInsensitive));
}
void add_element_data_to_map(const QString &element_name, const QString &text, game_map &map) {
	if (text.isNull() || text.isEmpty() || !text.compare("-")) {
		return;
	}
	map.insert(element_name.toLower(), text);
}
void add_element_data_to_map(QXmlStreamReader &xml, game_map &map) {
	QString element_name;
	QXmlStreamAttributes attribs;

	if (xml.tokenType() != QXmlStreamReader::StartElement) {
		return;
	}

	element_name = xml.name().toString();
	attribs = xml.attributes();

	xml.readNext();

	if (xml.tokenType() != QXmlStreamReader::EndElement) {
		return;
	}

	for (const QXmlStreamAttribute &a : attribs) {
		add_element_data_to_map(QString("%0_%1").arg(element_name, a.name().toString()),
			a.value().toString(), map);
	}
}
void populate_game_info(_nes20db &game_info, game_map &map) {
	game_info.prgrom.size = to_uint(map["prgrom_size"], 10);
	game_info.prgrom.crc32 = to_uint(map["prgrom_crc32"], 16);
	game_info.chrrom.size = to_uint(map["chrrom_size"], 10);
	game_info.chrrom.crc32 = to_uint(map["chrrom_crc32"], 16);
	game_info.trainer.size = to_uint(map["trainer_size"], 10);
	game_info.trainer.crc32 = to_uint(map["trainer_crc32"], 16);
	game_info.miscrom.size = to_uint(map["miscrom_size"], 10);
	game_info.miscrom.crc32 = to_uint(map["miscrom_crc32"], 16);
	game_info.rom.size = to_uint(map["rom_size"], 10);
	game_info.rom.crc32 = to_uint(map["rom_crc32"], 16);
	game_info.prgram.size = to_uint(map["prgram_size"], 10);
	game_info.prgnvram.size = to_uint(map["prgnvram_size"], 10);
	game_info.chrram.size = to_uint(map["chrram_size"], 10);
	game_info.chrnvram.size = to_uint(map["chrnvram_size"], 10);
	game_info.pcb.mapper = to_uint(map["pcb_mapper"], 10);
	game_info.pcb.submapper = to_uint(map["pcb_submapper"], 10);
	game_info.pcb.mirroring = to_mirroring(map["pcb_mirroring"]);
	game_info.pcb.battery = to_uint(map["pcb_battery"], 10);
	game_info.console.type = to_uint(map["console_type"], 10);
	game_info.console.region = to_uint(map["console_region"], 10);
	game_info.vs.hardware = to_uint(map["vs_hardware"], 10);
	game_info.vs.ppu = to_uint(map["vs_ppu"], 10);
	game_info.expansion.type = to_uint(map["expansion_type"], 10);
}
uint32_t to_uint(const QString &svalue, int base, int def) {
	uint32_t value = 0;
	bool ok = false;

	value = svalue.toUInt(&ok, base);
	return (ok ? value : def);
}
uint32_t to_mirroring(const QString &svalue) {
	// "H", meaning that byte 6's LSB nibble is set to 0..0
	// "V", meaning that byte 6's LSB nibble is set to 0..1
	// "4", meaning that byte 6's LSB nibble is set to 1..0
	// For mapper 030's idiosyncratic use of the NES header's mirroring bits:
	// "H", meaning that byte 6's LSB nibble is set to 0..0
	// "V", meaning that byte 6's LSB nibble is set to 0..1
	// "1", meaning that byte 6's LSB nibble is set to 1..0
	// "4", meaning that byte 6's LSB nibble is set to 1..1
	// For mapper 218's idiosyncratic use of the NES header's mirroring bits:
	// "H", meaning that byte 6's LSB nibble is set to 0..0
	// "V", meaning that byte 6's LSB nibble is set to 0..1
	// "0", meaning that byte 6's LSB nibble is set to 1..0
	// "1", meaning that byte 6's LSB nibble is set to 1..1
	if (!svalue.compare("H", Qt::CaseInsensitive)) {
		return (MIRRORING_HORIZONTAL);
	}
	if (!svalue.compare("V", Qt::CaseInsensitive)) {
		return (MIRRORING_VERTICAL);
	}
	if (!svalue.compare("4", Qt::CaseInsensitive)) {
		return (MIRRORING_FOURSCR);
	}
	if (!svalue.compare("0", Qt::CaseInsensitive)) {
		return (MIRRORING_SINGLE_SCR0);
	}
	if (!svalue.compare("1", Qt::CaseInsensitive)) {
		return (MIRRORING_SINGLE_SCR1);
	}
	return (MIRRORING_HORIZONTAL);
}
