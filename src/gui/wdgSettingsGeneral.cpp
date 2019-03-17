/*
 *  Copyright (C) 2010-2019 Fabio Cavallo (aka FHorse)
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

#include <QtWidgets/QFileDialog>
#include "wdgSettingsGeneral.moc"
#include "mainWindow.hpp"
#include "emu.h"
#include "emu_thread.h"
#include "conf.h"
#include "clock.h"

wdgSettingsGeneral::wdgSettingsGeneral(QWidget *parent) : QWidget(parent) {
	setupUi(this);

	connect(comboBox_Mode, SIGNAL(activated(int)), this, SLOT(s_mode(int)));
	connect(comboBox_Fast_Forward_velocity, SIGNAL(activated(int)), this, SLOT(s_fast_forward_velocity(int)));
	connect(comboBox_Rewind_minutes, SIGNAL(activated(int)), this, SLOT(s_rewind_minutes(int)));
	connect(comboBox_Language, SIGNAL(activated(int)), this, SLOT(s_language(int)));

	connect(pushButton_Game_Genie_rom_file, SIGNAL(clicked(bool)), this, SLOT(s_game_genie_rom_file(bool)));
	connect(pushButton_Game_Genie_rom_file_clear, SIGNAL(clicked(bool)), this, SLOT(s_game_genie_rom_file_clear(bool)));

	connect(pushButton_FDS_Bios, SIGNAL(clicked(bool)), this, SLOT(s_fds_bios_file(bool)));
	connect(pushButton_FDS_Bios_clear, SIGNAL(clicked(bool)), this, SLOT(s_fds_bios_file_clear(bool)));

	connect(checkBox_Save_battery_every_tot, SIGNAL(clicked(bool)), this, SLOT(s_save_battery_every_tot(bool)));
	connect(checkBox_Pause_when_in_background, SIGNAL(clicked(bool)), this, SLOT(s_pause_in_background(bool)));
	connect(checkBox_Save_settings_on_exit, SIGNAL(clicked(bool)), this, SLOT(s_save_settings_on_exit(bool)));
}
wdgSettingsGeneral::~wdgSettingsGeneral() {}

void wdgSettingsGeneral::changeEvent(QEvent *event) {
	if (event->type() == QEvent::LanguageChange) {
		retranslateUi(this);
	} else {
		QWidget::changeEvent(event);
	}
}
void wdgSettingsGeneral::showEvent(UNUSED(QShowEvent *event)) {
	int dim = label_Mode->size().height() - 10;

	icon_Mode->setPixmap(QIcon(":/icon/icons/mode.svg").pixmap(dim, dim));
	icon_Fast_Forward_velocity->setPixmap(QIcon(":/icon/icons/fast_forward.svg").pixmap(dim, dim));
	icon_Rewind_minutes->setPixmap(QIcon(":/icon/icons/rewind.svg").pixmap(dim, dim));
	icon_Language->setPixmap(QIcon(":/icon/icons/language.svg").pixmap(dim, dim));
}

void wdgSettingsGeneral::retranslateUi(QWidget *wdgSettingsGeneral) {
	Ui::wdgSettingsGeneral::retranslateUi(wdgSettingsGeneral);
	mainwin->qaction_shcut.mode_auto->setText(comboBox_Mode->itemText(0));
	mainwin->qaction_shcut.mode_ntsc->setText(comboBox_Mode->itemText(1));
	mainwin->qaction_shcut.mode_pal->setText(comboBox_Mode->itemText(2));
	mainwin->qaction_shcut.mode_dendy->setText(comboBox_Mode->itemText(3));
	update_widget();
}
void wdgSettingsGeneral::update_widget(void) {
	mode_set();
	fast_forward_velocity_set();
	rewind_minutes_set();
	language_set();

	if (ustrlen(cfg->gg_rom_file) != 0) {
		lineEdit_Game_Genie_rom_file->setEnabled(true);
		lineEdit_Game_Genie_rom_file->setText(uQString(cfg->gg_rom_file));
	} else {
		lineEdit_Game_Genie_rom_file->setEnabled(false);
		lineEdit_Game_Genie_rom_file->setText(tr("[Select a file]"));
	}

	if (ustrlen(cfg->fds_bios_file) != 0) {
		lineEdit_FDS_Bios->setEnabled(true);
		lineEdit_FDS_Bios->setText(uQString(cfg->fds_bios_file));
	} else {
		lineEdit_FDS_Bios->setEnabled(false);
		lineEdit_FDS_Bios->setText(tr("[Select a file]"));
	}

	checkBox_Save_battery_every_tot->setChecked(cfg->save_battery_ram_file);
	checkBox_Pause_when_in_background->setChecked(cfg->bck_pause);
	checkBox_Save_settings_on_exit->setChecked(cfg->save_on_exit);
}

void wdgSettingsGeneral::mode_set(void) {
	comboBox_Mode->setCurrentIndex(cfg->mode);
}
void wdgSettingsGeneral::fast_forward_velocity_set(void) {
	int velocity = 0;

	switch (cfg->ff_velocity) {
		default:
		case FF_2X:
			velocity = 0;
			break;
		case FF_3X:
			velocity = 1;
			break;
		case FF_4X:
			velocity = 2;
			break;
		case FF_5X:
			velocity = 3;
			break;
	}

	comboBox_Fast_Forward_velocity->setCurrentIndex(velocity);
}
void wdgSettingsGeneral::rewind_minutes_set(void) {
	comboBox_Rewind_minutes->setCurrentIndex(cfg->rewind_minutes);
}
void wdgSettingsGeneral::language_set(void) {
	comboBox_Language->setCurrentIndex(cfg->language);
}

void wdgSettingsGeneral::s_mode(int index) {
	int mode = index;
	bool reset = true;

	if (mode == cfg->mode) {
		return;
	}

	emu_thread_pause();

	cfg->mode = mode;

	if (cfg->mode == AUTO) {
		if (info.no_rom) {
			mode = NTSC;
		} else {
			switch (info.machine[DATABASE]) {
				case NTSC:
				case PAL:
				case DENDY:
					mode = info.machine[DATABASE];
					break;
				case DEFAULT:
					mode = info.machine[HEADER];
					break;
				default:
					mode = NTSC;
					break;
			}
		}
	}

	// se la nuova modalita' e' identica a quella attuale
	// non e' necessario fare un reset.
	if (mode == machine.type) {
		reset = FALSE;
	}

	machine = machinedb[mode - 1];

	if (reset) {
		QString ascii = uQString(opt_mode[machine.type].lname);

		text_add_line_info(1, "switched to [green]%s", ascii.toLatin1().constData());

		emu_reset(CHANGE_MODE);

		emu_frame_input_and_rewind();

		// controllo la paletta da utilizzare (per lo swap dell'emphasis del rosso e del verde in caso
		// di PAL e DENDY) quando cambio regione.
		gfx_palette_update();
	}

	emu_thread_continue();
}
void wdgSettingsGeneral::s_fast_forward_velocity(int index) {
	int velocity = index;

	switch (velocity) {
		case 0:
			velocity = FF_2X;
			break;
		case 1:
			velocity = FF_3X;
			break;
		case 2:
			velocity = FF_4X;
			break;
		case 3:
			velocity = FF_5X;
			break;
	}

	if (cfg->ff_velocity == velocity) {
		return;
	}

	cfg->ff_velocity = velocity;
	update_widget();

	if (nsf.enabled == FALSE) {
		if (fps.fast_forward == TRUE) {
			emu_thread_pause();
			fps_fast_forward();
			emu_thread_continue();
		}
	}
}
void wdgSettingsGeneral::s_rewind_minutes(int index) {
	int minutes = index;

	if (minutes == cfg->rewind_minutes) {
		return;
	}

	emu_thread_pause();
	cfg->rewind_minutes = minutes;
	rewind_init();
	emu_thread_continue();
}
void wdgSettingsGeneral::s_language(int index) {
	int lang = index;

	mainwin->set_language(lang);
}
void wdgSettingsGeneral::s_game_genie_rom_file(UNUSED(bool checked)) {
	QStringList filters;
	QString file;

	emu_pause(TRUE);

	filters.append(tr("All files"));

	filters[0].append(" (*.*)");

	file = QFileDialog::getOpenFileName(this, tr("Select Game Genie ROM file"),
			QFileInfo(uQString(cfg->gg_rom_file)).dir().absolutePath(), filters.join(";;"));

	if (file.isNull() == false) {
		QFileInfo fileinfo(file);

		if (fileinfo.exists()) {
			umemset(cfg->gg_rom_file, 0x00, usizeof(cfg->gg_rom_file));
			ustrncpy(cfg->gg_rom_file, uQStringCD(fileinfo.absoluteFilePath()), usizeof(cfg->gg_rom_file) - 1);
			update_widget();
		} else {
			text_add_line_info(1, "[red]error on game genie rom file");
		}
	}

	emu_pause(FALSE);
}
void wdgSettingsGeneral::s_game_genie_rom_file_clear(UNUSED(bool checked)) {
	umemset(cfg->gg_rom_file, 0x00, usizeof(cfg->gg_rom_file));
	update_widget();
}
void wdgSettingsGeneral::s_fds_bios_file(UNUSED(bool checked)) {
	QStringList filters;
	QString file;

	emu_pause(TRUE);

	filters.append(tr("All files"));

	filters[0].append(" (*.*)");

	file = QFileDialog::getOpenFileName(this, tr("Select FDS BIOS file"),
			QFileInfo(uQString(cfg->fds_bios_file)).dir().absolutePath(), filters.join(";;"));

	if (file.isNull() == false) {
		QFileInfo fileinfo(file);

		if (fileinfo.exists()) {
			umemset(cfg->fds_bios_file, 0x00, usizeof(cfg->fds_bios_file));
			ustrncpy(cfg->fds_bios_file, uQStringCD(fileinfo.absoluteFilePath()), usizeof(cfg->fds_bios_file) - 1);
			update_widget();
		} else {
			text_add_line_info(1, "[red]error on FDS BIOS file");
		}
	}

	emu_pause(FALSE);
}
void wdgSettingsGeneral::s_fds_bios_file_clear(UNUSED(bool checked)) {
	umemset(cfg->fds_bios_file, 0x00, usizeof(cfg->fds_bios_file));
	update_widget();
}
void wdgSettingsGeneral::s_save_battery_every_tot(UNUSED(bool checked)) {
	emu_thread_pause();
	if (!cfg->save_battery_ram_file) {
		info.bat_ram_frames = 0;
	}
	cfg->save_battery_ram_file = !cfg->save_battery_ram_file;
	emu_thread_continue();
}
void wdgSettingsGeneral::s_pause_in_background(UNUSED(bool checked)) {
	cfg->bck_pause = !cfg->bck_pause;
}
void wdgSettingsGeneral::s_save_settings_on_exit(UNUSED(bool checked)) {
	cfg->save_on_exit = !cfg->save_on_exit;
}
