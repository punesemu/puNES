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

#include <QtCore/QFileInfo>
#include <QtCore/QEvent>
#include <QtWidgets/QLabel>
#include "wdgStatusBar.moc"
#include "mainWindow.hpp"
#include "conf.h"
#include "patcher.h"

// -------------------------------- Statusbar -----------------------------------------

wdgStatusBar::wdgStatusBar(QWidget *parent) : QStatusBar(parent) {
	setObjectName("statusbar");
	setSizeGripEnabled(false);

	layout()->setContentsMargins(QMargins(0,0,0,0));
	layout()->setMargin(0);
	layout()->setSpacing(0);

	//setStyleSheet("QStatusBar::item { border: 1px solid; border-radius: 1px; } ");

	infosb = new infoStatusBar(this);
	addWidget(infosb);

	installEventFilter(this);
}
wdgStatusBar::~wdgStatusBar() {}

bool wdgStatusBar::eventFilter(QObject *obj, QEvent *event) {
	if (event->type() == QEvent::MouseButtonPress) {
		gui_set_focus();
	}

	return (QObject::eventFilter(obj, event));
}

void wdgStatusBar::update_statusbar(void) {
	infosb->update_label();
}
void wdgStatusBar::update_width(int w) {
	setFixedWidth(w);
	infosb->setFixedWidth(w);
}

// ---------------------------------- Info --------------------------------------------

infoStatusBar::infoStatusBar(QWidget *parent) : QWidget(parent) {
	hbox = new QHBoxLayout(this);
	hbox->setContentsMargins(QMargins(0,0,0,0));
	hbox->setMargin(0);
	hbox->setSpacing(SPACING);

	setLayout(hbox);

	label = new QLabel(this);
	label->setText("[no rom]");
	hbox->addWidget(label);
}
infoStatusBar::~infoStatusBar() {}

void infoStatusBar::update_label(void) {
	BYTE patch = FALSE;
	uTCHAR *rom;

	if ((cfg->cheat_mode == GAMEGENIE_MODE) && (gamegenie.phase == GG_EXECUTE)) {
		rom = gamegenie.rom;
		if (gamegenie.patch) {
			patch = TRUE;
		}
	} else {
		rom = info.rom.file;
	}

	if (patcher.patched == TRUE) {
		patch = TRUE;
	}

	if (info.no_rom | info.turn_off) {
		label->setText("");
		label->setToolTip("");
	} else {
		QFileInfo fileinfo = QFileInfo(uQString(rom));

		if (patch == TRUE) {
			label->setText("*" + fileinfo.fileName());
		} else {
			label->setText(fileinfo.fileName());
		}
		label->setToolTip(fileinfo.filePath());
	}
}
