/*
 *  Copyright (C) 2010-2020 Fabio Cavallo (aka FHorse)
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

#if defined (__linux__)
#include <unistd.h>
#include <fcntl.h>
#endif
#include <QtSvg/QSvgRenderer>
#include <QtGui/QPainter>
#include "dlgStdPad.moc"
#include "mainWindow.hpp"
#include "objSettings.hpp"
#include "clock.h"

#define SPT(ind) QString(std_pad_input_type[ind])
#define SPB(ind) QString(std_pad_button[ind])

static const char std_pad_input_type[2][4] = { "kbd", "joy" };
static const char std_pad_button[10][15] = {
	"A",      "B",     "Select", "Start",
	"Up",     "Down",  "Left",   "Right",
	"TurboA", "TurboB"
};

dlgStdPad::dlgStdPad(_cfg_port *cfg_port, QWidget *parent = 0) : QDialog(parent) {
	QFont f9;

	f9.setPointSize(9);
	f9.setWeight(QFont::Light);

	memset(&data, 0x00, sizeof(data));
	memcpy(&data.cfg, cfg_port, sizeof(_cfg_port));

	setupUi(this);

	js_update_detected_devices();

	groupBox_controller->setTitle(tr("Controller %1 : Standard Pad").arg(cfg_port->id));
	tabWidget_kbd_joy->setCurrentIndex(JOYSTICK);
	combo_id_init();

	for (int a = KEYBOARD; a <= JOYSTICK; a++) {
		QLineEdit *txt;
		QPushButton *bt;

		txt = findChild<QLineEdit *>("lineEdit_" + SPT(a) + "_info");

		if (txt->font().pointSize() > 9) {
			txt->setFont(f9);
		}

		{
			int h = txt->fontMetrics().size(0, "IQygp").height() + 10;

			txt->setFixedHeight(h);
		}

		bt = findChild<QPushButton *>("pushButton_" + SPT(a) + "_Sequence");
		bt->setProperty("myType", QVariant(a));
		connect(bt, SIGNAL(clicked(bool)), this, SLOT(s_in_sequence_clicked(bool)));

		bt = findChild<QPushButton *>("pushButton_" + SPT(a) + "_Unset_all");
		bt->setProperty("myType", QVariant(a));
		connect(bt, SIGNAL(clicked(bool)), this, SLOT(s_unset_all_clicked(bool)));

		bt = findChild<QPushButton *>("pushButton_" + SPT(a) + "_Defaults");
		bt->setProperty("myType", QVariant(a));
		connect(bt, SIGNAL(clicked(bool)), this, SLOT(s_defaults_clicked(bool)));

		for (int b = BUT_A; b < MAX_STD_PAD_BUTTONS; b++) {
			int vbutton = b + (a * MAX_STD_PAD_BUTTONS);
			QPushButton *unset;

			bt = findChild<QPushButton *>("pushButton_" + SPT(a) + "_" + SPB(b));
			unset = findChild<QPushButton *>("pushButton_" + SPT(a) + "_unset_" + SPB(b));

			if (bt->font().pointSize() > 9) {
				bt->setFont(f9);
			}

			if (a == KEYBOARD) {
				bt->setText(objInp::kbd_keyval_to_name(data.cfg.port->input[a][b]));
			} else {
				bt->setText(uQString(jsv_to_name(data.cfg.port->input[a][b])));
			}

			bt->installEventFilter(this);
			bt->setProperty("myVbutton", QVariant(vbutton));
			unset->setProperty("myVbutton", QVariant(vbutton));

			connect(bt, SIGNAL(clicked(bool)), this, SLOT(s_input_clicked(bool)));
			connect(unset, SIGNAL(clicked(bool)), this, SLOT(s_unset_clicked(bool)));
		}
	}
	
	tabWidget_kbd_joy->adjustSize();

	{
		comboBox_Controller_type->addItem(tr("Auto"));
		comboBox_Controller_type->addItem(tr("Original"));
		comboBox_Controller_type->addItem(tr("3rd-party"));
		comboBox_Controller_type->setCurrentIndex(data.cfg.port->type_pad);
		connect(comboBox_Controller_type, SIGNAL(activated(int)), this,
			SLOT(s_combobox_controller_type_activated(int)));
	}

	for (int i = TURBOA; i <= TURBOB; i++) {
		QSlider *tb = findChild<QSlider *>("horizontalSlider_" + SPB(i + TRB_A));

		tb->setRange(1, TURBO_BUTTON_DELAY_MAX);
		tb->setProperty("myTurbo", QVariant(i));
		tb->setValue(data.cfg.port->turbo[i].frequency);
		connect(tb, SIGNAL(valueChanged(int)), this, SLOT(s_slider_td_value_changed(int)));

		td_update_label(i, data.cfg.port->turbo[i].frequency);
	}

	pushButton_Apply->setProperty("myPointer", QVariant::fromValue(((void *)cfg_port)));
	connect(pushButton_Apply, SIGNAL(clicked(bool)), this, SLOT(s_apply_clicked(bool)));
	connect(pushButton_Discard, SIGNAL(clicked(bool)), this, SLOT(s_discard_clicked(bool)));

	setAttribute(Qt::WA_DeleteOnClose);

	adjustSize();
	setFixedSize(size());

	setFocusPolicy(Qt::StrongFocus);
	groupBox_controller->setFocus(Qt::ActiveWindowFocusReason);

	data.joy.timer = new QTimer(this);
	connect(data.joy.timer, SIGNAL(timeout()), this, SLOT(s_pad_joy_read_timer()));

	data.seq.timer = new QTimer(this);
	connect(data.seq.timer, SIGNAL(timeout()), this, SLOT(s_pad_in_sequence_timer()));

	installEventFilter(this);
}
dlgStdPad::~dlgStdPad() {}

bool dlgStdPad::eventFilter(QObject *obj, QEvent *event) {
	// mi interessa intercettare tutti i keyPress che arrivano, non solo quelli
	// che riguardano questo dialog.
	switch (event->type()) {
		case QEvent::KeyPress:
			return (keypress((QKeyEvent *)event));
			break;
		default:
			break;
	}

	return (QObject::eventFilter(obj, event));
}
void dlgStdPad::changeEvent(QEvent *event) {
	if (event->type() == QEvent::LanguageChange) {
		Ui::dlgStdPad::retranslateUi(this);
	} else {
		QDialog::changeEvent(event);
	}
}
void dlgStdPad::showEvent(QShowEvent *event) {
	QSvgRenderer svg(QString(":/pics/pics/Nes_controller.svg"));
	float ratio = (float)svg.defaultSize().width() / (float)svg.defaultSize().height();
	int w = image_pad->size().width();
	int h = (float)w / ratio;
	QImage image(w, h, QImage::Format_ARGB32);

	mainwin->shcjoy_stop();

	image.fill(Qt::transparent);

	{
		QPainter painter(&image);

		svg.render(&painter);
	}

	image_pad->resize(image.size());
	image_pad->setPixmap(QPixmap::fromImage(image, Qt::ColorOnly));

	QDialog::showEvent(event);
}
void dlgStdPad::closeEvent(QCloseEvent *event) {
	data.joy.timer->stop();
	data.seq.timer->stop();
	data.seq.active = false;
#if defined (__linux__)
	if (data.joy.fd) {
		::close(data.joy.fd);
		data.joy.fd = 0;
	}
#endif

	mainwin->shcjoy_start();

	data.no_other_buttons = false;
	data.vbutton = 0;

	QDialog::closeEvent(event);
}

bool dlgStdPad::keypress(QKeyEvent *event) {
	int type, vbutton;

	if (data.no_other_buttons == false) {
		return (true);
	}

	type = data.vbutton / MAX_STD_PAD_BUTTONS;
	vbutton = data.vbutton - (type * MAX_STD_PAD_BUTTONS);

	if (type == KEYBOARD) {
		if (event->key() != Qt::Key_Escape) {
			data.cfg.port->input[type][vbutton] = objInp::kbd_keyval_decode(event);
		}
		data.bp->setText(objInp::kbd_keyval_to_name(data.cfg.port->input[type][vbutton]));
	} else {
		// quando sto configurando il joystick, l'unico input da tastiera
		// che accetto e' l'escape.
		if (event->key() == Qt::Key_Escape) {
			data.joy.timer->stop();
			data.bp->setText(uQString(jsv_to_name(data.cfg.port->input[type][vbutton])));
		} else {
			return (true);
		}
	}

	info_entry_print(type, "");
	update_dialog();

	data.no_other_buttons = false;
	data.vbutton = 0;

	return (true);
}
void dlgStdPad::update_dialog(void) {
	bool mode = false;
	unsigned int joyId;

	if (data.seq.active == true) {
		return;
	}

	tabWidget_kbd_joy->setTabEnabled(KEYBOARD, true);
	tabWidget_kbd_joy->setTabEnabled(JOYSTICK, true);

	// keyboard
	label_kbd_ID->setEnabled(false);
	comboBox_kbd_ID->setEnabled(false);

	setEnable_tab_buttons(KEYBOARD, true);

	lineEdit_kbd_info->setEnabled(true);

	pushButton_kbd_Sequence->setEnabled(true);
	pushButton_kbd_Unset_all->setEnabled(true);
	pushButton_kbd_Defaults->setEnabled(true);

	// joystick
	joyId = comboBox_joy_ID->itemData(comboBox_joy_ID->currentIndex()).toInt();

	if (comboBox_joy_ID->count() > 1) {
		mode = true;
	}

	label_joy_ID->setEnabled(mode);
	comboBox_joy_ID->setEnabled(mode);

	if ((comboBox_joy_ID->count() > 1) && (joyId != name_to_jsn(uL("NULL")))) {
		mode = true;
	} else {
		mode = false;
	}

	setEnable_tab_buttons(JOYSTICK, mode);

	lineEdit_joy_info->setEnabled(mode);

	pushButton_joy_Sequence->setEnabled(mode);
	pushButton_joy_Unset_all->setEnabled(mode);
	pushButton_joy_Defaults->setEnabled(mode);

	// misc
	groupBox_Misc->setEnabled(true);
}
void dlgStdPad::combo_id_init(void) {
	BYTE disabled_line = 0, count = 0, current_line = name_to_jsn(uL("NULL"));

	comboBox_kbd_ID->addItem(tr("Keyboard"));

	for (int a = 0; a <= MAX_JOYSTICK; a++) {
		BYTE id = a;

		if (a < MAX_JOYSTICK) {
			if (js_is_connected(id) == EXIT_ERROR) {
				continue;
			}

			if (js_is_this(id, &data.cfg.port->joy_id)) {
				current_line = count;
			}

			comboBox_joy_ID->addItem(QString("js%1: ").arg(id) + uQString(js_name_device(id)));
		} else {
			if (count == 0) {
				break;
			}
			comboBox_joy_ID->addItem(tr("Disabled"));
			id = name_to_jsn(uL("NULL"));
			disabled_line = count;
		}

		comboBox_joy_ID->setItemData(count, id);
		count++;
	}

	if (count == 0) {
		comboBox_joy_ID->addItem(tr("No usable device"));
		tab_joy->setEnabled(false);
	} else {
		tab_joy->setEnabled(true);
	}

	if (count > 0) {
		if (js_is_null(&data.cfg.port->joy_id) || (current_line == name_to_jsn(uL("NULL")))) {
			comboBox_joy_ID->setCurrentIndex(disabled_line);
		} else {
			comboBox_joy_ID->setCurrentIndex(current_line);
		}
		connect(comboBox_joy_ID, SIGNAL(activated(int)), this, SLOT(s_combobox_joy_activated(int)));
	} else {
		comboBox_joy_ID->setCurrentIndex(0);
	}

	update_dialog();
}
void dlgStdPad::setEnable_tab_buttons(int type, bool mode) {
	for (int a = BUT_A; a < MAX_STD_PAD_BUTTONS; a++) {
		findChild<QLabel *>("label_" + SPT(type) + "_" + SPB(a))->setEnabled(mode);
		findChild<QLabel *>("label_" + SPT(type) + "_" + SPB(a))->setStyleSheet("");
		findChild<QPushButton *>("pushButton_" + SPT(type) + "_" + SPB(a))->setEnabled(mode);
		findChild<QPushButton *>("pushButton_" + SPT(type) +
				"_unset_" + SPB(a))->setEnabled(mode);
	}
}
void dlgStdPad::disable_tab_and_other(int type, int vbutton) {
	// altro tab
	tabWidget_kbd_joy->setTabEnabled(type == KEYBOARD ? JOYSTICK : KEYBOARD, false);

	// ID
	findChild<QLabel *>("label_" + SPT(type) + "_ID")->setEnabled(false);
	findChild<QComboBox *>("comboBox_" + SPT(type) + "_ID")->setEnabled(false);

	setEnable_tab_buttons(type, false);
	findChild<QLabel *>("label_" + SPT(type) + "_" + SPB(vbutton))->setEnabled(true);
	findChild<QLabel *>("label_" + SPT(type) + "_" + SPB(vbutton))->setStyleSheet(
			"background-color: cyan");
	findChild<QPushButton *>("pushButton_" + SPT(type) + "_" + SPB(vbutton))->setEnabled(true);

	// info
	//findChild<QPlainTextEdit *>("plainTextEdit_" + SPT(type) + "_info")->setEnabled(false);

	// in sequence, unset all, default
	findChild<QPushButton *>("pushButton_" + SPT(type) + "_Sequence")->setEnabled(false);
	findChild<QPushButton *>("pushButton_" + SPT(type) + "_Unset_all")->setEnabled(false);
	findChild<QPushButton *>("pushButton_" + SPT(type) + "_Defaults")->setEnabled(false);

	// misc
	groupBox_Misc->setEnabled(false);
}
void dlgStdPad::info_entry_print(int type, QString txt) {
	findChild<QLineEdit *>("lineEdit_" + SPT(type) + "_info")->setText(txt);
}
void dlgStdPad::js_press_event(void) {
	unsigned int type;

	type = data.vbutton / MAX_STD_PAD_BUTTONS;

	if (js_is_null(&data.cfg.port->joy_id)) {
		info_entry_print(type, tr("Select device first"));
		update_dialog();
		return;
	}

#if defined (__linux__)
	{
		_js_event jse;
		ssize_t size = sizeof(jse);
		char device[30];

		::sprintf(device, "%s%d", JS_DEV_PATH, data.cfg.port->joy_id);
		data.joy.fd = ::open(device, O_RDONLY | O_NONBLOCK);

		if (data.joy.fd < 0) {
			info_entry_print(type, tr("Error on open device %1").arg(device));
			update_dialog();
			return;
		}

		for (int i = 0; i < MAX_JOYSTICK; i++) {
			if (::read(data.joy.fd, &jse, size) < 0) {
				info_entry_print(type, tr("Error on reading controllers configurations"));
			}
		}

		// svuoto il buffer iniziale
		for (int i = 0; i < 10; i++) {
			if (::read(data.joy.fd, &jse, size) < 0) {
				;
			}
		}
	}
	data.joy.value = 0;
	data.joy.timer->start(30);
#else
	data.joy.value = 0;
	data.joy.timer->start(150);
#endif
}
void dlgStdPad::td_update_label(int type, int value) {
	QLabel *label = findChild<QLabel *>("label_value_slider_" + SPB(type + TRB_A));

	label->setText(QString("%1").arg(value, 2));
}

void dlgStdPad::s_combobox_joy_activated(int index) {
	unsigned int id = ((QComboBox *)sender())->itemData(index).toInt();

	js_set_id(&data.cfg.port->joy_id, id);
	update_dialog();
}
void dlgStdPad::s_input_clicked(UNUSED(bool checked)) {
	int vbutton = QVariant(((QPushButton *)sender())->property("myVbutton")).toInt();
	int type;

	if (data.no_other_buttons == true) {
		return;
	}

	data.bp = ((QPushButton *)sender());
	data.vbutton = vbutton;

	type = vbutton / MAX_STD_PAD_BUTTONS;
	vbutton -= (type * MAX_STD_PAD_BUTTONS);

	disable_tab_and_other(type, vbutton);

	data.no_other_buttons = true;
	data.bp->setText("...");

	data.bp->setFocus(Qt::ActiveWindowFocusReason);

	if (type == KEYBOARD) {
		info_entry_print(type, tr("Press a key (ESC for the previous value \"%1\")").arg(
				objInp::kbd_keyval_to_name(data.cfg.port->input[type][vbutton])));

	} else {
		info_entry_print(type, tr("Press a key (ESC for the previous value \"%1\")").arg(
				uQString(jsv_to_name(data.cfg.port->input[type][vbutton]))));
		js_press_event();
	}
}
void dlgStdPad::s_unset_clicked(UNUSED(bool checked)) {
	int vbutton = QVariant(((QPushButton *)sender())->property("myVbutton")).toInt();
	int type;

	type = vbutton / MAX_STD_PAD_BUTTONS;
	vbutton -= (type * MAX_STD_PAD_BUTTONS);
	data.cfg.port->input[type][vbutton] = 0;

	info_entry_print(type, "");

	findChild<QPushButton *>("pushButton_" + SPT(type) + "_" + SPB(vbutton))->setText("NULL");
}
void dlgStdPad::s_in_sequence_clicked(UNUSED(bool checked)) {
	data.seq.type = QVariant(((QPushButton *)sender())->property("myType")).toInt();

	info_entry_print(data.seq.type, "");
	data.seq.active = true;
	data.seq.counter = -1;
	data.seq.timer->start(150);
}
void dlgStdPad::s_unset_all_clicked(UNUSED(bool checked)) {
	int type = QVariant(((QPushButton *)sender())->property("myType")).toInt();

	info_entry_print(type, "");

	for (int i = BUT_A; i < MAX_STD_PAD_BUTTONS; i++) {
		findChild<QPushButton *>("pushButton_" + SPT(type) + "_unset_" + SPB(i))->click();
	}
}
void dlgStdPad::s_defaults_clicked(UNUSED(bool checked)) {
	int type = QVariant(((QPushButton *)sender())->property("myType")).toInt();

	info_entry_print(type, "");

	settings_inp_port_default(data.cfg.port, data.cfg.id - 1, type);

	for (int i = BUT_A; i < MAX_STD_PAD_BUTTONS; i++) {
		QPushButton *bt = findChild<QPushButton *>("pushButton_" + SPT(type) + "_" + SPB(i));

		if (type == KEYBOARD) {
			bt->setText(objInp::kbd_keyval_to_name(data.cfg.port->input[type][i]));
		} else {
			bt->setText(uQString(jsv_to_name(data.cfg.port->input[type][i])));
		}
	}
}
void dlgStdPad::s_combobox_controller_type_activated(int index) {
	BYTE state = RELEASED;

	data.cfg.port->type_pad = index;

	if (((data.cfg.port->type_pad == CTRL_PAD_AUTO) && (machine.type != DENDY)) || (data.cfg.port->type_pad == CTRL_PAD_ORIGINAL)) {
		state = PRESSED;
	}

	for (int b = 8; b < 24; b++) {
		data.cfg.port->data[b] = state;
	}
}
void dlgStdPad::s_slider_td_value_changed(int value) {
	int type = QVariant(((QSlider *)sender())->property("myTurbo")).toInt();

	data.cfg.port->turbo[type].frequency = value;
	data.cfg.port->turbo[type].counter = 0;
	td_update_label(type, value);
}
void dlgStdPad::s_pad_joy_read_timer(void) {
	DBWORD value = js_read_in_dialog(&data.cfg.port->joy_id, data.joy.fd);

	if (data.joy.value && !value) {
		unsigned int type, vbutton;

#if defined (__linux__)
		::close(data.joy.fd);
		data.joy.fd = 0;
#endif

		type = data.vbutton / MAX_STD_PAD_BUTTONS;
		vbutton = data.vbutton - (type * MAX_STD_PAD_BUTTONS);

		info_entry_print(type, "");
		data.cfg.port->input[type][vbutton] = data.joy.value;
		data.bp->setText(uQString(jsv_to_name(data.joy.value)));
		data.joy.timer->stop();

		update_dialog();

		data.no_other_buttons = false;
		data.vbutton = 0;
	}

	data.joy.value = value;
}
void dlgStdPad::s_pad_in_sequence_timer(void) {
	QPushButton *bt;
	static int order[MAX_STD_PAD_BUTTONS] = {
		UP,     DOWN,  LEFT,  RIGHT,
		SELECT, START, BUT_A, BUT_B,
		TRB_A,  TRB_B,
	};


	if (data.no_other_buttons == true) {
		return;
	}

	if (++data.seq.counter == MAX_STD_PAD_BUTTONS) {
		data.seq.timer->stop();
		data.seq.active = false;
		update_dialog();
		return;
	}

	bt = findChild<QPushButton *>(
			"pushButton_" + SPT(data.seq.type) + "_" + SPB(order[data.seq.counter]));
	bt->setEnabled(true);
	bt->click();
}
void dlgStdPad::s_apply_clicked(UNUSED(bool checked)) {
	_cfg_port *cfg_port = ((_cfg_port *)((QPushButton *)sender())->property(
		"myPointer").value<void *>());

	memcpy(cfg_port, &data.cfg, sizeof(_cfg_port));

	close();
}
void dlgStdPad::s_discard_clicked(UNUSED(bool checked)) {
	close();
}
