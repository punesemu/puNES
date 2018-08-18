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

#include "dlgPPUHacks.moc"
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
#include <QtGui/QDesktopWidget>
#else
#include <QtWidgets/QDesktopWidget>
#endif
#include "mainWindow.hpp"
#include "info.h"
#include "ppu.h"
#include "tas.h"
#include "clock.h"
#include "conf.h"
#include "settings.h"
#include "gui.h"

dlgPPUHacks::dlgPPUHacks(QWidget *parent) : QDialog(parent) {
	in_update = false;

	setupUi(this);

	spinBox_VB_Slines->setRange(0, 1000);
	spinBox_Postrender_Slines->setRange(0, 1000);

	setFont(parent->font());
	setStyleSheet(tools_stylesheet());

	checkBox_Unlimited_Sprites->setProperty("myIndex", QVariant(0));
	checkBox_Hide_Sprites->setProperty("myIndex", QVariant(1));
	checkBox_Hide_Background->setProperty("myIndex", QVariant(2));

	connect(checkBox_Unlimited_Sprites, SIGNAL(stateChanged(int)), this,
		SLOT(sprites_and_background_changed(int)));
	connect(checkBox_Hide_Sprites, SIGNAL(stateChanged(int)), this,
		SLOT(sprites_and_background_changed(int)));
	connect(checkBox_Hide_Background, SIGNAL(stateChanged(int)), this,
		SLOT(sprites_and_background_changed(int)));

	connect(checkBox_PPU_Overclock, SIGNAL(stateChanged(int)), this,
		SLOT(ppu_overclock_enabled_changed(int)));

	connect(checkBox_Disable_DMC_Control, SIGNAL(stateChanged(int)), this,
		SLOT(disable_dmc_control_changed(int)));

	connect(spinBox_VB_Slines, SIGNAL(valueChanged(int)), this,
		SLOT(ppu_overclock_vb_slines_changed(int)));
	connect(spinBox_Postrender_Slines, SIGNAL(valueChanged(int)), this,
		SLOT(ppu_overclock_pr_slines_changed(int)));

	connect(spinBox_VB_Slines, SIGNAL(editingFinished()), this,
		SLOT(ppu_overclock_slines_edit_finished()));
	connect(spinBox_Postrender_Slines, SIGNAL(editingFinished()), this,
		SLOT(ppu_overclock_slines_edit_finished()));

	connect(pushButton_Reset_Lag_Counter, SIGNAL(clicked(bool)), this,
		SLOT(lag_counter_reset_clicked(bool)));

	setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);

	setAttribute(Qt::WA_DeleteOnClose);

	{
		QFont f9;
		int w, h;

		f9.setPointSize(9);
		f9.setWeight(QFont::Light);

		if (pushButton_Reset_Lag_Counter->font().pointSize() > 9) {
			pushButton_Reset_Lag_Counter->setFont(f9);
		}

		if (plainTextEdit_Lag_Counter->font().pointSize() > 9) {
			plainTextEdit_Lag_Counter->setFont(f9);
		}

		w = plainTextEdit_Lag_Counter->fontMetrics().size(0, "000000000").width() + 10;
		h = plainTextEdit_Lag_Counter->fontMetrics().size(0, "1234567890").height() + 10;

		plainTextEdit_Lag_Counter->setFixedSize(w, h);
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
	if (event->type() == QEvent::LanguageChange) {
		PPU_Hacks::retranslateUi(this);
	}

	gui_control_pause_bck(event->type());

	return (QObject::eventFilter(obj, event));
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
	in_update = true;

	checkBox_Unlimited_Sprites->setChecked(cfg->unlimited_sprites);
	checkBox_Hide_Sprites->setChecked(cfg->hide_sprites);
	checkBox_Hide_Background->setChecked(cfg->hide_background);

	checkBox_PPU_Overclock->setChecked(cfg->ppu_overclock);
	checkBox_Disable_DMC_Control->setEnabled(cfg->ppu_overclock);

	label_VB_Slines->setEnabled(cfg->ppu_overclock);
	spinBox_VB_Slines->setEnabled(cfg->ppu_overclock);
	spinBox_VB_Slines->setValue(cfg->extra_vb_scanlines);

	label_Postrender_Slines->setEnabled(cfg->ppu_overclock);
	spinBox_Postrender_Slines->setEnabled(cfg->ppu_overclock);
	spinBox_Postrender_Slines->setValue(cfg->extra_pr_scanlines);

	lag_counter_update();

	in_update = false;
}
void dlgPPUHacks::lag_counter_update() {
	plainTextEdit_Lag_Counter->setPlainText(QString("%1").arg(tas.total_lag_frames));
}
void dlgPPUHacks::sprites_and_background_changed(int state) {
	int index = QVariant(((QCheckBox *)sender())->property("myIndex")).toInt();

	if (in_update == true) {
		return;
	}

	switch (index) {
		case 0:
			cfg->unlimited_sprites = !cfg->unlimited_sprites;
			break;
		case 1:
			cfg->hide_sprites = !cfg->hide_sprites;
			break;
		case 2:
			cfg->hide_background = !cfg->hide_background;
			break;
	}

	gui_update();
	gui_active_window();
	gui_set_focus();
}
void dlgPPUHacks::ppu_overclock_enabled_changed(int state) {
	if (in_update == true) {
		return;
	}

	emu_pause(TRUE);
	cfg->ppu_overclock = !cfg->ppu_overclock;
	update_dialog();
	ppu_overclock(TRUE);
	emu_pause(FALSE);

	settings_pgs_save();
	gui_update();
	gui_active_window();
	gui_set_focus();
}
void dlgPPUHacks::disable_dmc_control_changed(int state) {
	if (in_update == true) {
		return;
	}

	cfg->ppu_overclock_dmc_control_disabled = !cfg->ppu_overclock_dmc_control_disabled;

	settings_pgs_save();
	gui_active_window();
	gui_set_focus();
}
void dlgPPUHacks::ppu_overclock_vb_slines_changed(int i) {
	if (in_update == true) {
		return;
	}

	cfg->extra_vb_scanlines = i;
	ppu_overclock(FALSE);

	settings_pgs_save();
}
void dlgPPUHacks::ppu_overclock_pr_slines_changed(int i) {
	if (in_update == true) {
		return;
	}

	cfg->extra_pr_scanlines = i;
	ppu_overclock(FALSE);

	settings_pgs_save();
}
void dlgPPUHacks::ppu_overclock_slines_edit_finished() {
	//gui_active_window();
	//gui_set_focus();
}
void dlgPPUHacks::lag_counter_reset_clicked(bool checked) {
	emu_pause(TRUE);
	tas.total_lag_frames = 0;
	lag_counter_update();
	emu_pause(FALSE);

	gui_active_window();
	gui_set_focus();
}
void dlgPPUHacks::s_x_clicked(bool checked) {
	((mainWindow *) parent())->s_set_ppu_hacks();
}
