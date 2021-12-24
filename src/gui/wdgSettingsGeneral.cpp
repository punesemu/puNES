/*
 *  Copyright (C) 2010-2022 Fabio Cavallo (aka FHorse)
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

	setFocusProxy(widget_Mode);

	widget_Mode->setStyleSheet(button_stylesheet());
	widget_Fast_Forward_velocity->setStyleSheet(button_stylesheet());
	widget_Rewind_minutes->setStyleSheet(button_stylesheet());

	pushButton_Mode_Auto->setProperty("mtype", QVariant(AUTO));
	pushButton_Mode_NTSC->setProperty("mtype", QVariant(NTSC));
	pushButton_Mode_PAL->setProperty("mtype", QVariant(PAL));
	pushButton_Mode_Dendy->setProperty("mtype", QVariant(DENDY));

	connect(pushButton_Mode_Auto, SIGNAL(toggled(bool)), this, SLOT(s_mode(bool)));
	connect(pushButton_Mode_NTSC, SIGNAL(toggled(bool)), this, SLOT(s_mode(bool)));
	connect(pushButton_Mode_PAL, SIGNAL(toggled(bool)), this, SLOT(s_mode(bool)));
	connect(pushButton_Mode_Dendy, SIGNAL(toggled(bool)), this, SLOT(s_mode(bool)));

	pushButton_FF_2x->setProperty("mtype", QVariant(FF_2X));
	pushButton_FF_3x->setProperty("mtype", QVariant(FF_3X));
	pushButton_FF_4x->setProperty("mtype", QVariant(FF_4X));
	pushButton_FF_5x->setProperty("mtype", QVariant(FF_5X));

	connect(pushButton_FF_2x, SIGNAL(toggled(bool)), this, SLOT(s_fast_forward_velocity(bool)));
	connect(pushButton_FF_3x, SIGNAL(toggled(bool)), this, SLOT(s_fast_forward_velocity(bool)));
	connect(pushButton_FF_4x, SIGNAL(toggled(bool)), this, SLOT(s_fast_forward_velocity(bool)));
	connect(pushButton_FF_5x, SIGNAL(toggled(bool)), this, SLOT(s_fast_forward_velocity(bool)));

	pushButton_Rwn_off->setProperty("mtype", QVariant(RWND_0_MINUTES));
	pushButton_Rwn_2->setProperty("mtype", QVariant(RWND_2_MINUTES));
	pushButton_Rwn_5->setProperty("mtype", QVariant(RWND_5_MINUTES));
	pushButton_Rwn_15->setProperty("mtype", QVariant(RWND_15_MINUTES));
	pushButton_Rwn_30->setProperty("mtype", QVariant(RWND_30_MINUTES));
	pushButton_Rwn_60->setProperty("mtype", QVariant(RWND_60_MINUTES));
	pushButton_Rwn_unlimited->setProperty("mtype", QVariant(RWND_UNLIMITED_MINUTES));

	connect(pushButton_Rwn_off, SIGNAL(toggled(bool)), this, SLOT(s_rewind_minutes(bool)));
	connect(pushButton_Rwn_2, SIGNAL(toggled(bool)), this, SLOT(s_rewind_minutes(bool)));
	connect(pushButton_Rwn_5, SIGNAL(toggled(bool)), this, SLOT(s_rewind_minutes(bool)));
	connect(pushButton_Rwn_15, SIGNAL(toggled(bool)), this, SLOT(s_rewind_minutes(bool)));
	connect(pushButton_Rwn_30, SIGNAL(toggled(bool)), this, SLOT(s_rewind_minutes(bool)));
	connect(pushButton_Rwn_60, SIGNAL(toggled(bool)), this, SLOT(s_rewind_minutes(bool)));
	connect(pushButton_Rwn_unlimited, SIGNAL(toggled(bool)), this, SLOT(s_rewind_minutes(bool)));

	connect(comboBox_Language, SIGNAL(activated(int)), this, SLOT(s_language(int)));

	connect(pushButton_Game_Genie_rom_file, SIGNAL(clicked(bool)), this, SLOT(s_game_genie_rom_file(bool)));
	connect(pushButton_Game_Genie_rom_file_clear, SIGNAL(clicked(bool)), this, SLOT(s_game_genie_rom_file_clear(bool)));

	connect(pushButton_FDS_Bios, SIGNAL(clicked(bool)), this, SLOT(s_fds_bios_file(bool)));
	connect(pushButton_FDS_Bios_clear, SIGNAL(clicked(bool)), this, SLOT(s_fds_bios_file_clear(bool)));

	connect(checkBox_FDS_disk1sideA_at_reset, SIGNAL(clicked(bool)), this, SLOT(s_fds_disk1sideA_at_reset(bool)));
	connect(checkBox_FDS_switch_side_automatically, SIGNAL(clicked(bool)), this, SLOT(s_fds_switch_side_automatically(bool)));
	connect(checkBox_FDS_fast_forward, SIGNAL(clicked(bool)), this, SLOT(s_fds_fast_forward(bool)));

	connect(checkBox_Save_battery_every_tot, SIGNAL(clicked(bool)), this, SLOT(s_save_battery_every_tot(bool)));
	connect(checkBox_Pause_when_in_background, SIGNAL(clicked(bool)), this, SLOT(s_pause_in_background(bool)));
	connect(checkBox_Save_settings_on_exit, SIGNAL(clicked(bool)), this, SLOT(s_save_settings_on_exit(bool)));
	connect(checkBox_Multiple_instances, SIGNAL(clicked(bool)), this, SLOT(s_multiple_settings(bool)));
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
	int dim = fontMetrics().height();

	icon_General_settings->setPixmap(QIcon(":/icon/icons/settings.svg").pixmap(dim, dim));
	icon_Mode->setPixmap(QIcon(":/icon/icons/mode.svg").pixmap(dim, dim));
	icon_Fast_Forward_velocity->setPixmap(QIcon(":/icon/icons/fast_forward.svg").pixmap(dim, dim));
	icon_Rewind_minutes->setPixmap(QIcon(":/icon/icons/rewind.svg").pixmap(dim, dim));
	icon_Language->setPixmap(QIcon(":/icon/icons/language.svg").pixmap(dim, dim));
	icon_System_Roms->setPixmap(QIcon(":/icon/icons/microprocessor.svg").pixmap(dim, dim));
	icon_General_FDS->setPixmap(QIcon(":/icon/icons/fds_file.svg").pixmap(dim, dim));
	icon_General_misc->setPixmap(QIcon(":/icon/icons/misc.svg").pixmap(dim, dim));
	icon_Game_Genie_rom_file->setPixmap(QIcon(":/icon/icons/bios.svg").pixmap(dim, dim));
	icon_FDS_Bios->setPixmap(QIcon(":/icon/icons/bios.svg").pixmap(dim, dim));
}

void wdgSettingsGeneral::retranslateUi(QWidget *wdgSettingsGeneral) {
	Ui::wdgSettingsGeneral::retranslateUi(wdgSettingsGeneral);
	mainwin->qaction_shcut.mode_auto->setText(pushButton_Mode_Auto->text());
	mainwin->qaction_shcut.mode_ntsc->setText(pushButton_Mode_NTSC->text());
	mainwin->qaction_shcut.mode_pal->setText(pushButton_Mode_PAL->text());
	mainwin->qaction_shcut.mode_dendy->setText(pushButton_Mode_Dendy->text());
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

	checkBox_FDS_disk1sideA_at_reset->setChecked(cfg->fds_disk1sideA_at_reset);
	checkBox_FDS_switch_side_automatically->setChecked(cfg->fds_switch_side_automatically);
	checkBox_FDS_fast_forward->setChecked(cfg->fds_fast_forward);

	checkBox_Save_battery_every_tot->setChecked(cfg->save_battery_ram_file);
	checkBox_Pause_when_in_background->setChecked(cfg->bck_pause);
	checkBox_Save_settings_on_exit->setChecked(cfg->save_on_exit);
	checkBox_Multiple_instances->setChecked(cfg->multiple_instances);
}
void wdgSettingsGeneral::shcut_mode(int mode) {
	switch (mode) {
		case AUTO:
			pushButton_Mode_Auto->toggled(true);
			break;
		case NTSC:
			pushButton_Mode_NTSC->toggled(true);
			break;
		case PAL:
			pushButton_Mode_PAL->toggled(true);
			break;
		case DENDY:
			pushButton_Mode_Dendy->toggled(true);
			break;
	}
}

void wdgSettingsGeneral::mode_set(void) {
	qtHelper::pushbutton_set_checked(pushButton_Mode_Auto, false);
	qtHelper::pushbutton_set_checked(pushButton_Mode_NTSC, false);
	qtHelper::pushbutton_set_checked(pushButton_Mode_PAL, false);
	qtHelper::pushbutton_set_checked(pushButton_Mode_Dendy, false);
	switch (cfg->mode) {
		default:
		case AUTO:
			qtHelper::pushbutton_set_checked(pushButton_Mode_Auto, true);
			break;
		case NTSC:
			qtHelper::pushbutton_set_checked(pushButton_Mode_NTSC, true);
			break;
		case PAL:
			qtHelper::pushbutton_set_checked(pushButton_Mode_PAL, true);
			break;
		case DENDY:
			qtHelper::pushbutton_set_checked(pushButton_Mode_Dendy, true);
			break;
	}
}
void wdgSettingsGeneral::fast_forward_velocity_set(void) {
	qtHelper::pushbutton_set_checked(pushButton_FF_2x, false);
	qtHelper::pushbutton_set_checked(pushButton_FF_3x, false);
	qtHelper::pushbutton_set_checked(pushButton_FF_4x, false);
	qtHelper::pushbutton_set_checked(pushButton_FF_5x, false);
	switch (cfg->ff_velocity) {
		default:
		case FF_2X:
			qtHelper::pushbutton_set_checked(pushButton_FF_2x, true);
			break;
		case FF_3X:
			qtHelper::pushbutton_set_checked(pushButton_FF_3x, true);
			break;
		case FF_4X:
			qtHelper::pushbutton_set_checked(pushButton_FF_4x, true);
			break;
		case FF_5X:
			qtHelper::pushbutton_set_checked(pushButton_FF_5x, true);
			break;
	}
}
void wdgSettingsGeneral::rewind_minutes_set(void) {
	qtHelper::pushbutton_set_checked(pushButton_Rwn_off, false);
	qtHelper::pushbutton_set_checked(pushButton_Rwn_2, false);
	qtHelper::pushbutton_set_checked(pushButton_Rwn_5, false);
	qtHelper::pushbutton_set_checked(pushButton_Rwn_15, false);
	qtHelper::pushbutton_set_checked(pushButton_Rwn_30, false);
	qtHelper::pushbutton_set_checked(pushButton_Rwn_60, false);
	qtHelper::pushbutton_set_checked(pushButton_Rwn_unlimited, false);
	switch (cfg->rewind_minutes) {
		default:
		case RWND_0_MINUTES:
			qtHelper::pushbutton_set_checked(pushButton_Rwn_off, true);
			break;
		case RWND_2_MINUTES:
			qtHelper::pushbutton_set_checked(pushButton_Rwn_2, true);
			break;
		case RWND_5_MINUTES:
			qtHelper::pushbutton_set_checked(pushButton_Rwn_5, true);
			break;
		case RWND_15_MINUTES:
			qtHelper::pushbutton_set_checked(pushButton_Rwn_15, true);
			break;
		case RWND_30_MINUTES:
			qtHelper::pushbutton_set_checked(pushButton_Rwn_30, true);
			break;
		case RWND_60_MINUTES:
			qtHelper::pushbutton_set_checked(pushButton_Rwn_60, true);
			break;
		case RWND_UNLIMITED_MINUTES:
			qtHelper::pushbutton_set_checked(pushButton_Rwn_unlimited, true);
			break;
	}
}
void wdgSettingsGeneral::language_set(void) {
	int lang = 0;

	switch (cfg->language) {
		default:
		case LNG_CHINESE_SIMPLIFIED:
			lang = 0;
			break;
		case LNG_ENGLISH:
			lang = 1;
			break;
		case LNG_HUNGARIAN:
			lang = 2;
			break;
		case LNG_ITALIAN:
			lang = 3;
			break;
		case LNG_PORTUGUESEBR:
			lang = 4;
			break;
		case LNG_RUSSIAN:
			lang = 5;
			break;
		case LNG_SPANISH:
			lang = 6;
			break;
		case LNG_TURKISH:
			lang = 7;
			break;
	}

	comboBox_Language->setCurrentIndex(lang);
}

void wdgSettingsGeneral::s_mode(bool checked) {
	if (checked) {
		int mode = QVariant(((QPushButton *)sender())->property("mtype")).toInt();
		bool reset = true;

		if (cfg->mode == mode) {
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

#if defined (FULLSCREEN_RESFREQ)
		// mi salvo la vecchia modalita'
		info.old_machine_type = machine.type;
#endif

		// setto il tipo di sistema
		machine = machinedb[mode - 1];

		if (reset) {
			QString ascii = uQString(opt_mode[machine.type].lname);

			gui_overlay_info_append_msg_precompiled(22, (void *)&ascii);
			emu_reset(CHANGE_MODE);
			emu_frame_input_and_rewind();
			// controllo la paletta da utilizzare (per lo swap dell'emphasis del rosso e del verde in caso
			// di PAL e DENDY) quando cambio regione.
			gfx_palette_update();
		}

		emu_thread_continue();
	}
	mode_set();
}
void wdgSettingsGeneral::s_fast_forward_velocity(bool checked) {
	if (checked) {
		int velocity = QVariant(((QPushButton *)sender())->property("mtype")).toInt();

		if (cfg->ff_velocity == velocity) {
			return;
		}

		cfg->ff_velocity = velocity;

		if (nsf.enabled == FALSE) {
			if (fps.fast_forward == TRUE) {
				emu_thread_pause();
				fps_fast_forward_estimated_ms();
				emu_thread_continue();
			}
		}
	}
	update_widget();
}
void wdgSettingsGeneral::s_rewind_minutes(bool checked) {
	if (checked) {
		int minutes = QVariant(((QPushButton *)sender())->property("mtype")).toInt();

		if (minutes == cfg->rewind_minutes) {
			return;
		}

		emu_thread_pause();
		cfg->rewind_minutes = minutes;
		rewind_init();
		emu_thread_continue();
	}
	gui_update();
}
void wdgSettingsGeneral::s_language(int index) {
	int lang = index;

	switch (index) {
		default:
		case 0:
			lang = LNG_CHINESE_SIMPLIFIED;
			break;
		case 1:
			lang = LNG_ENGLISH;
			break;
		case 2:
			lang = LNG_HUNGARIAN;
			break;
		case 3:
			lang = LNG_ITALIAN;
			break;
		case 4:
			lang = LNG_PORTUGUESEBR;
			break;
		case 5:
			lang = LNG_RUSSIAN;
			break;
		case 6:
			lang = LNG_SPANISH;
			break;
		case 7:
			lang = LNG_TURKISH;
			break;
	}

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
			gui_overlay_info_append_msg_precompiled(23, NULL);
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
			gui_overlay_info_append_msg_precompiled(24, NULL);
		}
	}

	emu_pause(FALSE);
}
void wdgSettingsGeneral::s_fds_bios_file_clear(UNUSED(bool checked)) {
	umemset(cfg->fds_bios_file, 0x00, usizeof(cfg->fds_bios_file));
	update_widget();
}
void wdgSettingsGeneral::s_fds_disk1sideA_at_reset(UNUSED(bool checked)) {
	emu_thread_pause();
	cfg->fds_disk1sideA_at_reset = !cfg->fds_disk1sideA_at_reset;
	emu_thread_continue();
}
void wdgSettingsGeneral::s_fds_switch_side_automatically(UNUSED(bool checked)) {
	emu_thread_pause();
	cfg->fds_switch_side_automatically = !cfg->fds_switch_side_automatically;
	emu_thread_continue();
}
void wdgSettingsGeneral::s_fds_fast_forward(UNUSED(bool checked)) {
	emu_thread_pause();
	cfg->fds_fast_forward = !cfg->fds_fast_forward;
	emu_thread_continue();
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
void wdgSettingsGeneral::s_multiple_settings(UNUSED(bool checked)) {
	cfg->multiple_instances = !cfg->multiple_instances;
}
