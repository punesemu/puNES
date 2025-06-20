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

#ifndef OBJCHEAT_HPP_
#define OBJCHEAT_HPP_

#include <QtCore/QXmlStreamReader>
#include "cheat.h"

typedef QMap<QString, QString> chl_map;
typedef QList<chl_map> chl_list;

class objCheat final : public QObject {
	Q_OBJECT

	public:
		chl_list cheats;

	public:
		explicit objCheat(QObject *parent = nullptr);
		~objCheat() override;

	public:
		void read_game_cheats(QWidget *parent);
		void save_game_cheats(QWidget *parent) const;
		void clear_list(void);
		void apply_cheats(void) const;
		bool is_equal(int index, chl_map *find, bool dscription) const;
		int find_cheat(chl_map *find, bool description) const;

		static bool decode_ram(chl_map ch, _cheat *cheat);
		static void complete_ram(chl_map *cheat);

		static bool decode_gg(const QString &code, _cheat *cheat);
		static QString encode_gg(const _cheat *cheat);
		static void complete_gg(chl_map *cheat);

		static bool decode_rocky(const QString &code, _cheat *cheat);
		static QString encode_rocky(const _cheat *cheat);
		static void complete_rocky(chl_map *cheat);

	public:
		void import_Nestopia_xml(QWidget *parent, const QString &path);
		void import_MAME_xml(QWidget *parent, const QString &path);
		void import_FCEUX_cht(const QString &path);
		void import_libretro_cht(const QString &path);

	public:
		void save_Nestopia_xml(QWidget *parent, const QString &path) const;

	private:
		static chl_map parse_nestopia_cheat(QXmlStreamReader &xml);
		static QList<chl_map> parse_mame_cheat(QXmlStreamReader &xml);
		static chl_map parse_fceux_cheat(const QString &line);

		static void complete_from_code(chl_map *cheat, const _cheat *ch);
		static void ram_to_gg(chl_map *cheat);
		static void add_element_data_to_map(const QString &element_name, const QString &text, chl_map &map);
		static void add_element_data_to_map(QXmlStreamReader &xml, chl_map &map);
};

#endif /* OBJCHEAT_HPP_ */
