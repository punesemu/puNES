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

#include <QtWidgets/QScrollBar>
#if defined (__linux__)
#include <unistd.h>
#include <fcntl.h>
#endif
#include "wdgSettingsInput.moc"
#include "mainWindow.hpp"
#include "dlgSettings.hpp"
#include "dlgStdPad.hpp"
#include "emu_thread.h"

typedef struct _cb_ports {
	QString desc;
	int value;
} _cb_ports;

enum page_input_shcut_mode { UPDATE_ALL, BUTTON_PRESSED, NO_ACTION = 255 };

wdgSettingsInput::wdgSettingsInput(QWidget *parent) : QWidget(parent) {
	int i = 0;

	hide_from_setup_button = false;

	setupUi(this);

	setFocusProxy(tabWidget_Input);

	widget_cm->setStyleSheet(button_stylesheet());

	// setto la dimensione del font
	{
		QFont f = tableWidget_Shortcuts->font();
		int pointsize = f.pointSize() - 1;

		if (pointsize >= 8) {
			f.setPointSize(pointsize);
			tableWidget_Shortcuts->setFont(f);
		}
	}

	for (i = PORT1; i < PORT_MAX; i++) {
		input.cport[i].id = i + 1;
		input.cport[i].port = &port[i];
	}

	js_update_detected_devices();

	controller_ports_init();

	pushButton_cm_nes->setProperty("mtype", QVariant(CTRL_MODE_NES));
	pushButton_cm_famicom->setProperty("mtype", QVariant(CTRL_MODE_FAMICOM));
	pushButton_cm_fscore->setProperty("mtype", QVariant(CTRL_MODE_FOUR_SCORE));

	connect(pushButton_cm_nes, SIGNAL(toggled(bool)), this, SLOT(s_controller_mode(bool)));
	connect(pushButton_cm_famicom, SIGNAL(toggled(bool)), this, SLOT(s_controller_mode(bool)));
	connect(pushButton_cm_fscore, SIGNAL(toggled(bool)), this, SLOT(s_controller_mode(bool)));

	connect(comboBox_exp, SIGNAL(activated(int)), this, SLOT(s_expansion_port(int)));
	connect(comboBox_cp1, SIGNAL(activated(int)), this, SLOT(s_controller_port(int)));
	connect(comboBox_cp2, SIGNAL(activated(int)), this, SLOT(s_controller_port(int)));
	connect(comboBox_cp3, SIGNAL(activated(int)), this, SLOT(s_controller_port(int)));
	connect(comboBox_cp4, SIGNAL(activated(int)), this, SLOT(s_controller_port(int)));

	connect(pushButton_Input_reset, SIGNAL(clicked(bool)), this, SLOT(s_input_reset(bool)));

	connect(checkBox_Permit_updown, SIGNAL(clicked(bool)), this, SLOT(s_permit_updown_leftright(bool)));
	connect(checkBox_Hide_Zapper_cursor, SIGNAL(clicked(bool)), this, SLOT(s_hide_zapper_cursor(bool)));

	shortcuts_init();

	connect(pushButton_Shortcut_unset_all, SIGNAL(clicked(bool)), this, SLOT(s_shortcut_unset_all(bool)));
	connect(pushButton_Shortcut_reset, SIGNAL(clicked(bool)), this, SLOT(s_shortcut_reset(bool)));

	shcut.timeout.timer = new QTimer(this);
	connect(shcut.timeout.timer, SIGNAL(timeout(void)), this, SLOT(s_input_timeout(void)));

	shcut.joy.timer = new QTimer(this);
	connect(shcut.joy.timer, SIGNAL(timeout(void)), this, SLOT(s_joy_read_timer(void)));

	shortcuts_tableview_resize();

	tabWidget_Input->setCurrentIndex(0);
}
wdgSettingsInput::~wdgSettingsInput() {}

bool wdgSettingsInput::eventFilter(QObject *obj, QEvent *event) {
	switch (event->type()) {
		case QEvent::KeyPress:
			return (shortcut_keypressEvent((QKeyEvent *)event));
		default:
			break;
	}

	return (QObject::eventFilter(obj, event));
}
void wdgSettingsInput::changeEvent(QEvent *event) {
	if (event->type() == QEvent::LanguageChange) {
		retranslateUi(this);
	} else {
		QWidget::changeEvent(event);
	}
}
void wdgSettingsInput::showEvent(QShowEvent *event) {
	int dim = fontMetrics().height();

	icon_Ports->setPixmap(QIcon(":/icon/icons/Rs_232_Female.svg").pixmap(dim, dim));
	icon_cm->setPixmap(QIcon(":/icon/icons/mode.svg").pixmap(dim, dim));
	icon_exp->setPixmap(QIcon(":/icon/icons/circuit_board.svg").pixmap(dim, dim));
	icon_cp1->setPixmap(QIcon(":/icon/icons/game_controller.svg").pixmap(dim, dim));
	icon_cp2->setPixmap(QIcon(":/icon/icons/game_controller.svg").pixmap(dim, dim));
	icon_cp3->setPixmap(QIcon(":/icon/icons/game_controller.svg").pixmap(dim, dim));
	icon_cp4->setPixmap(QIcon(":/icon/icons/game_controller.svg").pixmap(dim, dim));
	icon_Input_misc->setPixmap(QIcon(":/icon/icons/misc.svg").pixmap(dim, dim));
	icon_Shortcuts->setPixmap(QIcon(":/icon/icons/shortcuts.svg").pixmap(dim, dim));
	icon_joy_ID->setPixmap(QIcon(":/icon/icons/input_config.svg").pixmap(dim, dim));

	mainwin->shcjoy_stop();

	QWidget::showEvent(event);
}
void wdgSettingsInput::hideEvent(QHideEvent *event) {
	shcut.timeout.timer->stop();
	shcut.joy.timer->stop();

#if defined (__linux__)
	if (shcut.joy.fd) {
		::close(shcut.joy.fd);
		shcut.joy.fd = 0;
	}
#endif

	mainwin->shcjoy_start();

	if (shcut.no_other_buttons == true) {
		shcut.timeout.seconds = 0;
		s_input_timeout();
	}

	QWidget::hideEvent(event);
}

void wdgSettingsInput::retranslateUi(QWidget *wdgSettingsInput) {
	Ui::wdgSettingsInput::retranslateUi(wdgSettingsInput);
	controller_ports_init();
	update_widget();
}
void wdgSettingsInput::update_widget(void) {
	ports_end_misc_set_enabled(true);

	controller_mode_set();
	expansion_port_set();
	controller_ports_set();
	shortcuts_set();

	checkBox_Permit_updown->setChecked(cfg->input.permit_updown_leftright);
	checkBox_Hide_Zapper_cursor->setChecked(cfg->input.hide_zapper_cursor);
}

void wdgSettingsInput::controller_ports_init(void) {
	// NES-001
	_cb_ports ctrl_mode_nes[] {
		{ tr("Disabled"),        CTRL_DISABLED },
		{ tr("Standard Pad"),    CTRL_STANDARD },
		{ tr("Zapper"),          CTRL_ZAPPER   },
		{ tr("Snes Mouse"),      CTRL_SNES_MOUSE },
		{ tr("Arkanoid Paddle"), CTRL_ARKANOID_PADDLE }
	};
	// Famicom
	_cb_ports ctrl_mode_famicom_expansion_port[] {
		{ tr("Standard Pads on Port3 and Port4"), CTRL_STANDARD },
		{ tr("Zapper"),                           CTRL_ZAPPER   },
		{ tr("Arkanoid Paddle"),                  CTRL_ARKANOID_PADDLE },
		{ tr("Oeka Kids Tablet"),                 CTRL_OEKA_KIDS_TABLET }
	};
	_cb_ports ctrl_mode_famicom_ports1[] {
		{ tr("Disabled"),        CTRL_DISABLED },
		{ tr("Standard Pad"),    CTRL_STANDARD },
		{ tr("Snes Mouse"),      CTRL_SNES_MOUSE }
	};
	_cb_ports ctrl_mode_famicom_ports2[] {
		{ tr("Disabled"),        CTRL_DISABLED },
		{ tr("Standard Pad"),    CTRL_STANDARD },
		{ tr("Snes Mouse"),      CTRL_SNES_MOUSE }
	};
	// Four Scoure
	_cb_ports ctrl_mode_four_score[] {
		{ tr("Disabled"),        CTRL_DISABLED },
		{ tr("Standard Pad"),    CTRL_STANDARD }
	};
	_cb_ports *ctrl_port1 = nullptr, *ctrl_port2 = nullptr;
	unsigned int i, length1 = 0, length2 = 0;

	switch (cfg->input.controller_mode) {
		case CTRL_MODE_NES:
			ctrl_port1 = (_cb_ports *)&ctrl_mode_nes;
			ctrl_port2 = (_cb_ports *)&ctrl_mode_nes;
			length1 = length2 = LENGTH(ctrl_mode_nes);
			break;
		case CTRL_MODE_FAMICOM:
			ctrl_port1 = (_cb_ports *)&ctrl_mode_famicom_ports1;
			ctrl_port2 = (_cb_ports *)&ctrl_mode_famicom_ports2;
			length1 = LENGTH(ctrl_mode_famicom_ports1);
			length2 = LENGTH(ctrl_mode_famicom_ports2);
			break;
		case CTRL_MODE_FOUR_SCORE:
			ctrl_port1 = (_cb_ports *)&ctrl_mode_four_score;
			ctrl_port2 = (_cb_ports *)&ctrl_mode_four_score;
			length1 = length2 = LENGTH(ctrl_mode_four_score);
			break;
	}

	// Expansion Port
	comboBox_exp->clear();
	for (i = 0; i < LENGTH(ctrl_mode_famicom_expansion_port); i++) {
		comboBox_exp->addItem(ctrl_mode_famicom_expansion_port[i].desc);
		comboBox_exp->setItemData(i, ctrl_mode_famicom_expansion_port[i].value);
	}

	// Ports
	controller_port_init(comboBox_cp1, &input.cport[0], ctrl_port1, length1);
	controller_port_init(comboBox_cp2, &input.cport[1], ctrl_port2, length2);
	controller_port_init(comboBox_cp3, &input.cport[2], (void *)ctrl_mode_four_score, LENGTH(ctrl_mode_four_score));
	controller_port_init(comboBox_cp4, &input.cport[3], (void *)ctrl_mode_four_score, LENGTH(ctrl_mode_four_score));
}
void wdgSettingsInput::controller_port_init(QComboBox *cb, _cfg_port *cfg_port, void *list, int length) {
	_cb_ports *cbp = (_cb_ports *) list;
	int i;

	cb->clear();

	for (i = 0; i < length; i++) {
		QList<QVariant> type;

		type.append(cbp[i].value);
		type.append(cfg_port->id - 1);
		type.append(QVariant::fromValue(((void *)cfg_port)));

		cb->addItem(cbp[i].desc);
		cb->setItemData(i, QVariant(type));
	}
}
void wdgSettingsInput::shortcuts_init(void) {
	QFont f9;

	f9.setPointSize(9);
	f9.setWeight(QFont::Light);

	shcut.joy.fd = 0;
	shcut.no_other_buttons = false;

	for (int a = 0; a < SET_MAX_NUM_SC; a++) {
		for (int b = 0; b < 2; b++) {
			shcut.text[b] << "";
		}
		shortcut_init(a + SET_INP_SC_OPEN, NULL);
	}

	shortcut_joy_id_init();

	shcut.bckColor = tableWidget_Shortcuts->item(0, 0)->background();

	if (lineEdit_Input_info->font().pointSize() > 9) {
		lineEdit_Input_info->setFont(f9);
	}

	shortcuts_update(UPDATE_ALL, NO_ACTION, NO_ACTION);
}
void wdgSettingsInput::shortcut_init(int index, QString *string) {
	int row = index - SET_INP_SC_OPEN;
	QTableWidgetItem *col;
	QHBoxLayout *layout;
	QWidget *widget;
	QPushButton *btext, *bicon;

	tableWidget_Shortcuts->insertRow(row);

	col = new QTableWidgetItem();
	col->setTextAlignment(Qt::AlignCenter);
	tableWidget_Shortcuts->setItem(row, 0, col);

	// keyboard
	if (string != NULL) {
		shcut.text[KEYBOARD].replace(row, (*string));
	} else {
		shcut.text[KEYBOARD].replace(row, (QString(*(QString *)settings_inp_rd_sc(index, KEYBOARD))));
	}
	widget = new QWidget(this);
	layout = new QHBoxLayout(widget);
	btext = new QPushButton(this);
	btext->setObjectName("value");
	btext->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
	btext->setProperty("myValue", QVariant(row));
	btext->setProperty("myType", QVariant(KEYBOARD));
	btext->installEventFilter(this);
	layout->addWidget(btext);
	connect(btext, SIGNAL(clicked(bool)), this, SLOT(s_shortcut(bool)));
	bicon = new QPushButton(this);
	bicon->setObjectName("default");
	bicon->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	bicon->setIcon(QIcon(":/icon/icons/default.svg"));
	bicon->setToolTip(tr("Default"));
	bicon->setProperty("myValue", QVariant(row));
	connect(bicon, SIGNAL(clicked(bool)), this, SLOT(s_shortcut_keyb_default(bool)));
	layout->addWidget(bicon);
	bicon = new QPushButton(this);
	bicon->setObjectName("unset");
	bicon->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	bicon->setIcon(QIcon(":/icon/icons/trash.svg"));
	bicon->setToolTip(tr("Unset"));
	bicon->setProperty("myValue", QVariant(row));
	connect(bicon, SIGNAL(clicked(bool)), this, SLOT(s_shortcut_keyb_unset(bool)));
	layout->addWidget(bicon);
	layout->setContentsMargins(0,0,0,0);
	layout->setSpacing(0);
	tableWidget_Shortcuts->setCellWidget(row, 1, widget);

	// joystick
	if (string != NULL) {
		shcut.text[JOYSTICK].replace(row, (*string));
	} else {
		shcut.text[JOYSTICK].replace(row, (QString(*(QString *)settings_inp_rd_sc(index, JOYSTICK))));
	}
	widget = new QWidget(this);
	layout = new QHBoxLayout(widget);
	btext = new QPushButton(this);
	btext->setObjectName("value");
	btext->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
	btext->setProperty("myValue", QVariant(row));
	btext->setProperty("myType", QVariant(JOYSTICK));
	btext->installEventFilter(this);
	connect(btext, SIGNAL(clicked(bool)), this, SLOT(s_shortcut(bool)));
	layout->addWidget(btext);
	bicon = new QPushButton(this);
	bicon->setObjectName("unset");
	bicon->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	bicon->setIcon(QIcon(":/icon/icons/trash.svg"));
	bicon->setToolTip(tr("Unset"));
	bicon->setProperty("myValue", QVariant(row));
	connect(bicon, SIGNAL(clicked(bool)), this, SLOT(s_shortcut_joy_unset(bool)));
	layout->addWidget(bicon);
	layout->setContentsMargins(0,0,0,0);
	layout->setSpacing(0);
	tableWidget_Shortcuts->setCellWidget(row, 2, widget);
}
void wdgSettingsInput::shortcut_joy_id_init(void) {
	BYTE disabled_line = 0, count = 0, current_line = name_to_jsn(uL("NULL"));
	int a;

	comboBox_joy_ID->clear();

	for (a = 0; a <= MAX_JOYSTICK; a++) {
		BYTE id = a;

		if (a < MAX_JOYSTICK) {
			if (js_is_connected(id) == EXIT_ERROR) {
				continue;
			}

			if (js_is_this(id, &cfg->input.shcjoy_id)) {
				current_line = count;
			}

			comboBox_joy_ID->addItem(QString("js%1: ").arg(id) + uQString(js_name_device(id)));
		} else {
			if (count == 0) {
				break;
			}
			comboBox_joy_ID->addItem("Disabled");
			id = name_to_jsn(uL("NULL"));
			disabled_line = count;
		}

		comboBox_joy_ID->setItemData(count, id);
		count++;
	}

	if (count == 0) {
		comboBox_joy_ID->addItem("No usable device");
	}

	if (count > 0) {
		if (js_is_null(&cfg->input.shcjoy_id) || (current_line == name_to_jsn(uL("NULL")))) {
			comboBox_joy_ID->setCurrentIndex(disabled_line);
		} else {
			comboBox_joy_ID->setCurrentIndex(current_line);
		}
		connect(comboBox_joy_ID, SIGNAL(activated(int)), this, SLOT(s_joy_id(int)));
	} else {
		comboBox_joy_ID->setCurrentIndex(0);
	}
}
void wdgSettingsInput::shortcut_update_text(QAction *action, int index) {
	QStringList text = action->text().remove('&').split('\t');
	int row = index - SET_INP_SC_OPEN;

	// action
#if defined (WITH_FFMPEG)
	if (index == SET_INP_SC_REC_AUDIO) {
		tableWidget_Shortcuts->item(row, 0)->setText(tr("Start/Stop AUDIO recording"));
		tableWidget_Shortcuts->item(row, 0)->setToolTip(tr("Start/Stop AUDIO recording"));
	} else if (index == SET_INP_SC_REC_VIDEO) {
		tableWidget_Shortcuts->item(row, 0)->setText(tr("Start/Stop VIDEO recording"));
		tableWidget_Shortcuts->item(row, 0)->setToolTip(tr("Start/Stop VIDEO recording"));
	} else {
		tableWidget_Shortcuts->item(row, 0)->setText(text.at(0));
		tableWidget_Shortcuts->item(row, 0)->setToolTip(text.at(0));
	}
#else
	if (index == SET_INP_SC_REC_AUDIO) {
		tableWidget_Shortcuts->item(row, 0)->setText(tr("Start/Stop WAV recording"));
		tableWidget_Shortcuts->item(row, 0)->setToolTip(tr("Start/Stop WAV recording"));
	} else {
		tableWidget_Shortcuts->item(row, 0)->setText(text.at(0));
		tableWidget_Shortcuts->item(row, 0)->setToolTip(text.at(0));
	}
#endif

	// keyboard
	tableWidget_Shortcuts->cellWidget(row, 1)->findChild<QPushButton *>("value")->setText(shcut.text[KEYBOARD].at(row));

	// joystick
	tableWidget_Shortcuts->cellWidget(row, 2)->findChild<QPushButton *>("value")->setText(shcut.text[JOYSTICK].at(row));
}
bool wdgSettingsInput::shortcut_keypressEvent(QKeyEvent *event) {
	if (shcut.no_other_buttons == false) {
		return (true);
	}

	if (shcut.type == KEYBOARD) {
		if ((event->key() == Qt::Key_Control) ||
			(event->key() == Qt::Key_Meta)  ||
			(event->key() == Qt::Key_Alt) ||
			(event->key() == Qt::Key_AltGr) ||
			(event->key() == Qt::Key_Shift)) {
			return (true);
		}

		if (event->key() != Qt::Key_Escape) {
			QString key = QKeySequence(event->key()).toString(QKeySequence::PortableText);

			switch (event->modifiers()) {
				case Qt::GroupSwitchModifier:
				case Qt::NoModifier:
				default:
					shcut.text[KEYBOARD].replace(shcut.row, key);
					break;
				case Qt::ControlModifier:
					shcut.text[KEYBOARD].replace(shcut.row, QString("Ctrl+%1").arg(key));
					break;
				case Qt::AltModifier:
					shcut.text[KEYBOARD].replace(shcut.row, QString("Alt+%1").arg(key));
					break;
				case Qt::MetaModifier:
					shcut.text[KEYBOARD].replace(shcut.row, QString("Meta+%1").arg(key));
					break;
			}
			settings_inp_wr_sc((void *)&shcut.text[KEYBOARD].at(shcut.row), shcut.row + SET_INP_SC_OPEN, KEYBOARD);
			mainwin->shortcuts();
		}
	} else {
		// quando sto configurando il joystick, l'unico input da tastiera che accetto e' l'escape
		if (event->key() != Qt::Key_Escape) {
			return (true);
		}
		shcut.joy.timer->stop();
	}

	shcut.timeout.timer->stop();
	input_info_print("");

	shortcuts_update(UPDATE_ALL, NO_ACTION, NO_ACTION);
	update_widget();

	shcut.no_other_buttons = false;

	return (true);
}
void wdgSettingsInput::shortcuts_update(int mode, int type, int row) {
	for (int i = 0; i < SET_MAX_NUM_SC; i++) {
		if (shcut.text[KEYBOARD].at(i + SET_INP_SC_OPEN).isEmpty()) {
			continue;
		}

		switch (mode) {
			case UPDATE_ALL: {
				QWidget *widget;
				bool joy_mode = false;
				BYTE joyId;

				if (comboBox_joy_ID->count() > 1) {
					joy_mode = true;
				}

				icon_joy_ID->setEnabled(joy_mode);
				label_joy_ID->setEnabled(joy_mode);
				comboBox_joy_ID->setEnabled(joy_mode);

				tableWidget_Shortcuts->item(i, 0)->setBackground(shcut.bckColor);

				widget = tableWidget_Shortcuts->cellWidget(i, 1);
				widget->setEnabled(true);
				widget->findChild<QPushButton *>("value")->setEnabled(true);
				widget->findChild<QPushButton *>("default")->setEnabled(true);

				tableWidget_Shortcuts->cellWidget(i, 1)->setEnabled(true);

				joyId = comboBox_joy_ID->itemData(comboBox_joy_ID->currentIndex()).toInt();

				if ((comboBox_joy_ID->count() > 1) && (joyId != name_to_jsn(uL("NULL")))) {
					joy_mode = true;
				} else {
					joy_mode = false;
				}

				widget = tableWidget_Shortcuts->cellWidget(i, 2);
				widget->setEnabled(joy_mode);
				widget->findChild<QPushButton *>("value")->setEnabled(joy_mode);
				widget->findChild<QPushButton *>("unset")->setEnabled(joy_mode);

				break;
			}
			case BUTTON_PRESSED: {
				QWidget *widget;
				BYTE joyId;

				ports_end_misc_set_enabled(false);

				icon_joy_ID->setEnabled(false);
				label_joy_ID->setEnabled(false);
				comboBox_joy_ID->setEnabled(false);

				if (row == i) {
					tableWidget_Shortcuts->item(i, 0)->setBackground(Qt::cyan);
				}

				if ((type == KEYBOARD) && (row == i)) {
					widget = tableWidget_Shortcuts->cellWidget(i, 1);
					widget->setEnabled(true);
					widget->findChild<QPushButton *>("default")->setEnabled(false);
				} else {
					tableWidget_Shortcuts->cellWidget(i, 1)->setEnabled(false);
				}

				joyId = comboBox_joy_ID->itemData(comboBox_joy_ID->currentIndex()).toInt();

				if ((comboBox_joy_ID->count() > 1) && (joyId != name_to_jsn(uL("NULL")))) {
					if ((type == JOYSTICK) && (row == i)) {
						widget = tableWidget_Shortcuts->cellWidget(i, 2);
						widget->setEnabled(true);
						widget->findChild<QPushButton *>("unset")->setEnabled(false);
					} else {
						tableWidget_Shortcuts->cellWidget(i, 2)->setEnabled(false);
					}
				}

				break;
			}
		}
	}
}
void wdgSettingsInput::shortcuts_tableview_resize(void) {
	QAction *p = new QAction(this);
	QString text = " Ctrl+Alt+Backspace ";
	int i, w, h;

	tableWidget_Shortcuts->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

	// creo uno shortucut fittizio per avere una width sufficientemente grande
	for (i = 0; i < 2; i++) {
		shcut.text[i] << "";
	}
	shortcut_init(SET_MAX_NUM_SC + SET_INP_SC_OPEN, &text);

	p->setText("test");
	shortcut_update_text(p, SET_MAX_NUM_SC);

	w = tableWidget_Shortcuts->verticalHeader()->width() + 2;
	for (i = 0; i < tableWidget_Shortcuts->columnCount(); i++) {
		w += tableWidget_Shortcuts->columnWidth(i);
	}
	w += tableWidget_Shortcuts->verticalScrollBar()->sizeHint().width() + 24;

	h = tableWidget_Shortcuts->horizontalHeader()->height() + 2;
	for (i = 0; i < 3; i++) {
		h += tableWidget_Shortcuts->rowHeight(i);
	}

	tableWidget_Shortcuts->setMinimumSize(w, h);
	tableWidget_Shortcuts->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	tableWidget_Shortcuts->removeRow(SET_MAX_NUM_SC);
}
void wdgSettingsInput::ports_end_misc_set_enabled(bool mode) {
	icon_cm->setEnabled(mode);
	icon_cp1->setEnabled(mode);
	icon_cp2->setEnabled(mode);
	icon_cp3->setEnabled(mode);
	icon_cp4->setEnabled(mode);
	icon_exp->setEnabled(mode);

	label_cm->setEnabled(mode);
	label_cp1->setEnabled(mode);
	label_cp2->setEnabled(mode);
	label_cp3->setEnabled(mode);
	label_cp4->setEnabled(mode);
	label_exp->setEnabled(mode);

	widget_cm->setEnabled(mode);
	comboBox_cp1->setEnabled(mode);
	comboBox_cp2->setEnabled(mode);
	comboBox_cp3->setEnabled(mode);
	comboBox_cp4->setEnabled(mode);
	comboBox_exp->setEnabled(mode);

	pushButton_cp1->setEnabled(mode);
	pushButton_cp2->setEnabled(mode);
	pushButton_cp3->setEnabled(mode);
	pushButton_cp4->setEnabled(mode);

	checkBox_Permit_updown->setEnabled(mode);
	checkBox_Hide_Zapper_cursor->setEnabled(mode);

	pushButton_Input_reset->setEnabled(mode);
}
void wdgSettingsInput::input_info_print(QString txt) {
	lineEdit_Input_info->setText(txt);
}

void wdgSettingsInput::controller_mode_set(void) {
	qtHelper::pushbutton_set_checked(pushButton_cm_nes, false);
	qtHelper::pushbutton_set_checked(pushButton_cm_famicom, false);
	qtHelper::pushbutton_set_checked(pushButton_cm_fscore, false);
	switch (cfg->input.controller_mode) {
		default:
		case CTRL_MODE_NES:
			qtHelper::pushbutton_set_checked(pushButton_cm_nes, true);
			break;
		case CTRL_MODE_FAMICOM:
			qtHelper::pushbutton_set_checked(pushButton_cm_famicom, true);
			break;
		case CTRL_MODE_FOUR_SCORE:
			qtHelper::pushbutton_set_checked(pushButton_cm_fscore, true);
			break;
	}
}
void wdgSettingsInput::expansion_port_set(void) {
	int index;

	for (index = 0; index < comboBox_exp->count(); index++) {
		if (cfg->input.expansion == comboBox_exp->itemData(index).toInt()) {
			comboBox_exp->setCurrentIndex(index);
		}
	}

	switch (cfg->input.controller_mode) {
		case CTRL_MODE_NES:
		case CTRL_MODE_FOUR_SCORE:
			comboBox_exp->setEnabled(false);
			break;
		case CTRL_MODE_FAMICOM:
			comboBox_exp->setEnabled(true);
			break;
	}
	icon_exp->setEnabled(comboBox_exp->isEnabled());
	label_exp->setEnabled(comboBox_exp->isEnabled());
}
void wdgSettingsInput::controller_ports_set(void) {
	int i, index;

	for (i = PORT1; i < PORT_MAX; i++) {
		_cfg_port *ctrl_in = &input.cport[i];
		QComboBox *cb = findChild<QComboBox *>(QString("comboBox_cp%1").arg(ctrl_in->id));
		QPushButton *pb = findChild<QPushButton *>(QString("pushButton_cp%1").arg(ctrl_in->id));
		bool mode = true, finded = false;

		for (index = 0; index < cb->count(); index++) {
			QList<QVariant> type = cb->itemData(index).toList();

			if (ctrl_in->port->type == type.at(0).toInt()) {
				finded = true;
				break;
			}
		}

		if (finded == false) {
			ctrl_in->port->type = CTRL_DISABLED;
			index = 0;
		}

		cb->setCurrentIndex(index);
		disconnect(pb, SIGNAL(clicked(bool)), this, SLOT(s_controller_port_setup(bool)));

		switch (ctrl_in->port->type) {
			case CTRL_DISABLED:
			case CTRL_ZAPPER:
			case CTRL_SNES_MOUSE:
			case CTRL_ARKANOID_PADDLE:
			default:
				pb->setEnabled(false);
				break;
			case CTRL_STANDARD:
				pb->setEnabled(true);
				pb->setProperty("myPointer", QVariant::fromValue(((void *) ctrl_in)));
				connect(pb, SIGNAL(clicked(bool)), this, SLOT(s_controller_port_setup(bool)));
				break;
		}

		if ((i >= PORT3) && (i <= PORT4)) {
			QLabel *ic = findChild<QLabel *>(QString("icon_cp%1").arg(ctrl_in->id));
			QLabel *lb = findChild<QLabel *>(QString("label_cp%1").arg(ctrl_in->id));

			switch (cfg->input.controller_mode) {
				case CTRL_MODE_NES:
					mode = false;
					break;
				case CTRL_MODE_FAMICOM:
					if (cfg->input.expansion != CTRL_STANDARD) {
						mode = false;
					}
					break;
				case CTRL_MODE_FOUR_SCORE:
					break;
			}

			ic->setEnabled(mode);
			lb->setEnabled(mode);
			cb->setEnabled(mode);

			if (mode == false) {
				pb->setEnabled(mode);
			}
		}
	}
}
void wdgSettingsInput::shortcuts_set(void) {
	if (comboBox_joy_ID->count() > 1) {
		comboBox_joy_ID->setItemText(comboBox_joy_ID->count() - 1, tr("Disabled"));
	} else {
		comboBox_joy_ID->setItemText(comboBox_joy_ID->count() - 1, tr("No usable device"));
	}

	shortcut_update_text(mainwin->action_Open, SET_INP_SC_OPEN);
	shortcut_update_text(mainwin->action_Quit, SET_INP_SC_QUIT);
	shortcut_update_text(mainwin->action_Turn_Off, SET_INP_SC_TURN_OFF);
	shortcut_update_text(mainwin->action_Hard_Reset, SET_INP_SC_HARD_RESET);
	shortcut_update_text(mainwin->action_Soft_Reset, SET_INP_SC_SOFT_RESET);
	shortcut_update_text(mainwin->action_Insert_Coin, SET_INP_SC_INSERT_COIN);
	shortcut_update_text(mainwin->action_Switch_sides, SET_INP_SC_SWITCH_SIDES);
	shortcut_update_text(mainwin->action_Eject_Insert_Disk, SET_INP_SC_EJECT_DISK);
	shortcut_update_text(mainwin->action_Start_Stop_Audio_recording, SET_INP_SC_REC_AUDIO);
#if defined (WITH_FFMPEG)
	shortcut_update_text(mainwin->action_Start_Stop_Video_recording, SET_INP_SC_REC_VIDEO);
#endif
	shortcut_update_text(mainwin->action_Fullscreen, SET_INP_SC_FULLSCREEN);
	shortcut_update_text(mainwin->action_Save_Screenshot, SET_INP_SC_SCREENSHOT);
	shortcut_update_text(mainwin->action_Save_Unaltered_NES_screen, SET_INP_SC_SCREENSHOT_1X);
	shortcut_update_text(mainwin->action_Pause, SET_INP_SC_PAUSE);
	shortcut_update_text(mainwin->action_Fast_Forward, SET_INP_SC_FAST_FORWARD);
	shortcut_update_text(mainwin->action_Toggle_GUI_in_window, SET_INP_SC_TOGGLE_GUI_IN_WINDOW);
	shortcut_update_text(mainwin->qaction_shcut.mode_auto, SET_INP_SC_MODE_AUTO);
	shortcut_update_text(mainwin->qaction_shcut.mode_ntsc, SET_INP_SC_MODE_NTSC);
	shortcut_update_text(mainwin->qaction_shcut.mode_pal, SET_INP_SC_MODE_PAL);
	shortcut_update_text(mainwin->qaction_shcut.mode_dendy, SET_INP_SC_MODE_DENDY);
	shortcut_update_text(mainwin->qaction_shcut.scale_1x, SET_INP_SC_SCALE_1X);
	shortcut_update_text(mainwin->qaction_shcut.scale_2x, SET_INP_SC_SCALE_2X);
	shortcut_update_text(mainwin->qaction_shcut.scale_3x, SET_INP_SC_SCALE_3X);
	shortcut_update_text(mainwin->qaction_shcut.scale_4x, SET_INP_SC_SCALE_4X);
	shortcut_update_text(mainwin->qaction_shcut.scale_5x, SET_INP_SC_SCALE_5X);
	shortcut_update_text(mainwin->qaction_shcut.scale_6x, SET_INP_SC_SCALE_6X);
	shortcut_update_text(mainwin->qaction_shcut.interpolation, SET_INP_SC_INTERPOLATION);
	shortcut_update_text(mainwin->qaction_shcut.integer_in_fullscreen, SET_INP_SC_INTEGER_FULLSCREEN);
	shortcut_update_text(mainwin->qaction_shcut.stretch_in_fullscreen, SET_INP_SC_STRETCH_FULLSCREEN);
	shortcut_update_text(mainwin->qaction_shcut.audio_enable, SET_INP_SC_AUDIO_ENABLE);
	shortcut_update_text(mainwin->qaction_shcut.save_settings, SET_INP_SC_SAVE_SETTINGS);
	shortcut_update_text(mainwin->action_Save_state, SET_INP_SC_SAVE_STATE);
	shortcut_update_text(mainwin->action_Load_state, SET_INP_SC_LOAD_STATE);
	shortcut_update_text(mainwin->action_Increment_slot, SET_INP_SC_INC_SLOT);
	shortcut_update_text(mainwin->action_Decrement_slot, SET_INP_SC_DEC_SLOT);
	shortcut_update_text(mainwin->qaction_shcut.rwnd.active, SET_INP_SC_RWND_ACTIVE_MODE);
	shortcut_update_text(mainwin->qaction_shcut.rwnd.step_backward, SET_INP_SC_RWND_STEP_BACKWARD);
	shortcut_update_text(mainwin->qaction_shcut.rwnd.step_forward, SET_INP_SC_RWND_STEP_FORWARD);
	shortcut_update_text(mainwin->qaction_shcut.rwnd.fast_backward, SET_INP_SC_RWND_FAST_BACKWARD);
	shortcut_update_text(mainwin->qaction_shcut.rwnd.fast_forward, SET_INP_SC_RWND_FAST_FORWARD);
	shortcut_update_text(mainwin->qaction_shcut.rwnd.play, SET_INP_SC_RWND_PLAY);
	shortcut_update_text(mainwin->qaction_shcut.rwnd.pause, SET_INP_SC_RWND_PAUSE);
}

void wdgSettingsInput::s_controller_mode(bool checked) {
	if (checked) {
		int mode = QVariant(((QPushButton *)sender())->property("mtype")).toInt();

		if (cfg->input.controller_mode == mode) {
			return;
		}

		emu_thread_pause();
		cfg->input.controller_mode = mode;
		controller_ports_init();
		input_init(SET_CURSOR);
		emu_thread_continue();
	}
	update_widget();
}
void wdgSettingsInput::s_expansion_port(int index) {
	int type = comboBox_exp->itemData(index).toInt();

	emu_thread_pause();
	cfg->input.expansion = type;
	controller_ports_init();
	input_init(SET_CURSOR);
	emu_thread_continue();
	update_widget();
}
void wdgSettingsInput::s_controller_port(int index) {
	QList<QVariant> type = ((QComboBox *)sender())->itemData(index).toList();
	_cfg_port *cfg_port = ((_cfg_port *)type.at(2).value<void *>());

	emu_thread_pause();
	cfg_port->port->type = type.at(0).toInt();
	input_init(SET_CURSOR);
	emu_thread_continue();
	update_widget();
}
void wdgSettingsInput::s_controller_port_setup(UNUSED(bool checked)) {
	_cfg_port *cfg_port = ((_cfg_port *)((QPushButton *)sender())->property("myPointer").value<void *>());

	switch (cfg_port->port->type) {
		case CTRL_DISABLED:
		case CTRL_ZAPPER:
			break;
		case CTRL_STANDARD:
			dlgStdPad *dlg = new dlgStdPad(cfg_port, this);

			hide_from_setup_button = true;
			dlgsettings->hide();
			dlg->exec();
			dlgsettings->show();
			hide_from_setup_button = false;

			shortcut_joy_id_init();
			shortcuts_update(UPDATE_ALL, NO_ACTION, NO_ACTION);
			update_widget();
			break;
	}
}
void wdgSettingsInput::s_input_reset(UNUSED(bool checked)) {
	_array_pointers_port array;

	for (int i = PORT1; i < PORT_MAX; i++) {
		array.port[i] = input.cport[i].port;
	}

	settings_inp_all_default(&cfg->input, &array);
	update_widget();
}
void wdgSettingsInput::s_permit_updown_leftright(UNUSED(bool checked)) {
	cfg->input.permit_updown_leftright = !cfg->input.permit_updown_leftright;
}
void wdgSettingsInput::s_hide_zapper_cursor(UNUSED(bool checked)) {
	cfg->input.hide_zapper_cursor = !cfg->input.hide_zapper_cursor;
}
void wdgSettingsInput::s_joy_id(int index) {
	unsigned int id = ((QComboBox *)sender())->itemData(index).toInt();

	js_set_id(&cfg->input.shcjoy_id, id);
	shortcuts_update(UPDATE_ALL, NO_ACTION, NO_ACTION);
}
void wdgSettingsInput::s_shortcut(UNUSED(bool checked)) {
	if (shcut.no_other_buttons == true) {
		return;
	}

	shcut.type = QVariant(((QObject *)sender())->property("myType")).toInt();
	shcut.row = QVariant(((QObject *)sender())->property("myValue")).toInt();
	shcut.bp = ((QPushButton *)sender());

	shortcuts_update(BUTTON_PRESSED, shcut.type, shcut.row);

	shcut.no_other_buttons = true;
	shcut.bp->setText("...");

	shcut.bp->setFocus(Qt::ActiveWindowFocusReason);

	shcut.timeout.seconds = 5;
	shcut.timeout.timer->start(1000);
	s_input_timeout();

	if (shcut.type == JOYSTICK) {
#if defined (__linux__)
		_js_event jse;
		ssize_t size = sizeof(jse);
		char device[30];

		::sprintf(device, "%s%d", JS_DEV_PATH, cfg->input.shcjoy_id);
		shcut.joy.fd = ::open(device, O_RDONLY | O_NONBLOCK);

		if (shcut.joy.fd < 0) {
			input_info_print(tr("Error on open device %1").arg(device));
			update_widget();
			return;
		}

		for (int i = 0; i < MAX_JOYSTICK; i++) {
			if (::read(shcut.joy.fd, &jse, size) < 0) {
				input_info_print(tr("Error on reading controllers configurations"));
			}
		}

		// svuoto il buffer iniziale
		for (int i = 0; i < 10; i++) {
			if (::read(shcut.joy.fd, &jse, size) < 0) {
				;
			}
		}
		shcut.joy.value = 0;
		shcut.joy.timer->start(30);
#else
		shcut.joy.value = 0;
		shcut.joy.timer->start(150);
#endif
	}
}
void wdgSettingsInput::s_shortcut_unset_all(UNUSED(bool checked)) {
	for (int i = 0; i < SET_MAX_NUM_SC; i++) {
		shcut.text[KEYBOARD].replace(i, "NULL");
		settings_inp_wr_sc((void *)&shcut.text[KEYBOARD].at(i), i + SET_INP_SC_OPEN, KEYBOARD);
		tableWidget_Shortcuts->cellWidget(i, 1)->findChild<QPushButton *>("value")->setText(shcut.text[KEYBOARD].at(i));

		shcut.text[JOYSTICK].replace(i, "NULL");
		settings_inp_wr_sc((void *)&shcut.text[JOYSTICK].at(i), i + SET_INP_SC_OPEN, JOYSTICK);
		tableWidget_Shortcuts->cellWidget(i, 2)->findChild<QPushButton *>("value")->setText(shcut.text[JOYSTICK].at(i));
	}
	mainwin->shortcuts();
	shortcuts_update(UPDATE_ALL, NO_ACTION, NO_ACTION);
}
void wdgSettingsInput::s_shortcut_reset(UNUSED(bool checked)) {
	js_set_id(&cfg->input.shcjoy_id, name_to_jsn(uL("NULL")));

	comboBox_joy_ID->setCurrentIndex(comboBox_joy_ID->count() - 1);

	for (int i = 0; i < SET_MAX_NUM_SC; i++) {
		shcut.text[KEYBOARD].replace(i, uQString(inp_cfg[i + SET_INP_SC_OPEN].def).split(",").at(KEYBOARD));
		settings_inp_wr_sc((void *)&shcut.text[KEYBOARD].at(i), i + SET_INP_SC_OPEN, KEYBOARD);
		tableWidget_Shortcuts->cellWidget(i, 1)->findChild<QPushButton *>("value")->setText(shcut.text[KEYBOARD].at(i));

		shcut.text[JOYSTICK].replace(i, uQString(inp_cfg[i + SET_INP_SC_OPEN].def).split(",").at(JOYSTICK));
		settings_inp_wr_sc((void *)&shcut.text[JOYSTICK].at(i), i + SET_INP_SC_OPEN, JOYSTICK);
		tableWidget_Shortcuts->cellWidget(i, 2)->findChild<QPushButton *>("value")->setText(shcut.text[JOYSTICK].at(i));
	}
	mainwin->shortcuts();
	shortcuts_update(UPDATE_ALL, NO_ACTION, NO_ACTION);
}
void wdgSettingsInput::s_shortcut_keyb_default(UNUSED(bool checked)) {
	int row = QVariant(((QObject *)sender())->property("myValue")).toInt();

	shcut.text[KEYBOARD].replace(row, uQString(inp_cfg[row + SET_INP_SC_OPEN].def).split(",").at(KEYBOARD));
	tableWidget_Shortcuts->cellWidget(row, 1)->findChild<QPushButton *>("value")->setText(shcut.text[KEYBOARD].at(row));
	settings_inp_wr_sc((void *)&shcut.text[KEYBOARD].at(row), row + SET_INP_SC_OPEN, KEYBOARD);
	mainwin->shortcuts();
}
void wdgSettingsInput::s_shortcut_keyb_unset(UNUSED(bool checked)) {
	int row = QVariant(((QObject *)sender())->property("myValue")).toInt();

	shcut.text[KEYBOARD].replace(row, "NULL");
	tableWidget_Shortcuts->cellWidget(row, 1)->findChild<QPushButton *>("value")->setText("NULL");
	settings_inp_wr_sc((void *)&shcut.text[KEYBOARD].at(row), row + SET_INP_SC_OPEN, KEYBOARD);
	mainwin->shortcuts();
}
void wdgSettingsInput::s_shortcut_joy_unset(UNUSED(bool checked)) {
	int row = QVariant(((QObject *)sender())->property("myValue")).toInt();

	shcut.text[JOYSTICK].replace(row, "NULL");
	tableWidget_Shortcuts->cellWidget(row, 2)->findChild<QPushButton *>("value")->setText("NULL");
	settings_inp_wr_sc((void *)&shcut.text[JOYSTICK].at(row), row + SET_INP_SC_OPEN, JOYSTICK);
}
void wdgSettingsInput::s_input_timeout(void) {
	input_info_print(tr("Press a key (ESC for the previous value \"%1\") - timeout in %2").arg(shcut.text[shcut.type].at(shcut.row),
		QString::number(shcut.timeout.seconds--)));

	if (shcut.timeout.seconds < 0) {
		QKeyEvent *event = new QKeyEvent(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);

		QCoreApplication::postEvent(shcut.bp, event);
	}
}
void wdgSettingsInput::s_joy_read_timer(void) {
	DBWORD value = js_read_in_dialog(&cfg->input.shcjoy_id, shcut.joy.fd);

	if (shcut.joy.value && !value) {
#if defined (__linux__)
		::close(shcut.joy.fd);
		shcut.joy.fd = 0;
#endif
		shcut.text[JOYSTICK].replace(shcut.row, uQString(jsv_to_name(shcut.joy.value)));
		settings_inp_wr_sc((void *)&shcut.text[JOYSTICK].at(shcut.row), shcut.row + SET_INP_SC_OPEN, JOYSTICK);

		shcut.timeout.timer->stop();
		shcut.joy.timer->stop();
		input_info_print("");

		shortcuts_update(UPDATE_ALL, NO_ACTION, NO_ACTION);
		update_widget();

		shcut.no_other_buttons = false;
	}

	shcut.joy.value = value;
}
