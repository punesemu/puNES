/*
 * cheatObject.hpp
 *
 *  Created on: 10/mar/2015
 *      Author: fhorse
 */

#ifndef CHEATOBJECT_HPP_
#define CHEATOBJECT_HPP_

#include <QtCore/QMap>
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
#include <QtXml/QXmlStreamReader>
#else
#include <QtCore/QXmlStreamReader>
#endif
#include "cheat.h"

typedef QMap<QString, QString> chl_map;
typedef QList<chl_map> chl_list;

class cheatObject : public QObject {
		Q_OBJECT

	public:
		chl_list cheats;

	public:
		cheatObject(QObject *parent);
		~cheatObject();
		void read_game_cheats();
		void save_game_cheats();
		void clear_list();
		void apply_cheats();
		bool is_equal(int index, chl_map *find, bool dscription);
		int find_cheat(chl_map *find, bool description);
		void import_XML(QString file_XML);
		void import_CHT(QString file_CHT);
		void save_XML(QString file_XML);
		void complete_gg(chl_map *cheat);
		void complete_rocky(chl_map *cheat);
		void complete_ram(chl_map *cheat);

	private:
		void complete_from_code(chl_map *cheat, _cheat *ch);
		bool decode_gg(QString code, _cheat *cheat);
		bool decode_rocky(QString code, _cheat *cheat);
		bool decode_ram(chl_map ch, _cheat *cheat);
		QString encode_gg(_cheat *cheat);

		chl_map parse_xml_cheat(QXmlStreamReader &xml);
		void add_element_data_to_map(QXmlStreamReader &xml, chl_map &map) const;
};

#endif /* CHEATOBJECT_HPP_ */
