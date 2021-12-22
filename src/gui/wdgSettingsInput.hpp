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

#ifndef WDGSETTINGSINPUT_HPP_
#define WDGSETTINGSINPUT_HPP_

#include <QtWidgets/QWidget>
#include "wdgSettingsInput.hh"
#include "dlgStdPad.hpp"

class wdgSettingsInput : public QWidget, public Ui::wdgSettingsInput {
		Q_OBJECT

	private:
		struct _input {
			_cfg_port cport[PORT_MAX];
		} input;
		struct _shcut {
			QStringList text[2];
			pixmapButton *bp;
			QBrush bckColor;
			bool no_other_buttons;
			BYTE type;
			BYTE row;
			struct _joy {
				WORD value;
				QTimer *timer;
			} joy;
			struct _timeout {
				int seconds;
				QTimer *timer;
			} timeout;
		} shcut;
		bool hide_from_setup_button;
		double last_control;
		dlgStdPad *dlg_std_pad;

	public:
		wdgSettingsInput(QWidget *parent = 0);
		~wdgSettingsInput();

	signals:
		void et_update_joy_combo(void);

	private:
		bool eventFilter(QObject *obj, QEvent *event);
		void changeEvent(QEvent *event);
		void showEvent(QShowEvent *event);
		void hideEvent(QHideEvent *event);

	public:
		void retranslateUi(QWidget *wdgSettingsInput);
		void update_widget(void);
		void update_joy_list(void);

	private:
		void controller_ports_init(void);
		void controller_port_init(QComboBox *cb, _cfg_port *cfg_port, void *list, int length);
		void shortcuts_init(void);
		void shortcut_init(int index, QString *string);
		void shortcut_joy_list_init(void);
		void shortcut_joy_combo_init(void);
		void shortcut_update_text(QAction *action, int index);
		bool shortcut_keypressEvent(QKeyEvent *event);
		void shortcuts_update(int mode, int type, int row);
		void shortcuts_tableview_resize(void);
		void ports_end_misc_set_enabled(bool mode);
		void info_entry_print(QString txt);
		void js_row_pixmapButton(int row);
		void js_pixmapButton(int index, DBWORD input, pixmapButton *bt);
		int js_jdev_index(void);

	private:
		void controller_mode_set(void);
		void expansion_port_set(void);
		void controller_ports_set(void);
		void shortcuts_set(void);

	private slots:
		void s_controller_mode(bool checked);
		void s_expansion_port(int index);
		void s_controller_port(int index);
		void s_controller_port_setup(bool checked);
		void s_input_reset(bool checked);
		void s_permit_updown_leftright(bool checked);
		void s_hide_zapper_cursor(bool checked);
		void s_joy_id(int index);
		void s_joy_index_changed(int index);
		void s_shortcut(bool checked);
		void s_shortcut_unset_all(bool checked);
		void s_shortcut_reset(bool checked);
		void s_shortcut_keyb_default(bool checked);
		void s_shortcut_keyb_unset(bool checked);
		void s_shortcut_joy_unset(bool checked);
		void s_input_timeout(void);
		void s_joy_read_timer(void);

	private slots:
		void s_et_update_joy_combo(void);
};

#endif /* WDGSETTINGSINPUT_HPP_ */
