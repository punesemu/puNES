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

#include "wdgSettingsCheats.hpp"
#include "mainWindow.hpp"
#include "emu_thread.h"
#include "conf.h"

wdgSettingsCheats::wdgSettingsCheats(QWidget *parent) : QWidget(parent) {
	setupUi(this);

	setFocusProxy(widget_Cheats_Mode);

	widget_Cheats_Mode->setStyleSheet(button_stylesheet());

	pushButton_Cheats_Mode_disabled->setProperty("mtype", QVariant(NOCHEAT_MODE));
	pushButton_Cheats_Mode_gg->setProperty("mtype", QVariant(GAMEGENIE_MODE));
	pushButton_Cheats_Mode_list->setProperty("mtype", QVariant(CHEATSLIST_MODE));

	connect(pushButton_Cheats_Mode_disabled, SIGNAL(toggled(bool)), this, SLOT(s_cheat_mode(bool)));
	connect(pushButton_Cheats_Mode_gg, SIGNAL(toggled(bool)), this, SLOT(s_cheat_mode(bool)));
	connect(pushButton_Cheats_Mode_list, SIGNAL(toggled(bool)), this, SLOT(s_cheat_mode(bool)));
}
wdgSettingsCheats::~wdgSettingsCheats() {}

void wdgSettingsCheats::changeEvent(QEvent *event) {
	if (event->type() == QEvent::LanguageChange) {
		retranslateUi(this);
	} else {
		QWidget::changeEvent(event);
	}
}
void wdgSettingsCheats::showEvent(QShowEvent *event) {
	int dim = fontMetrics().height();

	icon_Cheats_settings->setPixmap(QIcon(":/icon/icons/settings.svg").pixmap(dim, dim));
	QWidget::showEvent(event);
}

void wdgSettingsCheats::retranslateUi(QWidget *wdgSettingsCheats) {
	Ui::wdgSettingsCheats::retranslateUi(wdgSettingsCheats);
	update_widget();
}
void wdgSettingsCheats::update_widget(void) {
	cheat_mode_set();
}

void wdgSettingsCheats::cheat_mode_set(void) {
	qtHelper::pushbutton_set_checked(pushButton_Cheats_Mode_disabled, false);
	qtHelper::pushbutton_set_checked(pushButton_Cheats_Mode_gg, false);
	qtHelper::pushbutton_set_checked(pushButton_Cheats_Mode_list, false);
	switch (cfg->cheat_mode) {
		default:
		case NOCHEAT_MODE:
			qtHelper::pushbutton_set_checked(pushButton_Cheats_Mode_disabled, true);
			break;
		case GAMEGENIE_MODE:
			qtHelper::pushbutton_set_checked(pushButton_Cheats_Mode_gg, true);
			break;
		case CHEATSLIST_MODE:
			qtHelper::pushbutton_set_checked(pushButton_Cheats_Mode_list, true);
			break;
	}
	widget_Cheats_Editor->populate_cheat_table();
	cheat_editor_control();
}
void wdgSettingsCheats::cheat_editor_control(void) {
	switch (cfg->cheat_mode) {
		case NOCHEAT_MODE:
		case GAMEGENIE_MODE:
			widget_Cheats_Editor->setEnabled(false);
			break;
		case CHEATSLIST_MODE:
			widget_Cheats_Editor->setEnabled(true);
			break;
	}
}

void wdgSettingsCheats::s_cheat_mode(bool checked) {
	if (checked) {
		int mode = QVariant(((QPushButton *)sender())->property("mtype")).toInt();

		if (cfg->cheat_mode == mode) {
			return;
		}

		emu_thread_pause();
		cfg->cheat_mode = mode;
		cheat_editor_control();
		switch (cfg->cheat_mode) {
			case NOCHEAT_MODE:
				cheatslist_blank();
				break;
			case GAMEGENIE_MODE:
				cheatslist_blank();
				gamegenie_check_rom_present(TRUE);
				break;
			case CHEATSLIST_MODE:
				objcheat->apply_cheats();
				break;
		}
		emu_thread_continue();
	}
	cheat_mode_set();
}
