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
#include "dlgJsc.moc"
#include "dlgStdPad.hpp"
#include "settings.h"
#include "gui.h"

dlgJsc::dlgJsc(QWidget *parent) : QDialog(parent) {
	unsigned int i;

	js_guid_unset(&guid);
	timer = new QTimer(this);
	mutex = new QMutex();
	first_time = true;

	setupUi(this);

#if defined (_WIN32)
	label_Device->setVisible(false);
	label_Device_2points->setVisible(false);
	label_Device_desc->setVisible(false);
#endif

	if (font().pointSize() > 9) {
		QFont font;

		font.setPointSize(9);
		setFont(font);
	}

	connect(this, SIGNAL(et_update_joy_combo()), this, SLOT(s_et_update_joy_combo()));

	for (i = 1; i < LENGTH(js_axs_joyval); i++) {
		const uTCHAR *desc = js_axs_joyval[i].desc[2];
		QCheckBox *cb = findChild<QCheckBox *>("checkBox_" + uQString(desc));
		QLabel *l = findChild<QLabel *>("label_" + uQString(desc));

		if (cb) {
			connect(cb, SIGNAL(clicked(bool)), this, SLOT(s_axis_cb_clicked(bool)));
			l->setFixedWidth(QLabel("-00000").sizeHint().width());
		}
	}
	for (i = 1; i < LENGTH(js_btn_joyval); i++) {
		QCheckBox *cb = findChild<QCheckBox *>("checkBox_" + uQString(js_btn_joyval[i].desc[1]));

		if (cb) {
			connect(cb, SIGNAL(clicked(bool)), this, SLOT(s_button_cb_clicked(bool)));
		}
	}

	connect(comboBox_joy_ID, SIGNAL(activated(int)), this, SLOT(s_combobox_joy_activated(int)));
	connect(comboBox_joy_ID, SIGNAL(currentIndexChanged(int)), this, SLOT(s_combobox_joy_index_changed(int)));
	connect(pushButton_Save, SIGNAL(clicked(bool)), this, SLOT(s_save_clicked(bool)));

	connect(timer, SIGNAL(timeout()), this, SLOT(s_joy_read_timer()));

	adjustSize();
	setFixedSize(size());

	installEventFilter(this);
}
dlgJsc::~dlgJsc() {}

bool dlgJsc::eventFilter(QObject *obj, QEvent *event) {
	switch (event->type()) {
		case QEvent::WindowActivate:
		case QEvent::WindowDeactivate:
			gui_control_pause_bck(event->type());
			break;
		default:
			break;
	}

	return (QObject::eventFilter(obj, event));
}

void dlgJsc::showEvent(QShowEvent *event) {
	if (first_time == true) {
		first_time = false;
	} else {
		setGeometry(geom);
	}
	joy_combo_init();
	timer->start(50);
	QDialog::showEvent(event);
}
void dlgJsc::hideEvent(QHideEvent *event) {
	timer->stop();
	geom = geometry();
	QDialog::hideEvent(event);
}
void dlgJsc::closeEvent(QCloseEvent *event) {
	timer->stop();
	QDialog::closeEvent(event);
}

void dlgJsc::joy_combo_init(void) {
	BYTE current_index = 0, current_line = JS_NO_JOYSTICK;
	int i;

	comboBox_joy_ID->blockSignals(true);
	comboBox_joy_ID->clear();

	for (i = 0; i < joy_list.count; i++) {
		_cb_ports *cb = &joy_list.ele[i];

		if (js_is_this(cb->index, &guid)) {
			current_line = i;
		}
		comboBox_joy_ID->addItem(cb->desc);
		comboBox_joy_ID->setItemData(i, cb->index);
	}

	if (joy_list.count > 1) {
		if (js_is_null(&guid) || (current_line == JS_NO_JOYSTICK)) {
			current_index = joy_list.disabled_line;
		} else {
			current_index = current_line;
		}
	}
	comboBox_joy_ID->blockSignals(false);

	comboBox_joy_ID->setCurrentIndex(current_index);

	if (current_index == 0) {
		emit comboBox_joy_ID->currentIndexChanged(current_index);
	}
}
void dlgJsc::clear_all(void) {
	unsigned int i;

#if !defined (_WIN32)
	label_Device->setText("");
#endif
	label_GUID->setText("");
	label_USB->setText("");
	label_Axes_line->setText("0 Axes, 0 Hats");
	label_Buttons_line->setText("0 Buttons");

	// disabilito tutto
	for (i = 1; i < LENGTH(js_axs_joyval); i++) {
		const uTCHAR *desc = js_axs_joyval[i].desc[2];
		QCheckBox *cb = findChild<QCheckBox *>("checkBox_" + uQString(desc));
		QLabel *l = findChild<QLabel *>("label_" + uQString(desc));

		if (cb) {
			cb->setEnabled(false);
			cb->setChecked(false);
			l->setEnabled(false);
			l->setText("0");
		}
	}
	for (i = 1; i < LENGTH(js_btn_joyval); i++) {
		QCheckBox *cb = findChild<QCheckBox *>("checkBox_" + uQString(js_btn_joyval[i].desc[1]));

		if (cb) {
			cb->setEnabled(false);
			cb->setChecked(false);
			cb->setStyleSheet("");
		}
	}
}
void dlgJsc::update_info_lines(void) {
	_js_device *jdev = &jstick.jdd.devices[js_jdev_index()];
	int ad = axes_disabled();
	int hd = hats_disabled();
	int bd = buttons_disabled();
	QString tmp = "";

	tmp = QString("%1 Axes").arg(jdev->info.axes);
	if (ad) {
		tmp += QString(" [%1 enabled, %2 disabled]").arg(jdev->info.axes - ad).arg(ad);
	}
	tmp += QString(", %1 Hats").arg(jdev->info.hats * 2);
	if (hd) {
		tmp += QString(" [%1 enabled, %2 disabled]").arg((jdev->info.hats * 2) - hd).arg(hd);
	}
	label_Axes_line->setText(tmp);
	tmp = QString("%1 Buttons").arg(jdev->info.buttons);
	if (bd) {
		tmp += QString(" [%1 enabled, %2 disabled]").arg(jdev->info.buttons - bd).arg(bd);
	}
	label_Buttons_line->setText(tmp);
}
int dlgJsc::js_jdev_index(void) {
	int jdev_index = 255;

	if ((comboBox_joy_ID->count() > 1) && comboBox_joy_ID->currentData().isValid()) {
		jdev_index = comboBox_joy_ID->currentData().toInt();
	}
	return (jdev_index);
}
int dlgJsc::axes_disabled(void) {
	_js_device *jdev = &jstick.jdd.devices[js_jdev_index()];
	unsigned int i, disabled = 0;

	for (i = 0; i < JS_MAX_AXES; i++) {
		_js_axis *jsx = &jdev->data.axis[i];

		disabled = jsx->used ?
			jsx->enabled ? disabled : disabled + 1 :
			disabled;
	}
	return (disabled);
}
int dlgJsc::hats_disabled(void) {
	_js_device *jdev = &jstick.jdd.devices[js_jdev_index()];
	unsigned int i, disabled = 0;

	for (i = 0; i < JS_MAX_HATS; i++) {
		_js_axis *jsx = &jdev->data.hat[i];

		disabled = jsx->used ?
			jsx->enabled ? disabled : disabled + 1 :
			disabled;
	}
	return (disabled);
}
int dlgJsc::buttons_disabled(void) {
	_js_device *jdev = &jstick.jdd.devices[js_jdev_index()];
	unsigned int i, disabled = 0;

	for (i = 0; i < JS_MAX_BUTTONS; i++) {
		_js_button *jsx = &jdev->data.button[i];

		disabled = jsx->used ?
			jsx->enabled ? disabled : disabled + 1 :
			disabled;
	}
	return (disabled);
}

void dlgJsc::s_joy_read_timer(void) {
	int jdev_index = js_jdev_index();

	mutex->lock();

	if (jdev_index != 255) {
		if (jdev_index < MAX_JOYSTICK) {
			_js_device *jdev = &jstick.jdd.devices[jdev_index];
			unsigned int i, a;

			for (i = 0; i < LENGTH(js_axs_type); i++) {
				for (a = 0; a < js_axs_type[i]; a++) {
					_js_axis *jsx = !i ? &jdev->data.axis[a] : &jdev->data.hat[a];

					if (jsx->used == TRUE) {
						unsigned int b;

						for (b = 1; b < LENGTH(js_axs_joyval); b++) {
							if (jsx->offset == js_axs_joyval[b].offset) {
								const uTCHAR *desc = js_axs_joyval[b].desc[2];
								QCheckBox *cb = findChild<QCheckBox *>("checkBox_" + uQString(desc));
								QLabel *l = findChild<QLabel *>("label_" + uQString(desc));

								if (cb) {
									cb->setStyleSheet(jsx->enabled ? "" : "QCheckBox { background-color : #FFCFCF; }");
									l->setText(jsx->enabled ? QString("%1").arg(jsx->value): "0");
								}
							}
						}
					}
				}
			}
			for (i = 0; i < JS_MAX_BUTTONS; i++) {
				_js_button *jsx = &jdev->data.button[i];

				if (jsx->used == TRUE) {
					for (a = 1; a < LENGTH(js_btn_joyval); a++) {
						if (jsx->offset == js_btn_joyval[a].offset) {
							QCheckBox *cb = findChild<QCheckBox *>("checkBox_" + uQString(js_btn_joyval[a].desc[1]));

							if (cb) {
								cb->setStyleSheet(jsx->enabled ?
									jsx->value ? "QCheckBox { background-color : #ACFFAC; }" : "" :
									"QCheckBox { background-color : #FFCFCF; }");
							}
						}
					}
				}
			}
		}
	}

	mutex->unlock();
}
void dlgJsc::s_combobox_joy_activated(int index) {
	unsigned int jdev_index = ((QComboBox *)sender())->itemData(index).toInt();

	if (comboBox_joy_ID->count() == 1) {
		return;
	}
	js_guid_set(jdev_index, &guid);
}
void dlgJsc::s_combobox_joy_index_changed(UNUSED(int index)) {
	int jdev_index = js_jdev_index();
	static int old_jdev_index = -1;

	if (jdev_index != old_jdev_index) {
		unsigned int i, a;

		mutex->lock();

		clear_all();

		if (jdev_index < MAX_JOYSTICK) {
			_js_device *jdev = &jstick.jdd.devices[jdev_index];

#if !defined (_WIN32)
			label_Device->setText(QString(jdev->dev));
#endif
			label_GUID->setText(uQString(js_guid_to_string(&jdev->guid)));
			label_USB->setText(QString("bustype %1 - vid:pid %2:%3 - version %4").
				arg(jdev->usb.bustype, 4, 16, QChar('0')).
				arg(jdev->usb.vendor_id, 4, 16, QChar('0')).
				arg(jdev->usb.product_id, 4, 16, QChar('0')).
				arg(jdev->usb.version, 4, 16, QChar('0')));

			// abilito solo quello che serve
			for (i = 0; i < LENGTH(js_axs_type); i++) {
				for (a = 0; a < js_axs_type[i]; a++) {
					_js_axis *jsx = !i ? &jdev->data.axis[a] : &jdev->data.hat[a];

					if (jsx->used == TRUE) {
						unsigned int b;

						for (b = 1; b < LENGTH(js_axs_joyval); b++) {
							if (jsx->offset == js_axs_joyval[b].offset) {
								const uTCHAR *desc = js_axs_joyval[b].desc[2];
								QCheckBox *cb = findChild<QCheckBox *>("checkBox_" + uQString(desc));
								QLabel *l = findChild<QLabel *>("label_" + uQString(desc));

								if (cb) {
									cb->setEnabled(true);
									cb->setChecked(jsx->enabled);
									cb->setProperty("myIndex", QVariant((i << 8) | a));
									l->setEnabled(true);
								}
							}
						}
					}
				}
			}
			for (i = 0; i < JS_MAX_BUTTONS; i++) {
				_js_button *jsx = &jdev->data.button[i];

				if (jsx->used == TRUE) {
					for (a = 1; a < LENGTH(js_btn_joyval); a++) {
						if (jsx->offset == js_btn_joyval[a].offset) {
							QCheckBox *cb = findChild<QCheckBox *>("checkBox_" + uQString(js_btn_joyval[a].desc[1]));

							if (cb) {
								cb->setEnabled(true);
								cb->setChecked(jsx->enabled);
								cb->setProperty("myIndex", QVariant(i));
							}
						}
					}
				}
			}
			update_info_lines();
		}
		old_jdev_index = jdev_index;

		mutex->unlock();
	}
}
void dlgJsc::s_axis_cb_clicked(UNUSED(bool checked)) {
	int index = QVariant(((QCheckBox *)sender())->property("myIndex")).toInt();
	_js_device *jdev = &jstick.jdd.devices[js_jdev_index()];
	_js_axis *jsx = index & 0x100 ? &jdev->data.hat[index & 0xFF] : &jdev->data.axis[index];

	jsx->enabled = !jsx->enabled;
	update_info_lines();
}
void dlgJsc::s_button_cb_clicked(UNUSED(bool checked)) {
	int index = QVariant(((QCheckBox *)sender())->property("myIndex")).toInt();
	_js_device *jdev = &jstick.jdd.devices[js_jdev_index()];
	_js_button *jsx = &jdev->data.button[index];

	jsx->enabled = !jsx->enabled;
	update_info_lines();
}
void dlgJsc::s_save_clicked(UNUSED(bool checked)) {
	if (js_jdev_index() != 255) {
		_js_device *jdev = &jstick.jdd.devices[js_jdev_index()];
		unsigned int i, a;
		_js_data data;

		memcpy(&data, &jdev->data, sizeof(_js_data));

		settings_jsc_parse(jdev->index);
		for (i = 0; i < JS_MAX_BUTTONS; i++) {
			_js_button *jsx1 = &jdev->data.button[i];
			_js_button *jsx2 = &data.button[i];

			jsx1->enabled = jsx2->enabled;
		}
		for (i = 0; i < LENGTH(js_axs_type); i++) {
			for (a = 0; a < js_axs_type[i]; a++) {
				_js_axis *jsx1 = !i ? &jdev->data.axis[a] : &jdev->data.hat[a];
				_js_axis *jsx2 = !i ? &data.axis[a] : &data.hat[a];

				jsx1->enabled = jsx2->enabled;
			}
		}
		settings_jsc_save();
	}
}

void dlgJsc::s_et_update_joy_combo(void) {
	// se la combox e' aperta o sto configurando i pulsanti, non devo aggiornarne il contenuto
	if (comboBox_joy_ID->view()->isVisible() == false) {
		joy_combo_init();
	}
}
