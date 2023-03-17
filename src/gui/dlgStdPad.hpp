/*
 *  Copyright (C) 2010-2023 Fabio Cavallo (aka FHorse)
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
#include <QtWidgets/QPushButton>
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

class pixmapButton: public QPushButton {
	private:
		QPixmap pixmap;

	public:
		explicit pixmapButton(QWidget *parent = nullptr);
		~pixmapButton() override;

	protected:
		void paintEvent(QPaintEvent *e) override;

	public:
		void setPixmap(const QPixmap &pixmap);
};

#include "ui_dlgStdPad.h"

class dlgStdPad : public QDialog, public Ui::dlgStdPad {
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
			pixmapButton *bp;
			_cfg_port_tmp cfg;
			bool no_other_buttons;
			BYTE vbutton;
			BYTE exec_js_init;
			int deadzone;
		} data;
		typedef struct _color_label {
			_color_label() : normal("#EFEFEF"), selected("#BBF591") {}
			QString normal;
			QString selected;
		} _color_label;
		typedef struct _color_frame {
			_color_frame() : button("#BAB9B7"), kbd("#DEDDDA"), joy("#E8E5DC"), tbrd("#A69F8A") {}
			QString button;
			QString kbd;
			QString joy;
			QString tbrd;
		} _color_frame;
		int last_js_index;
		QString style_label = "QLabel {"\
			"	background-color: %0;"\
			"	border-width: 1px;"\
			"	border-color: #BAB9B7;"\
			"	border-style: solid;"\
			"	padding: 2px;"\
			"	padding-left: 3px;"\
			"	padding-right: 3px;"\
			"	border-radius: 3px;"\
			"	color: #000;"\
			"	font-size: 8pt;"\
			"	font-weight: bold;"\
			"}"\
			"QLabel:disabled {"\
			"	background-color: #EFEFEF;"\
			"	border-color: #BABDB6;"\
			"	color: #BABDB6;"\
			"}";
		QString style_pixmapbutton = "QPushButton {"\
			"	border-width: 1px;"\
			"	border-color: #8F8F91;"\
			"	border-style: solid;"\
			"	border-radius: 6px;"\
			"	background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #F6F7FA, stop: 1 #DADBDE);"\
			"	padding: 5px;"\
			"	padding-left: 3px;"\
			"	padding-right: 3px;"\
			"	min-width: 80px;"\
			"	font-size: 8pt;"\
			"}"
		 	"QPushButton:disabled {"\
			"	background-color: #EFEFEF;"\
			"	border-color: #C2C2C2;"\
			"	color: #BABDB6;"\
			"}"\
			"QPushButton:hover {"\
			"	background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #DADBDE, stop: 1 #F6F7FA);"\
			"}";
		QString style_frame = "QFrame {"\
			"	border-width: 1px;"\
			"	border-color: %0;"\
			"	border-style: solid;"\
			"	border-radius: 6px;"\
			"	background-color: %1;"\
			"	font-size: 8pt;"\
			"}";

	public:
		explicit dlgStdPad(QWidget *parent = nullptr, _cfg_port *cfg_port = nullptr);
		~dlgStdPad() override;

	signals:
		void et_update_joy_combo(void);

	protected:
		bool eventFilter(QObject *obj, QEvent *event) override;
		void changeEvent(QEvent *event) override;
		void showEvent(QShowEvent *event) override;
		void closeEvent(QCloseEvent *event) override;

	private:
		bool keypress(QKeyEvent *event);
		void update_dialog(void);
		void joy_combo_init(void);
		void setEnable_tab_buttons(int type, bool mode);
		void disable_tab_and_other(int type, int vbutton);
		void info_entry_print(int type, const QString &txt);
		void js_press_event(void);
		void td_update_label(int type, int value);
		void deadzone_update_label(int value);
		void js_pixmapButton(int index, DBWORD input, pixmapButton *bt);
		int js_jdev_index(void);

	private slots:
		void s_combobox_joy_activated(int index);
		void s_combobox_joy_index_changed(int index);
		void s_input_clicked(bool checked);
		void s_default_clicked(bool checked);
		void s_unset_clicked(bool checked);
		void s_in_sequence_clicked(bool checked);
		void s_unset_all_clicked(bool checked);
		void s_defaults_clicked(bool checked);
		void s_deadzone_slider_value_changed(int value);
		void s_deadzone_default_clicked(bool checked);
		void s_combobox_controller_type_activated(int index);
		void s_slider_td_value_changed(int value);
		void s_pad_joy_read_timer(void);
		void s_pad_in_sequence_timer(void);
		void s_apply_clicked(bool checked);
		void s_discard_clicked(bool checked);

	private slots:
		void s_et_update_joy_combo(void);
};

#endif /* DLGSTDPAD_HPP_ */
