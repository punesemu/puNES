/*
 *  Copyright (C) 2010-2020 Fabio Cavallo (aka FHorse)
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

#include <QtWidgets/QDesktopWidget>
#include <QtGui/QScreen>
#include "dlgVsSystem.moc"
#include "mainWindow.hpp"
#include "dlgSettings.hpp"
#include "vs_system.h"
#include "clock.h"
#include "gui.h"

dlgVsSystem::dlgVsSystem(QWidget *parent) : QDialog(parent) {
	in_update = false;

	setupUi(this);

	setStyleSheet(tools_stylesheet());

	pushButton_Left_Coin->setProperty("myIndex", QVariant(1));
	pushButton_Right_Coin->setProperty("myIndex", QVariant(2));
	pushButton_Service_Coin->setProperty("myIndex", QVariant(3));

	connect(pushButton_Left_Coin, SIGNAL(clicked(bool)), this, SLOT(s_coins_clicked(bool)));
	connect(pushButton_Right_Coin, SIGNAL(clicked(bool)), this, SLOT(s_coins_clicked(bool)));
	connect(pushButton_Service_Coin, SIGNAL(clicked(bool)), this, SLOT(s_coins_clicked(bool)));
	connect(pushButton_Defaults_Dip_Switches, SIGNAL(clicked(bool)), this, SLOT(s_defaults_clicked(bool)));

	checkBox_ds1->setProperty("myIndex", QVariant(1));
	checkBox_ds2->setProperty("myIndex", QVariant(2));
	checkBox_ds3->setProperty("myIndex", QVariant(3));
	checkBox_ds4->setProperty("myIndex", QVariant(4));
	checkBox_ds5->setProperty("myIndex", QVariant(5));
	checkBox_ds6->setProperty("myIndex", QVariant(6));
	checkBox_ds7->setProperty("myIndex", QVariant(7));
	checkBox_ds8->setProperty("myIndex", QVariant(8));

	connect(checkBox_ds1, SIGNAL(stateChanged(int)), this, SLOT(s_ds_changed(int)));
	connect(checkBox_ds2, SIGNAL(stateChanged(int)), this, SLOT(s_ds_changed(int)));
	connect(checkBox_ds3, SIGNAL(stateChanged(int)), this, SLOT(s_ds_changed(int)));
	connect(checkBox_ds4, SIGNAL(stateChanged(int)), this, SLOT(s_ds_changed(int)));
	connect(checkBox_ds5, SIGNAL(stateChanged(int)), this, SLOT(s_ds_changed(int)));
	connect(checkBox_ds6, SIGNAL(stateChanged(int)), this, SLOT(s_ds_changed(int)));
	connect(checkBox_ds7, SIGNAL(stateChanged(int)), this, SLOT(s_ds_changed(int)));
	connect(checkBox_ds8, SIGNAL(stateChanged(int)), this, SLOT(s_ds_changed(int)));

	setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);

	setAttribute(Qt::WA_DeleteOnClose);

	{
		QFont f9;
		int h;

		f9.setPointSize(9);
		f9.setWeight(QFont::Light);

		if (lineEdit_Coin_Counter->font().pointSize() > 9) {
			lineEdit_Coin_Counter->setFont(f9);
		}

		h = lineEdit_Coin_Counter->fontMetrics().size(0, "1234567890").height() + 10;

		lineEdit_Coin_Counter->setFixedHeight(h);
	}

	adjustSize();
	// se setto il fixed size, su windows xp non mi visualizza il dialog correttamente.
	//setFixedSize(size());

	{
		QMargins vgbm = verticalLayout_groupBox_Vs_System->contentsMargins();
		QMargins vdia = verticalLayout_Vs_System->contentsMargins();
		QPushButton *close = new QPushButton(this);
		int x, y, w, h;

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
dlgVsSystem::~dlgVsSystem() {}

bool dlgVsSystem::eventFilter(QObject *obj, QEvent *event) {
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
void dlgVsSystem::changeEvent(QEvent *event) {
	if (event->type() == QEvent::LanguageChange) {
		Ui::dlgVsSystem::retranslateUi(this);
	} else {
		QWidget::changeEvent(event);
	}
}

int dlgVsSystem::update_pos(int startY) {
	int x = parentWidget()->pos().x() + parentWidget()->frameGeometry().width();
	int y = parentWidget()->geometry().y() + startY;
	QRect g = QGuiApplication::primaryScreen()->virtualGeometry();

	if ((x + frameGeometry().width() - g.left()) > g.width()) {
		x = parentWidget()->pos().x() - frameGeometry().width();
	}
	move(QPoint(x, y));

	if (isHidden() == true) {
		return (0);
	}

	return (frameGeometry().height());
}
void dlgVsSystem::update_dialog(void) {
	in_update = true;

	lineEdit_Coin_Counter->setText(QString("%1").arg(vs_system.coins.counter));

	checkBox_ds1->setChecked((cfg->dipswitch & 0x01) >> 0);
	checkBox_ds2->setChecked((cfg->dipswitch & 0x02) >> 1);
	checkBox_ds3->setChecked((cfg->dipswitch & 0x04) >> 2);
	checkBox_ds4->setChecked((cfg->dipswitch & 0x08) >> 3);
	checkBox_ds5->setChecked((cfg->dipswitch & 0x10) >> 4);
	checkBox_ds6->setChecked((cfg->dipswitch & 0x20) >> 5);
	checkBox_ds7->setChecked((cfg->dipswitch & 0x40) >> 6);
	checkBox_ds8->setChecked((cfg->dipswitch & 0x80) >> 7);

	groupBox_Vs_System->setEnabled(vs_system.enabled);

	in_update = false;
}
void dlgVsSystem::insert_coin(int index) {
	int base = vs_system_cn_next();

	switch (index) {
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
void dlgVsSystem::s_ds_changed(int state) {
	int index = QVariant(((QCheckBox *)sender())->property("myIndex")).toInt();

	if (in_update == true) {
		return;
	}

	switch (index) {
		case 1:
			cfg->dipswitch = (cfg->dipswitch & 0xFE) | (state ? 0x01 : 0x00);
			break;
		case 2:
			cfg->dipswitch = (cfg->dipswitch & 0xFD) | (state ? 0x02 : 0x00);
			break;
		case 3:
			cfg->dipswitch = (cfg->dipswitch & 0xFB) | (state ? 0x04 : 0x00);
			break;
		case 4:
			cfg->dipswitch = (cfg->dipswitch & 0xF7) | (state ? 0x08 : 0x00);
			break;
		case 5:
			cfg->dipswitch = (cfg->dipswitch & 0xEF) | (state ? 0x10 : 0x00);
			break;
		case 6:
			cfg->dipswitch = (cfg->dipswitch & 0xDF) | (state ? 0x20 : 0x00);
			break;
		case 7:
			cfg->dipswitch = (cfg->dipswitch & 0xBF) | (state ? 0x40 : 0x00);
			break;
		case 8:
			cfg->dipswitch = (cfg->dipswitch & 0x7F) | (state ? 0x80 : 0x00);
			break;
	}
	settings_pgs_save();
	gui_active_window();
	gui_set_focus();
}
void dlgVsSystem::s_defaults_clicked(UNUSED(bool checked)) {
	cfg->dipswitch = info.default_dipswitches;
	update_dialog();

	settings_pgs_save();
	gui_active_window();
	gui_set_focus();
}
void dlgVsSystem::s_x_clicked(UNUSED(bool checked)) {
	mainwin->s_set_vs_window();
}
