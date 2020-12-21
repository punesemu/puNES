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

#include "dlgSettings.moc"
#include "mainWindow.hpp"

dlgSettings::dlgSettings(QWidget *parent) : QDialog(parent) {
	setupUi(this);

	geom.setX(cfg->lg_settings.x);
	geom.setY(cfg->lg_settings.y);
	geom.setWidth(cfg->lg_settings.w);
	geom.setHeight(cfg->lg_settings.h);

	connect(pushButton_Save_Settings, SIGNAL(clicked(bool)), this, SLOT(s_save_settings(bool)));
	connect(pushButton_Close_Settings, SIGNAL(clicked(bool)), this, SLOT(s_close_settings(bool)));

	widget_Settings_Cheats->widget_Cheats_Editor->pushButton_Hide_Show_Tools->setVisible(false);

#if !defined (WITH_FFMPEG)
	tabWidget_Settings->removeTab(6);
#endif

	adjustSize();

	installEventFilter(this);
}
dlgSettings::~dlgSettings() {}

bool dlgSettings::eventFilter(QObject *obj, QEvent *event) {
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
void dlgSettings::changeEvent(QEvent *event) {
	if (event->type() == QEvent::LanguageChange) {
		retranslateUi(this);
	} else {
		QDialog::changeEvent(event);
	}
}
void dlgSettings::hideEvent(QHideEvent *event) {
	geom = geometry();
	QDialog::hideEvent(event);
}

void dlgSettings::retranslateUi(QDialog *dlgSettings) {
	Ui::dlgSettings::retranslateUi(dlgSettings);
	mainwin->qaction_shcut.save_settings->setText(pushButton_Save_Settings->text());
	mainwin->qaction_shcut.rwnd.active->setText(tr("Rewind Mode On/Off"));
	mainwin->qaction_shcut.rwnd.step_backward->setText(tr("Rewind Step Backward"));
	mainwin->qaction_shcut.rwnd.step_forward->setText(tr("Rewind Step Forward"));
	mainwin->qaction_shcut.rwnd.fast_backward->setText(tr("Rewind Fast Backward"));
	mainwin->qaction_shcut.rwnd.fast_forward->setText(tr("Rewind Fast Forward"));
	mainwin->qaction_shcut.rwnd.play->setText(tr("Rewind Play"));
	mainwin->qaction_shcut.rwnd.pause->setText(tr("Rewind Pause"));
}
void dlgSettings::update_dialog(void) {
	update_tab_general();
	update_tab_video();
	update_tab_audio();
	update_tab_input();
	update_tab_ppu();
	update_tab_cheats();
	update_tab_recording();
}
void dlgSettings::change_rom(void) {
	widget_Settings_Video->change_rom();
}
void dlgSettings::shcut_mode(int mode) {
	widget_Settings_General->shcut_mode(mode);
}
void dlgSettings::shcut_scale(int scale) {
	widget_Settings_Video->shcut_scale(scale + 1);
}

void dlgSettings::update_tab_general(void) {
	widget_Settings_General->update_widget();
}
void dlgSettings::update_tab_video(void) {
	widget_Settings_Video->update_widget();
}
void dlgSettings::update_tab_input(void) {
	widget_Settings_Input->update_widget();
}
void dlgSettings::update_tab_ppu(void) {
	widget_Settings_PPU->update_widget();
}
void dlgSettings::update_tab_cheats(void) {
	widget_Settings_Cheats->update_widget();
}

void dlgSettings::update_tab_audio(void) {
	widget_Settings_Audio->update_widget();
}
void dlgSettings::update_tab_recording(void) {
#if defined (WITH_FFMPEG)
	widget_Settings_Recording->update_widget();
#endif
}

void dlgSettings::s_save_settings(UNUSED(bool checked)) {
	settings_save();
}
void dlgSettings::s_close_settings(UNUSED(bool checked)) {
	hide();
}
