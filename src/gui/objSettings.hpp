/*
 *  Copyright (C) 2010-2019 Fabio Cavallo (aka FHorse)
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

#ifndef OBJSETTINGS_HPP_
#define OBJSETTINGS_HPP_

#include "settings.h"
#include <QtCore/QSettings>
#include <QtCore/QFile>
#include <QtGui/QKeySequence>
#include <QtCore/QStringList>
#include <QtGui/QKeyEvent>
#include "conf.h"
#include "overscan.h"
#include "gui.h"

class objSettings : public QSettings {
		Q_OBJECT

	public:
		QStringList val;

	protected:
		const _list_settings *set;
		int listEle;

	public:
		objSettings(Format f, QString file, int list_ele);
		~objSettings();

	public:
		void wr(void);
		void wr(QString group);
		int val_to_int(int index, const uTCHAR *buffer);
		void cpy_utchar_to_val(int index, uTCHAR *src);

	protected:
		virtual void setup(void);
		virtual void to_cfg(QString group);
		virtual void fr_cfg(QString group);
		virtual void after_the_defaults(void);
		void rd(void);
		void rd(QString group);
		int val_to_int(int index);
		void int_to_val(int index, int value);
		void cpy_val_to_utchar(int index, uTCHAR *dst, int length);

	private:
		void rd_key(int index);
		void rd_key(QString group, int index);
		void wr_key(int index);
		void wr_key(QString group, int index);
		void wr_all_keys(void);
};

class objSet : public objSettings {
		Q_OBJECT

	public:
		objSet(Format f, QString file, int list_ele);
		~objSet();

	public:
		double val_to_double(WORD round, const uTCHAR *buffer);
		void oscan_val_to_int(int index, _overscan_borders *ob, const uTCHAR *buffer);
		void oscan_default(_overscan_borders *ob, BYTE mode);

	protected:
		virtual void setup(void);
		virtual void to_cfg(QString group);
		virtual void fr_cfg(QString group);
		virtual void after_the_defaults(void);

	private:
		double val_to_double(int index, WORD round);
		void double_to_val(int index, double value);
		int channel_convert_index(int index);
		void channel_decode(int index, QString val);
		void channel_default(int index);
		void channel_val_to_int(int index);
		QString channel_val(int index);
		void lastpos_val_to_int(int index, _last_pos *last_pos);
		QString lastpos_val(_last_pos *last_pos);
		void oscan_val_to_int(int index, _overscan_borders *ob);
		QString oscan_val(_overscan_borders *ob);
};

class objPgs : public objSettings {
	public:
		objPgs(Format f, QString file, int list_ele);
		~objPgs();

	protected:
		void setup(void);
		void to_cfg(QString group);
		void fr_cfg(QString group);
};

class objInp : public objSettings {
	public:
		objInp(Format f, QString file, int list_ele);
		~objInp();

	public:
		static QString kbd_keyval_to_name(const DBWORD value);
		static DBWORD kbd_keyval_decode(QKeyEvent *keyEvent);
		void set_kbd_joy_default(_port *port, int index, int mode);
		void set_all_input_default(_config_input *config_input, _array_pointers_port *array);
		void *sc_val_to_qstring_pntr(int index, int type);
		void sc_qstring_pntr_to_val(void *str, int index, int type);

	protected:
		void setup(void);
		void to_cfg(QString group);
		void fr_cfg(QString group);

	private:
		int kbd_val_to_int(int index);
		void kbd_rd(int index, int pIndex);
		void kbd_wr(int index, int pIndex);
		DBWORD kbd_name(QString name);
		DBWORD kbd_keyval_from_name(int index, QString name);
		int joy_val_to_int(int index);
		void joy_rd(int index, int pIndex);
		void joy_wr(int index, int pIndex);
		int joyid_val_to_int(int index);
		void joyid_int_to_val(int index, int id);
#if defined (__WIN32__)
		void joyguid_val_to_guid(int index, GUID *guid);
		void joyguid_guid_to_val(int index, GUID guid);
#endif
		int tb_delay_val_to_int(int index);
};

typedef struct _emu_settings {
	QSettings::Format cfg;
	objSet *set;
	objPgs *pgs;
	objInp *inp;
	BYTE list;
} _emu_settings;

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC bool rd_cfg_file(QIODevice &device, QSettings::SettingsMap &map);
EXTERNC bool wr_cfg_file(QIODevice &device, const QSettings::SettingsMap &map);

#undef EXTERNC

#endif /* OBJSETTINGS_HPP_ */
