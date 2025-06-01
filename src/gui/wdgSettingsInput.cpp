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

#include <QtWidgets/QScrollBar>
#include "wdgSettingsInput.hpp"
#include "mainWindow.hpp"
#include "dlgSettings.hpp"
#include "dlgStdPad.hpp"
#include "dlgKeyboard.hpp"
#include "emu_thread.h"
#include "conf.h"

enum page_input_shcut_mode { UPDATE_ALL, BUTTON_PRESSED, NO_ACTION = 255 };

// ----------------------------- keySequenceEdit ---------------------------------------

bool keySequenceEdit::event(QEvent *event) {
	// comsume ALL key presses including Tab
	if (event->type() == QEvent::KeyPress) {
		keyPressEvent(static_cast<QKeyEvent *>(event));
		return (true);
	}
	return (QKeySequenceEdit::event(event));
}

// ---------------------------- wdgSettingsInput --------------------------------------

wdgSettingsInput::wdgSettingsInput(QWidget *parent) : QWidget(parent) {
	int i = 0;

	hide_from_setup_button = false;
	last_control = gui_get_ms();
	wdg_dlg_std_pad = nullptr;

	setupUi(this);

	setFocusProxy(tabWidget_Input);

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

	connect(comboBox_joy_ID, SIGNAL(activated(int)), this, SLOT(s_joy_id(int)));
	connect(comboBox_joy_ID, SIGNAL(currentIndexChanged(int)), this, SLOT(s_joy_index_changed(int)));

	connect(pushButton_Shortcut_unset_all, SIGNAL(clicked(bool)), this, SLOT(s_shortcut_unset_all(bool)));
	connect(pushButton_Shortcut_reset, SIGNAL(clicked(bool)), this, SLOT(s_shortcut_reset(bool)));

	shcut.timeout.timer = new QTimer(this);
	connect(shcut.timeout.timer, SIGNAL(timeout()), this, SLOT(s_input_timeout()));

	shcut.joy.timer = new QTimer(this);
	connect(shcut.joy.timer, SIGNAL(timeout()), this, SLOT(s_joy_read_timer()));

	shortcuts_tableview_resize();

	connect(this, SIGNAL(et_update_joy_combo()), this, SLOT(s_et_update_joy_combo()));

	tabWidget_Input->setCurrentIndex(0);

	{
		int dim = fontMetrics().height();

		icon_Ports->setPixmap(QIcon(":/icon/icons/rs_232_female.svgz").pixmap(dim, dim));
		icon_cm->setPixmap(QIcon(":/icon/icons/mode.svgz").pixmap(dim, dim));
		icon_exp->setPixmap(QIcon(":/icon/icons/circuit_board.svgz").pixmap(dim, dim));
		icon_cp1->setPixmap(QIcon(":/icon/icons/game_controller.svgz").pixmap(dim, dim));
		icon_cp2->setPixmap(QIcon(":/icon/icons/game_controller.svgz").pixmap(dim, dim));
		icon_cp3->setPixmap(QIcon(":/icon/icons/game_controller.svgz").pixmap(dim, dim));
		icon_cp4->setPixmap(QIcon(":/icon/icons/game_controller.svgz").pixmap(dim, dim));
		icon_Input_misc->setPixmap(QIcon(":/icon/icons/misc.svgz").pixmap(dim, dim));
		icon_Shortcuts->setPixmap(QIcon(":/icon/icons/shortcuts.svgz").pixmap(dim, dim));
		icon_joy_ID->setPixmap(QIcon(":/icon/icons/input_config.svgz").pixmap(dim, dim));
	}
}
wdgSettingsInput::~wdgSettingsInput() = default;

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
	mainwin->wd->shcjoy_stop();
	QWidget::showEvent(event);
}
void wdgSettingsInput::hideEvent(QHideEvent *event) {
	shcut.timeout.timer->stop();
	shcut.joy.timer->stop();

	mainwin->wd->shcjoy_start();

	if (shcut.no_other_buttons) {
		shcut.timeout.seconds = 0;
		s_input_timeout();
	}

	QWidget::hideEvent(event);
}

void wdgSettingsInput::retranslateUi(QWidget *wdgSettingsInput) {
	Ui::wdgSettingsInput::retranslateUi(wdgSettingsInput);
	controller_ports_init();
	update_widget();
	adjustSize();
	updateGeometry();
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
void wdgSettingsInput::update_joy_list(void) {
	double this_control = gui_get_ms();

	if ((this_control - last_control) >= JS_MS_UPDATE_DETECT_DEVICE_DLG) {
		last_control = this_control;
		shortcut_joy_list_init();
		emit et_update_joy_combo();
	}
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
		{ tr("Oeka Kids Tablet"),                 CTRL_OEKA_KIDS_TABLET },
		{ tr("Family BASIC Keyboard"),            CTRL_FAMILY_BASIC_KEYBOARD },
		{ tr("Subor Keyboard"),                   CTRL_SUBOR_KEYBOARD }
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
		comboBox_exp->setItemData((int)i, ctrl_mode_famicom_expansion_port[i].index);
	}

	// Ports
	controller_port_init(comboBox_cp1, &input.cport[0], ctrl_port1, (int)length1);
	controller_port_init(comboBox_cp2, &input.cport[1], ctrl_port2, (int)length2);
	controller_port_init(comboBox_cp3, &input.cport[2], (void *)ctrl_mode_four_score, LENGTH(ctrl_mode_four_score));
	controller_port_init(comboBox_cp4, &input.cport[3], (void *)ctrl_mode_four_score, LENGTH(ctrl_mode_four_score));
}
void wdgSettingsInput::controller_port_init(QComboBox *cb, _cfg_port *cfg_port, void *list, int length) {
	_cb_ports *cbp = (_cb_ports *)list;
	bool found = false;
	int i;

	cb->clear();

	for (i = 0; i < length; i++) {
		QList<QVariant> type;

		type.append(cbp[i].index);
		type.append(cfg_port->id - 1);
		type.append(QVariant::fromValue(((void *)cfg_port)));

		cb->addItem(cbp[i].desc);
		cb->setItemData(i, QVariant(type));

		if (cbp[i].index == cfg_port->port->type) {
			found = true;
		}
	}
	if (!found) {
		cfg_port->port->type = cbp[0].index;
	}
}
void wdgSettingsInput::shortcuts_init(void) {
	QFont f9;

	f9.setPointSize(9);
	f9.setWeight(QFont::Light);

	shcut.no_other_buttons = false;

	for (int a = 0; a < SET_MAX_NUM_SC; a++) {
		for (QStringList &b : shcut.text) {
			b << "";
		}
		shortcut_init(a + SET_INP_SC_OPEN, nullptr);
	}

	shortcut_joy_list_init();
	shortcut_joy_combo_init();

	shcut.fg = tableWidget_Shortcuts->item(0, 0)->foreground();
	shcut.bg = tableWidget_Shortcuts->item(0, 0)->background();

	if (label_Input_info->font().pointSize() > 9) {
		label_Input_info->setFont(f9);
	}

	shortcuts_update(UPDATE_ALL, NO_ACTION, NO_ACTION);
}
void wdgSettingsInput::shortcut_init(int index, QString *string) {
	int row = index - SET_INP_SC_OPEN;
	QTableWidgetItem *col;
	QHBoxLayout *layout;
	QWidget *widget;
	themePushButton *bicon;
	QFontMetrics fm = tableWidget_Shortcuts->fontMetrics();
	int icon_size = 21;
	int btexth = fm.boundingRect("Ppj0q").height() < (icon_size + 6) ? icon_size + 6 : fm.boundingRect("Ppj0q").height();

	tableWidget_Shortcuts->insertRow(row);

	col = new QTableWidgetItem();
	col->setTextAlignment(Qt::AlignCenter);
	tableWidget_Shortcuts->setItem(row, 0, col);

	// keyboard
	if (string != nullptr) {
		shcut.text[KEYBOARD].replace(row, (*string));
	} else {
		shcut.text[KEYBOARD].replace(row, (QString(*(QString *)settings_inp_rd_sc(index, KEYBOARD))));
	}
	widget = new QWidget(this);
	layout = new QHBoxLayout(widget);
	{
		QKeySequenceEdit *btext;

		btext = new keySequenceEdit();
		btext->setObjectName("value");
		btext->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
		btext->setMinimumHeight(btexth);
		btext->setProperty("myValue", QVariant(row));
		btext->setProperty("myType", QVariant(KEYBOARD));
		connect(btext, SIGNAL(editingFinished()), this, SLOT(s_shortcut_keyb()));
		layout->addWidget(btext);
	}
	bicon = new themePushButton(this);
	bicon->setObjectName("default");
	bicon->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
	bicon->setIcon(QIcon(":/icon/icons/default.svgz"));
	bicon->setToolTip(tr("Default"));
	bicon->setProperty("myValue", QVariant(row));
	connect(bicon, SIGNAL(clicked(bool)), this, SLOT(s_shortcut_keyb_default(bool)));
	layout->addWidget(bicon);
	bicon = new themePushButton(this);
	bicon->setObjectName("unset");
	bicon->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
	bicon->setIcon(QIcon(":/icon/icons/trash.svgz"));
	bicon->setToolTip(tr("Unset"));
	bicon->setProperty("myValue", QVariant(row));
	connect(bicon, SIGNAL(clicked(bool)), this, SLOT(s_shortcut_keyb_unset(bool)));
	layout->addWidget(bicon);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(0);
	tableWidget_Shortcuts->setCellWidget(row, 1, widget);

	// joystick
	if (string != nullptr) {
		shcut.text[JOYSTICK].replace(row, (*string));
	} else {
		shcut.text[JOYSTICK].replace(row, (QString(*(QString *)settings_inp_rd_sc(index, JOYSTICK))));
	}
	widget = new QWidget(this);
	layout = new QHBoxLayout(widget);
	{
		pixmapPushButton *btext;

		btext = new pixmapPushButton(this);
		btext->setObjectName("value");
		btext->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
		btext->setIconSize(QSize(21, 21));
		btext->setMinimumHeight(btexth);
		btext->setProperty("myValue", QVariant(row));
		btext->setProperty("myType", QVariant(JOYSTICK));
		btext->installEventFilter(this);
		connect(btext, SIGNAL(clicked(bool)), this, SLOT(s_shortcut_joy(bool)));
		layout->addWidget(btext);
	}
	bicon = new themePushButton(this);
	bicon->setObjectName("unset");
	bicon->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
	bicon->setIcon(QIcon(":/icon/icons/trash.svgz"));
	bicon->setToolTip(tr("Unset"));
	bicon->setProperty("myValue", QVariant(row));
	connect(bicon, SIGNAL(clicked(bool)), this, SLOT(s_shortcut_joy_unset(bool)));
	layout->addWidget(bicon);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(0);
	tableWidget_Shortcuts->setCellWidget(row, 2, widget);
}
void wdgSettingsInput::shortcut_joy_list_init(void) {
	int i;

	for (i = 0; i <= MAX_JOYSTICK; i++) {
		_cb_ports *cb = &joy_list.ele[i];

		cb->desc = "";
		cb->index = 0;
	}
	joy_list.count = 0;
	joy_list.disabled_line = 0;

	for (i = 0; i <= MAX_JOYSTICK; i++) {
		_cb_ports *cb = &joy_list.ele[joy_list.count];
		BYTE index = i;

		if (i < MAX_JOYSTICK) {
			if (!js_is_connected(index)) {
				continue;
			}
			cb->desc = QString("js%1: ").arg(index) + uQString(js_jdev_desc(index));
		} else {
			if (joy_list.count == 0) {
				break;
			}
			cb->desc = tr("Disabled");
			index = JS_NO_JOYSTICK;
			joy_list.disabled_line = joy_list.count;
		}
		cb->index = index;
		joy_list.count++;
	}

	if (joy_list.count == 0) {
		joy_list.ele[0].desc = tr("No usable device");
		joy_list.count++;
	}
}
void wdgSettingsInput::shortcut_joy_combo_init(void) {
	int i, last_index = comboBox_joy_ID->currentIndex();
	QString last_text = comboBox_joy_ID->itemText(last_index);
	BYTE current_index = 0, current_line = JS_NO_JOYSTICK;

	comboBox_joy_ID->blockSignals(true);
	comboBox_joy_ID->clear();

	for (i = 0; i < joy_list.count; i++) {
		_cb_ports *cb = &joy_list.ele[i];

		if (js_is_this(cb->index, &cfg->input.jguid_sch)) {
			current_line = i;
		}
		comboBox_joy_ID->addItem(cb->desc);
		comboBox_joy_ID->setItemData(i, cb->index);
	}

	if (joy_list.count > 1) {
		if (js_is_null(&cfg->input.jguid_sch) || (current_line == JS_NO_JOYSTICK)) {
			current_index = joy_list.disabled_line;
		} else {
			current_index = current_line;
		}
	}

	comboBox_joy_ID->blockSignals(false);

	comboBox_joy_ID->setCurrentIndex(current_index);

	if ((last_index == current_index) && (last_text != comboBox_joy_ID->itemText(last_index))){
		s_joy_index_changed(0);
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
	tableWidget_Shortcuts->cellWidget(row, 1)->findChild<QKeySequenceEdit *>("value")->setKeySequence(shcut.text[KEYBOARD].at(row));

	// joystick
	js_row_pixmapPushButton(row);
}
bool wdgSettingsInput::shortcut_keypressEvent(QKeyEvent *event) {
	if (!shcut.no_other_buttons) {
		return (true);
	}

	if (shcut.type == JOYSTICK) {
		if ((event->key() != Qt::Key_Escape) || (event->modifiers() != Qt::MetaModifier)) {
			return (true);
		}
		shcut.joy.timer->stop();
	}

	shcut.timeout.timer->stop();
	info_entry_print("");

	shortcuts_update(UPDATE_ALL, NO_ACTION, NO_ACTION);
	update_widget();

	shcut.no_other_buttons = false;

	return (true);
}
void wdgSettingsInput::shortcuts_update(int mode, int type, int row) {
	int i;

	for (i = 0; i < SET_MAX_NUM_SC; i++) {
		if (shcut.text[KEYBOARD].at(i + SET_INP_SC_OPEN).isEmpty()) {
			continue;
		}

		switch (mode) {
			case UPDATE_ALL: {
				QWidget *widget;
				bool joy_mode = false;
				BYTE joy_index;

				if (comboBox_joy_ID->count() > 1) {
					joy_mode = true;
				}

				icon_joy_ID->setEnabled(joy_mode);
				label_joy_ID->setEnabled(joy_mode);
				comboBox_joy_ID->setEnabled(joy_mode);

				tableWidget_Shortcuts->item(i, 0)->setForeground(shcut.fg);
				tableWidget_Shortcuts->item(i, 0)->setBackground(shcut.bg);

				widget = tableWidget_Shortcuts->cellWidget(i, 1);
				widget->setEnabled(true);
				widget->findChild<QKeySequenceEdit *>("value")->setEnabled(true);
				widget->findChild<themePushButton *>("default")->setEnabled(true);

				tableWidget_Shortcuts->cellWidget(i, 1)->setEnabled(true);

				joy_index = comboBox_joy_ID->itemData(comboBox_joy_ID->currentIndex()).toInt();

				if ((comboBox_joy_ID->count() > 1) && (joy_index != JS_NO_JOYSTICK)) {
					joy_mode = true;
				} else {
					joy_mode = false;
				}

				widget = tableWidget_Shortcuts->cellWidget(i, 2);
				widget->setEnabled(joy_mode);
				widget->findChild<pixmapPushButton *>("value")->setEnabled(joy_mode);
				widget->findChild<themePushButton *>("unset")->setEnabled(joy_mode);

				break;
			}
			case BUTTON_PRESSED: {
				QWidget *widget;
				BYTE joy_index;

				ports_end_misc_set_enabled(false);

				icon_joy_ID->setEnabled(false);
				label_joy_ID->setEnabled(false);
				comboBox_joy_ID->setEnabled(false);

				if (row == i) {
					QColor color = theme::get_grayed_color(Qt::cyan);

					tableWidget_Shortcuts->item(i, 0)->setForeground(theme::get_foreground_color(color));
					tableWidget_Shortcuts->item(i, 0)->setBackground(color);
				}

				if ((type == KEYBOARD) && (row == i)) {
					widget = tableWidget_Shortcuts->cellWidget(i, 1);
					widget->setEnabled(true);
					widget->findChild<themePushButton *>("default")->setEnabled(false);
				} else {
					tableWidget_Shortcuts->cellWidget(i, 1)->setEnabled(false);
				}

				joy_index = comboBox_joy_ID->itemData(comboBox_joy_ID->currentIndex()).toInt();

				if ((comboBox_joy_ID->count() > 1) && (joy_index != JS_NO_JOYSTICK)) {
					if ((type == JOYSTICK) && (row == i)) {
						widget = tableWidget_Shortcuts->cellWidget(i, 2);
						widget->setEnabled(true);
						widget->findChild<themePushButton *>("unset")->setEnabled(false);
					} else {
						tableWidget_Shortcuts->cellWidget(i, 2)->setEnabled(false);
					}
				}

				break;
			}
			default:
				break;
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

	pushButton_ep->setEnabled(mode);
	pushButton_cp1->setEnabled(mode);
	pushButton_cp2->setEnabled(mode);
	pushButton_cp3->setEnabled(mode);
	pushButton_cp4->setEnabled(mode);

	checkBox_Permit_updown->setEnabled(mode);
	checkBox_Hide_Zapper_cursor->setEnabled(mode);

	pushButton_Input_reset->setEnabled(mode);
}
void wdgSettingsInput::info_entry_print(const QString &txt) {
	label_Input_info->setText(txt);
}
void wdgSettingsInput::js_row_pixmapPushButton(int row) {
	pixmapPushButton *pbt = tableWidget_Shortcuts->cellWidget(row, 2)->findChild<pixmapPushButton *>("value");

	js_pixmapPushButton(js_jdev_index(), js_joyval_from_name(uQStringCD(shcut.text[JOYSTICK].at(row))), pbt);
}
void wdgSettingsInput::js_pixmapPushButton(int index, DBWORD in, pixmapPushButton *bt) {
	QString icon, desc;

	gui_js_joyval_icon_desc(index, in, &icon, &desc);
	bt->setIcon(QIcon());
	bt->setPixmap(QPixmap(icon));
	bt->setText(desc == "" ? "NULL" : desc);
}
int wdgSettingsInput::js_jdev_index(void) {
	int jdev_index = JS_NO_JOYSTICK;

	if (comboBox_joy_ID->currentData().isValid()) {
		jdev_index = comboBox_joy_ID->currentData().toInt();
	}
	return (jdev_index);
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

	disconnect(pushButton_ep, SIGNAL(clicked(bool)), this, SLOT(s_expansion_port_setup(bool)));

	switch (cfg->input.expansion) {
		case CTRL_FAMILY_BASIC_KEYBOARD:
		case CTRL_SUBOR_KEYBOARD:
			pushButton_ep->setEnabled(comboBox_exp->isEnabled());
			connect(pushButton_ep, SIGNAL(clicked(bool)), this, SLOT(s_expansion_port_setup(bool)));
			break;
		default:
			pushButton_ep->setEnabled(false);
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
		themePushButton *pb = findChild<themePushButton *>(QString("pushButton_cp%1").arg(ctrl_in->id));
		bool mode = true, finded = false;

		for (index = 0; index < cb->count(); index++) {
			QList<QVariant> type = cb->itemData(index).toList();

			if (ctrl_in->port->type == type.at(0).toInt()) {
				finded = true;
				break;
			}
		}

		if (!finded) {
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
				pb->setProperty("myPointer", QVariant::fromValue(((void *)ctrl_in)));
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

			if (!mode) {
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

	shortcut_update_text(mainwin->wd->action_Open, SET_INP_SC_OPEN);
	shortcut_update_text(mainwin->wd->action_Start_Stop_Audio_recording, SET_INP_SC_REC_AUDIO);
#if defined (WITH_FFMPEG)
	shortcut_update_text(mainwin->wd->action_Start_Stop_Video_recording, SET_INP_SC_REC_VIDEO);
#endif
	shortcut_update_text(mainwin->wd->action_Quit, SET_INP_SC_QUIT);
	shortcut_update_text(mainwin->wd->action_Turn_Off, SET_INP_SC_TURN_OFF);
	shortcut_update_text(mainwin->wd->action_Hard_Reset, SET_INP_SC_HARD_RESET);
	shortcut_update_text(mainwin->wd->action_Soft_Reset, SET_INP_SC_SOFT_RESET);
	shortcut_update_text(mainwin->wd->action_Insert_Coin, SET_INP_SC_INSERT_COIN);
	shortcut_update_text(mainwin->wd->action_Switch_sides, SET_INP_SC_SWITCH_SIDES);
	shortcut_update_text(mainwin->wd->action_Eject_Insert_Disk, SET_INP_SC_EJECT_DISK);
	shortcut_update_text(mainwin->wd->action_Change_Disk, SET_INP_SC_CHANGE_DISK);
	shortcut_update_text(mainwin->wd->action_Fullscreen, SET_INP_SC_FULLSCREEN);
	shortcut_update_text(mainwin->wd->action_Save_Screenshot, SET_INP_SC_SCREENSHOT);
	shortcut_update_text(mainwin->wd->action_Save_Unaltered_NES_screen, SET_INP_SC_SCREENSHOT_1X);
	shortcut_update_text(mainwin->wd->action_Pause, SET_INP_SC_PAUSE);
	shortcut_update_text(mainwin->wd->action_Toogle_Fast_Forward, SET_INP_SC_TOGGLE_FAST_FORWARD);
	shortcut_update_text(mainwin->wd->action_Toggle_GUI_in_window, SET_INP_SC_TOGGLE_GUI_IN_WINDOW);
	shortcut_update_text(mainwin->wd->action_Shout_into_Microphone, SET_INP_SC_SHOUT_INTO_MIC);
	shortcut_update_text(mainwin->wd->action_Virtual_Keyboard, SET_INP_SC_TOGGLE_NES_KEYBOARD);

	shortcut_update_text(mainwin->wd->qaction_shcut.mode_auto, SET_INP_SC_MODE_AUTO);
	shortcut_update_text(mainwin->wd->qaction_shcut.mode_ntsc, SET_INP_SC_MODE_NTSC);
	shortcut_update_text(mainwin->wd->qaction_shcut.mode_pal, SET_INP_SC_MODE_PAL);
	shortcut_update_text(mainwin->wd->qaction_shcut.mode_dendy, SET_INP_SC_MODE_DENDY);
	shortcut_update_text(mainwin->wd->qaction_shcut.scale_1x, SET_INP_SC_SCALE_1X);
	shortcut_update_text(mainwin->wd->qaction_shcut.scale_2x, SET_INP_SC_SCALE_2X);
	shortcut_update_text(mainwin->wd->qaction_shcut.scale_3x, SET_INP_SC_SCALE_3X);
	shortcut_update_text(mainwin->wd->qaction_shcut.scale_4x, SET_INP_SC_SCALE_4X);
	shortcut_update_text(mainwin->wd->qaction_shcut.scale_5x, SET_INP_SC_SCALE_5X);
	shortcut_update_text(mainwin->wd->qaction_shcut.scale_6x, SET_INP_SC_SCALE_6X);
	shortcut_update_text(mainwin->wd->qaction_shcut.interpolation, SET_INP_SC_INTERPOLATION);
	shortcut_update_text(mainwin->wd->qaction_shcut.integer_in_fullscreen, SET_INP_SC_INTEGER_FULLSCREEN);
	shortcut_update_text(mainwin->wd->qaction_shcut.stretch_in_fullscreen, SET_INP_SC_STRETCH_FULLSCREEN);
	shortcut_update_text(mainwin->wd->qaction_shcut.toggle_menubar_in_fullscreen, SET_INP_SC_TOGGLE_MENUBAR_IN_FULLSCREEN);
	shortcut_update_text(mainwin->wd->qaction_shcut.toggle_capture_input, SET_INP_SC_TOGGLE_CAPTURE_INPUT);
	shortcut_update_text(mainwin->wd->qaction_shcut.audio_enable, SET_INP_SC_AUDIO_ENABLE);
	shortcut_update_text(mainwin->wd->qaction_shcut.save_settings, SET_INP_SC_SAVE_SETTINGS);
	shortcut_update_text(mainwin->wd->qaction_shcut.hold_fast_forward, SET_INP_SC_HOLD_FAST_FORWARD);
	shortcut_update_text(mainwin->wd->action_Save_state, SET_INP_SC_SAVE_STATE);
	shortcut_update_text(mainwin->wd->action_Load_state, SET_INP_SC_LOAD_STATE);
	shortcut_update_text(mainwin->wd->action_Increment_slot, SET_INP_SC_INC_SLOT);
	shortcut_update_text(mainwin->wd->action_Decrement_slot, SET_INP_SC_DEC_SLOT);
	shortcut_update_text(mainwin->wd->qaction_shcut.rwnd.active, SET_INP_SC_RWND_ACTIVE_MODE);
	shortcut_update_text(mainwin->wd->qaction_shcut.rwnd.step_backward, SET_INP_SC_RWND_STEP_BACKWARD);
	shortcut_update_text(mainwin->wd->qaction_shcut.rwnd.step_forward, SET_INP_SC_RWND_STEP_FORWARD);
	shortcut_update_text(mainwin->wd->qaction_shcut.rwnd.fast_backward, SET_INP_SC_RWND_FAST_BACKWARD);
	shortcut_update_text(mainwin->wd->qaction_shcut.rwnd.fast_forward, SET_INP_SC_RWND_FAST_FORWARD);
	shortcut_update_text(mainwin->wd->qaction_shcut.rwnd.play, SET_INP_SC_RWND_PLAY);
	shortcut_update_text(mainwin->wd->qaction_shcut.rwnd.pause, SET_INP_SC_RWND_PAUSE);
}

void wdgSettingsInput::s_controller_mode(bool checked) {
	if (checked) {
		int mode = QVariant(((themePushButton *)sender())->property("mtype")).toInt();

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
void wdgSettingsInput::s_expansion_port_setup(UNUSED(bool checked)) {
	switch (cfg->input.expansion) {
		case CTRL_FAMILY_BASIC_KEYBOARD:
		case CTRL_SUBOR_KEYBOARD:
			mainwin->wd->action_Virtual_Keyboard->trigger();
			break;
		default:
			break;
	}
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
	_cfg_port *cfg_port = ((_cfg_port *)((themePushButton *)sender())->property("myPointer").value<void *>());

	switch (cfg_port->port->type) {
		case CTRL_DISABLED:
		case CTRL_ZAPPER:
			break;
		case CTRL_STANDARD:
			wdg_dlg_std_pad = new wdgDlgStdPad(this, cfg_port);

			hide_from_setup_button = true;
			dlgsettings->hide();
			wdg_dlg_std_pad->exec();
			wdg_dlg_std_pad = nullptr;
			dlgsettings->show();
			hide_from_setup_button = false;
			s_et_update_joy_combo();
			update_widget();
			break;
	}
}
void wdgSettingsInput::s_input_reset(UNUSED(bool checked)) {
	_array_pointers_port array;

	for (int i = PORT1; i < PORT_MAX; i++) {
		array.port[i] = input.cport[i].port;
	}

	settings_inp_all_defaults(&cfg->input, &array);
	update_widget();
}
void wdgSettingsInput::s_permit_updown_leftright(UNUSED(bool checked)) {
	cfg->input.permit_updown_leftright = !cfg->input.permit_updown_leftright;
}
void wdgSettingsInput::s_hide_zapper_cursor(UNUSED(bool checked)) {
	cfg->input.hide_zapper_cursor = !cfg->input.hide_zapper_cursor;
}
void wdgSettingsInput::s_joy_id(int index) {
	int data = ((QComboBox *)sender())->itemData(index).toInt();

	if (comboBox_joy_ID->count() == 1) {
		return;
	}

	js_guid_set(data, &cfg->input.jguid_sch);
	shortcuts_update(UPDATE_ALL, NO_ACTION, NO_ACTION);
}
void wdgSettingsInput::s_joy_index_changed(UNUSED(int index)) {
	int i;

	for (i = 0; i < SET_MAX_NUM_SC; i++) {
		js_row_pixmapPushButton(i);
	}
}
void wdgSettingsInput::s_shortcut_keyb(void) {
	QKeySequenceEdit *se = ((QKeySequenceEdit *)sender());
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	int value = se->keySequence()[0];
#else
	int value = se->keySequence()[0].toCombined();
#endif
	QKeySequence shortcut(value);

	shcut.type = QVariant(((QObject *)sender())->property("myType")).toInt();
	shcut.row = QVariant(((QObject *)sender())->property("myValue")).toInt();
	shcut.bp = nullptr;

	se->setKeySequence(shortcut);

	shcut.text[KEYBOARD].replace(shcut.row, shortcut.toString());

	settings_inp_wr_sc((void *)&shcut.text[KEYBOARD].at(shcut.row), shcut.row + SET_INP_SC_OPEN, KEYBOARD);
	mainwin->wd->shortcuts();

	shortcuts_update(UPDATE_ALL, NO_ACTION, NO_ACTION);
	update_widget();

	shcut.no_other_buttons = false;
}
void wdgSettingsInput::s_shortcut_joy(UNUSED(bool checked)) {
	if (shcut.no_other_buttons) {
		return;
	}

	shcut.type = QVariant(((QObject *)sender())->property("myType")).toInt();
	shcut.row = QVariant(((QObject *)sender())->property("myValue")).toInt();
	shcut.bp = ((pixmapPushButton *)sender());

	shortcuts_update(BUTTON_PRESSED, shcut.type, shcut.row);

	shcut.no_other_buttons = true;
	shcut.bp->setPixmap(QPixmap(""));
	shcut.bp->setText("...");
	shcut.bp->setFocus(Qt::ActiveWindowFocusReason);

	shcut.timeout.seconds = 5;
	shcut.timeout.timer->start(1000);
	s_input_timeout();

	shcut.joy.value = 0;
	shcut.joy.timer->start(150);
}
void wdgSettingsInput::s_shortcut_unset_all(UNUSED(bool checked)) {
	int i;

	for (i = 0; i < SET_MAX_NUM_SC; i++) {
		shcut.text[KEYBOARD].replace(i, "NULL");
		settings_inp_wr_sc((void *)&shcut.text[KEYBOARD].at(i), i + SET_INP_SC_OPEN, KEYBOARD);
		tableWidget_Shortcuts->cellWidget(i, 1)->findChild<QKeySequenceEdit *>("value")->setKeySequence(shcut.text[KEYBOARD].at(i));

		shcut.text[JOYSTICK].replace(i, "NULL");
		settings_inp_wr_sc((void *)&shcut.text[JOYSTICK].at(i), i + SET_INP_SC_OPEN, JOYSTICK);
		js_row_pixmapPushButton(i);
	}
	mainwin->wd->shortcuts();
	shortcuts_update(UPDATE_ALL, NO_ACTION, NO_ACTION);
}
void wdgSettingsInput::s_shortcut_reset(UNUSED(bool checked)) {
	int i;

	js_guid_unset(&cfg->input.jguid_sch);

	comboBox_joy_ID->setCurrentIndex(comboBox_joy_ID->count() - 1);

	for (i = 0; i < SET_MAX_NUM_SC; i++) {
		shcut.text[KEYBOARD].replace(i, uQString(inp_cfg[i + SET_INP_SC_OPEN].def).split(",").at(KEYBOARD));
		settings_inp_wr_sc((void *)&shcut.text[KEYBOARD].at(i), i + SET_INP_SC_OPEN, KEYBOARD);
		tableWidget_Shortcuts->cellWidget(i, 1)->findChild<QKeySequenceEdit *>("value")->setKeySequence(shcut.text[KEYBOARD].at(i));

		shcut.text[JOYSTICK].replace(i, uQString(inp_cfg[i + SET_INP_SC_OPEN].def).split(",").at(JOYSTICK));
		settings_inp_wr_sc((void *)&shcut.text[JOYSTICK].at(i), i + SET_INP_SC_OPEN, JOYSTICK);
		js_row_pixmapPushButton(i);
	}
	mainwin->wd->shortcuts();
	shortcuts_update(UPDATE_ALL, NO_ACTION, NO_ACTION);
}
void wdgSettingsInput::s_shortcut_keyb_default(UNUSED(bool checked)) {
	int row = QVariant(((QObject *)sender())->property("myValue")).toInt();

	shcut.text[KEYBOARD].replace(row, uQString(inp_cfg[row + SET_INP_SC_OPEN].def).split(",").at(KEYBOARD));
	tableWidget_Shortcuts->cellWidget(row, 1)->findChild<QKeySequenceEdit *>("value")->setKeySequence(shcut.text[KEYBOARD].at(row));
	settings_inp_wr_sc((void *)&shcut.text[KEYBOARD].at(row), row + SET_INP_SC_OPEN, KEYBOARD);
	mainwin->wd->shortcuts();
}
void wdgSettingsInput::s_shortcut_keyb_unset(UNUSED(bool checked)) {
	int row = QVariant(((QObject *)sender())->property("myValue")).toInt();

	shcut.text[KEYBOARD].replace(row, "NULL");
	tableWidget_Shortcuts->cellWidget(row, 1)->findChild<QKeySequenceEdit *>("value")->clear();
	settings_inp_wr_sc((void *)&shcut.text[KEYBOARD].at(row), row + SET_INP_SC_OPEN, KEYBOARD);
	mainwin->wd->shortcuts();
}
void wdgSettingsInput::s_shortcut_joy_unset(UNUSED(bool checked)) {
	int row = QVariant(((QObject *)sender())->property("myValue")).toInt();

	shcut.text[JOYSTICK].replace(row, "NULL");
	js_row_pixmapPushButton(row);
	settings_inp_wr_sc((void *)&shcut.text[JOYSTICK].at(row), row + SET_INP_SC_OPEN, JOYSTICK);
}
void wdgSettingsInput::s_input_timeout(void) {
	info_entry_print(tr("Press a key - timeout in %1").arg(QString::number(shcut.timeout.seconds--)));

	if (shcut.timeout.seconds < 0) {
		QKeyEvent *event = new QKeyEvent(QEvent::KeyPress, Qt::Key_Escape, Qt::MetaModifier);

		QCoreApplication::postEvent(shcut.bp, event);
	}
}
void wdgSettingsInput::s_joy_read_timer(void) {
	DBWORD value = js_jdev_read_in_dialog(&cfg->input.jguid_sch);

	if (shcut.joy.value && !value) {
		shcut.text[JOYSTICK].replace(shcut.row, uQString(js_joyval_to_name(shcut.joy.value)));
		settings_inp_wr_sc((void *)&shcut.text[JOYSTICK].at(shcut.row), shcut.row + SET_INP_SC_OPEN, JOYSTICK);

		shcut.timeout.timer->stop();
		shcut.joy.timer->stop();
		info_entry_print("");

		shortcuts_update(UPDATE_ALL, NO_ACTION, NO_ACTION);
		update_widget();

		shcut.no_other_buttons = false;
	}

	shcut.joy.value = value;
}

void wdgSettingsInput::s_et_update_joy_combo(void) {
	// se la combox e' aperta o sono in attessa di impostare uno shortcut, non devo aggiornarne il contenuto
	if (!comboBox_joy_ID->view()->isVisible() &&
		!shcut.timeout.timer->isActive() &&
		!shcut.joy.timer->isActive()) {
		shortcut_joy_combo_init();
		shortcuts_update(UPDATE_ALL, NO_ACTION, NO_ACTION);
	}
	if (wdg_dlg_std_pad) {
		emit wdg_dlg_std_pad->wd->et_update_joy_combo();
	}
	gui_wdgdlgjsc_emit_update_joy_combo();
}
