/*
 * cheatObject.hpp
 *
 *  Created on: 10/mar/2015
 *      Author: fhorse
 */

#ifndef CHEATOBJECT_HPP_
#define CHEATOBJECT_HPP_

#include <QtCore/QMap>
#include <QtXml/QXmlStreamReader>
#include "cheat.h"

class cheatObject : public QObject {
		Q_OBJECT

	public:
		QList<QMap<QString, QString>> cheats;

	private:

	public:
		cheatObject(QObject *parent);
		~cheatObject();
		void read_game_cheats();
		void save_game_cheats();
		void clear_list();
		void apply_cheats();

	private:
		void import_XML(QString fileXML);
		void save_XML(QString fileXML);

		bool gamegenie_decode(QString gamegenie_code, _cheat *cheat);
		bool ram_decode(QMap<QString, QString> ch, _cheat *cheat);

		QMap<QString, QString> parse_xml_cheat(QXmlStreamReader& xml);
		void add_element_data_to_map(QXmlStreamReader& xml, QMap<QString, QString>& map) const;
};


#endif /* CHEATOBJECT_HPP_ */
