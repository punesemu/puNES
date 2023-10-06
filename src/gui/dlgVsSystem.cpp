/*
 *  Copyright (C) 2010-2023 Fabio Cavallo (aka FHorse)
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

#include <QtGui/QScreen>
#include "dlgVsSystem.hpp"
#include "mainWindow.hpp"
#include "dlgSettings.hpp"
#include "vs_system.h"
#include "clock.h"
#include "conf.h"
#include "gui.h"
#include "dipswitch.h"
#include "emu_thread.h"

dlgVsSystem::dlgVsSystem(QWidget *parent) : QDialog(parent) {
	setupUi(this);

	setStyleSheet(tools_stylesheet());

	widget_Monitor->setStyleSheet(button_stylesheet());

	pushButton_Left_Coin->setProperty("myIndex", QVariant(1));
	pushButton_Right_Coin->setProperty("myIndex", QVariant(2));
	pushButton_Service_Coin->setProperty("myIndex", QVariant(3));

	pushButton_Left_Monitor->setProperty("myIndex", QVariant(0));
	pushButton_Right_Monitor->setProperty("myIndex", QVariant(1));

	connect(pushButton_Left_Coin, SIGNAL(clicked(bool)), this, SLOT(s_coins_clicked(bool)));
	connect(pushButton_Right_Coin, SIGNAL(clicked(bool)), this, SLOT(s_coins_clicked(bool)));
	connect(pushButton_Service_Coin, SIGNAL(clicked(bool)), this, SLOT(s_coins_clicked(bool)));
	connect(pushButton_Left_Monitor, SIGNAL(clicked(bool)), this, SLOT(s_monitor_clicked(bool)));
	connect(pushButton_Right_Monitor, SIGNAL(clicked(bool)), this, SLOT(s_monitor_clicked(bool)));
	connect(pushButton_Dip_Switches, SIGNAL(clicked(bool)), this, SLOT(s_ds_clicked(bool)));
	connect(pushButton_Defaults_Dip_Switches, SIGNAL(clicked(bool)), this, SLOT(s_ds_defaults_clicked(bool)));

	checkBox_ds1->setProperty("myIndex", QVariant(1));
	checkBox_ds2->setProperty("myIndex", QVariant(2));
	checkBox_ds3->setProperty("myIndex", QVariant(3));
	checkBox_ds4->setProperty("myIndex", QVariant(4));
	checkBox_ds5->setProperty("myIndex", QVariant(5));
	checkBox_ds6->setProperty("myIndex", QVariant(6));
	checkBox_ds7->setProperty("myIndex", QVariant(7));
	checkBox_ds8->setProperty("myIndex", QVariant(8));
	checkBox_ds9->setProperty("myIndex", QVariant(9));
	checkBox_ds10->setProperty("myIndex", QVariant(10));
	checkBox_ds11->setProperty("myIndex", QVariant(11));
	checkBox_ds12->setProperty("myIndex", QVariant(12));
	checkBox_ds13->setProperty("myIndex", QVariant(13));
	checkBox_ds14->setProperty("myIndex", QVariant(14));
	checkBox_ds15->setProperty("myIndex", QVariant(15));
	checkBox_ds16->setProperty("myIndex", QVariant(16));

	connect(checkBox_ds1, SIGNAL(stateChanged(int)), this, SLOT(s_ds_changed(int)));
	connect(checkBox_ds2, SIGNAL(stateChanged(int)), this, SLOT(s_ds_changed(int)));
	connect(checkBox_ds3, SIGNAL(stateChanged(int)), this, SLOT(s_ds_changed(int)));
	connect(checkBox_ds4, SIGNAL(stateChanged(int)), this, SLOT(s_ds_changed(int)));
	connect(checkBox_ds5, SIGNAL(stateChanged(int)), this, SLOT(s_ds_changed(int)));
	connect(checkBox_ds6, SIGNAL(stateChanged(int)), this, SLOT(s_ds_changed(int)));
	connect(checkBox_ds7, SIGNAL(stateChanged(int)), this, SLOT(s_ds_changed(int)));
	connect(checkBox_ds8, SIGNAL(stateChanged(int)), this, SLOT(s_ds_changed(int)));
	connect(checkBox_ds9, SIGNAL(stateChanged(int)), this, SLOT(s_ds_changed(int)));
	connect(checkBox_ds10, SIGNAL(stateChanged(int)), this, SLOT(s_ds_changed(int)));
	connect(checkBox_ds11, SIGNAL(stateChanged(int)), this, SLOT(s_ds_changed(int)));
	connect(checkBox_ds12, SIGNAL(stateChanged(int)), this, SLOT(s_ds_changed(int)));
	connect(checkBox_ds13, SIGNAL(stateChanged(int)), this, SLOT(s_ds_changed(int)));
	connect(checkBox_ds14, SIGNAL(stateChanged(int)), this, SLOT(s_ds_changed(int)));
	connect(checkBox_ds15, SIGNAL(stateChanged(int)), this, SLOT(s_ds_changed(int)));
	connect(checkBox_ds16, SIGNAL(stateChanged(int)), this, SLOT(s_ds_changed(int)));

	setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);

	setAttribute(Qt::WA_DeleteOnClose);

	{
		QFont f9;
		int h = 0;

		f9.setPointSize(9);
		f9.setWeight(QFont::Light);

		if (lineEdit_Coin_Counter->font().pointSize() > 9) {
			lineEdit_Coin_Counter->setFont(f9);
		}

		h = lineEdit_Coin_Counter->fontMetrics().size(0, "1234567890").height() + 10;

		lineEdit_Coin_Counter->setFixedHeight(h);
	}

	adjustSize();
	// Se setto il fixed size, su windows xp non mi visualizza il dialog correttamente.
	//setFixedSize(size());

	{
		QMargins vgbm = verticalLayout_groupBox_Vs_System->contentsMargins();
		QMargins vdia = verticalLayout_Vs_System->contentsMargins();
		QPushButton *close = new QPushButton(this);
		int x = 0, y = 0, w = 0, h = 0;

		w = close->fontMetrics().size(0, "x").width() + 10;
		h = close->fontMetrics().size(0, "x").height() + 5;
		x = normalGeometry().width() - w - vdia.right() - 2 - vgbm.right();
		y = vdia.top() + 2 + 1;

		close->setGeometry(x, y, w, h);
		close->setText("x");

		connect(close, SIGNAL(clicked(bool)), this, SLOT(s_x_clicked(bool)));

		vgbm.setTop(close->sizeHint().height() + 2);
		verticalLayout_groupBox_Vs_System->setContentsMargins(vgbm);
	}

	installEventFilter(this);
}
dlgVsSystem::~dlgVsSystem() = default;

bool dlgVsSystem::eventFilter(QObject *obj, QEvent *event) {
	switch (event->type()) {
		case QEvent::WindowActivate:
		case QEvent::WindowDeactivate:
			gui_control_pause_bck(event->type());
			break;
		default:
			break;
	}
	return (QDialog::eventFilter(obj, event));
}
void dlgVsSystem::changeEvent(QEvent *event) {
	if (event->type() == QEvent::LanguageChange) {
		Ui::dlgVsSystem::retranslateUi(this);
	} else {
		QWidget::changeEvent(event);
	}
}
void dlgVsSystem::showEvent(UNUSED(QShowEvent *event)) {
	int dim = fontMetrics().height();

	icon_Coins->setPixmap(QIcon(":/icon/icons/insert_coins.svgz").pixmap(dim, dim));
	icon_Coin_Counter->setPixmap(QIcon(":/icon/icons/stereo_delay.svgz").pixmap(dim, dim));
	icon_Monitor->setPixmap(QIcon(":/icon/icons/monitor_screen.svgz").pixmap(dim, dim));
	icon_Dipswitches->setPixmap(QIcon(":/icon/icons/dipswitch.svgz").pixmap(dim, dim));
}

int dlgVsSystem::update_pos(int startY) {
	int x = parentWidget()->pos().x() + parentWidget()->frameGeometry().width();
	int y = parentWidget()->geometry().y() + startY;
	QRect g = QGuiApplication::primaryScreen()->virtualGeometry();

	if ((x + frameGeometry().width() - g.left()) > g.width()) {
		x = parentWidget()->pos().x() - frameGeometry().width();
	}
	move(QPoint(x, y));

	if (isHidden()) {
		return (0);
	}

	return (frameGeometry().height());
}
void dlgVsSystem::update_dialog(void) {
	groupBox_Vs_System->setEnabled(vs_system.enabled);

	lineEdit_Coin_Counter->setText(QString("%1").arg(vs_system.coins.counter));

	qtHelper::pushbutton_set_checked(pushButton_Left_Monitor, false);
	qtHelper::pushbutton_set_checked(pushButton_Right_Monitor, false);
	if (cfg->vs_monitor == 0) {
		qtHelper::pushbutton_set_checked(pushButton_Left_Monitor, true);
	} else {
		qtHelper::pushbutton_set_checked(pushButton_Right_Monitor, true);
	}

	qtHelper::checkbox_set_checked(checkBox_ds1, (cfg->dipswitch & 0x01) >> 0);
	qtHelper::checkbox_set_checked(checkBox_ds2, (cfg->dipswitch & 0x02) >> 1);
	qtHelper::checkbox_set_checked(checkBox_ds3, (cfg->dipswitch & 0x04) >> 2);
	qtHelper::checkbox_set_checked(checkBox_ds4, (cfg->dipswitch & 0x08) >> 3);
	qtHelper::checkbox_set_checked(checkBox_ds5, (cfg->dipswitch & 0x10) >> 4);
	qtHelper::checkbox_set_checked(checkBox_ds6, (cfg->dipswitch & 0x20) >> 5);
	qtHelper::checkbox_set_checked(checkBox_ds7, (cfg->dipswitch & 0x40) >> 6);
	qtHelper::checkbox_set_checked(checkBox_ds8, (cfg->dipswitch & 0x80) >> 7);

	qtHelper::checkbox_set_checked(checkBox_ds9, ((cfg->dipswitch >> 8) & 0x01) >> 0);
	qtHelper::checkbox_set_checked(checkBox_ds10, ((cfg->dipswitch >> 8) & 0x02) >> 1);
	qtHelper::checkbox_set_checked(checkBox_ds11, ((cfg->dipswitch >> 8) & 0x04) >> 2);
	qtHelper::checkbox_set_checked(checkBox_ds12, ((cfg->dipswitch >> 8) & 0x08) >> 3);
	qtHelper::checkbox_set_checked(checkBox_ds13, ((cfg->dipswitch >> 8) & 0x10) >> 4);
	qtHelper::checkbox_set_checked(checkBox_ds14, ((cfg->dipswitch >> 8) & 0x20) >> 5);
	qtHelper::checkbox_set_checked(checkBox_ds15, ((cfg->dipswitch >> 8) & 0x40) >> 6);
	qtHelper::checkbox_set_checked(checkBox_ds16, ((cfg->dipswitch >> 8) & 0x80) >> 7);

	widget_Monitor->setEnabled(vs_system.special_mode.type == VS_DS_Normal);
	widget_Dip_Switches2->setEnabled(vs_system.special_mode.type == VS_DS_Normal);

	pushButton_Dip_Switches->setEnabled(dipswitch_type_length() > 0);
}
void dlgVsSystem::insert_coin(int index) {
	int base = vs_system_cn_next()

	switch (index) {
		default:
		case 1:
			vs_system.coins.left = base;
			break;
		case 2:
			vs_system.coins.right = base;
			break;
		case 3:
			vs_system.coins.service = base;
			break;
	}
}

void dlgVsSystem::s_coins_clicked(UNUSED(bool checked)) {
	int index = QVariant(((QCheckBox *)sender())->property("myIndex")).toInt();

	insert_coin(index);
	gui_active_window();
	gui_set_focus();
}
void dlgVsSystem::s_monitor_clicked(UNUSED(bool checked)) {
	emu_thread_pause();
	cfg->vs_monitor = !cfg->vs_monitor;
	emu_thread_continue();

	update_dialog();
	gui_active_window();
	gui_set_focus();
}
void dlgVsSystem::s_ds_changed(int state) {
	int index = QVariant(((QCheckBox *)sender())->property("myIndex")).toInt();

	switch (index) {
		case 1:
			cfg->dipswitch = (cfg->dipswitch & 0xFFFE) | (state ? 0x0001 : 0x0000);
			break;
		case 2:
			cfg->dipswitch = (cfg->dipswitch & 0xFFFD) | (state ? 0x0002 : 0x0000);
			break;
		case 3:
			cfg->dipswitch = (cfg->dipswitch & 0xFFFB) | (state ? 0x0004 : 0x0000);
			break;
		case 4:
			cfg->dipswitch = (cfg->dipswitch & 0xFFF7) | (state ? 0x0008 : 0x0000);
			break;
		case 5:
			cfg->dipswitch = (cfg->dipswitch & 0xFFEF) | (state ? 0x0010 : 0x0000);
			break;
		case 6:
			cfg->dipswitch = (cfg->dipswitch & 0xFFDF) | (state ? 0x0020 : 0x0000);
			break;
		case 7:
			cfg->dipswitch = (cfg->dipswitch & 0xFFBF) | (state ? 0x0040 : 0x0000);
			break;
		case 8:
			cfg->dipswitch = (cfg->dipswitch & 0xFF7F) | (state ? 0x0080 : 0x0000);
			break;
		case 9:
			cfg->dipswitch = (cfg->dipswitch & 0xFEFF) | (state ? 0x0100 : 0x0000);
			break;
		case 10:
			cfg->dipswitch = (cfg->dipswitch & 0xFDFF) | (state ? 0x0200 : 0x0000);
			break;
		case 11:
			cfg->dipswitch = (cfg->dipswitch & 0xFBFF) | (state ? 0x0400 : 0x0000);
			break;
		case 12:
			cfg->dipswitch = (cfg->dipswitch & 0xF7FF) | (state ? 0x0800 : 0x0000);
			break;
		case 13:
			cfg->dipswitch = (cfg->dipswitch & 0xEFFF) | (state ? 0x1000 : 0x0000);
			break;
		case 14:
			cfg->dipswitch = (cfg->dipswitch & 0xDFFF) | (state ? 0x2000 : 0x0000);
			break;
		case 15:
			cfg->dipswitch = (cfg->dipswitch & 0xBFFF) | (state ? 0x4000 : 0x0000);
			break;
		case 16:
			cfg->dipswitch = (cfg->dipswitch & 0x7FFF) | (state ? 0x8000 : 0x0000);
			break;
		default:
			break;
	}
	settings_pgs_save();
	gui_active_window();
	gui_set_focus();
}
void dlgVsSystem::s_ds_clicked(UNUSED(bool checked)) {
	emu_thread_pause();
	gui_dipswitch_dialog();
	emu_thread_continue();

	update_dialog();
	gui_active_window();
	gui_set_focus();
}
void dlgVsSystem::s_ds_defaults_clicked(UNUSED(bool checked)) {
	cfg->dipswitch = dipswitch.def;
	update_dialog();

	settings_pgs_save();
	gui_active_window();
	gui_set_focus();
}
void dlgVsSystem::s_x_clicked(UNUSED(bool checked)) {
	mainwin->s_set_vs_window();
}
