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
#include "dlgPPUHacks.moc"
#include "mainWindow.hpp"

dlgPPUHacks::dlgPPUHacks(QWidget *parent) : QDialog(parent) {
	setupUi(this);

	setStyleSheet(tools_stylesheet());

	setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);

	setAttribute(Qt::WA_DeleteOnClose);

	{
		QFont f9;
		int w, h;

		f9.setPointSize(9);
		f9.setWeight(QFont::Light);

		if (widget_wdgSettingsPPU->pushButton_Reset_Lag_Counter->font().pointSize() > 9) {
			widget_wdgSettingsPPU->pushButton_Reset_Lag_Counter->setFont(f9);
		}

		if (widget_wdgSettingsPPU->lineEdit_Lag_Counter->font().pointSize() > 9) {
			widget_wdgSettingsPPU->lineEdit_Lag_Counter->setFont(f9);
		}

		w = widget_wdgSettingsPPU->lineEdit_Lag_Counter->fontMetrics().size(0, "000000000").width() + 10;
		h = widget_wdgSettingsPPU->lineEdit_Lag_Counter->fontMetrics().size(0, "1234567890").height() + 10;

		widget_wdgSettingsPPU->lineEdit_Lag_Counter->setFixedSize(w, h);
	}

	adjustSize();
	setFixedSize(size());

	{
		QMargins vgbm = verticalLayout_groupBox_PPU_Hacks->contentsMargins();
		QMargins vdia = verticalLayout_PPU_Hacks->contentsMargins();
		QPushButton *close = new QPushButton(this);
		int x, y, w, h;

		w = close->fontMetrics().size(0, "x").width() + 10;
		h = close->fontMetrics().size(0, "x").height() + 5;
		x = normalGeometry().width() - w -	vdia.right() - 2 - vgbm.right();
		y = vdia.top() + 2 + 1;

		close->setGeometry(x, y, w, h);
		close->setText("x");

		connect(close, SIGNAL(clicked(bool)), this, SLOT(s_x_clicked(bool)));

		vgbm.setTop(close->sizeHint().height() + 2);
		verticalLayout_groupBox_PPU_Hacks->setContentsMargins(vgbm);
	}

	installEventFilter(this);
}
dlgPPUHacks::~dlgPPUHacks() {}

bool dlgPPUHacks::eventFilter(QObject *obj, QEvent *event) {
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
void dlgPPUHacks::changeEvent(QEvent *event) {
	if (event->type() == QEvent::LanguageChange) {
		Ui::dlgPPUHacks::retranslateUi(this);
	} else {
		QDialog::changeEvent(event);
	}
}

int dlgPPUHacks::update_pos(int startY) {
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
void dlgPPUHacks::update_dialog(void) {
	widget_wdgSettingsPPU->update_widget();
}

void dlgPPUHacks::s_x_clicked(bool checked) {
	mainwin->s_set_ppu_hacks();
}
