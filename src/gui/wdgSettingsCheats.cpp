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

#include "wdgSettingsCheats.moc"
#include "mainWindow.hpp"
#include "emu_thread.h"
#include "conf.h"

wdgSettingsCheats::wdgSettingsCheats(QWidget *parent) : QWidget(parent) {
	setupUi(this);

	connect(comboBox_Cheats_Mode, SIGNAL(activated(int)), this, SLOT(s_cheat_mode(int)));
}
wdgSettingsCheats::~wdgSettingsCheats() {}

void wdgSettingsCheats::changeEvent(QEvent *event) {
	if (event->type() == QEvent::LanguageChange) {
		retranslateUi(this);
	} else {
		QWidget::changeEvent(event);
	}
}

void wdgSettingsCheats::retranslateUi(QWidget *wdgSettingsCheats) {
	Ui::wdgSettingsCheats::retranslateUi(wdgSettingsCheats);
	update_widget();
}
void wdgSettingsCheats::update_widget(void) {
	cheat_mode_set();
}

void wdgSettingsCheats::cheat_mode_set(void) {
	comboBox_Cheats_Mode->setCurrentIndex(cfg->cheat_mode);
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

void wdgSettingsCheats::s_cheat_mode(int index) {
	int mode = index;

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
