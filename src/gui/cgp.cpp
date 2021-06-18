/*
 *  Copyright (C) 2010-2021 Fabio Cavallo (aka FHorse)
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
#include "cgp.h"
#include "shaders.h"

static bool cgp_value(QSettings *set, QString key, QString &value);
static bool cgp_rd_file(QIODevice &device, QSettings::SettingsMap &map);

BYTE cgp_parse(const uTCHAR *file) {
	static const QSettings::Format cfg = QSettings::registerFormat("cgp", cgp_rd_file, NULL);
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
		fprintf(stderr, "CGP: Shader format non supported.");
		return (EXIT_ERROR);
#endif
	} else if (QString::compare(fi.suffix(), "glslp", Qt::CaseInsensitive) == 0) {
#if defined (WITH_OPENGL)
		se.type = MS_GLSLP;
# else
		fprintf(stderr, "CGP: Shader format non supported.");
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
		if (cgp_value(set, key, value) == FALSE) {
			value.replace('\\', '/');
			ustrncpy(sp->path, uQStringCD(QFileInfo(fi.absolutePath() + '/' + value).absoluteFilePath()), usizeof(sp->path) - 1);
		} else {
			delete(set);
			return (EXIT_ERROR);
		}

		// alias
		key = QString("alias%1").arg(i);
		if (cgp_value(set, key, value) == FALSE) {
			::strncpy(sp->alias, qPrintable(value), sizeof(sp->alias) - 1);
		}

		// mipmap_input
		key = QString("mipmap_input%1").arg(i);
		if (cgp_value(set, key, value) == FALSE) {
			if (QString::compare(value, "true", Qt::CaseInsensitive) == 0) {
				sp->mipmap_input = TRUE;
			}
		}

		// filter_linear
		key = QString("filter_linear%1").arg(i);
		if (cgp_value(set, key, value) == FALSE) {
			if (QString::compare(value, "false", Qt::CaseInsensitive) == 0) {
				sp->linear = TEXTURE_LINEAR_DISAB;
			} else if (QString::compare(value, "true", Qt::CaseInsensitive) == 0) {
				sp->linear = TEXTURE_LINEAR_ENAB;
			}
		}

		// float_framebuffer
		key = QString("float_framebuffer%1").arg(i);
		if (cgp_value(set, key, value) == FALSE) {
			if (QString::compare(value, "true", Qt::CaseInsensitive) == 0) {
				sp->fbo_flt = TRUE;
			}
		}

		// srgb_framebuffer
		key = QString("srgb_framebuffer%1").arg(i);
		if (cgp_value(set, key, value) == FALSE) {
			if (QString::compare(value, "true", Qt::CaseInsensitive) == 0) {
				sp->fbo_srgb = TRUE;
			}
		}

		// frame_count_mod
		key = QString("frame_count_mod%1").arg(i);
		if (cgp_value(set, key, value) == FALSE) {
			sp->frame_count_mod = value.toFloat();
		}

		// wrap_mode
		key = QString("wrap_mode%1").arg(i);
		if (cgp_value(set, key, value) == FALSE) {
			if (QString::compare(value, "clamp_to_border", Qt::CaseInsensitive) == 0) {
				sp->wrap = TEXTURE_WRAP_BORDER;
			} else if (QString::compare(value, "clamp_to_edge", Qt::CaseInsensitive) == 0) {
				sp->wrap = TEXTURE_WRAP_EDGE;
			} else if (QString::compare(value, "repeat", Qt::CaseInsensitive) == 0) {
				sp->wrap = TEXTURE_WRAP_REPEAT;
			} else if (QString::compare(value, "mirrored_repeat", Qt::CaseInsensitive) == 0) {
				sp->wrap = TEXTURE_WRAP_MIRRORED_REPEAT;
			} else {
				fprintf(stderr, "[CGP] : Invalid %s attribute. Using default value.\n", qPrintable(key));
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
						fprintf(stderr, "[CGP] : Invalid scale_type_x attribute.\n");
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
						fprintf(stderr, "[CGP] : Invalid scale_type_y attribute.\n");
						delete(set);
						return (EXIT_ERROR);
					}
				}

				// x
				key = QString("scale%1").arg(i);
				if (sc->type.x == SHADER_SCALE_ABSOLUTE) {
					if (cgp_value(set, key, value) == FALSE) {
						sc->abs.x = value.toFloat();
					} else {
						key = QString("scale_x%1").arg(i);
						if (cgp_value(set, key, value) == FALSE) {
							sc->abs.x = value.toFloat();
						}
					}
				} else {
					if (cgp_value(set, key, value) == FALSE) {
						sc->scale.x = value.toFloat();
					} else {
						key = QString("scale_x%1").arg(i);
						if (cgp_value(set, key, value) == FALSE) {
							sc->scale.x = value.toFloat();
						}
					}
				}

				// y
				key = QString("scale%1").arg(i);
				if (sc->type.y == SHADER_SCALE_ABSOLUTE) {
					if (cgp_value(set, key, value) == FALSE) {
						sc->abs.y = value.toFloat();
					} else {
						key = QString("scale_y%1").arg(i);
						if (cgp_value(set, key, value) == FALSE) {
							sc->abs.y = value.toFloat();
						}
					}
				} else {
					if (cgp_value(set, key, value) == FALSE) {
						sc->scale.y = value.toFloat();
					} else {
						key = QString("scale_y%1").arg(i);
						if (cgp_value(set, key, value) == FALSE) {
							sc->scale.y = value.toFloat();
						}
					}
				}
			}
		}
	}

	// textures
	if (cgp_value(set, "textures", value) == FALSE) {
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

			if (finded == true) {
				continue;
			}

			lp = &se.lp[se.luts++];

			// name
			::strncpy(lp->name, qPrintable(ele), sizeof(lp->name) - 1);

			// path
			if (cgp_value(set, ele, value) == FALSE) {
				value.replace('\\', '/');
				ustrncpy(lp->path, uQStringCD(QFileInfo(fi.absolutePath() + '/' + value).absoluteFilePath()), usizeof(lp->path) - 1);
			}

			// mipmap
			key = ele + "_mipmap";
			if (cgp_value(set, key, value) == FALSE) {
				if (QString::compare(value, "true", Qt::CaseInsensitive) == 0) {
					lp->mipmap = TRUE;
				}
			}

			// linear
			key = ele + "_linear";
			if (cgp_value(set, key, value) == FALSE) {
				if (QString::compare(value, "false", Qt::CaseInsensitive) == 0) {
					lp->linear = TEXTURE_LINEAR_DISAB;
				} else if (QString::compare(value, "true", Qt::CaseInsensitive) == 0) {
					lp->linear = TEXTURE_LINEAR_ENAB;
				}
			}

			// wrap_mode
			key = ele + "_wrap_mode";
			if (cgp_value(set, key, value) == FALSE) {
				if (QString::compare(value, "clamp_to_border", Qt::CaseInsensitive) == 0) {
					lp->wrap = TEXTURE_WRAP_BORDER;
				} else if (QString::compare(value, "clamp_to_edge", Qt::CaseInsensitive) == 0) {
					lp->wrap = TEXTURE_WRAP_EDGE;
				} else if (QString::compare(value, "repeat", Qt::CaseInsensitive) == 0) {
					lp->wrap = TEXTURE_WRAP_REPEAT;
				} else if (QString::compare(value, "mirrored_repeat", Qt::CaseInsensitive) == 0) {
					lp->wrap = TEXTURE_WRAP_MIRRORED_REPEAT;
				} else {
					fprintf(stderr, "[CGP] : Invalid %s attribute. Using default value.\n", qPrintable(key));
					lp->wrap = TEXTURE_WRAP_BORDER;
				}
			}
		}
	}

	// parameters
	if (cgp_value(set, "parameters", value) == FALSE) {
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

			if (finded == true) {
				continue;
			}

			prm = &se.param[se.params++];

			// name
			::strncpy(prm->name, qPrintable(ele), sizeof(prm->name) - 1);

			// value
			if (cgp_value(set, ele, value) == FALSE) {
				QString qfloat;

				for (const QChar c : qAsConst(value)) {
					if (c.isDigit() || (c == '.') || (c == ',')) {
						qfloat.append(c);
					}
				}
				prm->value = qfloat.toFloat();
			}
		}
	}

	delete(set);

	memcpy(&shader_effect, &se, sizeof(_shader_effect));

	return (EXIT_OK);
}
BYTE cgp_pragma_param(char *code, const uTCHAR *path) {
	QTextStream stream(code);
	QFile file(uQString(path));
	QString line;
	_param_shd param;

	if (path && path[0]) {
		if (file.open(QIODevice::ReadOnly) == false) {
			ufprintf(stderr, uL("CGP: Can't open file '" uPERCENTs "'\n"), path);
			return (EXIT_ERROR);
		}
		stream.setDevice(&file);
	}

	do {
		line = stream.readLine();

		::memset(&param, 0x00, sizeof(_param_shd));

		if (line.startsWith("#pragma parameter")) {
			QRegExp rx("[-+]?[0-9]*(\\.[0-9]+)");
			int i, count = 0, pos = 0;
			bool finded;

			// sscanf non e' "locale indipendente" percio' lo utilizzo solo per
			// ricavare nome e descrizione del parametro.
			count = ::sscanf(line.toUtf8().constData(), "#pragma parameter %63s \"%63[^\"]\" ", param.name, param.desc);

			if (count < 2) {
				continue;
			}

			line = line.remove(QRegExp("#pragma parameter.*\""));

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
					::memcpy(&shader_effect.param[shader_effect.params], &param, sizeof(_param_shd));
					shader_effect.params++;
					fprintf(stderr, "CGP: Findend parameter %s = %f\n", param.name, param.value);
				}
			}
		}
	} while (!line.isNull());

	if (file.isOpen()) {
		file.close();
	}
	return (EXIT_OK);
}

static bool cgp_value(QSettings *set, QString key, QString &value) {
	value = "";

	if (set->allKeys().contains(key, Qt::CaseInsensitive)) {
		value = set->value(key).toString();
		return (FALSE);
	}

	return (TRUE);
}
static bool cgp_rd_file(QIODevice &device, QSettings::SettingsMap &map) {
	QTextStream in(&device);

	in.setCodec("UTF-8");

	while (!in.atEnd()) {
		QString line = in.readLine();

		if (line.isEmpty() || line.startsWith("#") || line.startsWith("*")) {
			continue;
		}

		QStringList splitted = line.split("=");
		QString key, value;

		if (splitted.count() == 2) {
			key = QString(splitted.at(0)).replace(QRegExp("\\s*$"), "");
			value = splitted.at(1).trimmed();
			// rimuovo i commenti che possono esserci sulla riga
			value = value.remove(QRegExp("#.*"));
			value = value.remove(QRegExp("//.*"));
			value = value.remove('"');
			value = value.trimmed();

			map[key] = value;
		}
	}

	return (true);
}
