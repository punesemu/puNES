/*
 * dlgInput.cpp
 *
 *  Created on: 25/nov/2014
 *      Author: fhorse
 */

#include "dlgInput.moc"
#include "mainWindow.hpp"
#include "dlgStdPad.hpp"
#include "emu.h"
#if defined (SDL)
#include "opengl.h"
#endif
#include "gui.h"

dlgInput::dlgInput(QWidget *parent = 0) : QDialog(parent) {
	memset(&data, 0x00, sizeof(data));
	memcpy(&data.settings, &cfg->input, sizeof(_config_input));

	for (int i = PORT1; i < PORT_MAX; i++) {
		data.port[i].id = i + 1;
		memcpy(&data.port[i].port, &port[i], sizeof(_port));
	}

	setupUi(this);

	comboBox_cm->addItem(trUtf8("NES"));
	comboBox_cm->addItem(trUtf8("Famicom"));
	comboBox_cm->addItem(trUtf8("Four Score"));
	comboBox_cm->setItemData(0, 0);
	comboBox_cm->setItemData(1, 1);
	comboBox_cm->setItemData(2, 2);
	connect(comboBox_cm, SIGNAL(activated(int)), this, SLOT(s_combobox_cm_activated(int)));

	combobox_cp_init(comboBox_cp1, &data.port[0]);
	combobox_cp_init(comboBox_cp2, &data.port[1]);
	combobox_cp_init(comboBox_cp3, &data.port[2]);
	combobox_cp_init(comboBox_cp4, &data.port[3]);

	connect(checkBox_Permit_updown, SIGNAL(stateChanged(int)), this,
			SLOT(s_checkbox_state_changed(int)));

	update_dialog();

	connect(pushButton_Default, SIGNAL(clicked(bool)), this, SLOT(s_default_clicked(bool)));
	connect(pushButton_Apply, SIGNAL(clicked(bool)), this, SLOT(s_apply_clicked(bool)));
	connect(pushButton_Discard, SIGNAL(clicked(bool)), this, SLOT(s_discard_clicked(bool)));

	setAttribute(Qt::WA_DeleteOnClose);
	setFixedSize(width(), height());

    /* disabilito la gestiore del docus della finestra principale */
	gui.main_win_lfp = FALSE;

	emu_pause(TRUE);
}
dlgInput::~dlgInput() {}
void dlgInput::closeEvent(QCloseEvent *e) {
	emu_pause(FALSE);

	/* restituisco alla finestra principale la gestione del focus */
	gui.main_win_lfp = TRUE;

	QDialog::closeEvent(e);

	delete (this);
}
void dlgInput::update_dialog(void) {
	bool mode = true;
	_cfg_port *ctrl_in;

	comboBox_cm->setCurrentIndex(data.settings.controller_mode);

	if (data.settings.controller_mode == CTRL_MODE_NES) {
		mode = false;
	}

	for (int i = PORT1; i < PORT_MAX; i++) {
		ctrl_in = &data.port[i];

		QComboBox *cb = findChild<QComboBox *>(
				QString("comboBox_cp%1").arg(ctrl_in->id));
		QPushButton *pb = findChild<QPushButton *>(
				QString("pushButton_cp%1").arg(ctrl_in->id));

		cb->setCurrentIndex(ctrl_in->port.type);
		pb->disconnect(pb, SIGNAL(clicked(bool)));

		switch (ctrl_in->port.type) {
			case CTRL_DISABLED:
			case CTRL_ZAPPER:
				pb->setEnabled(false);
				break;
			case CTRL_STANDARD:
				pb->setEnabled(true);
				pb->setProperty("myPointer", QVariant::fromValue(static_cast<void *>(ctrl_in)));
				connect(pb, SIGNAL(clicked(bool)), this, SLOT(s_setup_clicked(bool)));
				break;
		}

		if ((i >= PORT3) && (i <= PORT4)) {
			QLabel *lb = findChild<QLabel *>(QString("label_cp%1").arg(ctrl_in->id));

			lb->setEnabled(mode);
			cb->setEnabled(mode);

			if (mode == false) {
				pb->setEnabled(mode);
			}
		}
	}

	checkBox_Permit_updown->setChecked(data.settings.permit_updown_leftright);
}
void dlgInput::combobox_cp_init(QComboBox *cb, _cfg_port *cfg_port) {
	static struct _ctrl_list_element {
		int type;
		QString name;
	} ctrl_list[] = {
		{ CTRL_DISABLED, trUtf8("Disabled")     },
		{ CTRL_STANDARD, trUtf8("Standard Pad") },
		{ CTRL_ZAPPER,   trUtf8("Zapper")       }
	};

	{
		int i, length = LENGTH(ctrl_list) - ((cfg_port->id - 1) >> 1);

		for (i = 0; i < length; i++) {
			QList<QVariant> type;

			type.append(ctrl_list[i].type);
			type.append(cfg_port->id - 1);
			type.append(QVariant::fromValue(static_cast<void *>(cfg_port)));

			cb->addItem(ctrl_list[i].name);
			cb->setItemData(i, QVariant(type));
		}
	}

	connect(cb, SIGNAL(activated(int)), this, SLOT(s_combobox_cp_activated(int)));
}
void dlgInput::s_combobox_cm_activated(int index) {
	int type = comboBox_cm->itemData(index).toInt();

	data.settings.controller_mode = type;
	update_dialog();
}
void dlgInput::s_combobox_cp_activated(int index) {
	QList<QVariant> type = qobject_cast<QComboBox *>(sender())->itemData(index).toList();
	_cfg_port *cfg_port = static_cast<_cfg_port *>(type.at(2).value<void *>());

	cfg_port->port.type = type.at(0).toInt();
	update_dialog();
}
void dlgInput::s_setup_clicked(bool checked) {
	_cfg_port *cfg_port = static_cast<_cfg_port *>(qobject_cast<QPushButton *>(sender())->property(
			"myPointer").value<void *>());

	switch (cfg_port->port.type) {
		case CTRL_DISABLED:
		case CTRL_ZAPPER:
			break;
		case CTRL_STANDARD:
			dlgStdPad *dlg = new dlgStdPad(cfg_port, this);

			hide();
			dlg->show();
			break;
	}
}
void dlgInput::s_checkbox_state_changed(int state) {
	data.settings.permit_updown_leftright = state;
}
void dlgInput::s_default_clicked(bool checked) {
	_array_pointers_port array;

	for (int i = PORT1; i < PORT_MAX; i++) {
		array.port[i] = &data.port[i].port;
	}

	settings_inp_all_default(&data.settings, &array);

	update_dialog();
}
void dlgInput::s_apply_clicked(bool checked) {
#if defined (SDL)
	if (opengl.rotation && (input_zapper_is_connected((_port *) &data.port) == TRUE)) {
		mainWindow::s_set_effect();
	}
#endif

	memcpy(&cfg->input, &data.settings, sizeof(_config_input));

	for (int i = PORT1; i < PORT_MAX; i++) {
		if (data.port[i].port.type != port[i].type) {
			for (int a = TRB_A; a <= TRB_B; a++) {
				int type = a - TRB_A;

				data.port[i].port.turbo[type].active = 0;
				data.port[i].port.turbo[type].counter = 0;
			}
		}
		memcpy(&port[i], &data.port[i].port, sizeof(_port));
	}

	// Faccio l'update del menu per i casi dello zapper e degli effetti
	gui_update();

	settings_inp_save();

	input_init();

	js_quit();
	js_init();

	close();
}
void dlgInput::s_discard_clicked(bool checked) {
	close();
}
