/*
 * dlgStdPad.cpp
 *
 *  Created on: 08/dic/2014
 *      Author: fhorse
 */

#include "dlgStdPad.moc"
#include "settingsObject.hpp"
#include "dlgInput.hpp"
#include "common.h"
#include "input.h"
#include "gui.h"

#define SPT(ind) QString(std_pad_input_type[ind])
#define SPB(ind) QString(std_pad_button[ind])

enum std_pad_enum { MAX_JOYSTICK = 16 };

static const char std_pad_input_type[2][4] = { "kbd", "joy" };
static const char std_pad_button[10][15] = {
	"A",      "B",     "Select", "Start",
	"Up",     "Down",  "Left",   "Right",
	"TurboA", "TurboB"
};

dlgStdPad::dlgStdPad(_cfg_port *cfg_port, QWidget *parent = 0) : QDialog(parent) {
	QFont f9, f8;

	f9.setPointSize(9);
	f9.setWeight(QFont::Light);
	f8.setPointSize(8);
	f8.setWeight(QFont::Light);

	memset(&data, 0x00, sizeof(data));
	memcpy(&data.cfg, cfg_port, sizeof(_cfg_port));

	setupUi(this);

	setFont(parent->font());

	groupBox_controller->setTitle(trUtf8("Controller %1 : Standard Pad").arg(cfg_port->id));
	tabWidget->setCurrentIndex(0);
	combo_id_init();

	for (int a = KEYBOARD; a <= JOYSTICK; a++) {
		QPlainTextEdit *txt;
		QPushButton *bt;

		txt = findChild<QPlainTextEdit *>("plainTextEdit_" + SPT(a) + "_info");

		if (txt->font().pointSize() > 9) {
			txt->setFont(f9);
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
			if (unset->font().pointSize() > 8) {
				unset->setFont(f8);
			}

			if (a == KEYBOARD) {
				bt->setText(inpObject::kbd_keyval_to_name(data.cfg.port.input[a][b]));
			} else {
				bt->setText(jsv_to_name(data.cfg.port.input[a][b]));
			}

			bt->installEventFilter(this);
			bt->setProperty("myVbutton", QVariant(vbutton));
			unset->setProperty("myVbutton", QVariant(vbutton));

			connect(bt, SIGNAL(clicked(bool)), this, SLOT(s_input_clicked(bool)));
			connect(unset, SIGNAL(clicked(bool)), this, SLOT(s_unset_clicked(bool)));
		}
	}

	{
		comboBox_Controller_type->addItem(trUtf8("Original"));
		comboBox_Controller_type->addItem(trUtf8("3rd-party"));
		comboBox_Controller_type->setCurrentIndex(data.cfg.port.type_pad);
		connect(comboBox_Controller_type, SIGNAL(activated(int)), this,
				SLOT(s_combobox_controller_type_activated(int)));
	}

	for (int i = TURBOA; i <= TURBOB; i++) {
		QSlider *tb = findChild<QSlider *>("horizontalSlider_" + SPB(i + TRB_A));
		QLabel *label = findChild<QLabel *>("label_value_slider_" + SPB(i + TRB_A));

		tb->setRange(1, TURBO_BUTTON_DELAY_MAX);
		tb->setProperty("myTurbo", QVariant(i));
		tb->setValue(data.cfg.port.turbo[i].frequency);
		connect(tb, SIGNAL(valueChanged(int)), this, SLOT(s_slider_td_value_changed(int)));

		label->setFixedWidth(label->sizeHint().width());
		td_update_label(i, data.cfg.port.turbo[i].frequency);
	}

	pushButton_Apply->setProperty("myPointer", QVariant::fromValue(static_cast<void *>(cfg_port)));
	connect(pushButton_Apply, SIGNAL(clicked(bool)), this, SLOT(s_apply_clicked(bool)));
	connect(pushButton_Discard, SIGNAL(clicked(bool)), this, SLOT(s_discard_clicked(bool)));

	setAttribute(Qt::WA_DeleteOnClose);
	setFixedSize(width(), height());

	setFocusPolicy(Qt::StrongFocus);
	groupBox_controller->setFocus(Qt::ActiveWindowFocusReason);

	installEventFilter(this);
}
dlgStdPad::~dlgStdPad() {}
void dlgStdPad::update_dialog(void) {
	bool mode = false;
	unsigned int joyId;

	if (data.in_sequence == true) {
		return;
	}

	tabWidget->setTabEnabled(KEYBOARD, true);
	tabWidget->setTabEnabled(JOYSTICK, true);

	// keyboard
	label_kbd_ID->setEnabled(false);
	comboBox_kbd_ID->setEnabled(false);

	setEnable_tab_buttons(KEYBOARD, true);

	plainTextEdit_kbd_info->setEnabled(true);

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

	if ((comboBox_joy_ID->count() > 1) && (joyId != name_to_jsn("NULL"))) {
		mode = true;
	} else {
		mode = false;
	}

	setEnable_tab_buttons(JOYSTICK, mode);

	plainTextEdit_joy_info->setEnabled(mode);

	pushButton_joy_Sequence->setEnabled(mode);
	pushButton_joy_Unset_all->setEnabled(mode);
	pushButton_joy_Defaults->setEnabled(mode);

	// misc
	groupBox_Misc->setEnabled(true);
}
void dlgStdPad::combo_id_init(void) {
	BYTE disabled_line = 0, count = 0, current_line = name_to_jsn("NULL");

	comboBox_kbd_ID->addItem(trUtf8("Keyboard"));

	for (int a = 0; a <= MAX_JOYSTICK; a++) {
		BYTE id = a;

		if (a < MAX_JOYSTICK) {
			if (js_is_connected(id) == EXIT_ERROR) {
				continue;
			}

			if (id == data.cfg.port.joy_id) {
				current_line = count;
			}

			comboBox_joy_ID->addItem(QString("js%1: ").arg(id) + js_name_device(id));
		} else {
			if (count == 0) {
				break;
			}
			comboBox_joy_ID->addItem(trUtf8("Disabled"));
			id = name_to_jsn("NULL");
			disabled_line = count;
		}

		comboBox_joy_ID->setItemData(count, id);
		count++;
	}

	if (count == 0) {
		comboBox_joy_ID->addItem(trUtf8("No usable device"));
		tab_joy->setEnabled(false);
	} else {
		tab_joy->setEnabled(true);
	}

	if (count > 0) {
		if (data.cfg.port.joy_id == name_to_jsn("NULL")
				|| (current_line == name_to_jsn("NULL"))) {
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
	tabWidget->setTabEnabled(type == KEYBOARD ? JOYSTICK : KEYBOARD, false);

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
	findChild<QPlainTextEdit *>("plainTextEdit_" + SPT(type) + "_info")->setPlainText(txt);
}
void dlgStdPad::js_press_event(void) {
	unsigned int type, vbutton;
	DBWORD value = 0;

	type = data.vbutton / MAX_STD_PAD_BUTTONS;
	vbutton = data.vbutton - (type * MAX_STD_PAD_BUTTONS);

	if (data.cfg.port.joy_id == name_to_jsn("NULL")) {
		info_entry_print(type, "Select device first");
		update_dialog();
		return;
	}

	if (js_read_in_dialog(data.cfg.port.joy_id, &data.wait_js_input, &value, MAX_JOYSTICK)
			== EXIT_OK) {
		data.cfg.port.input[type][vbutton] = value;
		data.bp->setText(jsv_to_name(value));
	}

	info_entry_print(type, "");
	update_dialog();

	data.wait_js_input = FALSE;
	data.no_other_buttons = false;
	data.vbutton = 0;
}
void dlgStdPad::td_update_label(int type, int value) {
	QLabel *label = findChild<QLabel *>("label_value_slider_" + SPB(type + TRB_A));

	label->setText(QString("%1").arg(value, 2));
}
bool dlgStdPad::eventFilter(QObject *obj, QEvent *event) {
	if (obj == this) {
		switch (event->type()) {
			case QEvent::Close:
				data.wait_js_input = FALSE;
				data.force_exit_in_sequence = true;
				data.in_sequence = false;
				data.no_other_buttons = false;
				data.vbutton = 0;
				qobject_cast<dlgInput *>(parent())->show();
				break;
			case QEvent::KeyPress: {
				QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
				int type, vbutton;

				if (data.no_other_buttons == false) {
					return (true);
				}

				type = data.vbutton / MAX_STD_PAD_BUTTONS;
				vbutton = data.vbutton - (type * MAX_STD_PAD_BUTTONS);

				if (type == KEYBOARD) {
					if (keyEvent->key() != Qt::Key_Escape) {
						data.cfg.port.input[type][vbutton] = inpObject::kbd_keyval_decode(keyEvent);
					}
					data.bp->setText(
					        inpObject::kbd_keyval_to_name(data.cfg.port.input[type][vbutton]));
				} else {
					// quando sto configurando il joystick, l'unico input da tastiera
					// che accetto e' l'escape.
					if (keyEvent->key() == Qt::Key_Escape) {
						data.bp->setText(jsv_to_name(data.cfg.port.input[type][vbutton]));
					} else {
						return (true);
					}
				}

				info_entry_print(type, "");
				update_dialog();

				data.wait_js_input = FALSE;
				data.no_other_buttons = false;
				data.vbutton = 0;

				return (true);
			}
			default:
				break;
		}
	} else {
		// controllo il comportamento dello [Space] e dell'[Enter] dei QPushButton
		// che altrimenti non verrebbero mai intercettati perche' gestiti direttamente
		// dalle QT (nel caso di QPushButton).
		switch (event->type()) {
			case QEvent::KeyPress:
				if (data.no_other_buttons == true) {
					return (this->eventFilter(this, event));
				}
				break;
			default:
				break;
		}
	}

	return (QObject::eventFilter(obj, event));
}
void dlgStdPad::s_combobox_joy_activated(int index) {
	unsigned int id = qobject_cast<QComboBox *>(sender())->itemData(index).toInt();

	data.cfg.port.joy_id = id;
	update_dialog();
}
void dlgStdPad::s_input_clicked(bool checked) {
	int vbutton = QVariant(qobject_cast<QPushButton *>(sender())->property("myVbutton")).toInt();
	int type;

	if (data.no_other_buttons == true) {
		return;
	}

	data.bp = qobject_cast<QPushButton *>(sender());
	data.vbutton = vbutton;

	type = vbutton / MAX_STD_PAD_BUTTONS;
	vbutton -= (type * MAX_STD_PAD_BUTTONS);

	disable_tab_and_other(type, vbutton);

	data.no_other_buttons = true;
	data.bp->setText("...");

	data.bp->setFocus(Qt::ActiveWindowFocusReason);

	if (type == KEYBOARD) {
		info_entry_print(type,
			"Press a key (ESC for the previous value \"" +
			inpObject::kbd_keyval_to_name(data.cfg.port.input[type][vbutton]) + "\")"
		);
	} else {
		info_entry_print(type,
			"Press a key (ESC for the previous value \"" +
			QString(jsv_to_name(data.cfg.port.input[type][vbutton])) + "\")"
		);
		js_press_event();
	}
}
void dlgStdPad::s_unset_clicked(bool checked) {
	int vbutton = QVariant(qobject_cast<QPushButton *>(sender())->property("myVbutton")).toInt();
	int type;

	type = vbutton / MAX_STD_PAD_BUTTONS;
	vbutton -= (type * MAX_STD_PAD_BUTTONS);
	data.cfg.port.input[type][vbutton] = 0;

	info_entry_print(type, "");

	findChild<QPushButton *>("pushButton_" + SPT(type) + "_" + SPB(vbutton))->setText("NULL");
}
void dlgStdPad::s_in_sequence_clicked(bool checked) {
	int type = QVariant(qobject_cast<QPushButton *>(sender())->property("myType")).toInt();
	static int order[MAX_STD_PAD_BUTTONS] = {
		UP,     DOWN,  LEFT,  RIGHT,
		SELECT, START, BUT_A, BUT_B,
		TRB_A,  TRB_B,
	};

	info_entry_print(type, "");
	data.in_sequence = true;

	for (int i = BUT_A; i < MAX_STD_PAD_BUTTONS; i++) {
		QPushButton *bt;

		if (data.force_exit_in_sequence == true) {
			return;
		}

		bt = findChild<QPushButton *>("pushButton_" + SPT(type) + "_" + SPB(order[i]));
		bt->setEnabled(true);
		bt->click();

		while (data.no_other_buttons == true) {
			gui_flush();
			gui_sleep(30);
		}
	}

	data.in_sequence = false;
	update_dialog();
}
void dlgStdPad::s_unset_all_clicked(bool checked) {
	int type = QVariant(qobject_cast<QPushButton *>(sender())->property("myType")).toInt();

	info_entry_print(type, "");

	for (int i = BUT_A; i < MAX_STD_PAD_BUTTONS; i++) {
		findChild<QPushButton *>("pushButton_" + SPT(type) + "_unset_" + SPB(i))->click();
	}
}
void dlgStdPad::s_defaults_clicked(bool checked) {
	int type = QVariant(qobject_cast<QPushButton *>(sender())->property("myType")).toInt();

	info_entry_print(type, "");

	settings_inp_port_default(&data.cfg.port, data.cfg.id - 1, type);

	for (int i = BUT_A; i < MAX_STD_PAD_BUTTONS; i++) {
		QPushButton *bt = findChild<QPushButton *>("pushButton_" + SPT(type) + "_" + SPB(i));

		if (type == KEYBOARD) {
			bt->setText(inpObject::kbd_keyval_to_name(data.cfg.port.input[type][i]));
		} else {
			bt->setText(jsv_to_name(data.cfg.port.input[type][i]));
		}
	}
}
void dlgStdPad::s_combobox_controller_type_activated(int index) {
	BYTE state = RELEASED;

	data.cfg.port.type_pad = index;

	if (data.cfg.port.type_pad == CTRL_PAD_ORIGINAL) {
		state = PRESSED;
	}

	for (int b = 8; b < 24; b++) {
		data.cfg.port.data[b] = state;
	}
}
void dlgStdPad::s_slider_td_value_changed(int value) {
	int type = QVariant(qobject_cast<QSlider *>(sender())->property("myTurbo")).toInt();

	data.cfg.port.turbo[type].frequency = value;
	data.cfg.port.turbo[type].counter = 0;
	td_update_label(type, value);
}
void dlgStdPad::s_apply_clicked(bool checked) {
	_cfg_port *cfg_port = static_cast<_cfg_port *>(qobject_cast<QPushButton *>(sender())->property(
		"myPointer").value<void *>());

	memcpy(cfg_port, &data.cfg, sizeof(_cfg_port));

	close();
}
void dlgStdPad::s_discard_clicked(bool checked) {
	close();
}
