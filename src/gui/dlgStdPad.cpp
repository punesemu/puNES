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

#include <QtWidgets/QAbstractItemView>
#include <QtSvg/QSvgRenderer>
#include <QtGui/QPainter>
#include <QtCore/QBuffer>
#include "dlgStdPad.moc"
#include "mainWindow.hpp"
#include "objSettings.hpp"
#include "clock.h"
#include "gui.h"

#define SPT(ind) QString(std_pad_input_type[ind])
#define SPB(ind) QString(std_pad_button[ind])

static const char std_pad_input_type[2][4] = { "kbd", "joy" };
static const char std_pad_button[10][15] = {
	"A",      "B",     "Select", "Start",
	"Up",     "Down",  "Left",   "Right",
	"TurboA", "TurboB"
};

_joy_list joy_list;

dlgStdPad::dlgStdPad(_cfg_port *cfg_port, QWidget *parent = 0) : QDialog(parent) {
	int i;

	memset(&data, 0x00, sizeof(data));

	last_js_index = MAX_JOYSTICK;

	data.cfg.id = cfg_port->id;
	memcpy(&data.cfg.port, cfg_port->port, sizeof(_port));

	setupUi(this);

	setFocusProxy(tabWidget_kbd_joy);

	groupBox_controller->setStyleSheet(group_title_bold_stylesheet());
	groupBox_Misc->setStyleSheet(group_title_bold_stylesheet());

	groupBox_controller->setTitle(tr("Controller %1 : Standard Pad").arg(cfg_port->id));

	comboBox_kbd_ID->addItem(tr("Keyboard"));

	connect(comboBox_joy_ID, SIGNAL(activated(int)), this, SLOT(s_combobox_joy_activated(int)));
	connect(comboBox_joy_ID, SIGNAL(currentIndexChanged(int)), this, SLOT(s_combobox_joy_index_changed(int)));
	connect(horizontalSlider_joy_Deadzone, SIGNAL(valueChanged(int)), this, SLOT(s_deadzone_slider_value_changed(int)));

	joy_combo_init();

	for (i = KEYBOARD; i < INPUT_TYPES; i++) {
		QPushButton *bt;
		int a;

		bt = findChild<QPushButton *>("pushButton_" + SPT(i) + "_Sequence");
		bt->setProperty("myType", QVariant(i));
		connect(bt, SIGNAL(clicked(bool)), this, SLOT(s_in_sequence_clicked(bool)));

		bt = findChild<QPushButton *>("pushButton_" + SPT(i) + "_Unset_all");
		bt->setProperty("myType", QVariant(i));
		connect(bt, SIGNAL(clicked(bool)), this, SLOT(s_unset_all_clicked(bool)));

		bt = findChild<QPushButton *>("pushButton_" + SPT(i) + "_Defaults");
		bt->setProperty("myType", QVariant(i));
		connect(bt, SIGNAL(clicked(bool)), this, SLOT(s_defaults_clicked(bool)));

		for (a = BUT_A; a < MAX_STD_PAD_BUTTONS; a++) {
			int vbutton = a + (i * MAX_STD_PAD_BUTTONS);
			QPushButton *def, *unset;
			pixmapButton *pbt;

			pbt = findChild<pixmapButton *>("pushButton_" + SPT(i) + "_" + SPB(a));
			def = findChild<QPushButton *>("pushButton_" + SPT(i) + "_default_" + SPB(a));
			unset = findChild<QPushButton *>("pushButton_" + SPT(i) + "_unset_" + SPB(a));

			pbt->setIcon(QIcon(""));
			if (i == KEYBOARD) {
				pbt->setText(objInp::kbd_keyval_to_name(data.cfg.port.input[i][a]));
			} else {
				js_pixmapButton(js_jdev_index(), data.cfg.port.input[i][a], pbt);
			}

			pbt->installEventFilter(this);
			pbt->setProperty("myVbutton", QVariant(vbutton));
			def->setProperty("myVbutton", QVariant(vbutton));
			unset->setProperty("myVbutton", QVariant(vbutton));

			connect(pbt, SIGNAL(clicked(bool)), this, SLOT(s_input_clicked(bool)));
			connect(def, SIGNAL(clicked(bool)), this, SLOT(s_default_clicked(bool)));
			connect(unset, SIGNAL(clicked(bool)), this, SLOT(s_unset_clicked(bool)));
		}
	}
	
	label_kbd_Deadzone_value_slider->setText(QString("%1").arg(0, 2));
	if (js_jdev_index() != JS_NO_JOYSTICK) {
		horizontalSlider_joy_Deadzone->setValue(jstick.jdd.devices[js_jdev_index()].deadzone);
	}
	connect(pushButton_joy_Deadzone, SIGNAL(clicked(bool)), this, SLOT(s_deadzone_default_clicked(bool)));

	{
		comboBox_Controller_type->addItem(tr("Auto"));
		comboBox_Controller_type->addItem(tr("Original"));
		comboBox_Controller_type->addItem(tr("3rd-party"));
		comboBox_Controller_type->setCurrentIndex(data.cfg.port.type_pad);
		connect(comboBox_Controller_type, SIGNAL(activated(int)), this, SLOT(s_combobox_controller_type_activated(int)));
	}

	{
		int w = QLabel("00").sizeHint().width();

		label_value_slider_TurboA->setFixedWidth(w);
		label_value_slider_TurboB->setFixedWidth(w);
	}

	for (i = TURBOA; i <= TURBOB; i++) {
		QSlider *tb = findChild<QSlider *>("horizontalSlider_" + SPB(i + TRB_A));

		tb->setRange(1, TURBO_BUTTON_DELAY_MAX);
		tb->setProperty("myTurbo", QVariant(i));
		tb->setValue(data.cfg.port.turbo[i].frequency);
		connect(tb, SIGNAL(valueChanged(int)), this, SLOT(s_slider_td_value_changed(int)));

		td_update_label(i, data.cfg.port.turbo[i].frequency);
	}

	pushButton_Apply->setProperty("myPointer", QVariant::fromValue(((void *)cfg_port)));

	connect(pushButton_Apply, SIGNAL(clicked(bool)), this, SLOT(s_apply_clicked(bool)));
	connect(pushButton_Discard, SIGNAL(clicked(bool)), this, SLOT(s_discard_clicked(bool)));

	setAttribute(Qt::WA_DeleteOnClose);

	setFixedSize(size());

	data.joy.timer = new QTimer(this);
	connect(data.joy.timer, SIGNAL(timeout()), this, SLOT(s_pad_joy_read_timer()));

	data.seq.timer = new QTimer(this);
	connect(data.seq.timer, SIGNAL(timeout()), this, SLOT(s_pad_in_sequence_timer()));

	connect(this, SIGNAL(et_update_joy_combo()), this, SLOT(s_et_update_joy_combo()));

	if (joy_list.count < 2) {
		tabWidget_kbd_joy->setCurrentIndex(KEYBOARD);
	} else if (gui.dlg_tabWidget_kbd_joy_index[data.cfg.id] >= 0) {
		tabWidget_kbd_joy->setCurrentIndex(gui.dlg_tabWidget_kbd_joy_index[data.cfg.id]);
	} else {
		tabWidget_kbd_joy->setCurrentIndex(JOYSTICK);
	}

	installEventFilter(this);
}
dlgStdPad::~dlgStdPad() {}

bool dlgStdPad::eventFilter(QObject *obj, QEvent *event) {
	// mi interessa intercettare tutti i keyPress che arrivano, non solo quelli
	// che riguardano questo dialog.
	switch (event->type()) {
		case QEvent::KeyPress:
			if (data.no_other_buttons == true) {
				return (keypress((QKeyEvent *)event));
			}
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

	if (data.exec_js_init) {
		js_quit(FALSE);
		js_init(FALSE);
	}

	mainwin->shcjoy_start();

	data.no_other_buttons = false;
	data.vbutton = 0;

	gui.dlg_tabWidget_kbd_joy_index[data.cfg.id] = tabWidget_kbd_joy->currentIndex();

	QDialog::closeEvent(event);
}

bool dlgStdPad::keypress(QKeyEvent *event) {
	int type, vbutton;

	type = data.vbutton / MAX_STD_PAD_BUTTONS;
	vbutton = data.vbutton - (type * MAX_STD_PAD_BUTTONS);

	if (type == KEYBOARD) {
		if (event->key() != Qt::Key_Escape) {
			data.cfg.port.input[type][vbutton] = objInp::kbd_keyval_decode(event);
		}
		data.bp->setText(objInp::kbd_keyval_to_name(data.cfg.port.input[type][vbutton]));
	} else {
		// quando sto configurando il joystick, l'unico input da tastiera che accetto e' l'escape.
		if (event->key() == Qt::Key_Escape) {
			data.joy.timer->stop();
			js_pixmapButton(js_jdev_index(), data.cfg.port.input[type][vbutton], data.bp);
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
	unsigned int joy_index;

	if (data.seq.active == true) {
		return;
	}

	tabWidget_kbd_joy->setTabEnabled(KEYBOARD, true);
	tabWidget_kbd_joy->setTabEnabled(JOYSTICK, true);

	// keyboard
	label_kbd_ID->setEnabled(false);
	comboBox_kbd_ID->setEnabled(false);

	setEnable_tab_buttons(KEYBOARD, true);

	label_kbd_info->setEnabled(true);

	pushButton_kbd_Sequence->setEnabled(true);
	pushButton_kbd_Unset_all->setEnabled(true);
	pushButton_kbd_Defaults->setEnabled(true);

	label_kbd_Deadzone_slider->setEnabled(false);
	horizontalSlider_kbd_Deadzone->setEnabled(false);
	label_kbd_Deadzone_value_slider->setEnabled(false);
	pushButton_kbd_Deadzone->setEnabled(false);

	// joystick
	joy_index = comboBox_joy_ID->itemData(comboBox_joy_ID->currentIndex()).toInt();

	if (comboBox_joy_ID->count() > 1) {
		mode = true;
	}

	label_joy_ID->setEnabled(mode);
	comboBox_joy_ID->setEnabled(mode);

	if ((comboBox_joy_ID->count() > 1) && (joy_index != JS_NO_JOYSTICK)) {
		mode = true;
	} else {
		mode = false;
	}

	setEnable_tab_buttons(JOYSTICK, mode);

	label_joy_info->setEnabled(mode);

	pushButton_joy_Sequence->setEnabled(mode);
	pushButton_joy_Unset_all->setEnabled(mode);
	pushButton_joy_Defaults->setEnabled(mode);

	label_joy_Deadzone_slider->setEnabled(mode);
	horizontalSlider_joy_Deadzone->setEnabled(mode);
	label_joy_Deadzone_value_slider->setEnabled(mode);
	pushButton_joy_Deadzone->setEnabled(mode);

	// misc
	groupBox_Misc->setEnabled(true);
}
void dlgStdPad::joy_combo_init(void) {
	BYTE current_index = 0, current_line = JS_NO_JOYSTICK;
	int i;

	comboBox_joy_ID->blockSignals(true);
	comboBox_joy_ID->clear();

	for (i = 0; i < joy_list.count; i++) {
		_cb_ports *cb = &joy_list.ele[i];

		if (js_is_this(cb->index, &data.cfg.port.jguid)) {
			current_line = i;
		}
		comboBox_joy_ID->addItem(cb->desc);
		comboBox_joy_ID->setItemData(i, cb->index);
	}

	if (joy_list.count > 1) {
		if (js_is_null(&data.cfg.port.jguid) || (current_line == JS_NO_JOYSTICK)) {
			current_index = joy_list.disabled_line;
		} else {
			current_index = current_line;
		}
	}
	comboBox_joy_ID->blockSignals(false);

	comboBox_joy_ID->setCurrentIndex(current_index);

	update_dialog();
}
void dlgStdPad::setEnable_tab_buttons(int type, bool mode) {
	int i;

	for (i = BUT_A; i < MAX_STD_PAD_BUTTONS; i++) {
		findChild<QLabel *>("label_" + SPT(type) + "_" + SPB(i))->setEnabled(mode);
		findChild<QLabel *>("label_" + SPT(type) + "_" + SPB(i))->setStyleSheet("");
		findChild<pixmapButton *>("pushButton_" + SPT(type) + "_" + SPB(i))->setEnabled(mode);
		findChild<QPushButton *>("pushButton_" + SPT(type) + "_default_" + SPB(i))->setEnabled(mode);
		findChild<QPushButton *>("pushButton_" + SPT(type) + "_unset_" + SPB(i))->setEnabled(mode);
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
	findChild<QLabel *>("label_" + SPT(type) + "_" + SPB(vbutton))->setStyleSheet("background-color: cyan");
	findChild<pixmapButton *>("pushButton_" + SPT(type) + "_" + SPB(vbutton))->setEnabled(true);

	// in sequence, unset all, default
	findChild<QPushButton *>("pushButton_" + SPT(type) + "_Sequence")->setEnabled(false);
	findChild<QPushButton *>("pushButton_" + SPT(type) + "_Unset_all")->setEnabled(false);
	findChild<QPushButton *>("pushButton_" + SPT(type) + "_Defaults")->setEnabled(false);

	// misc
	groupBox_Misc->setEnabled(false);
}
void dlgStdPad::info_entry_print(int type, QString txt) {
	findChild<QLabel *>("label_" + SPT(type) + "_info")->setText(txt);
}
void dlgStdPad::js_press_event(void) {
	unsigned int type;

	type = data.vbutton / MAX_STD_PAD_BUTTONS;

	if (js_is_null(&data.cfg.port.jguid)) {
		info_entry_print(type, tr("Select device first"));
		update_dialog();
		return;
	}

	data.joy.value = 0;
	data.joy.timer->start(150);
}
void dlgStdPad::td_update_label(int type, int value) {
	QLabel *label = findChild<QLabel *>("label_value_slider_" + SPB(type + TRB_A));

	label->setText(QString("%1").arg(value, 2));
}
void dlgStdPad::deadzone_update_label(int value) {
	label_joy_Deadzone_value_slider->setText(QString("%1").arg(value, 2));
}
void dlgStdPad::js_pixmapButton(int index, DBWORD input, pixmapButton *bt) {
	QString icon, desc;

	gui_js_joyval_icon_desc(index, input, &icon, &desc);
	bt->setIcon(QIcon());
	bt->setPixmap(QPixmap(icon));
	bt->setText(desc);
}

int dlgStdPad::js_jdev_index(void) {
	int jdev_index = JS_NO_JOYSTICK;

	if (comboBox_joy_ID->currentData().isValid()) {
		jdev_index = comboBox_joy_ID->currentData().toInt();
	}
	return (jdev_index);
}

void dlgStdPad::s_combobox_joy_activated(int index) {
	unsigned int jdev_index = ((QComboBox *)sender())->itemData(index).toInt();

	if (comboBox_joy_ID->count() == 1) {
		return;
	}
	js_guid_set(jdev_index, &data.cfg.port.jguid);
	update_dialog();
}
void dlgStdPad::s_combobox_joy_index_changed(UNUSED(int index)) {
	int a, jdev_index = js_jdev_index();

	if (jdev_index != last_js_index) {
		if (jdev_index < MAX_JOYSTICK) {
			_js_device *jdev = &jstick.jdd.devices[jdev_index];

			memcpy(data.cfg.port.input[JOYSTICK], jdev->stdctrl, js_jdev_sizeof_stdctrl());
			horizontalSlider_joy_Deadzone->setValue(jdev->deadzone);
		} else {
			memset(data.cfg.port.input[JOYSTICK], 0x00, js_jdev_sizeof_stdctrl());
			horizontalSlider_joy_Deadzone->setValue(0);
		}
		last_js_index = jdev_index;
	}

	for (a = KEYBOARD; a < INPUT_TYPES; a++) {
		pixmapButton *bt;
		int b;

		for (b = BUT_A; b < MAX_STD_PAD_BUTTONS; b++) {
			bt = findChild<pixmapButton *>("pushButton_" + SPT(a) + "_" + SPB(b));

			if (a == KEYBOARD) {
				bt->setText(objInp::kbd_keyval_to_name(data.cfg.port.input[a][b]));
			} else {
				js_pixmapButton(jdev_index, data.cfg.port.input[a][b], bt);
			}
		}
	}
}
void dlgStdPad::s_input_clicked(UNUSED(bool checked)) {
	int type, vbutton = QVariant(((pixmapButton *)sender())->property("myVbutton")).toInt();

	if (data.no_other_buttons == true) {
		return;
	}

	data.bp = ((pixmapButton *)sender());
	data.vbutton = vbutton;

	type = vbutton / MAX_STD_PAD_BUTTONS;
	vbutton -= (type * MAX_STD_PAD_BUTTONS);

	disable_tab_and_other(type, vbutton);

	data.no_other_buttons = true;
	data.bp->setPixmap(QPixmap(""));
	data.bp->setText("...");

	data.bp->setFocus(Qt::ActiveWindowFocusReason);

	if (type == KEYBOARD) {
		info_entry_print(type, tr("Press a key (ESC for the previous value \"%1\")").arg(
			objInp::kbd_keyval_to_name(data.cfg.port.input[type][vbutton])));
	} else {
		QString icon, desc;

		gui_js_joyval_icon_desc(js_jdev_index(), data.cfg.port.input[type][vbutton], &icon, &desc);

		if (icon != "") {
			QPixmap pixmap = QPixmap(icon).scaled(16, 16, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
			QByteArray byteArray;
			QBuffer buffer(&byteArray);

			pixmap.save(&buffer, "PNG");
			icon = QString(" <html><img src=\"data:image/png;base64,") + byteArray.toBase64() + "\"/></hmtl>";
		}

		info_entry_print(type, tr("Press a key (ESC for the previous value \"%1\"%2)").arg(desc).arg(icon));
		js_press_event();
	}
}
void dlgStdPad::s_default_clicked(UNUSED(bool checked)) {
	int index, type, vbutton = QVariant(((QPushButton *)sender())->property("myVbutton")).toInt();

	type = vbutton / MAX_STD_PAD_BUTTONS;
	vbutton -= (type * MAX_STD_PAD_BUTTONS);
	index = type == KEYBOARD ? data.cfg.id - 1 : js_jdev_index();

	info_entry_print(type, "");

	settings_inp_port_button_default(vbutton, &data.cfg.port, index, type);

	{
		pixmapButton *bt = findChild<pixmapButton *>("pushButton_" + SPT(type) + "_" + SPB(vbutton));

		if (type == KEYBOARD) {
			bt->setText(objInp::kbd_keyval_to_name(data.cfg.port.input[type][vbutton]));
		} else {
			js_pixmapButton(index, data.cfg.port.input[type][vbutton], bt);
		}
	}
}
void dlgStdPad::s_unset_clicked(UNUSED(bool checked)) {
	int type, vbutton = QVariant(((QPushButton *)sender())->property("myVbutton")).toInt();
	pixmapButton *bt;

	type = vbutton / MAX_STD_PAD_BUTTONS;
	vbutton -= (type * MAX_STD_PAD_BUTTONS);
	data.cfg.port.input[type][vbutton] = 0;

	info_entry_print(type, "");

	bt = findChild<pixmapButton *>("pushButton_" + SPT(type) + "_" + SPB(vbutton));
	bt->setPixmap(QPixmap(""));
	bt->setText("NULL");
}
void dlgStdPad::s_in_sequence_clicked(UNUSED(bool checked)) {
	data.seq.type = QVariant(((QPushButton *)sender())->property("myType")).toInt();

	info_entry_print(data.seq.type, "");
	data.seq.active = true;
	data.seq.counter = -1;
	data.seq.timer->start(150);
}
void dlgStdPad::s_unset_all_clicked(UNUSED(bool checked)) {
	int i, type = QVariant(((QPushButton *)sender())->property("myType")).toInt();

	info_entry_print(type, "");

	for (i = BUT_A; i < MAX_STD_PAD_BUTTONS; i++) {
		findChild<QPushButton *>("pushButton_" + SPT(type) + "_unset_" + SPB(i))->click();
	}
}
void dlgStdPad::s_defaults_clicked(UNUSED(bool checked)) {
	int i, index, type = QVariant(((QPushButton *)sender())->property("myType")).toInt();

	index = type == KEYBOARD ? data.cfg.id - 1 : js_jdev_index();

	info_entry_print(type, "");

	settings_inp_port_defaults(&data.cfg.port, index, type);

	for (i = BUT_A; i < MAX_STD_PAD_BUTTONS; i++) {
		pixmapButton *bt = findChild<pixmapButton *>("pushButton_" + SPT(type) + "_" + SPB(i));

		if (type == KEYBOARD) {
			bt->setText(objInp::kbd_keyval_to_name(data.cfg.port.input[type][i]));
		} else {
			js_pixmapButton(index, data.cfg.port.input[type][i], bt);
		}
	}
}
void dlgStdPad::s_deadzone_slider_value_changed(int value) {
	data.deadzone = value;
	deadzone_update_label(value);
}
void dlgStdPad::s_deadzone_default_clicked(UNUSED(bool checked)) {
	horizontalSlider_joy_Deadzone->setValue(settings_jsc_deadzone_default());
}
void dlgStdPad::s_combobox_controller_type_activated(int index) {
	BYTE state = RELEASED;
	int i;

	data.cfg.port.type_pad = index;

	if (((data.cfg.port.type_pad == CTRL_PAD_AUTO) && (machine.type != DENDY)) || (data.cfg.port.type_pad == CTRL_PAD_ORIGINAL)) {
		state = PRESSED;
	}
	for (i = 8; i < INPUT_DECODE_COUNTS; i++) {
		data.cfg.port.data[i] = state;
	}
}
void dlgStdPad::s_slider_td_value_changed(int value) {
	int type = QVariant(((QSlider *)sender())->property("myTurbo")).toInt();

	data.cfg.port.turbo[type].frequency = value;
	data.cfg.port.turbo[type].counter = 0;
	td_update_label(type, value);
}
void dlgStdPad::s_pad_joy_read_timer(void) {
	DBWORD value = js_jdev_read_in_dialog(&data.cfg.port.jguid);

	if (data.joy.value && (data.joy.value != value)) {
		unsigned int type, vbutton;

		type = data.vbutton / MAX_STD_PAD_BUTTONS;
		vbutton = data.vbutton - (type * MAX_STD_PAD_BUTTONS);

		info_entry_print(type, "");
		data.cfg.port.input[type][vbutton] = data.joy.value;
		js_pixmapButton(js_jdev_index(), data.cfg.port.input[type][vbutton], data.bp);
		data.joy.timer->stop();

		update_dialog();

		data.no_other_buttons = false;
		data.vbutton = 0;
	}

	data.joy.value = value;
}
void dlgStdPad::s_pad_in_sequence_timer(void) {
	static int order[MAX_STD_PAD_BUTTONS] = {
		UP,     DOWN,  LEFT,  RIGHT,
		SELECT, START, BUT_A, BUT_B,
		TRB_A,  TRB_B,
	};
	QPushButton *bt;

	if (data.no_other_buttons == true) {
		return;
	}

	if (++data.seq.counter == MAX_STD_PAD_BUTTONS) {
		data.seq.timer->stop();
		data.seq.active = false;
		update_dialog();
		return;
	}

	bt = findChild<pixmapButton *>("pushButton_" + SPT(data.seq.type) + "_" + SPB(order[data.seq.counter]));
	bt->setEnabled(true);
	bt->click();
}
void dlgStdPad::s_apply_clicked(UNUSED(bool checked)) {
	_cfg_port *cfg_port = ((_cfg_port *)((QPushButton *)sender())->property("myPointer").value<void *>());

	data.exec_js_init = (cfg_port->id != data.cfg.id) | (memcmp(cfg_port->port, &data.cfg.port, sizeof(_port)) != 0);
	cfg_port->id = data.cfg.id;
	memcpy(cfg_port->port, &data.cfg.port, sizeof(_port));

	if (js_jdev_index() != JS_NO_JOYSTICK) {
		_js_device *jdev = &jstick.jdd.devices[js_jdev_index()];

		settings_jsc_parse(jdev->index);
		memcpy(jdev->stdctrl, data.cfg.port.input[JOYSTICK], js_jdev_sizeof_stdctrl());
		jdev->deadzone = data.deadzone;
		settings_jsc_save();
	}

	close();
}
void dlgStdPad::s_discard_clicked(UNUSED(bool checked)) {
	close();
}

void dlgStdPad::s_et_update_joy_combo(void) {
	// se la combox e' aperta o sto configurando i pulsanti, non devo aggiornarne il contenuto
	if ((comboBox_joy_ID->view()->isVisible() == false) &&
		(data.seq.timer->isActive() == false) &&
		(data.joy.timer->isActive() == false)) {
		joy_combo_init();
	}
}

// ----------------------------------------------------------------------------------------------

pixmapButton::pixmapButton(QWidget *parent) : QPushButton(parent) {}
pixmapButton::~pixmapButton() {}

void pixmapButton::paintEvent(QPaintEvent *e) {
	QPushButton::paintEvent(e);

	if (!pixmap.isNull()) {
		QPixmap img = pixmap.scaled(iconSize(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
		const int y = (height() - img.height()) / 2;
		QPainter painter(this);

		painter.drawPixmap(5, y, img);
	}
}

void pixmapButton::setPixmap(const QPixmap &pixmap) {
	this->pixmap = pixmap;
}
