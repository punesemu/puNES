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

#ifndef OBJSETTINGS_HPP_
#define OBJSETTINGS_HPP_

#include <QtCore/QSettings>
#include <QtCore/QFile>
#include <QtGui/QKeySequence>
#include <QtCore/QStringList>
#include <QtGui/QKeyEvent>
#include "settings.h"
#include "conf.h"
#include "gui.h"

class objSettings : public QSettings {
	Q_OBJECT

	public:
		QStringList val;

	protected:
		const _list_settings *set;
		int listEle;

	public:
		objSettings(Format f, const QString &file, int list_ele);
		~objSettings() override;

	public:
		virtual void setup(void);

	protected:
		virtual void to_cfg(const QString &group);
		virtual void fr_cfg(const QString &group);
		virtual void after_the_defaults(void);

	protected:
		virtual void rd(void);
		virtual void rd(const QString &group);
		virtual void rd_key(int index);

	private:
		void rd_key(const QString &group, int index);

	public:
		void wr(void);
		void wr(const QString &group);

	private:
		void wr_key(const QString &group, int index);

	protected:
		virtual void wr_key(int index);
		virtual void wr_all_keys(void);

	public:
		int val_to_int(int index, const uTCHAR *buffer);
		void cpy_utchar_to_val(int index, uTCHAR *src);

	protected:
		int val_to_int(int index);
		void int_to_val(int index, int value);
		void cpy_val_to_utchar(int index, uTCHAR *dst, int length);
};
class objSet : public objSettings {
	public:
		objSet(Format f, const QString &file, int list_ele);
		~objSet() override;

	public:
		void setup(void) override;

	protected:
		void to_cfg(const QString &group) override;
		void fr_cfg(const QString &group) override;
		void after_the_defaults(void) override;

	public:
		void oscan_default(_overscan_borders *ob, BYTE mode);
		void oscan_val_to_int(int index, _overscan_borders *ob, const uTCHAR *buffer);

	private:
		void oscan_val_to_int(int index, _overscan_borders *ob);
		QString oscan_val(_overscan_borders *ob);

#if defined (FULLSCREEN_RESFREQ)
	public:
		void resolution_val_to_int(int *w, int *h, const uTCHAR *buffer);

	private:
		void resolution_val_to_int(int index, int *w, int *h);
		QString resolution_val(const int *w, const int *h);
#endif

	private:
		void ntsc_val_to_double(int index, void *ntsc_frmt);
		void ntsc_val_to_double(void *ntsc_frmt, const uTCHAR *buffer);
		QString ntsc_val(void *ntsc_frmt);

	private:
		void ntsc_bisqwit_val_to_double(int index, void *ntsc_frmt);
		void ntsc_bisqwit_val_to_double(void *ntsc_frmt, const uTCHAR *buffer);
		QString ntsc_bisqwit_val(void *ntsc_frmt);

	private:
		void ntsc_lmp88959_val_to_int(int index, void *ntsc_frmt);
		void ntsc_lmp88959_val_to_int(void *ntsc_frmt, const uTCHAR *buffer);
		QString ntsc_lmp88959_val(void *ntsc_frmt);
		void ntsc_nesrgb_lmp88959_val_to_int(int index, void *ntsc_frmt);
		void ntsc_nesrgb_lmp88959_val_to_int(void *ntsc_frmt, const uTCHAR *buffer);
		QString ntsc_nesrgb_lmp88959_val(void *ntsc_frmt);

	private:
		void pal_lmp88959_val_to_int(int index, void *ntsc_frmt);
		void pal_lmp88959_val_to_int(void *ntsc_frmt, const uTCHAR *buffer);
		QString pal_lmp88959_val(void *ntsc_frmt);
		void pal_nesrgb_lmp88959_val_to_int(int index, void *ntsc_frmt);
		void pal_nesrgb_lmp88959_val_to_int(void *ntsc_frmt, const uTCHAR *buffer);
		QString pal_nesrgb_lmp88959_val(void *ntsc_frmt);

	private:
		int channel_convert_index(int index);
		void channel_decode(int index, const QString &val);
		void channel_default(int index);
		void channel_val_to_int(int index);
		QString channel_val(int index);

	public:
		double val_to_double(WORD round, const uTCHAR *buffer);

	private:
		double val_to_double(int index, WORD round);
		void double_to_val(int index, double value);
		void last_geometry_val_to_int(int index, _last_geometry *lg);
		QString last_geometry_val(_last_geometry *lg);
};
class objPgs : public objSettings {
	public:
		objPgs(Format f, const QString &file, int list_ele);
		~objPgs() override;

	public:
		void setup(void) override;

	protected:
		void to_cfg(const QString &group) override;
		void fr_cfg(const QString &group) override;
};
class objInp : public objSettings {
	public:
		objInp(Format f, const QString &file, int list_ele);
		~objInp() override;

	public:
		void setup(void) override;

	protected:
		void to_cfg(const QString &group) override;
		void fr_cfg(const QString &group) override;

	public:
		void set_all_input_defaults(_config_input *config_input, _array_pointers_port *array);
		void *sc_val_to_qstring_pntr(int index, int type);
		void sc_qstring_pntr_to_val(void *str, int index, int type);
		DBWORD kbd_keyval_from_name(int index, const QString &name);
		static QString kbd_keyval_to_name(DBWORD value);
		static DBWORD kbd_keyval_decode(QKeyEvent *keyEvent);
		void kbd_default(int button, _port *prt, int index);
		void kbd_defaults(_port *prt, int index);
		static QString nscode_to_name(DBWORD value);
		static DBWORD nscode_from_name(QString name);
		DBWORD nes_keyboard_nscode_default(const QString &name);
		DBWORD nes_keyboard_nscode(const QString &name);
		void nes_keyboard_set_nscode(const QString &name, DBWORD nscode);

	private:
		void kbd_rd(int index, int pIndex);
		void kbd_wr(int index, int pIndex);
		DBWORD _kbd_keyval_from_name(const QString &name);
		int kbd_keyval_to_int(int index);
		int nes_keyboard_index(const QString &name);

	private:
		void js_val_to_guid(int index, _input_guid *guid);
		void js_guid_to_val(int index, _input_guid *guid);

	private:
		int tb_delay_val_to_int(int index);
};
class objShp : public objSettings {
	public:
		objShp(Format f, const QString &file, int list_ele);
		~objShp() override;

	public:
		void setup(void) override;

	protected:
		void to_cfg(const QString &group) override;
		void fr_cfg(const QString &group) override;

	protected:
		void rd(void) override;
		void rd(const QString &group) override;

	private:
		void rd_pshd_key(void *pshd, int index);
		void wr_pshd_key(void *pshd, int index);

	protected:
		void wr_all_keys(void) override;

	private:
		float val_to_float(int index);
		void float_to_val(int index, float value);
};
class objJsc : public objSettings {
	private:
		int jindex;

	public:
		objJsc(Format f, const QString &file, int list_ele, int index);
		~objJsc() override;

	protected:
		void to_cfg(const QString &group) override;
		void fr_cfg(const QString &group) override;

	protected:
		void rd_key(int index) override;

	public:
		int jsc_deadzone_default(void);

	private:
		int jsc_joyval_to_int(int index);
		qulonglong val_to_ulonglong(int index);
		void ulonglong_to_val(int index, qulonglong value);
};

typedef struct _emu_settings {
	QSettings::Format cfg;
	objSet *set;
	objInp *inp;
	objPgs *pgs;
	objShp *shp;
	objJsc *jsc;
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
