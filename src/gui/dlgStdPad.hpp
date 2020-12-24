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

#ifndef DLGSTDPAD_HPP_
#define DLGSTDPAD_HPP_

#include <QtCore/QtGlobal>
#include <QtCore/QTimer>
#include <QtWidgets/QDialog>
#include "dlgStdPad.hh"
#include "wdgSettingsInput.hpp"

class dlgStdPad : public QDialog, public Ui::dlgStdPad {
		Q_OBJECT

	private:
		struct _data {
			struct _joy {
				int fd;
				WORD value;
				QTimer *timer;
			} joy;
			struct _seq {
				bool active;
				int type;
				int counter;
				QTimer *timer;
			} seq;
			bool no_other_buttons;
			QPushButton *bp;
			BYTE vbutton;
			_cfg_port cfg;
		} data;

	public:
		dlgStdPad(_cfg_port *cfg_port, QWidget *parent);
		~dlgStdPad();

	protected:
		bool eventFilter(QObject *obj, QEvent *event);
		void changeEvent(QEvent *event);
		void showEvent(QShowEvent *event);
		void closeEvent(QCloseEvent *event);

	private:
		bool keypress(QKeyEvent *event);
		void update_dialog(void);
		void combo_id_init(void);
		void setEnable_tab_buttons(int type, bool mode);
		void disable_tab_and_other(int type, int vbutton);
		void info_entry_print(int type, QString txt);
		void js_press_event(void);
		void td_update_label(int type, int value);

	private slots:
		void s_combobox_joy_activated(int index);
		void s_input_clicked(bool checked);
		void s_unset_clicked(bool checked);
		void s_in_sequence_clicked(bool checked);
		void s_unset_all_clicked(bool checked);
		void s_defaults_clicked(bool checked);
		void s_combobox_controller_type_activated(int index);
		void s_slider_td_value_changed(int value);
		void s_pad_joy_read_timer(void);
		void s_pad_in_sequence_timer(void);
		void s_apply_clicked(bool checked);
		void s_discard_clicked(bool checked);
};

#endif /* DLGSTDPAD_HPP_ */
