/*
 *  Copyright (C) 2010-2024 Fabio Cavallo (aka FHorse)
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

#include "wdgSettingsPPU.hpp"
#include "mainWindow.hpp"
#include "emu_thread.h"
#include "conf.h"
#include "gui.h"
#include "settings.h"

wdgSettingsPPU::wdgSettingsPPU(QWidget *parent) : QWidget(parent) {
	setupUi(this);

	setFocusProxy(checkBox_Hide_Sprites);

	connect(checkBox_Hide_Sprites, SIGNAL(clicked(bool)), this, SLOT(s_hide_sprites(bool)));
	connect(checkBox_Hide_Background, SIGNAL(clicked(bool)), this, SLOT(s_hide_background(bool)));
	connect(checkBox_Unlimited_Sprites, SIGNAL(clicked(bool)), this, SLOT(s_unlimited_sprites(bool)));
	connect(checkBox_Unlimited_Sprites_Auto, SIGNAL(clicked(bool)), this, SLOT(s_unlimited_sprites_auto(bool)));

	pushButton_Overclock_Pergame_setting_on->setProperty("mtype", QVariant(PERGAME_ON));
	pushButton_Overclock_Pergame_setting_off->setProperty("mtype", QVariant(PERGAME_OFF));
	pushButton_Overclock_Pergame_setting_use_def->setProperty("mtype", QVariant(PERGAME_DEFAULT));

	connect(pushButton_Overclock_Pergame_setting_on, SIGNAL(toggled(bool)), this, SLOT(s_overclock(bool)));
	connect(pushButton_Overclock_Pergame_setting_off, SIGNAL(toggled(bool)), this, SLOT(s_overclock(bool)));
	connect(pushButton_Overclock_Pergame_setting_use_def, SIGNAL(toggled(bool)), this, SLOT(s_overclock(bool)));

	pushButton_Overclock_Def_value_on->setProperty("mtype", QVariant(PERGAME_ON));
	pushButton_Overclock_Def_value_off->setProperty("mtype", QVariant(PERGAME_OFF));

	connect(pushButton_Overclock_Def_value_on, SIGNAL(toggled(bool)), this, SLOT(s_overclock_def_value(bool)));
	connect(pushButton_Overclock_Def_value_off, SIGNAL(toggled(bool)), this, SLOT(s_overclock_def_value(bool)));

	spinBox_Pergame_VB_Slines->setRange(0, 1000);
	spinBox_Pergame_Postrender_Slines->setRange(0, 1000);

	spinBox_Pergame_VB_Slines->setProperty("mtype", QVariant(0));
	pushButton_Pergame_VB_Slines->setProperty("mtype", QVariant(0));
	spinBox_Pergame_Postrender_Slines->setProperty("mtype", QVariant(0));
	pushButton_Pergame_Postrender_Slines->setProperty("mtype", QVariant(0));
	checkBox_Pergame_Disable_DMC_Control->setProperty("mtype", QVariant(0));

	connect(spinBox_Pergame_VB_Slines, SIGNAL(valueChanged(int)), this, SLOT(s_overclock_vb_slines(int)));
	connect(pushButton_Pergame_VB_Slines, SIGNAL(clicked(bool)), this, SLOT(s_overclock_vb_slines_reset(bool)));
	connect(spinBox_Pergame_Postrender_Slines, SIGNAL(valueChanged(int)), this, SLOT(s_overclock_pr_slines(int)));
	connect(pushButton_Pergame_Postrender_Slines, SIGNAL(clicked(bool)), this, SLOT(s_overclock_pr_slines_reset(bool)));
	connect(checkBox_Pergame_Disable_DMC_Control, SIGNAL(clicked(bool)), this, SLOT(s_overclock_disable_dmc_control(bool)));

	spinBox_Def_VB_Slines->setRange(0, 1000);
	spinBox_Def_Postrender_Slines->setRange(0, 1000);

	spinBox_Def_VB_Slines->setProperty("mtype", QVariant(1));
	pushButton_Def_VB_Slines->setProperty("mtype", QVariant(1));
	spinBox_Def_Postrender_Slines->setProperty("mtype", QVariant(1));
	pushButton_Def_Postrender_Slines->setProperty("mtype", QVariant(1));
	checkBox_Def_Disable_DMC_Control->setProperty("mtype", QVariant(1));

	connect(spinBox_Def_VB_Slines, SIGNAL(valueChanged(int)), this, SLOT(s_overclock_vb_slines(int)));
	connect(pushButton_Def_VB_Slines, SIGNAL(clicked(bool)), this, SLOT(s_overclock_vb_slines_reset(bool)));
	connect(spinBox_Def_Postrender_Slines, SIGNAL(valueChanged(int)), this, SLOT(s_overclock_pr_slines(int)));
	connect(pushButton_Def_Postrender_Slines, SIGNAL(clicked(bool)), this, SLOT(s_overclock_pr_slines_reset(bool)));
	connect(checkBox_Def_Disable_DMC_Control, SIGNAL(clicked(bool)), this, SLOT(s_overclock_disable_dmc_control(bool)));

	connect(pushButton_Reset_Lag_Counter, SIGNAL(clicked(bool)), this, SLOT(s_lag_counter_reset(bool)));

	connect(comboBox_CPUPPU_Alignment, SIGNAL(activated(int)), this, SLOT(s_cpuppu_aligment(int)));

	installEventFilter(this);
}
wdgSettingsPPU::~wdgSettingsPPU() = default;

void wdgSettingsPPU::changeEvent(QEvent *event) {
	if (event->type() == QEvent::LanguageChange) {
		Ui::wdgSettingsPPU::retranslateUi(this);
	} else {
		QWidget::changeEvent(event);
	}
}
void wdgSettingsPPU::showEvent(QShowEvent *event) {
	int dim = fontMetrics().height();

	icon_Sprites_and_background->setPixmap(QIcon(":/icon/icons/background.svgz").pixmap(dim, dim));
	icon_PPU_Overclock->setPixmap(QIcon(":/icon/icons/speedometer.svgz").pixmap(dim, dim));
	icon_PPU_Advanced->setPixmap(QIcon(":/icon/icons/chip.svgz").pixmap(dim, dim));

	QWidget::showEvent(event);
}

void wdgSettingsPPU::update_widget(void) {
	checkBox_Hide_Sprites->setChecked(cfg->hide_sprites);
	checkBox_Hide_Background->setChecked(cfg->hide_background);
	checkBox_Unlimited_Sprites->setChecked(cfg->unlimited_sprites);
	checkBox_Unlimited_Sprites_Auto->setEnabled(cfg->unlimited_sprites);
	checkBox_Unlimited_Sprites_Auto->setChecked(cfg->unlimited_sprites_auto);

	overclock_set();
	overclock_slines_set();

	comboBox_CPUPPU_Alignment->setCurrentIndex(cfg->ppu_alignment);

	lag_counter_update();
}
void wdgSettingsPPU::lag_counter_update(void) {
	lineEdit_Lag_Counter->setText(QString("%1").arg(info.lag_frame.totals));
}

void wdgSettingsPPU::overclock_set(void) {
	// per-game
	groupBox_Pergame_Slines->setEnabled(cfg->oclock_all.pergame.enabled == PERGAME_ON);

	qtHelper::pushbutton_set_checked(pushButton_Overclock_Pergame_setting_on, false);
	qtHelper::pushbutton_set_checked(pushButton_Overclock_Pergame_setting_off, false);
	qtHelper::pushbutton_set_checked(pushButton_Overclock_Pergame_setting_use_def, false);
	switch (cfg->oclock_all.pergame.enabled) {
		case PERGAME_ON:
			qtHelper::pushbutton_set_checked(pushButton_Overclock_Pergame_setting_on, true);
			break;
		default:
		case PERGAME_OFF:
			qtHelper::pushbutton_set_checked(pushButton_Overclock_Pergame_setting_off, true);
			break;
		case PERGAME_DEFAULT:
			qtHelper::pushbutton_set_checked(pushButton_Overclock_Pergame_setting_use_def, true);
			break;
	}
	// default
	groupBox_Def_Slines->setEnabled((cfg->oclock_all.pergame.enabled == PERGAME_DEFAULT) &&
		(cfg->oclock_all.def.enabled == PERGAME_ON));

	qtHelper::pushbutton_set_checked(pushButton_Overclock_Def_value_on, false);
	qtHelper::pushbutton_set_checked(pushButton_Overclock_Def_value_off, false);
	if (cfg->oclock_all.def.enabled == PERGAME_ON) {
		qtHelper::pushbutton_set_checked(pushButton_Overclock_Def_value_on, true);
	} else {
		qtHelper::pushbutton_set_checked(pushButton_Overclock_Def_value_off, true);
	}
}
void  wdgSettingsPPU::overclock_slines_set(void) {
	qtHelper::spinbox_set_value(spinBox_Pergame_VB_Slines, cfg->oclock_all.pergame.extra_slines.vblank);
	qtHelper::spinbox_set_value(spinBox_Pergame_Postrender_Slines, cfg->oclock_all.pergame.extra_slines.postrender);
	qtHelper::checkbox_set_checked(checkBox_Pergame_Disable_DMC_Control, cfg->oclock_all.pergame.dmc_control_disabled);

	qtHelper::spinbox_set_value(spinBox_Def_VB_Slines, cfg->oclock_all.def.extra_slines.vblank);
	qtHelper::spinbox_set_value(spinBox_Def_Postrender_Slines, cfg->oclock_all.def.extra_slines.postrender);
	qtHelper::checkbox_set_checked(checkBox_Def_Disable_DMC_Control, cfg->oclock_all.def.dmc_control_disabled);
}

void wdgSettingsPPU::s_hide_sprites(UNUSED(bool checked)) {
	emu_thread_pause();
	cfg->hide_sprites = !cfg->hide_sprites;
	emu_thread_continue();
}
void wdgSettingsPPU::s_hide_background(UNUSED(bool checked)) {
	emu_thread_pause();
	cfg->hide_background = !cfg->hide_background;
	emu_thread_continue();
}
void wdgSettingsPPU::s_unlimited_sprites(UNUSED(bool checked)) {
	emu_thread_pause();
	cfg->unlimited_sprites = !cfg->unlimited_sprites;
	emu_thread_continue();
	update_widget();
}
void wdgSettingsPPU::s_unlimited_sprites_auto(UNUSED(bool checked)) {
	emu_thread_pause();
	cfg->unlimited_sprites_auto = !cfg->unlimited_sprites_auto;
	emu_thread_continue();
}
void wdgSettingsPPU::s_overclock(bool checked) {
	if (checked) {
		emu_thread_pause();
		cfg->oclock_all.pergame.enabled = QVariant(((themePushButton *)sender())->property("mtype")).toInt();
		cfg->oclock = cfg->oclock_all.pergame.enabled == PERGAME_DEFAULT
			? &cfg->oclock_all.def
			: &cfg->oclock_all.pergame;
		ppu_overclock(0, TRUE);
		ppu_overclock(1, TRUE);
		settings_pgs_save();
		emu_thread_continue();
	}
	overclock_set();
}
void wdgSettingsPPU::s_overclock_def_value(bool checked) {
	if (checked) {
		emu_thread_pause();
		cfg->oclock_all.def.enabled = QVariant(((themePushButton *)sender())->property("mtype")).toInt();
		cfg->oclock = cfg->oclock_all.pergame.enabled == PERGAME_DEFAULT
			? &cfg->oclock_all.def
			: &cfg->oclock_all.pergame;
		ppu_overclock(0, TRUE);
		ppu_overclock(1, TRUE);
		settings_pgs_save();
		emu_thread_continue();
	}
	overclock_set();
}
void wdgSettingsPPU::s_overclock_vb_slines(int i) {
	_config_overclock *oclock = QVariant(((themePushButton *)sender())->property("mtype")).toInt() == 0
		? &cfg->oclock_all.pergame
		: &cfg->oclock_all.def;

	emu_thread_pause();
	oclock->extra_slines.vblank = i;
	ppu_overclock(0, FALSE);
	ppu_overclock(1, FALSE);
	settings_pgs_save();
	emu_thread_continue();
}
void wdgSettingsPPU::s_overclock_vb_slines_reset(UNUSED(bool checked)) {
	s_overclock_vb_slines(0);
	overclock_slines_set();
}
void wdgSettingsPPU::s_overclock_pr_slines(int i) {
	_config_overclock *oclock = QVariant(((themePushButton *)sender())->property("mtype")).toInt() == 0
		? &cfg->oclock_all.pergame
		: &cfg->oclock_all.def;

	emu_thread_pause();
	oclock->extra_slines.postrender = i;
	ppu_overclock(0, FALSE);
	ppu_overclock(1, FALSE);
	settings_pgs_save();
	emu_thread_continue();
}
void wdgSettingsPPU::s_overclock_pr_slines_reset(UNUSED(bool checked)) {
	s_overclock_pr_slines(0);
	overclock_slines_set();
}
void wdgSettingsPPU::s_overclock_disable_dmc_control(UNUSED(bool checked)) {
	_config_overclock *oclock = QVariant(((themePushButton *)sender())->property("mtype")).toInt() == 0
		? &cfg->oclock_all.pergame
		: &cfg->oclock_all.def;

	emu_thread_pause();
	oclock->dmc_control_disabled = !oclock->dmc_control_disabled;
	settings_pgs_save();
	emu_thread_continue();
}
void wdgSettingsPPU::s_lag_counter_reset(UNUSED(bool checked)) {
	emu_thread_pause();
	info.lag_frame.totals = 0;
	emu_thread_continue();
	lag_counter_update();
}
void wdgSettingsPPU::s_cpuppu_aligment(int index) {
	if (cfg->ppu_alignment == index) {
		return;
	} else if (index == PPU_ALIGMENT_INC_AT_RESET) {
		ppu_alignment_reset();
	}
	cfg->ppu_alignment = index;
	gui_update_status_bar();
}
