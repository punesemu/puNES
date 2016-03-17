/*
 *  Copyright (C) 2010-2016 Fabio Cavallo (aka FHorse)
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

#include <QtCore/QSettings>
#include <QtCore/QStringList>
#include <QtCore/QTextStream>
#include "cgp.h"
#include "shaders.h"

void cgp_pragma_param(const char *code) {
	QTextStream stream(code);
	QString line;
	_param_shd param;


	do {
		line = stream.readLine();

		memset (&param, 0x00, sizeof(_param_shd));

		if (line.startsWith("#pragma parameter")) {
			QRegExp rx("\\d*\\.\\d+");
			int i, count = 0, pos = 0;
			bool finded;

			// sscanf non e' locale indipendente percio' lo utilizzo solo per
			// ricavare nome e descrizione del parametro.
			count = sscanf(line.toUtf8().constData(), "#pragma parameter %63s \"%63[^\"]\" ",
					param.name, param.desc);

			if (count < 2) {
				continue;
			}

			while ((pos = rx.indexIn(line, pos)) != -1) {
				switch (count++) {
					case 2:
						param.initial = rx.cap(0).toFloat();
						break;
					case 3:
						param.min = rx.cap(0).toFloat();
						break;
					case 4:
						param.max = rx.cap(0).toFloat();
						break;
					case 5:
						param.step = rx.cap(0).toFloat();
						break;
				}
				pos += rx.matchedLength();
			}

			if (count < 5) {
				continue;
			}

			param.value = param.initial;

			finded = false;

			for (i = 0; i < shader_effect.params; i++) {
				_param_shd *ctrl = &shader_effect.param[i];

				if (::strncmp(param.name, ctrl->name, sizeof(param.name)) == 0) {
					finded = true;
					break;
				}
			}

			if (finded == false) {
				if (shader_effect.params < MAX_PARAM) {
					::memcpy(&shader_effect.param[shader_effect.params], &param,
							sizeof(_param_shd));
					shader_effect.params++;
				}
			}
		}
	} while (!line.isNull());
}
