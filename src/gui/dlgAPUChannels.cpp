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

#include <QtWidgets/QDesktopWidget>
#include "dlgAPUChannels.moc"
#include "mainWindow.hpp"
#include "dlgSettings.hpp"
#include "conf.h"
#include "gui.h"

dlgAPUChannels::dlgAPUChannels(QWidget *parent) : QDialog(parent) {
	setupUi(this);

	setStyleSheet(tools_stylesheet());

	connect(checkBox_Swap_Duty_Cycles, SIGNAL(clicked(bool)), this, SLOT(s_swap_duty_cycles(bool)));

	setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);

	setAttribute(Qt::WA_DeleteOnClose);

	adjustSize();
	setFixedSize(size());

	{
		QMargins vgbm = verticalLayout_groupBox_APU_Channels->contentsMargins();
		QMargins vdia = verticalLayout_APU_Channels->contentsMargins();
		QPushButton *close = new QPushButton(this);
		int x, y, w, h;

		w = close->fontMetrics().size(0, "x").width() + 10;
		h = close->fontMetrics().size(0, "x").height() + 5;
		x = normalGeometry().width() - w -	vdia.right() - 2 - vgbm.right();
		y = vdia.top() + 2 + 1;

		close->setGeometry(x, y, w, h);
		close->setText("x");

		connect(close, SIGNAL(clicked(bool)), this, SLOT(s_x(bool)));

		vgbm.setTop(close->sizeHint().height() + 2);
		verticalLayout_groupBox_APU_Channels->setContentsMargins(vgbm);
	}

	installEventFilter(this);
}
dlgAPUChannels::~dlgAPUChannels() {}

bool dlgAPUChannels::eventFilter(QObject *obj, QEvent *event) {
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
void dlgAPUChannels::changeEvent(QEvent *event) {
	if (event->type() == QEvent::LanguageChange) {
		Ui::dlgAPUChannels::retranslateUi(this);
	} else {
		QDialog::changeEvent(event);
	}
}

int dlgAPUChannels::update_pos(int startY) {
	int screenNumber = qApp->desktop()->screenNumber(parentWidget());
	int x = parentWidget()->pos().x() + parentWidget()->frameGeometry().width();
	int y = parentWidget()->geometry().y() + startY;

	if ((x + frameGeometry().width() - qApp->desktop()->screenGeometry(screenNumber).left()) >
		qApp->desktop()->screenGeometry(screenNumber).width()) {
		x = parentWidget()->pos().x() - frameGeometry().width();
	}
	move(QPoint(x, y));

	if (isHidden() == true) {
		return (0);
	}

	return (frameGeometry().height());
}
void dlgAPUChannels::update_dialog(void) {
	widget_wdgAPUChannels->update_widget();
	checkBox_Swap_Duty_Cycles->setChecked(cfg->swap_duty);
}

void dlgAPUChannels::s_swap_duty_cycles(bool checked) {
	dlgsettings->widget_wdgSettingsAudio->checkBox_Swap_Duty_Cycles->click();
}
void dlgAPUChannels::s_x(bool checked) {
	mainwin->s_set_apu_channels();
}
