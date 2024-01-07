/*
 *  Copyright (C) 2010-2024 Fabio Cavallo (aka FHorse)
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
#include <QtCore/QFileInfo>
#include <QtCore/QRegularExpression>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QtCore/QStringEncoder>
#endif
#include "mainWindow.hpp"
#include "cgp.h"
#include "shaders.h"

static bool cgp_value(QSettings *set, const QString &key, QString &value);
static bool cgp_rd_file(QIODevice &device, QSettings::SettingsMap &map);

BYTE cgp_parse(const uTCHAR *file) {
	static const QSettings::Format cfg = QSettings::registerFormat("cgp", cgp_rd_file, nullptr);
	QSettings *set;
	QFileInfo fi(uQString(file));
	QString key, value;
	QStringList list;
	_shader_effect se;
	bool finded;
	uint i;

	shader_se_set_default(&se);

	if (QString::compare(fi.suffix(), "cgp", Qt::CaseInsensitive) == 0) {
#if defined (WITH_OPENGL_CG) || defined (WITH_D3D9)
		se.type = MS_CGP;
# else
		log_error(uL("cgp;shader format non supported"));
		return (EXIT_ERROR);
#endif
	} else if (QString::compare(fi.suffix(), "glslp", Qt::CaseInsensitive) == 0) {
#if defined (WITH_OPENGL)
		se.type = MS_GLSLP;
# else
		log_error(uL("cgp;shader format non supported"));
		return (EXIT_ERROR);
#endif
	} else {
		return (EXIT_ERROR);
	}

	set = new QSettings(fi.canonicalFilePath(), cfg);

	// shaders
	if (set->allKeys().contains("shaders", Qt::CaseInsensitive)) {
		se.pass = set->value("shaders").toUInt();
	}
	if (se.pass > MAX_PASS) {
		se.pass = MAX_PASS;
	}

	// pass
	for (i = 0; i < se.pass; i++) {
		_shader_pass *sp = &se.sp[i];

		sp->type = se.type;

		// shader
		key = QString("shader%1").arg(i);
		if (!cgp_value(set, key, value)) {
			value.replace('\\', '/');
			ustrncpy(sp->path, uQStringCD(QFileInfo(fi.absolutePath() + '/' + value).absoluteFilePath()), usizeof(sp->path) - 1);
		} else {
			delete (set);
			return (EXIT_ERROR);
		}

		// alias
		key = QString("alias%1").arg(i);
		if (!cgp_value(set, key, value)) {
			::strncpy(sp->alias, qPrintable(value), sizeof(sp->alias) - 1);
		}

		// mipmap_input
		key = QString("mipmap_input%1").arg(i);
		if (!cgp_value(set, key, value)) {
			if (QString::compare(value, "true", Qt::CaseInsensitive) == 0) {
				sp->mipmap_input = TRUE;
			}
		}

		// filter_linear
		key = QString("filter_linear%1").arg(i);
		if (!cgp_value(set, key, value)) {
			if (QString::compare(value, "false", Qt::CaseInsensitive) == 0) {
				sp->linear = TEXTURE_LINEAR_DISAB;
			} else if (QString::compare(value, "true", Qt::CaseInsensitive) == 0) {
				sp->linear = TEXTURE_LINEAR_ENAB;
			}
		}

		// float_framebuffer
		key = QString("float_framebuffer%1").arg(i);
		if (!cgp_value(set, key, value)) {
			if (QString::compare(value, "true", Qt::CaseInsensitive) == 0) {
				sp->fbo_flt = TRUE;
			}
		}

		// srgb_framebuffer
		key = QString("srgb_framebuffer%1").arg(i);
		if (!cgp_value(set, key, value)) {
			if (QString::compare(value, "true", Qt::CaseInsensitive) == 0) {
				sp->fbo_srgb = TRUE;
			}
		}

		// frame_count_mod
		key = QString("frame_count_mod%1").arg(i);
		if (!cgp_value(set, key, value)) {
			sp->frame_count_mod = value.toInt();
		}

		// wrap_mode
		key = QString("wrap_mode%1").arg(i);
		if (!cgp_value(set, key, value)) {
			if (QString::compare(value, "clamp_to_border", Qt::CaseInsensitive) == 0) {
				sp->wrap = TEXTURE_WRAP_BORDER;
			} else if (QString::compare(value, "clamp_to_edge", Qt::CaseInsensitive) == 0) {
				sp->wrap = TEXTURE_WRAP_EDGE;
			} else if (QString::compare(value, "repeat", Qt::CaseInsensitive) == 0) {
				sp->wrap = TEXTURE_WRAP_REPEAT;
			} else if (QString::compare(value, "mirrored_repeat", Qt::CaseInsensitive) == 0) {
				sp->wrap = TEXTURE_WRAP_MIRRORED_REPEAT;
			} else {
				log_warning(uL("cgp;invalid " uPs("") " attribute, using default value"), qPrintable(key));
				sp->wrap = TEXTURE_WRAP_BORDER;
			}
		}

		// scale
		{
			QString sc_type, sc_type_x, sc_type_y;
			_shader_scale *sc = &sp->sc;

			sc_type = QString("scale_type%1").arg(i);
			cgp_value(set, sc_type, value);
			sc_type = value;

			sc_type_x = QString("scale_type_x%1").arg(i);
			cgp_value(set, sc_type_x, value);
			sc_type_x = value;

			sc_type_y = QString("scale_type_y%1").arg(i);
			cgp_value(set, sc_type_y, value);
			sc_type_y = value;

			if (!sc_type.isEmpty() || !sc_type_x.isEmpty() || !sc_type_y.isEmpty()) {
				if (sc_type.length() > 0) {
					sc_type_x = sc_type;
					sc_type_y = sc_type;
				}

				if (sc_type_x.length() > 0) {
					if (QString::compare(sc_type_x, "source", Qt::CaseInsensitive) == 0) {
						sc->type.x = SHADER_SCALE_INPUT;
					} else if (QString::compare(sc_type_x, "viewport", Qt::CaseInsensitive) == 0) {
						sc->type.x = SHADER_SCALE_VIEWPORT;
					} else if (QString::compare(sc_type_x, "absolute", Qt::CaseInsensitive) == 0) {
						sc->type.x = SHADER_SCALE_ABSOLUTE;
					} else {
						log_warning(uL("cgp;invalid scale_type_x attribute"));
						delete(set);
						return (EXIT_ERROR);
					}
				}

				if (sc_type_y.length() > 0) {
					if (QString::compare(sc_type_y, "source", Qt::CaseInsensitive) == 0) {
						sc->type.y = SHADER_SCALE_INPUT;
					} else if (QString::compare(sc_type_y, "viewport", Qt::CaseInsensitive) == 0) {
						sc->type.y = SHADER_SCALE_VIEWPORT;
					} else if (QString::compare(sc_type_y, "absolute", Qt::CaseInsensitive) == 0) {
						sc->type.y = SHADER_SCALE_ABSOLUTE;
					} else {
						log_warning(uL("cgp;invalid scale_type_y attribute"));
						delete(set);
						return (EXIT_ERROR);
					}
				}

				// x
				key = QString("scale%1").arg(i);
				if (sc->type.x == SHADER_SCALE_ABSOLUTE) {
					if (!cgp_value(set, key, value)) {
						sc->abs.x = value.toUInt();
					} else {
						key = QString("scale_x%1").arg(i);
						if (!cgp_value(set, key, value)) {
							sc->abs.x = value.toUInt();
						}
					}
				} else {
					if (!cgp_value(set, key, value)) {
						sc->scale.x = value.toFloat();
					} else {
						key = QString("scale_x%1").arg(i);
						if (!cgp_value(set, key, value)) {
							sc->scale.x = value.toFloat();
						}
					}
				}

				// y
				key = QString("scale%1").arg(i);
				if (sc->type.y == SHADER_SCALE_ABSOLUTE) {
					if (!cgp_value(set, key, value)) {
						sc->abs.y = value.toUInt();
					} else {
						key = QString("scale_y%1").arg(i);
						if (!cgp_value(set, key, value)) {
							sc->abs.y = value.toUInt();
						}
					}
				} else {
					if (!cgp_value(set, key, value)) {
						sc->scale.y = value.toFloat();
					} else {
						key = QString("scale_y%1").arg(i);
						if (!cgp_value(set, key, value)) {
							sc->scale.y = value.toFloat();
						}
					}
				}
			}
		}
	}

	// textures
	if (!cgp_value(set, "textures", value)) {
		list = value.split(';');

		foreach (const QString &ele, list) {
			_lut_pass *lp;

			if (!ele.length()) {
				continue;
			}

			finded = false;

			for (i = 0; i < se.luts; i++) {
				lp = &se.lp[i];
				if (QString(lp->name) == ele) {
					finded = true;
					break;
				}
			}

			if (finded) {
				continue;
			}

			lp = &se.lp[se.luts++];

			// name
			::strncpy(lp->name, qPrintable(ele), sizeof(lp->name) - 1);

			// path
			if (!cgp_value(set, ele, value)) {
				value.replace('\\', '/');
				ustrncpy(lp->path, uQStringCD(QFileInfo(fi.absolutePath() + '/' + value).absoluteFilePath()), usizeof(lp->path) - 1);
			}

			// mipmap
			key = ele + "_mipmap";
			if (!cgp_value(set, key, value)) {
				if (QString::compare(value, "true", Qt::CaseInsensitive) == 0) {
					lp->mipmap = TRUE;
				}
			}

			// linear
			key = ele + "_linear";
			if (!cgp_value(set, key, value)) {
				if (QString::compare(value, "false", Qt::CaseInsensitive) == 0) {
					lp->linear = TEXTURE_LINEAR_DISAB;
				} else if (QString::compare(value, "true", Qt::CaseInsensitive) == 0) {
					lp->linear = TEXTURE_LINEAR_ENAB;
				}
			}

			// wrap_mode
			key = ele + "_wrap_mode";
			if (!cgp_value(set, key, value)) {
				if (QString::compare(value, "clamp_to_border", Qt::CaseInsensitive) == 0) {
					lp->wrap = TEXTURE_WRAP_BORDER;
				} else if (QString::compare(value, "clamp_to_edge", Qt::CaseInsensitive) == 0) {
					lp->wrap = TEXTURE_WRAP_EDGE;
				} else if (QString::compare(value, "repeat", Qt::CaseInsensitive) == 0) {
					lp->wrap = TEXTURE_WRAP_REPEAT;
				} else if (QString::compare(value, "mirrored_repeat", Qt::CaseInsensitive) == 0) {
					lp->wrap = TEXTURE_WRAP_MIRRORED_REPEAT;
				} else {
					log_warning(uL("cgp;invalid " uPs("") " attribute, using default value"), qPrintable(key));
					lp->wrap = TEXTURE_WRAP_BORDER;
				}
			}
		}
	}

	// parameters
	if (!cgp_value(set, "parameters", value)) {
		list = value.split(';');

		foreach (const QString &ele, list) {
			_param_shd *prm;

			finded = false;

			for (i = 0; i < se.params; i++) {
				prm = &se.param[i];
				if (QString(prm->name) == ele) {
					finded = true;
					break;
				}
			}

			if (finded) {
				continue;
			}

			prm = &se.param[se.params++];

			// name
			::strncpy(prm->name, qPrintable(ele), sizeof(prm->name) - 1);

			// value
			if (!cgp_value(set, ele, value)) {
				QString qfloat;

#if (QT_VERSION >= QT_VERSION_CHECK(5, 7, 0))
				for (QChar c : qAsConst(value)) {
#else
				for (QChar c : value) {
#endif
					if (c.isDigit() || (c == '.') || (c == ',')) {
						qfloat.append(c);
					}
				}
				prm->value = qfloat.toFloat();
			}
		}
	}

	delete (set);

	memcpy(&shader_effect, &se, sizeof(_shader_effect));

	return (EXIT_OK);
}
BYTE cgp_pragma_param(const char *code, const uTCHAR *path) {
	QTextStream stream(code);
	QFile file(uQString(path));
	QString line;
	_param_shd param;
	int i, params = 0;

	if (path && path[0]) {
		if (!file.open(QIODevice::ReadOnly)) {
			log_error(uL("cgp;can't open file '" uPs("") "'"), path);
			return (EXIT_ERROR);
		}
		stream.setDevice(&file);
	}

	do {
		line = stream.readLine();

		::memset(&param, 0x00, sizeof(_param_shd));

		if (line.startsWith("#pragma parameter")) {
			bool finded;
			int count;

			// sscanf non e' "locale indipendente" percio' lo utilizzo solo per
			// ricavare nome e descrizione del parametro.
			count = ::sscanf(line.toUtf8().constData(), "#pragma parameter %63s \"%63[^\"]\" ", param.name, param.desc);

			if (count < 2) {
				continue;
			}

			{
				static QRegularExpression rx("#pragma parameter.*\"");

				line = line.remove(rx);
			}

			{
				static QRegularExpression rx("[-+]?[0-9]*\\.[0-9]+");
				QRegularExpressionMatchIterator iterator = rx.globalMatch(line);

				while (iterator.hasNext()) {
					QRegularExpressionMatch match = iterator.next();

					switch (count++) {
						case 2:
							param.initial = match.captured(0).toFloat();
							break;
						case 3:
							param.min = match.captured(0).toFloat();
							break;
						case 4:
							param.max = match.captured(0).toFloat();
							break;
						case 5:
							param.step = match.captured(0).toFloat();
							break;
					}
				}
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

			if (!finded) {
				if (shader_effect.params < MAX_PARAM) {
					::memcpy(&shader_effect.param[shader_effect.params], &param, sizeof(_param_shd));
					shader_effect.params++;
					params++;
				}
			}
		}
	} while (!line.isNull());

	if (params) {
		log_info(uL("shader parameters"));
		for (i = 0; i < params; i++) {
			_param_shd *ps = &shader_effect.param[i];

			log_info_box(uL("%d;%s = %f"), i, ps->name, ps->value);
		}
	}

	if (file.isOpen()) {
		file.close();
	}
	return (EXIT_OK);
}

static bool cgp_value(QSettings *set, const QString &key, QString &value) {
	value = "";

	if (set->allKeys().contains(key, Qt::CaseInsensitive)) {
		value = set->value(key).toString();
		return (FALSE);
	}

	return (TRUE);
}
static bool cgp_rd_file(QIODevice &device, QSettings::SettingsMap &map) {
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
			key = QString(splitted.at(0)).replace(qtHelper::rx_any_numbers, "");
			value = splitted.at(1).trimmed();
			// rimuovo i commenti che possono esserci sulla riga
			value = value.remove(qtHelper::rx_comment_0);
			value = value.remove(qtHelper::rx_comment_1);
			value = value.remove('"');
			value = value.trimmed();

			map[key] = value;
		}
	}

	return (true);
}
