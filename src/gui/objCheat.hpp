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

#ifndef OBJCHEAT_HPP_
#define OBJCHEAT_HPP_

#include <QtCore/QXmlStreamReader>
#include "cheat.h"

typedef QMap<QString, QString> chl_map;
typedef QList<chl_map> chl_list;

class objCheat : public QObject {
	Q_OBJECT

	public:
		chl_list cheats;

	public:
		objCheat(QObject *parent = 0);
		~objCheat();

	public:
		void read_game_cheats(QWidget *parent);
		void save_game_cheats(QWidget *parent);
		void clear_list(void);
		void apply_cheats(void);
		bool is_equal(int index, chl_map *find, bool dscription);
		int find_cheat(chl_map *find, bool description);

		void complete_ram(chl_map *cheat);
		bool decode_ram(chl_map ch, _cheat *cheat);

		void complete_gg(chl_map *cheat);
		QString encode_gg(_cheat *cheat);
		bool decode_gg(QString code, _cheat *cheat);

		void complete_rocky(chl_map *cheat);
		bool decode_rocky(QString code, _cheat *cheat);
		QString encode_rocky(_cheat *cheat);

	public:
		void import_Nestopia_xml(QWidget *parent, QString path);
		void import_MAME_xml(QWidget *parent, QString path);
		void import_FCEUX_cht(QString path);
		void import_libretro_cht(QString path);

	public:
		void save_Nestopia_xml(QWidget *parent, QString path);

	private:
		chl_map parse_nestopia_cheat(QXmlStreamReader &xml);
		QList<chl_map> parse_mame_cheat(QXmlStreamReader &xml);
		chl_map parse_fceux_cheat(QString line);

		void complete_from_code(chl_map *cheat, _cheat *ch);
		void ram_to_gg(chl_map *cheat);
		void add_element_data_to_map(QString element_name, QString text, chl_map &map) const;
		void add_element_data_to_map(QXmlStreamReader &xml, chl_map &map) const;
};

#endif /* OBJCHEAT_HPP_ */
