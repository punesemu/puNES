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

#include <QtCore/QBuffer>
#include <QtGui/QPainter>
#include <QtWidgets/QAbstractItemView>
#include <QtWidgets/QStylePainter>
#include "objSettings.hpp"
#include "dlgStdPad.hpp"
#include "mainWindow.hpp"
#include "clock.h"
#include "gui.h"
#include "input/standard_controller.h"

#define SPT(ind) QString(std_pad_input_type[ind])
#define SPB(ind) QString(std_pad_button[ind])

static constexpr char std_pad_input_type[2][4] = { "kbd", "joy" };
static constexpr char std_pad_button[10][15] = {
	"A",      "B",     "Select", "Start",
	"Up",     "Down",  "Left",   "Right",
	"TurboA", "TurboB"
};

_joy_list joy_list;

typedef struct _color_label {
	_color_label() :
	normal(theme::get_theme_adaptive_color("#EFEFEF")),
	selected(theme::get_theme_adaptive_color("#BBF591")) {}
	QColor normal;
	QColor selected;
} _color_label;
typedef struct _color_frame {
	_color_frame() :
	button(theme::get_theme_adaptive_color("#BAB9B7")),
	kbd(theme::get_theme_adaptive_color(QApplication::palette().light().color().darker(theme::is_dark_theme() ? 105 : 115))),
	joy(theme::get_theme_adaptive_color(QApplication::palette().light().color().darker(theme::is_dark_theme() ? 105 : 115))),
	tbrd(theme::get_theme_adaptive_color("#A69F8A")) {}
	QColor button;
	QColor kbd;
	QColor joy;
	QColor tbrd;
} _color_frame;

// ----------------------------------------------------------------------------------------------

wdgDlgStdPad::wdgDlgStdPad(QWidget *parent, _cfg_port *cfg_port) : wdgTitleBarDialog(parent) {
	setAttribute(Qt::WA_DeleteOnClose);
	wd = new dlgStdPad(this, cfg_port);
	setWindowTitle(wd->windowTitle());
	setWindowIcon(QIcon(":/icon/icons/nes_file.svgz"));
	set_border_color("mediumvioletred");
	set_buttons(barButton::Close);
	set_permit_resize(false);
	add_widget(wd);

	connect(wd->pushButton_Discard, SIGNAL(clicked(bool)), this, SLOT(close(void)));
}
wdgDlgStdPad::~wdgDlgStdPad() = default;

// ----------------------------------------------------------------------------------------------

dlgStdPad::dlgStdPad(QWidget *parent, _cfg_port *cfg_port) : QWidget(parent) {
	int i;

	memset(&data, 0x00, sizeof(data));

	last_js_index = MAX_JOYSTICK;

	data.cfg.id = cfg_port->id;
	memcpy(&data.cfg.port, cfg_port->port, sizeof(_port));

	setupUi(this);

	stylesheet_update();

	setFocusProxy(tabWidget_kbd_joy);

	frame_kbd_buttons->setLayoutDirection(Qt::LeftToRight);
	frame_joy_buttons->setLayoutDirection(Qt::LeftToRight);

	groupBox_controller->setTitle(tr("Controller %1 : Standard Pad").arg(cfg_port->id));

	comboBox_kbd_ID->addItem(tr("Keyboard"));

	connect(comboBox_joy_ID, SIGNAL(activated(int)), this, SLOT(s_combobox_joy_activated(int)));
	connect(comboBox_joy_ID, SIGNAL(currentIndexChanged(int)), this, SLOT(s_combobox_joy_index_changed(int)));
	connect(horizontalSlider_joy_Deadzone, SIGNAL(valueChanged(int)), this, SLOT(s_deadzone_slider_value_changed(int)));

	joy_combo_init();

	for (i = KEYBOARD; i < INPUT_TYPES; i++) {
		themePushButton *bt = findChild<themePushButton *>("pushButton_" + SPT(i) + "_Sequence");

		bt->setProperty("myType", QVariant(i));
		connect(bt, SIGNAL(clicked(bool)), this, SLOT(s_in_sequence_clicked(bool)));

		bt = findChild<themePushButton *>("pushButton_" + SPT(i) + "_Unset_all");
		bt->setProperty("myType", QVariant(i));
		connect(bt, SIGNAL(clicked(bool)), this, SLOT(s_unset_all_clicked(bool)));

		bt = findChild<themePushButton *>("pushButton_" + SPT(i) + "_Defaults");
		bt->setProperty("myType", QVariant(i));
		connect(bt, SIGNAL(clicked(bool)), this, SLOT(s_defaults_clicked(bool)));

		for (int a = BUT_A; a < MAX_STD_PAD_BUTTONS; a++) {
			const int vbutton = a + (i * MAX_STD_PAD_BUTTONS);
			QPushButton *def = findChild<QPushButton *>("pushButton_" + SPT(i) + "_default_" + SPB(a));
			QPushButton *unset = findChild<QPushButton *>("pushButton_" + SPT(i) + "_unset_" + SPB(a));
			pixmapPushButton *pbt = findChild<pixmapPushButton *>("pushButton_" + SPT(i) + "_" + SPB(a));

			pbt->setIcon(QIcon(""));
			if (i == KEYBOARD) {
				pbt->setText(objInp::kbd_keyval_to_name(data.cfg.port.input[i][a]));
			} else {
				js_pixmapPushButton(js_jdev_index(), data.cfg.port.input[i][a], pbt);
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

	label_kbd_Deadzone_value_slider->setFixedWidth(QLabel("00").sizeHint().width());
	label_kbd_Deadzone_value_slider->setText(QString("%1").arg(0, 2));
	label_joy_Deadzone_value_slider->setFixedWidth(QLabel("00").sizeHint().width());
	label_joy_Deadzone_value_slider->setText(QString("%1").arg(0, 2));
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
		const int w = QLabel("00").sizeHint().width();

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
dlgStdPad::~dlgStdPad() = default;

bool dlgStdPad::eventFilter(QObject *obj, QEvent *event) {
	// mi interessa intercettare tutti i keyPress che arrivano, non solo quelli
	// che riguardano questo dialog.
	switch (event->type()) {
		case QEvent::KeyPress:
			if (data.no_other_buttons) {
				return (keypress((QKeyEvent *)event));
			}
			break;
		default:
			break;
	}
	return (QWidget::eventFilter(obj, event));
}
void dlgStdPad::changeEvent(QEvent *event) {
	if (event->type() == QEvent::LanguageChange) {
		retranslateUi(this);
	} else if (event->type() == QEvent::PaletteChange) {
		stylesheet_update();
	} else {
		QWidget::changeEvent(event);
	}
}
void dlgStdPad::closeEvent(QCloseEvent *event) {
	data.joy.timer->stop();
	data.seq.timer->stop();
	data.seq.active = false;

	if (data.exec_js_init) {
		js_quit(FALSE);
		js_init(FALSE);
	}

	mainwin->wd->shcjoy_start();

	data.no_other_buttons = false;
	data.vbutton = 0;

	gui.dlg_tabWidget_kbd_joy_index[data.cfg.id] = tabWidget_kbd_joy->currentIndex();

	QWidget::closeEvent(event);
}

void dlgStdPad::stylesheet_update(void) const {
	tabWidget_kbd_joy->setStyleSheet("QTabWidget { font-weight: normal; }");

	frame_kbd_buttons->setStyleSheet(stylesheet_frame(_color_frame().tbrd, _color_frame().kbd));
	frame_joy_buttons->setStyleSheet(stylesheet_frame(_color_frame().tbrd, _color_frame().joy));

	for (int i = KEYBOARD; i < INPUT_TYPES; i++) {
		for (int a = BUT_A; a < MAX_STD_PAD_BUTTONS; a++) {
			QPushButton *def = findChild<QPushButton *>("pushButton_" + SPT(i) + "_default_" + SPB(a));
			QPushButton *unset = findChild<QPushButton *>("pushButton_" + SPT(i) + "_unset_" + SPB(a));
			pixmapPushButton *pbt = findChild<pixmapPushButton *>("pushButton_" + SPT(i) + "_" + SPB(a));
			QFrame *fbt = findChild<QFrame *>("frame_" + SPT(i) + "_" + SPB(a));
			QLabel *lb = findChild<QLabel *>("label_" + SPT(i) + "_" + SPB(a));

			def->setStyleSheet(stylesheet_left_button());
			unset->setStyleSheet(stylesheet_right_button());
			pbt->setStyleSheet(stylesheet_pixmapbutton());
			fbt->setStyleSheet(stylesheet_frame(_color_frame().button, _color_frame().button));
			lb->setStyleSheet(stylesheet_label(_color_label().normal));
		}
	}
}

QString dlgStdPad::stylesheet_label(const QColor &background) {
	const QColor border = theme::get_theme_adaptive_color("#BAB9B7");
	const QColor text = theme::get_theme_color("#000000");
	const QColor disabled_border = theme::get_theme_adaptive_color("#BABDB6");
	const QColor disabled_background = theme::get_theme_adaptive_color("#EFEFEF");
	const QColor disabled_text = theme::get_theme_color("#BABDB6");
	const QString stylesheet =
		"QLabel {"\
		"	background-color: %0;"\
		"	border-width: 1px;"\
		"	border-color: %1;"\
		"	border-style: solid;"\
		"	padding: 2px;"\
		"	padding-left: 3px;"\
		"	padding-right: 3px;"\
		"	border-radius: 3px;"\
		"	color: %3;"\
		"	font-size: 8pt;"\
		"	font-weight: bold;"\
		"}"\
		"QLabel:disabled {"\
		"	border-color: %4;"\
		"	background-color: %5;"\
		"	color: %6;"\
		"}";

	return (stylesheet
		.arg(background.name())
		.arg(border.name())
		.arg(text.name())
		.arg(disabled_border.name())
		.arg(disabled_background.name())
		.arg(disabled_text.name()));
}
QString dlgStdPad::stylesheet_frame(const QColor &border, const QColor &background) {
	const QString stylesheet =
		"QFrame {"\
		"	border-width: 1px;"\
		"	border-color: %0;"\
		"	border-style: solid;"\
		"	border-radius: 6px;"\
		"	background-color: %1;"\
		"	font-size: 8pt;"\
		"}";

	return (stylesheet
		.arg(border.name())
		.arg(background.name()));
}
QString dlgStdPad::stylesheet_pixmapbutton(void) {
	const QColor border = theme::get_theme_adaptive_color("#8F8F91");
	const QColor gradient0 = theme::get_theme_color("#F6F7FA");
	const QColor gradient1 = theme::get_theme_color("#DADBDE");
	const QColor disabled_border = theme::get_theme_adaptive_color("#C2C2C2");
	const QColor disabled_background = theme::get_theme_adaptive_color("#EFEFEF");
	const QColor disabled_text = theme::get_theme_adaptive_color("#BABDB6");
	const QColor hover_gradient0 = theme::get_theme_adaptive_color("#DADBDE");
	const QColor hover_gradient1 = theme::get_theme_adaptive_color("#F6F7FA");
	const QString stylesheet =
		"QPushButton {"\
		"	border-width: 1px;"\
		"	border-color: %0;"\
		"	border-style: solid;"\
		"	border-top-left-radius: 6px;"\
		"	border-top-right-radius: 6px;"\
		"	border-bottom-left-radius: 0px;"\
		"	border-bottom-right-radius: 0px;"\
		"	background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 %1, stop: 1 %2);"\
		"	padding: 5px;"\
		"	padding-left: 3px;"\
		"	padding-right: 3px;"\
		"	min-width: 80px;"\
		"	font-size: 8pt;"\
		"}"\
		"QPushButton:disabled {"\
		"	border-color: %3;"\
		"	background-color: %4;"\
		"	color: %5;"\
		"}"\
		"QPushButton:focus {"\
		"	border-color: %8;"\
		"	background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 %6, stop: 1 %7);"\
		"}"\
		"QPushButton:hover {"\
		"	background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 %6, stop: 1 %7);"\
		"}"\
		"QPushButton:hover:focus {"\
		"	border-color: %8;"\
		"}";

	return (stylesheet
		.arg(border.name())
		.arg(gradient0.name())
		.arg(gradient1.name())
		.arg(disabled_border.name())
		.arg(disabled_background.name())
		.arg(disabled_text.name())
		.arg(hover_gradient0.name())
		.arg(hover_gradient1.name())
		.arg(theme::get_focus_color().name()));
}
QString dlgStdPad::stylesheet_left_button(void) const {
	const QColor base_color = theme::get_theme_color(palette().light().color());
	const QColor lightGray = theme::is_dark_theme() ? base_color.lighter(125) : base_color.darker(145);
	const QColor darkGray = lightGray.darker(125);
	const QString stylesheet =
		"QPushButton {"\
		"	margin-top: 1;"\
		"	border-top: 1px;"\
		"	border-bottom: 1px;"\
		"	border-left: 1px;"\
		"	border-style: solid;"\
		"	border-bottom-left-radius: 6px;"\
		"	border-color: %1;"\
		"	padding: 2px;"\
		"}"\
		"QPushButton:disabled {"\
		"	color: gray;"\
		"}"\
		"QPushButton:focus {"\
		"	border-color: %2;"\
		"	background-color: %0;"\
		"}"\
		"QPushButton:hover {"\
		"	background-color: %0;"\
		"}"\
		"QPushButton:hover:focus {"\
		"	border-color: %2;"\
		"}"\
		"QPushButton:pressed {"\
		"	margin-top: 1;"\
		"	border-top: 1px;"\
		"	border-bottom: 1px;"\
		"	border-left: 1px;"\
		"	border-style: inset;"\
		"	border-bottom-left-radius: 6px;"\
		"	border-color: %1;"\
		"	padding: 2px;"\
		"	background-color: %1;"\
		"}"\
		"QPushButton:pressed:focus {"\
		"	border-color: %2;"\
		"}";

	return (stylesheet
		.arg(lightGray.name())
		.arg(darkGray.name())
		.arg(theme::get_focus_color().name()));
}
QString dlgStdPad::stylesheet_right_button(void) const {
	return (stylesheet_left_button()
		.replace("border-left", "border-right")
		.replace("border-bottom-left-radius", "border-bottom-right-radius"));
}

bool dlgStdPad::keypress(QKeyEvent *event) {
	const int type = data.vbutton / MAX_STD_PAD_BUTTONS;
	const int vbutton = data.vbutton - (type * MAX_STD_PAD_BUTTONS);

	if (type == KEYBOARD) {
		if (event->key() != Qt::Key_Escape) {
			data.cfg.port.input[type][vbutton] = objInp::kbd_keyval_decode(event);
		}
		data.bp->setText(objInp::kbd_keyval_to_name(data.cfg.port.input[type][vbutton]));
	} else {
		// quando sto configurando il joystick, l'unico input da tastiera che accetto e' l'escape.
		if (event->key() == Qt::Key_Escape) {
			data.joy.timer->stop();
			js_pixmapPushButton(js_jdev_index(), data.cfg.port.input[type][vbutton], data.bp);
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
void dlgStdPad::update_dialog(void) const {
	bool mode = false;

	if (data.seq.active) {
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
	const unsigned int joy_index = comboBox_joy_ID->itemData(comboBox_joy_ID->currentIndex()).toInt();

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

	comboBox_joy_ID->blockSignals(true);
	comboBox_joy_ID->clear();

	for (int i = 0; i < joy_list.count; i++) {
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
void dlgStdPad::setEnable_tab_buttons(const int type, const bool mode) const {
	for (int i = BUT_A; i < MAX_STD_PAD_BUTTONS; i++) {
		findChild<QLabel *>("label_" + SPT(type) + "_" + SPB(i))->setEnabled(mode);
		findChild<QLabel *>("label_" + SPT(type) + "_" + SPB(i))->setStyleSheet(stylesheet_label(_color_label().normal));
		findChild<pixmapPushButton *>("pushButton_" + SPT(type) + "_" + SPB(i))->setEnabled(mode);
		findChild<QPushButton *>("pushButton_" + SPT(type) + "_default_" + SPB(i))->setEnabled(mode);
		findChild<QPushButton *>("pushButton_" + SPT(type) + "_unset_" + SPB(i))->setEnabled(mode);
	}
}
void dlgStdPad::disable_tab_and_other(const int type, const int vbutton) const {
	// altro tab
	tabWidget_kbd_joy->setTabEnabled(type == KEYBOARD ? JOYSTICK : KEYBOARD, false);

	// ID
	findChild<QLabel *>("label_" + SPT(type) + "_ID")->setEnabled(false);
	findChild<QComboBox *>("comboBox_" + SPT(type) + "_ID")->setEnabled(false);

	setEnable_tab_buttons(type, false);
	findChild<QLabel *>("label_" + SPT(type) + "_" + SPB(vbutton))->setEnabled(true);
	findChild<QLabel *>("label_" + SPT(type) + "_" + SPB(vbutton))->setStyleSheet(stylesheet_label(_color_label().selected));
	findChild<pixmapPushButton *>("pushButton_" + SPT(type) + "_" + SPB(vbutton))->setEnabled(true);

	// in sequence, unset all, default
	findChild<themePushButton *>("pushButton_" + SPT(type) + "_Sequence")->setEnabled(false);
	findChild<themePushButton *>("pushButton_" + SPT(type) + "_Unset_all")->setEnabled(false);
	findChild<themePushButton *>("pushButton_" + SPT(type) + "_Defaults")->setEnabled(false);

	// misc
	groupBox_Misc->setEnabled(false);
}
void dlgStdPad::info_entry_print(const int type, const QString &txt) const {
	findChild<QLabel *>("label_" + SPT(type) + "_info")->setText(txt);
}
void dlgStdPad::js_press_event(void) {
	const int type = data.vbutton / MAX_STD_PAD_BUTTONS;

	if (js_is_null(&data.cfg.port.jguid)) {
		info_entry_print(type, tr("Select device first"));
		update_dialog();
		return;
	}

	data.joy.value = 0;
	data.joy.timer->start(150);
}
void dlgStdPad::td_update_label(const int type, const int value) const {
	QLabel *label = findChild<QLabel *>("label_value_slider_" + SPB(type + TRB_A));

	label->setText(QString("%1").arg(value, 2));
}
void dlgStdPad::deadzone_update_label(const int value) const {
	label_joy_Deadzone_value_slider->setText(QString("%1").arg(value, 2));
}
void dlgStdPad::js_pixmapPushButton(const int index, const DBWORD input, pixmapPushButton *bt) {
	QString icon, desc;

	gui_js_joyval_icon_desc(index, input, &icon, &desc);
	bt->setIcon(QIcon());
	bt->setPixmap(QPixmap(icon));
	bt->setText(desc);
}

int dlgStdPad::js_jdev_index(void) const {
	int jdev_index = JS_NO_JOYSTICK;

	if (comboBox_joy_ID->currentData().isValid()) {
		jdev_index = comboBox_joy_ID->currentData().toInt();
	}
	return (jdev_index);
}

void dlgStdPad::s_combobox_joy_activated(int index) {
	const int jdev_index = ((QComboBox *)sender())->itemData(index).toInt();

	if (comboBox_joy_ID->count() == 1) {
		return;
	}
	js_guid_set(jdev_index, &data.cfg.port.jguid);
	update_dialog();
}
void dlgStdPad::s_combobox_joy_index_changed(UNUSED(int index)) {
	const int jdev_index = js_jdev_index();

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

	for (int a = KEYBOARD; a < INPUT_TYPES; a++) {
		for (int b = BUT_A; b < MAX_STD_PAD_BUTTONS; b++) {
			pixmapPushButton *bt = findChild<pixmapPushButton *>("pushButton_" + SPT(a) + "_" + SPB(b));

			if (a == KEYBOARD) {
				bt->setText(objInp::kbd_keyval_to_name(data.cfg.port.input[a][b]));
			} else {
				js_pixmapPushButton(jdev_index, data.cfg.port.input[a][b], bt);
			}
		}
	}
}
void dlgStdPad::s_input_clicked(UNUSED(bool checked)) {
	int vbutton = QVariant(((pixmapPushButton *)sender())->property("myVbutton")).toInt();
	const int type = vbutton / MAX_STD_PAD_BUTTONS;

	if (data.no_other_buttons) {
		return;
	}

	data.bp = ((pixmapPushButton *)sender());
	data.vbutton = vbutton;

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

		info_entry_print(type, tr("Press a key (ESC for the previous value \"%1\"%2)").arg(desc, icon));
		js_press_event();
	}
}
void dlgStdPad::s_default_clicked(UNUSED(bool checked)) {
	int vbutton = QVariant(((QPushButton *)sender())->property("myVbutton")).toInt();
	const int type = vbutton / MAX_STD_PAD_BUTTONS;
	const int index = type == KEYBOARD ? data.cfg.id - 1 : js_jdev_index();

	vbutton -= (type * MAX_STD_PAD_BUTTONS);

	info_entry_print(type, "");

	settings_inp_port_button_default(vbutton, &data.cfg.port, index, type);

	{
		pixmapPushButton *bt = findChild<pixmapPushButton *>("pushButton_" + SPT(type) + "_" + SPB(vbutton));

		if (type == KEYBOARD) {
			bt->setText(objInp::kbd_keyval_to_name(data.cfg.port.input[type][vbutton]));
		} else {
			js_pixmapPushButton(index, data.cfg.port.input[type][vbutton], bt);
		}
	}
}
void dlgStdPad::s_unset_clicked(UNUSED(bool checked)) {
	int vbutton = QVariant(((QPushButton *)sender())->property("myVbutton")).toInt();
	const int type = vbutton / MAX_STD_PAD_BUTTONS;

	vbutton -= (type * MAX_STD_PAD_BUTTONS);
	data.cfg.port.input[type][vbutton] = 0;

	info_entry_print(type, "");

	pixmapPushButton *bt = findChild<pixmapPushButton *>("pushButton_" + SPT(type) + "_" + SPB(vbutton));
	bt->setPixmap(QPixmap(""));
	bt->setText("NULL");
}
void dlgStdPad::s_in_sequence_clicked(UNUSED(bool checked)) {
	data.seq.type = QVariant(((themePushButton *)sender())->property("myType")).toInt();

	info_entry_print(data.seq.type, "");
	data.seq.active = true;
	data.seq.counter = -1;
	data.seq.timer->start(150);
}
void dlgStdPad::s_unset_all_clicked(UNUSED(bool checked)) const {
	const int type = QVariant(((themePushButton *)sender())->property("myType")).toInt();

	info_entry_print(type, "");

	for (int i = BUT_A; i < MAX_STD_PAD_BUTTONS; i++) {
		findChild<QPushButton *>("pushButton_" + SPT(type) + "_unset_" + SPB(i))->click();
	}
}
void dlgStdPad::s_defaults_clicked(UNUSED(bool checked)) {
	const int type = QVariant(((themePushButton *)sender())->property("myType")).toInt();
	const int index = type == KEYBOARD ? data.cfg.id - 1 : js_jdev_index();

	info_entry_print(type, "");

	settings_inp_port_defaults(&data.cfg.port, index, type);

	for (int i = BUT_A; i < MAX_STD_PAD_BUTTONS; i++) {
		pixmapPushButton *bt = findChild<pixmapPushButton *>("pushButton_" + SPT(type) + "_" + SPB(i));

		if (type == KEYBOARD) {
			bt->setText(objInp::kbd_keyval_to_name(data.cfg.port.input[type][i]));
		} else {
			js_pixmapPushButton(index, data.cfg.port.input[type][i], bt);
		}
	}
}
void dlgStdPad::s_deadzone_slider_value_changed(int value) {
	data.deadzone = value;
	deadzone_update_label(value);
}
void dlgStdPad::s_deadzone_default_clicked(UNUSED(bool checked)) const {
	horizontalSlider_joy_Deadzone->setValue(settings_jsc_deadzone_default());
}
void dlgStdPad::s_combobox_controller_type_activated(int index) {
	BYTE state = RELEASED;

	data.cfg.port.type_pad = index;

	if (((data.cfg.port.type_pad == CTRL_PAD_AUTO) && (machine.type != DENDY)) || (data.cfg.port.type_pad == CTRL_PAD_ORIGINAL)) {
		state = PRESSED;
	}
	for (int i = 8; i < INPUT_DECODE_COUNTS; i++) {
		input_data_set_standard_controller(i, state, &data.cfg.port);
	}
}
void dlgStdPad::s_slider_td_value_changed(int value) {
	const int type = QVariant(((QSlider *)sender())->property("myTurbo")).toInt();

	data.cfg.port.turbo[type].frequency = value;
	data.cfg.port.turbo[type].counter = 0;
	td_update_label(type, value);
}
void dlgStdPad::s_pad_joy_read_timer(void) {
	const DBWORD value = js_jdev_read_in_dialog(&data.cfg.port.jguid);

	if (data.joy.value && (data.joy.value != value)) {
		const int type = data.vbutton / MAX_STD_PAD_BUTTONS;
		const int vbutton = data.vbutton - (type * MAX_STD_PAD_BUTTONS);

		info_entry_print(type, "");
		data.cfg.port.input[type][vbutton] = data.joy.value;
		js_pixmapPushButton(js_jdev_index(), data.cfg.port.input[type][vbutton], data.bp);
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

	if (data.no_other_buttons) {
		return;
	}

	if (++data.seq.counter == MAX_STD_PAD_BUTTONS) {
		data.seq.timer->stop();
		data.seq.active = false;
		update_dialog();
		return;
	}

	pixmapPushButton *bt = findChild<pixmapPushButton *>(
		"pushButton_" + SPT(data.seq.type) + "_" + SPB(order[data.seq.counter]));
	bt->setEnabled(true);
	bt->click();
}
void dlgStdPad::s_apply_clicked(UNUSED(bool checked)) {
	_cfg_port *cfg_port = ((_cfg_port *)((themePushButton *)sender())->property("myPointer").value<void *>());

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

	pushButton_Discard->click();
}

void dlgStdPad::s_et_update_joy_combo(void) {
	// se la combox e' aperta o sto configurando i pulsanti, non devo aggiornarne il contenuto
	if (!comboBox_joy_ID->view()->isVisible() &&
		!data.no_other_buttons &&
		!data.seq.timer->isActive() &&
		!data.joy.timer->isActive()) {
		joy_combo_init();
	}
}
