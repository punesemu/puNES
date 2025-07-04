/*
 *  Copyright (C) 2010-2026 Fabio Cavallo (aka FHorse)
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
#include <QtWidgets/QPushButton>
#include "ui_dlgStdPad.h"
#include "wdgTitleBarWindow.hpp"
#include "common.h"
#include "input.h"

typedef struct _cfg_port {
	BYTE id;
	_port *port;
} _cfg_port;
typedef struct _cfg_port_tmp {
	BYTE id;
	_port port;
} _cfg_port_tmp;
typedef struct _cb_ports {
	QString desc;
	int index;
} _cb_ports;
typedef struct _joy_list {
	_cb_ports ele[MAX_JOYSTICK + 1];
	int count{};
	int disabled_line{};
} _joy_list;

extern _joy_list joy_list;

// ----------------------------------------------------------------------------------------------

class dlgStdPad final : public QWidget, public Ui::dlgStdPad {
	Q_OBJECT

	private:
		struct _data {
			struct _joy {
				WORD value;
				QTimer *timer;
			} joy;
			struct _seq {
				bool active;
				int type;
				int counter;
				QTimer *timer;
			} seq;
			pixmapPushButton *bp;
			_cfg_port_tmp cfg;
			bool no_other_buttons;
			BYTE vbutton;
			BYTE exec_js_init;
			int deadzone;
		} data;
		int last_js_index;

	signals:
		void et_update_joy_combo(void);

	public:
		explicit dlgStdPad(QWidget *parent = nullptr, _cfg_port *cfg_port = nullptr);
		~dlgStdPad() override;

	protected:
		bool eventFilter(QObject *obj, QEvent *event) override;
		void changeEvent(QEvent *event) override;
		void closeEvent(QCloseEvent *event) override;

	private:
		void stylesheet_update(void) const;

	private:
		static QString stylesheet_label(const QColor &background);
		static QString stylesheet_frame(const QColor &border, const QColor &background);
		static QString stylesheet_pixmapbutton(void);
		QString stylesheet_left_button(void) const;
		QString stylesheet_right_button(void) const;

	private:
		bool keypress(QKeyEvent *event);
		void update_dialog(void) const;
		void joy_combo_init(void);
		void setEnable_tab_buttons(int type, bool mode) const;
		void disable_tab_and_other(int type, int vbutton) const;
		void info_entry_print(int type, const QString &txt) const;
		void js_press_event(void);
		void td_update_label(int type, int value) const;
		void deadzone_update_label(int value) const;
		static void js_pixmapPushButton(int index, DBWORD input, pixmapPushButton *bt);
		int js_jdev_index(void) const;

	private slots:
		void s_combobox_joy_activated(int index);
		void s_combobox_joy_index_changed(int index);
		void s_input_clicked(bool checked);
		void s_default_clicked(bool checked);
		void s_unset_clicked(bool checked);
		void s_in_sequence_clicked(bool checked);
		void s_unset_all_clicked(bool checked) const;
		void s_defaults_clicked(bool checked);
		void s_deadzone_slider_value_changed(int value);
		void s_deadzone_default_clicked(bool checked) const;
		void s_combobox_controller_type_activated(int index);
		void s_slider_td_value_changed(int value);
		void s_pad_joy_read_timer(void);
		void s_pad_in_sequence_timer(void);
		void s_apply_clicked(bool checked);

	private slots:
		void s_et_update_joy_combo(void);
};

// ----------------------------------------------------------------------------------------------

class wdgDlgStdPad final : public wdgTitleBarDialog {
	public:
		dlgStdPad *wd;

	public:
		explicit wdgDlgStdPad(QWidget *parent = nullptr, _cfg_port *cfg_port = nullptr);
		~wdgDlgStdPad() override;
};

#endif /* DLGSTDPAD_HPP_ */
