/*
 *  Copyright (C) 2010-2017 Fabio Cavallo (aka FHorse)
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

#ifndef DLGINPUT_HPP_
#define DLGINPUT_HPP_

#include <QtCore/QtGlobal>
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
#include <QtGui/QDialog>
#else
#include <QtWidgets/QDialog>
#endif
#include "dlgInput.hh"
#include "conf.h"

typedef struct {
	BYTE id;
	_port port;
} _cfg_port;

class dlgInput : public QDialog, public Ui::Input_dialog {
		Q_OBJECT

	private:
		struct _data {
			_config_input settings;
			_cfg_port port[PORT_MAX];
		} data;
		struct _shcut {
			QStringList text[2];

			QPushButton *bp;
			QBrush bckColor;

			struct _joy {
				int fd;
				WORD value;
				QTimer *timer;
			} joy;

			struct _timeout {
				int seconds;
				QTimer *timer;
			} timeout;

			bool no_other_buttons;

			BYTE type;
			BYTE row;
		} shcut;

	public:
		dlgInput(QWidget *parent);
		~dlgInput();

	private:
		bool eventFilter(QObject *obj, QEvent *event);
		void update_dialog();
		void combobox_cp_init(QComboBox *cb, _cfg_port *cfg_port, void *list, int length);
		void combobox_cp_init();
		void setup_shortcuts();
		void combo_joy_id_init();
		void update_groupbox_shortcuts(int mode, int type, int row);
		void populate_shortcut(int index);
		void update_text_shortcut(QAction *action, int index);
		void info_entry_print(QString txt);
		bool keypressEvent(QEvent *event);

	private slots:
		void s_combobox_cm_activated(int index);
		void s_combobox_cexp_activated(int index);
		void s_combobox_cp_activated(int index);
		void s_setup_clicked(bool checked);
		void s_checkbox_permit_updown_leftright_changed(int state);
		void s_checkbox_hide_zapper_cursor_changed(int state);
		void s_combobox_joy_activated(int index);
		void s_shortcut_clicked(bool checked);
		void s_keyb_shortcut_default(bool checked);
		void s_joy_shortcut_unset(bool checked);
		void s_joy_read_timer();
		void s_button_timeout();
		void s_default_clicked(bool checked);
		void s_apply_clicked(bool checked);
		void s_discard_clicked(bool checked);
};

#endif /* DLGINPUT_HPP_ */
